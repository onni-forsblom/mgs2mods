// A mod by Lucky Rapidflower (2025) for bmn's ASI plugin
#include "MGS2.framework.h"
#include "MGS2.InventoryData.h"
#include "regex"
#include "set"
#include "Utils.h"

namespace MGS2::Pickup {
	const char* Category = "Pickup";
	static const int UndefId = 255;

	// Item categories
	//  (could put this in another file and share with item rando)
	enum PickupCategory : std::uint8_t {
		Item, 
		Ammo, // bullets basically
		StandaloneWeapon // weapons, grenades etc.
	};

	enum PickupSpawnType : std::uint8_t {
		Regular,
		Timed, // Item will despawn after a while
		NeverSpawn
	};

	struct DefaultPickupIdentifiers {
		std::uint8_t Id = UndefId;
		PickupCategory Category = PickupCategory::Item;

		// Define the < operator for ordering
		bool operator<(const DefaultPickupIdentifiers& other) const {
			if (Category != other.Category) {
				return Category < other.Category;
			}
			return Id < other.Id;
		}
	};

	struct DefaultPickupData {
		PickupSpawnType SpawnType = PickupSpawnType::Regular;
		short Amount = SHRT_MIN;
		DefaultPickupIdentifiers NewPickupIdentifiers;
	};

	static std::map<DefaultPickupIdentifiers, DefaultPickupData> DefaultPickupIdentifiersToDataMap;

	struct PickupInstanceIdentifiers {
		std::string AreaCode;
		float X;
		float Y;
		float Z;

		// Define the < operator for ordering
		bool operator<(const PickupInstanceIdentifiers& other) const {
			if (AreaCode != other.AreaCode) {
				return AreaCode < other.AreaCode;
			}
			if (X != other.X) {
				return X < other.X;
			}
			if (Y != other.Y) {
				return Y < other.Y;
			}
			return Z < other.Z;
		}
	};

	struct PickupInstanceData {
		float Xoffset = 0;
		float Yoffset = 0;
		float Zoffset = 0;
		DefaultPickupData DefaultPickupDataReplacement;
	};

	static std::map<PickupInstanceIdentifiers, PickupInstanceData> PickupInstanceIdentifiersToDataMap;

	tFUN_Int_IntIntInt oFUN_00799210;
	int hkFUN_00799210(int param_1, int param_2, int param_3) {

		int result = oFUN_00799210(param_1, param_2, param_3);

		try_mgs2

			// Set up variables

			int& itemId = *(int*)(param_1 + 0x44);
			int& itemCategory = *(int*)(param_1 + 0x40);
			int& itemAmount = *(int*)(param_1 + 0x48);
			PickupSpawnType newPickupSpawnType = PickupSpawnType::Regular;

			// Set the new default pickup data, if we have it for this pickup type

			auto newDefaultPickupDataIt = DefaultPickupIdentifiersToDataMap.find({(std::uint8_t)itemId, (PickupCategory)itemCategory});

			if (newDefaultPickupDataIt != DefaultPickupIdentifiersToDataMap.end()) {

				newPickupSpawnType = newDefaultPickupDataIt->second.SpawnType;

				if (newDefaultPickupDataIt->second.Amount != SHRT_MIN) { // min amount indicates to not change amount
					itemAmount = newDefaultPickupDataIt->second.Amount;
				}

				if (newDefaultPickupDataIt->second.NewPickupIdentifiers.Id != UndefId) {
					itemId = newDefaultPickupDataIt->second.NewPickupIdentifiers.Id;
					itemCategory = newDefaultPickupDataIt->second.NewPickupIdentifiers.Category;
				}
			}

			// Set the new pickup instance data if it exists (overriding the new default data as needed)

			// 1 if pickup is will eventually despawn with time (~ dropped by guard), 0 if it does not
			// Items dropped by guards do not have to be checked for specific instance changes
			int& isTimedPickup = *(int*)(param_1 + 0xA8);

			if (isTimedPickup == 0) {

				float& pickupX = *(float*)(param_1 + 0x230);
				float& pickupY = *(float*)(param_1 + 0x234);
				float& pickupZ = *(float*)(param_1 + 0x238);
				auto pickupInstanceDataIt = PickupInstanceIdentifiersToDataMap.find({Mem::AreaCode, pickupX, pickupY, pickupZ});

				if (pickupInstanceDataIt != PickupInstanceIdentifiersToDataMap.end()) {

					if (newDefaultPickupDataIt->second.SpawnType == PickupSpawnType::NeverSpawn) {
						return -1;
					}

					newPickupSpawnType = pickupInstanceDataIt->second.DefaultPickupDataReplacement.SpawnType;

					if (pickupInstanceDataIt->second.DefaultPickupDataReplacement.Amount != SHRT_MIN) {
						itemAmount = pickupInstanceDataIt->second.DefaultPickupDataReplacement.Amount;
					}

					if (pickupInstanceDataIt->second.DefaultPickupDataReplacement.NewPickupIdentifiers.Id != UndefId) {
						itemId = pickupInstanceDataIt->second.DefaultPickupDataReplacement.NewPickupIdentifiers.Id;
						itemCategory = pickupInstanceDataIt->second.DefaultPickupDataReplacement.NewPickupIdentifiers.Category;
					}

					pickupX += pickupInstanceDataIt->second.Xoffset;
					pickupY += pickupInstanceDataIt->second.Yoffset;
					pickupZ += pickupInstanceDataIt->second.Zoffset;
				}
			}

			switch (newPickupSpawnType)
			{
			case MGS2::Pickup::Timed:
				isTimedPickup = 1;
				break;
			case MGS2::Pickup::NeverSpawn:
				return -1;
			default:
				break;
			}
		catch_mgs2(Category, "799210");

		return result;
	}

