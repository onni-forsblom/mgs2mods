// A mod by Lucky Rapidflower (2025) for bmn's ASI plugin
#include "MGS2.framework.h"
#include "MGS2.Pickup.h"
#include "MGS2.InventoryData.h"
#include "regex"
#include "set"
#include "Utils.h"

namespace MGS2::Pickup {
	const char* Category = "Pickup";
	static const std::unordered_map<std::string, std::string> AreaVariationToAreaCodeMap = {
		{"w11b","w11a"}, // deep sea dock
		{"w11c","w11a"}, // deep sea dock
		{"w12c","w12a"}, // strut A roof
		{"w15b","w15a"}, // BC bridge
		{"w16b","w16a"}, // dining hall
		{"w16c","w16a"}, // dining hall
		{"w20c","w20b"}, // heliport
		{"w20d","w20b"}, // heliport
		{"w21b","w21a"}, // EF connecting bridge
		{"w23b","w23a"}, // FA bridge
		{"w31f","w31c"}, // shell 2 core B1 filtration chamber
		{"w31d","w31a"}, // shell 2 core 1F
		{"w25d","w25c"}, // KL bridge
		{"w32b","w32a"} // Emma sniping
	};

	static std::map<DefaultPickupIdentifiers, DefaultPickupData> DefaultPickupIdentifiersToDataMap;

	static std::map<PickupInstanceIdentifiers, PickupInstanceData> PickupInstanceIdentifiersToDataMap;

	// INT_MIN means do not replace the drop amount
	static int NPCAmmoDropAmountReplacement = INT_MIN;
	// (Could make a system where you can set the NPC item drop data for each item separately, but this will do)

	// For items dropped by Snake to Raiden that could be considered integral to the Harrier and Tengu fights
	bool DoNotAlterItemsDroppedBySnakeToRaiden = false;

	// Account for areas that have slightly different variations under a different code
	static std::string GetMainAreaCode(std::string areaCode) {
		auto areaCodeIt = AreaVariationToAreaCodeMap.find(areaCode);
		if (areaCodeIt != AreaVariationToAreaCodeMap.end()) {
			return areaCodeIt->second;
		}
		return areaCode;
	}

	static void SetDefaultPickupData(DefaultPickupData newDefaultPickupData, PickupAddresses& pickupAddresses, PickupSpawnType& newPickupSpawnType) {
		int progress = Mem::Progress();

		if (
			(progress < newDefaultPickupData.MinProgress)
			|| ( (newDefaultPickupData.MaxProgress > -1) && (progress > newDefaultPickupData.MaxProgress) )
		) {
			newPickupSpawnType = NeverSpawn;
		}
		else {
			newPickupSpawnType = newDefaultPickupData.SpawnType;
		}

		if (newDefaultPickupData.Amount != SHRT_MIN) { // min amount indicates to not change amount
			pickupAddresses.Amount = newDefaultPickupData.Amount;
		}

		if (newDefaultPickupData.NewPickupIdentifiers.Id != UndefId) {
			pickupAddresses.Id = newDefaultPickupData.NewPickupIdentifiers.Id;
			pickupAddresses.Category = newDefaultPickupData.NewPickupIdentifiers.Category;
		}
	}

