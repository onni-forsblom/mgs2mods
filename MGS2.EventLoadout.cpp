// A mod by Lucky Rapidflower (2025) for bmn's ASI plugin
#include "MGS2.framework.h"
#include "MGS2.InventoryData.h"
#include "regex"
#include "set"

namespace MGS2::EventLoadout {
	const char* Category = "EventLoadout";

	std::unordered_map<std::string, Stage> NameToStageMap{
		{"tanker", Stage::Tanker},
		{"plant", Stage::Plant},
		{"missions", Stage::Missions},
		{"bosssurvival", Stage::BossSurvival},
		{"snaketalea", Stage::TalesA},
		{"snaketaleb", Stage::TalesB},
		{"snaketalec", Stage::TalesC},
		{"snaketaled", Stage::TalesD},
		{"snaketalee", Stage::TalesE}
	};

	struct ItemData {
		std::uint8_t Number;
		short Amount;
		short Capacity;

		ItemData(std::uint8_t number = 255, short amount = -2, short capacity = -1)
			: Number(number), Amount(amount), Capacity(capacity) {
		}
	};

	struct LoadoutData {
		std::vector<ItemData> WeaponsData;
		std::vector<ItemData> EquipmentData;
		short WeaponToEquip;
		short EquipmentToEquip;
		short Progress;

		LoadoutData() = default;

		LoadoutData(std::vector<ItemData> weaponsData, std::vector<ItemData> equipmentData,
			short weaponToEquip = -1, short equipmentToEquip = -1, short progress = -1)
			: WeaponsData(weaponsData), EquipmentData(equipmentData), WeaponToEquip(weaponToEquip), EquipmentToEquip(equipmentToEquip), Progress(progress) {
		}
	};


	std::unordered_map<MGS2::Stage, std::unordered_map<short, LoadoutData>> StageToProgressToLoadoutMap;


	static void SetItemsData(std::vector<ItemData> itemsData, bool isTankerOrSnakeBossSurvival = true, bool isWeapons = true) {

		const int otherInventoryOffset = 0x150; // add this to tanker inventory start address to get the inventory start address for Raiden, VR and Snake Tales
		const int equipmentOffset = 0x90; // add this to the inventory start address to get the equipment start address

		// Do math to get the inventory start address

		char* inventoryStartPtr = (char*)0x118AF1C;
		inventoryStartPtr += isTankerOrSnakeBossSurvival ? 0 : otherInventoryOffset;
		inventoryStartPtr += isWeapons ? 0 : equipmentOffset;

		// The value to add to the item amount address to get the item max capacity address
		size_t maxOffset = isWeapons ? Mem::WeaponMaxOffset : Mem::ItemMaxOffset;

		for (auto &itemData : itemsData) {
			const int maxItemNumber = 40; // No items beyond this number
			if (itemData.Number > maxItemNumber) {
				continue;
			}

			// Calculate the quantity address for the item and set the value 
			// -2 indicates we do not want to change the value

			const int itemAmountByteSize = 2;
			short* itemAmountPtr = (short*)((inventoryStartPtr + itemData.Number * itemAmountByteSize));
			if (itemData.Amount != -2) {
				*itemAmountPtr = itemData.Amount;
			}
			

			// Calculate the max capacity address for the item and set the value
			// -1 indicates we do not want to change the value
			if (itemData.Capacity != -1) {
				short* itemCapacityPtr = (short*)(((char*)itemAmountPtr + maxOffset));
				*itemCapacityPtr = itemData.Capacity;
			}
		}
	}

	// On load
	tFUN_Void oFUN_00884ca0;
	void __cdecl hkFUN_00884ca0() {

		oFUN_00884ca0();

		try_mgs2

			if (StageToProgressToLoadoutMap.empty()) {
				return;
			}

			const Stage stage = Mem::Stage();

			if (stage == Stage::None) {
				return;
			}

			const int progress = Mem::Progress();

			// If we cannot find the stage from the map, stop here

			auto progressToLoadoutDataMapIt = StageToProgressToLoadoutMap.find(stage);
			if (progressToLoadoutDataMapIt == StageToProgressToLoadoutMap.end()) {
				return;
			}

			// If we cannot find the progress flag from the map, stop here

			auto itemsDataIt = progressToLoadoutDataMapIt->second.find(progress);
			if (itemsDataIt == progressToLoadoutDataMapIt->second.end()) {
				return;
			}

			char* characterCode = Mem::CharacterCode;
			const char snakeBossSurvivalCharCode[8] = "r_sna_b";

			// Get this for determining the inventory address
			bool isTankerOrSnakeBossSurvival =
				(stage == Stage::Tanker
					|| characterCode == snakeBossSurvivalCharCode);

			// Finally, set all the data for the items and progress

			SetItemsData(itemsDataIt->second.WeaponsData, isTankerOrSnakeBossSurvival);
			SetItemsData(itemsDataIt->second.EquipmentData, isTankerOrSnakeBossSurvival, false);

			if (itemsDataIt->second.WeaponToEquip != -1) {
				*Mem::EquippedWeapon = itemsDataIt->second.WeaponToEquip;
			}
			if (itemsDataIt->second.EquipmentToEquip != -1) {
				*Mem::EquippedItem = itemsDataIt->second.EquipmentToEquip;
			}
			if (itemsDataIt->second.Progress != -1) {
				Mem::SetProgress(itemsDataIt->second.Progress);
			}

		catch_mgs2(Category, "884CA0")
	}
	