	// Tries to get the item id from item/weapon map
	// And the category by checking if it is in the form Weapon.ammo (or weapon.anything really)
	static DefaultPickupIdentifiers GetDefaultPickupIdentifiersFromStr(std::vector<std::string> words) {

		DefaultPickupIdentifiers defaultPickupIdentifiers;

		auto ItemMapIt = InventoryData::WeaponMap.find(words.at(0));
		if (ItemMapIt != InventoryData::WeaponMap.end()) {
			defaultPickupIdentifiers.Id = ItemMapIt->second;
			defaultPickupIdentifiers.Category = (words.size() > (1)) ?
				PickupCategory::Ammo : PickupCategory::StandaloneWeapon;
		}
		else {
			ItemMapIt = InventoryData::ItemMap.find(words.at(0));
			if (ItemMapIt != InventoryData::ItemMap.end()) {
				defaultPickupIdentifiers.Id = ItemMapIt->second;
			}
		}

		return defaultPickupIdentifiers;
	}

	// Parses default pickup data (amount, spawn type, replacement item)
	static DefaultPickupData GetNewDefaultPickupData(CSimpleIniA& ini, const char* pickupSectionChar) {

		DefaultPickupData newDefaultPickupData;

		const char* spawnKey = "spawn";
		const std::string spawnStr = ToLower(ini.GetValue(pickupSectionChar, spawnKey, ""));
		if (spawnStr == "timed") {
			newDefaultPickupData.SpawnType = PickupSpawnType::Timed;
		}
		else {
			if (!ini.GetBoolValue(pickupSectionChar, spawnKey, true)) {
				newDefaultPickupData.SpawnType = PickupSpawnType::NeverSpawn;
			}
		}

		newDefaultPickupData.Amount = ConfigParser::ParseInteger(ini, pickupSectionChar, "amount", SHRT_MIN, SHRT_MIN, SHRT_MAX);

		std::string replacementItemString = ini.GetValue(pickupSectionChar, "newitem", "");
		if (replacementItemString != "") {
			newDefaultPickupData.NewPickupIdentifiers = GetDefaultPickupIdentifiersFromStr(ParseWords(replacementItemString));
		}

		return newDefaultPickupData;
	}

	static bool HasNewPickupData() {
		return !DefaultPickupIdentifiersToDataMap.empty()
			|| !PickupInstanceIdentifiersToDataMap.empty();
	}

	void Run(CSimpleIniA& ini) {
		if (ini.IsEmpty() || (!ini.GetBoolValue(Category, "Enabled", false))) {
			return;
		}

		// Warning on starting a new game if pickup data was parsed
		NewGameInfo::AddWarning("Pickup", &HasNewPickupData);

		// Regex to match "Pickup" with at least one dot (ignores upper/lower case)
		std::regex pickupSectionRegex(R"(Pickup\.-?\w+(\.-?\w+)*)", std::regex_constants::icase);

		// Iterate through all (Pickup) sections
		std::list<CSimpleIniA::Entry> sections;
		ini.GetAllSections(sections);
		for (const auto& section : sections) {
			// If the section name and regex do not match, move onto the next section
			if (!std::regex_match(section.pItem, pickupSectionRegex)) {
				continue;
			}
			const char* pickupSectionChar = section.pItem;

			std::vector<std::string> pickupSectionWords = ParseWords(pickupSectionChar);

			// remove the 'Pickup' part
			pickupSectionWords.erase(pickupSectionWords.begin());

			// Handle default pickup data here
			if (pickupSectionWords.size() > 0
				&& pickupSectionWords.size() < 3) {

				DefaultPickupIdentifiers defaultPickupIdentifiers;

				defaultPickupIdentifiers = GetDefaultPickupIdentifiersFromStr(pickupSectionWords);
				if (defaultPickupIdentifiers.Id == UndefId) {
					continue;
				}

				DefaultPickupIdentifiersToDataMap[defaultPickupIdentifiers] = GetNewDefaultPickupData(ini, pickupSectionChar);
			}
			else if (pickupSectionWords.size() == 4) {
				std::string pickupAreaCode = pickupSectionWords[0];
				pickupSectionWords.erase(pickupSectionWords.begin());
				float pickupXYZ[3];
				bool xyzSuccess = true;
				for (int i = 0; i < 3; i++) {
					if (!TrySetNumFromStr(pickupXYZ[i], pickupSectionWords[i])) {
						xyzSuccess = false;
						break;						
					}
				}
				if (!xyzSuccess) {
					continue;
				}
				PickupInstanceIdentifiers pickupInstanceIdentifiers(pickupAreaCode, pickupXYZ[0], pickupXYZ[1], pickupXYZ[2]);
				PickupInstanceData pickupInstanceData;
				pickupInstanceData.DefaultPickupDataReplacement = GetNewDefaultPickupData(ini, pickupSectionChar);
				
				pickupInstanceData.Xoffset = ini.GetDoubleValue(pickupSectionChar, "xoffset");
				pickupInstanceData.Yoffset = ini.GetDoubleValue(pickupSectionChar, "yoffset");
				pickupInstanceData.Zoffset = ini.GetDoubleValue(pickupSectionChar, "zoffset");

				PickupInstanceIdentifiersToDataMap[pickupInstanceIdentifiers] = pickupInstanceData;
			}
		}

		// Hook function to item box spawn if there is a need
		if (HasNewPickupData()) {
			oFUN_00799210 = (tFUN_Int_IntIntInt)mem::TrampHook32((BYTE*)0x799210, (BYTE*)hkFUN_00799210, 6);
		}
	}
}