	tFUN_Int_IntIntInt oFUN_00799210;
	int hkFUN_00799210(int param_1, int param_2, int param_3) {

		int result = oFUN_00799210(param_1, param_2, param_3);

		try_mgs2

			// Set up variables

			PickupAddresses pickupAddresses(param_1);
			PickupSpawnType newPickupSpawnType = PickupSpawnType::Regular;

			// 1 if pickup is will eventually despawn with time (~ dropped by enemy / by Snake to Raiden), 0 if it does not
			// Items dropped by guards do not have to be checked for specific instance changes
			int& isTimedPickup = *(int*)(param_1 + 0xA8);

			// If this is an item dropped by Snake to Raiden and we do not want to alter those,
			// just return here
			if (DoNotAlterItemsDroppedBySnakeToRaiden
				&& (isTimedPickup == 1) // to check if it is a dropped item
				&& ( (strcmp(Mem::AreaCode, "w25a") == 0) && (pickupAddresses.Id == 7)) // stinger ammo dropped by Snake during the Harrier fight 
					|| (Mem::Progress() > 396) // For items dropped by Snake during Tengu fights
					){
				return result;
			}



			// Set the new default pickup data, if we have it for this pickup type

			auto newDefaultPickupDataIt = DefaultPickupIdentifiersToDataMap.find(
				{(std::uint8_t)pickupAddresses.Id, (PickupCategory)pickupAddresses.Category});

			if (newDefaultPickupDataIt != DefaultPickupIdentifiersToDataMap.end()) {
				SetDefaultPickupData(newDefaultPickupDataIt->second, pickupAddresses, newPickupSpawnType);
			}

			// Set the new pickup instance data if it exists (overriding the new default data as needed)

			if (isTimedPickup == 0) {

				float& pickupX = *(float*)(param_1 + 0x230);
				float& pickupY = *(float*)(param_1 + 0x234);
				float& pickupZ = *(float*)(param_1 + 0x238);

				auto pickupInstanceDataIt = PickupInstanceIdentifiersToDataMap.find({GetMainAreaCode(Mem::AreaCode), (int)pickupX, (int)pickupY, (int)pickupZ});

				if (pickupInstanceDataIt != PickupInstanceIdentifiersToDataMap.end()) {

					if (pickupInstanceDataIt->second.DefaultPickupDataReplacement.SpawnType == PickupSpawnType::NeverSpawn) {
						return -1;
					}

					SetDefaultPickupData(pickupInstanceDataIt->second.DefaultPickupDataReplacement, pickupAddresses, newPickupSpawnType);

					if (pickupInstanceDataIt->second.X != std::numeric_limits<float>::lowest()) {
						pickupX = pickupInstanceDataIt->second.X;
					}
					if (pickupInstanceDataIt->second.Y != std::numeric_limits<float>::lowest()) {
						pickupY = pickupInstanceDataIt->second.Y;
					}
					if (pickupInstanceDataIt->second.Z != std::numeric_limits<float>::lowest()) {
						pickupZ = pickupInstanceDataIt->second.Z;
					}
				}
			}
			// For if we want change the ammo amount dropped by an NPC
			else if (NPCAmmoDropAmountReplacement > INT_MIN
				&& pickupAddresses.Category != PickupCategory::Item) {
				pickupAddresses.Amount = NPCAmmoDropAmountReplacement;
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
		newDefaultPickupData.MinProgress = ConfigParser::ParseInteger(ini, pickupSectionChar, "minprogress", -1, SHRT_MIN, SHRT_MAX);
		newDefaultPickupData.MaxProgress = ConfigParser::ParseInteger(ini, pickupSectionChar, "maxprogress", -1, SHRT_MIN, SHRT_MAX);

		std::string replacementItemString = ini.GetValue(pickupSectionChar, "newitem", "");
		if (replacementItemString != "") {
			newDefaultPickupData.NewPickupIdentifiers = GetDefaultPickupIdentifiersFromStr(ParseWords(replacementItemString));
		}

		return newDefaultPickupData;
	}

	static bool HasNewPickupData() {
		return !DefaultPickupIdentifiersToDataMap.empty()
			|| !PickupInstanceIdentifiersToDataMap.empty()
			|| (NPCAmmoDropAmountReplacement > INT_MIN);
	}

	void Run(CSimpleIniA& ini) {
		// If there is already pickup data, do not parse the .ini file
		if (HasNewPickupData()) {
			return;
		}

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

			// Handle default pickup here
			if (pickupSectionWords.size() > 0
				&& pickupSectionWords.size() < 3) {

				DefaultPickupIdentifiers defaultPickupIdentifiers;

				defaultPickupIdentifiers = GetDefaultPickupIdentifiersFromStr(pickupSectionWords);
				if (defaultPickupIdentifiers.Id == UndefId) {
					continue;
				}

				DefaultPickupIdentifiersToDataMap[defaultPickupIdentifiers] = GetNewDefaultPickupData(ini, pickupSectionChar);
			}
			// Handle pickup instance here
			else if (pickupSectionWords.size() == 4) {
				// First get the main area code for the identifier

				std::string pickupAreaCode = GetMainAreaCode(pickupSectionWords[0]);
				pickupSectionWords.erase(pickupSectionWords.begin());

				// Then X, Y and Z for the identifier

				int pickupXYZ[3];
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

				// Then set the instance data according to parameters

				PickupInstanceIdentifiers pickupInstanceIdentifiers(pickupAreaCode, pickupXYZ[0], pickupXYZ[1], pickupXYZ[2]);
				PickupInstanceData pickupInstanceData;
				pickupInstanceData.DefaultPickupDataReplacement = GetNewDefaultPickupData(ini, pickupSectionChar);
				
				pickupInstanceData.X = ini.GetDoubleValue(pickupSectionChar, "x", std::numeric_limits<float>::lowest());
				pickupInstanceData.Y = ini.GetDoubleValue(pickupSectionChar, "y", std::numeric_limits<float>::lowest());
				pickupInstanceData.Z = ini.GetDoubleValue(pickupSectionChar, "z", std::numeric_limits<float>::lowest());

				PickupInstanceIdentifiersToDataMap[pickupInstanceIdentifiers] = pickupInstanceData;
			}
		}

		// Hook function to item box spawn if there is a need
		if (HasNewPickupData()) {
			oFUN_00799210 = (tFUN_Int_IntIntInt)mem::TrampHook32((BYTE*)0x799210, (BYTE*)hkFUN_00799210, 6);
		}
	}

	void Run(default_pickup_ids_to_data_map defaultPickupIdentifiersToDataMap, pickup_instance_ids_to_data_map pickupInstanceIdentifiersToDataMap,
		int npcAmmoDropAmountReplacement, bool doNotAlterItemsDroppedBySnakeToRaiden)
	{
		DefaultPickupIdentifiersToDataMap = defaultPickupIdentifiersToDataMap;
		PickupInstanceIdentifiersToDataMap = pickupInstanceIdentifiersToDataMap;

		NPCAmmoDropAmountReplacement = npcAmmoDropAmountReplacement;

		DoNotAlterItemsDroppedBySnakeToRaiden = doNotAlterItemsDroppedBySnakeToRaiden;

		// Hook function to item box spawn if there is a need
		if (HasNewPickupData()) {
			oFUN_00799210 = (tFUN_Int_IntIntInt)mem::TrampHook32((BYTE*)0x799210, (BYTE*)hkFUN_00799210, 6);
		}
	}
}