	// For turning a string lowercase
	static std::string ToLower(std::string str) {
		std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) {
			return std::tolower(c);
		});
		return str;
	}

	// Tries to get a short value from a string and set it to a suitable range
	static bool TrySetShortFromStr(short& variable, const std::string& value) {
		try {
			long longValue = std::stol(value);
			variable = std::clamp(longValue, (long)SHRT_MIN, (long)SHRT_MAX);
			return true;
		}
		catch (...) {
			return false;
		}
	}

	bool NewGameInfoCallback() {
		return !StageToProgressToLoadoutMap.empty();
	}

	void Run(CSimpleIniA& ini) {

		// If no data has been loaded from the ini file or the mod is disabled
		// do not go further
		if (ini.IsEmpty() || (!ini.GetBoolValue(Category, "Enabled", false))) {
			return;
		}

		// Warning on starting a new game if loadout data was parsed
		NewGameInfo::AddWarning("Event Loadout", &NewGameInfoCallback);

		// Define the regex pattern to match sections like [EventLoadout.Stage.Flag]
		std::regex inventorySectionRegex(R"(^EventLoadout\.[^.]+\.\d+$)");

		// Iterate through all (EventLoadout) sections
		std::list<CSimpleIniA::Entry> sections;
		ini.GetAllSections(sections);
		for (auto& section : sections) {

			// If the section name and regex do not match, move onto the next section
			if (!std::regex_match(section.pItem, inventorySectionRegex)) {
				continue;
			}

			// Get the stage name and progress value from the section name

			std::string loadoutSectionStr = section.pItem;

			const size_t dotPos1 = loadoutSectionStr.find('.');
			const size_t dotPos2 = loadoutSectionStr.find('.', dotPos1 + 1);

			std::string stageName = loadoutSectionStr.substr(
				dotPos1 + 1,
				dotPos2 - dotPos1 - 1
			);
			stageName = ToLower(stageName);

			// If the stage name is invalid, move to the next section

			auto stageIt = NameToStageMap.find(stageName);
			if (stageIt == NameToStageMap.end()) {
				continue;
			}

			short progressFlag;
			if (!TrySetShortFromStr(progressFlag, loadoutSectionStr.substr(dotPos2 + 1))) {
				continue;
			}

			// Create the items data entry for the stage and progress value

			LoadoutData& loadoutData = StageToProgressToLoadoutMap[stageIt->second][progressFlag];

			// Iterate through the section keys
			std::list<CSimpleIniA::Entry> sectionKeys;
			ini.GetAllKeys(section.pItem, sectionKeys);
			for (auto& sectionKey : sectionKeys) {

				std::string value = ToLower(ini.GetValue(section.pItem, sectionKey.pItem));
				std::string sectionKeyStr = ToLower(sectionKey.pItem);

				// Check if we want to set something other than item data (equipped item[s], progress)
				// and act accordingly
				const std::set<std::string> weaponEquipStrings = {"weapon", "wpn"};
				if (weaponEquipStrings.contains(sectionKeyStr)) {
					loadoutData.WeaponToEquip = InventoryData::WeaponMap[
						value
					];
					continue;
				}
				const std::set<std::string> equipmentEquipStrings = {"equipment", "equip", "item"};
				if (equipmentEquipStrings.contains(sectionKeyStr)) {
					loadoutData.EquipmentToEquip = InventoryData::ItemMap[
						value
					];
					continue;
				}
				const std::set<std::string> progressStrings = {"progress", "flag", "progressflag"};
				if (progressStrings.contains(sectionKeyStr)) {
					TrySetShortFromStr(loadoutData.Progress, value);
					continue;
				}

				// Start setting data for an inventory item

				ItemData itemData;
				size_t slashPos = value.find('/');

				// If the string value has no slash, set the item amount to the retrieved value
				if (slashPos == std::string::npos) {
					if (!TrySetShortFromStr(itemData.Amount, value)) {
						continue;
					}
				}
				// Else, the value is of form A/B
				// Set the item amount to A and max capacity to B
				// If A or B is missing, use the default value (which indicates to not change the item amount/capacity)
				else {
					std::string amountStr = value.substr(0, slashPos);
					if (amountStr != "") {
						if (!TrySetShortFromStr(itemData.Amount, amountStr)) {
							continue;
						}
					}

					std::string capacityStr = value.substr(slashPos + 1);
					if (capacityStr != "") {
						if (!TrySetShortFromStr(itemData.Capacity, capacityStr)) {
							continue;
						}
					}
				}
				
				// Set the item number based on which map the key is found
				// and add the item to the appropriate container
				auto itemIt = InventoryData::WeaponMap.find(sectionKeyStr);
				if (itemIt != InventoryData::WeaponMap.end()) {
					itemData.Number = itemIt->second;
					loadoutData.WeaponsData.push_back(itemData);
					continue;
				}
				itemIt = InventoryData::ItemMap.find(sectionKeyStr);
				if (itemIt != InventoryData::ItemMap.end()) {
					itemData.Number = itemIt->second;
					loadoutData.EquipmentData.push_back(itemData);
				}
			}
		}

		// Hook function that activates on load
		oFUN_00884ca0 = (tFUN_Void)mem::TrampHook32((BYTE*)0x884CA0, (BYTE*)hkFUN_00884ca0, 6);
	}
}
