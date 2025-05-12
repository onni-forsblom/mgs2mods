// A mod by Lucky Rapidflower (2025) for bmn's ASI plugin
#include "MGS2.framework.h"
#include "MGS2.EventLoadout.h"
#include "MGS2.InventoryData.h"
#include "MGS2.AlertModeManager.h"
#include "regex"
#include "set"
#include "Utils.h"

namespace MGS2::EventLoadout {
	const char* Category = "EventLoadout";
	static int AlertModeToSet = 255;

	// For fights like Fatman where you can gain a lot of ammo (currently just intended for a challenge mod)
	static bool ResetAmmoAfterSpecificFights = false;
	std::vector<ItemData> FightStartItemData;


	static const std::unordered_map<std::string, Stage> NameToStageMap{
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

	static const std::unordered_map<std::string, Difficulty> NameToDifficultyMap{
		{"veryeasy", Difficulty::VeryEasy},
		{"easy", Difficulty::Easy},
		{"normal", Difficulty::Normal},
		{"hard", Difficulty::Hard},
		{"extreme", Difficulty::Extreme},
		{"e-extreme", Difficulty::EuroExtreme},
	};

	static const std::unordered_map<std::string, std::uint8_t*> NameToFlagAddressMap{
		{"itemflags", (std::uint8_t*)0x118AED4},
		{"tankerflags", (std::uint8_t*)0x118DB21},
		{"engineroomflags", (std::uint8_t*)0x118E4E4},
	};

	static const std::unordered_map<std::string, short> NameToCamStatusAddressOffsetMap{
		{"deckccam", 0},
		{"parcelroomnwcam", 610},
		{"parcelroomswcam", 611},
		{"parcelroomsecam", 612},
		{"shell1coresouthcam", 950}
	};

	static const std::unordered_map<Stage, std::unordered_map<short, LocationIdentifier>> StageToProgressToSpecialEventLocationMap{
		{
			Plant,
			{
				{62, // BC bridge after Fortune cutscene
					{
						"w15a",
						-54748,
						995,
						-45276
					}
				}
			}
		}
	};

	static stage_to_progress_to_loadout_u_map StageToProgressToLoadoutMap;

	static void SetItemsData(std::vector<ItemData> itemsData, bool isTankerOrSnakeBossSurvival = true, bool isWeapons = true,
		bool setStoredItemsToo = false /* item values that are stored between area loads / reset to on continue */) {

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

				if (setStoredItemsToo) {
					const int storedItemOffset = 0x1598;
					short* storedItemAmountPtr = (short*)((char*)itemAmountPtr + storedItemOffset);
					*storedItemAmountPtr = itemData.Amount;
				}
			}

			// Calculate the max capacity address for the item and set the value
			// -1 indicates we do not want to change the value
			if (itemData.Capacity != -1) {
				short* itemCapacityPtr = (short*)(((char*)itemAmountPtr + maxOffset));
				*itemCapacityPtr = itemData.Capacity;
			}
		}
	}

	// Checks if employing an event loadout should only occur at a certain player location (area & x, y, z coordinates)
	// For cases where progress is set after a cutscene and the player character is at a specific location (for example)
	static bool LocationDoesNotMatchSpecialEvent(const Stage stage, const int progress, bool& setStoredItemsToo) {

		// If we cannot find the stage from the map, stop here

		auto progressToSpecialEventLocationMapIt = StageToProgressToSpecialEventLocationMap.find(stage);
		if (progressToSpecialEventLocationMapIt == StageToProgressToSpecialEventLocationMap.end()) {
			return false;
		}

		// If we cannot find the progress flag from the map, stop here

		auto specialEventLocationIt = progressToSpecialEventLocationMapIt->second.find(progress);
		if (specialEventLocationIt == progressToSpecialEventLocationMapIt->second.end()) {
			return false;
		}

		float& charX = *(float*)0xA18A00;
		float& charY = *(float*)0xA18A04;
		float& charZ = *(float*)0xA18A08;

		const int marginOfError = 50;

		// If the player character was not near the specific required location, do not employ the loadout
		if ((strcmp(specialEventLocationIt->second.AreaCode, Mem::AreaCode) != 0)
			|| std::abs(specialEventLocationIt->second.X - charX) > marginOfError
			|| std::abs(specialEventLocationIt->second.Y - charY) > marginOfError
			|| std::abs(specialEventLocationIt->second.Z - charZ) > marginOfError
			) {
			return true;
		}

		// Else, use the loadout (and set stored items too because player coordinates are different after a load)
		setStoredItemsToo = true;
		return false;
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

			if (ResetAmmoAfterSpecificFights){
				if (stage == Stage::Plant) {
					switch (progress)
					{
					// Fatman fight
					case 118:
						// Store the ammo amount for the Socom and M9
						{
							// (Make sure there is only the necessary item data)
							FightStartItemData.clear();

							ItemData socomData;
							socomData.Number = 3;
							socomData.Amount = *(short*)0x118B072; // Current socom ammo amount

							ItemData m9Data;
							m9Data.Number = 1;
							m9Data.Amount = *(short*)0x118B06E; // Current M9 ammo amount

							FightStartItemData = { socomData, m9Data };
						}
						break;
					// After Fatman
					case 120:
						// Reset ammo for the Socom and M9 to pre-fight (including the amount stored between area loads)
						if (!FightStartItemData.empty()) {
							SetItemsData(FightStartItemData, false, true, true);
							// (Make sure there is only the necessary item data)
							FightStartItemData.clear();
						}
						break;
					default:
						break;
					}
				}
			}

			// Check if loadout should be used only at a certain location
			bool setStoredItemsToo = false;
			if (LocationDoesNotMatchSpecialEvent(stage, progress, setStoredItemsToo)) {
				return;
			}

			// If we cannot find the stage from the map, stop here

			auto progressToLoadoutDataMapIt = StageToProgressToLoadoutMap.find(stage);
			if (progressToLoadoutDataMapIt == StageToProgressToLoadoutMap.end()) {
				return;
			}

			// If we cannot find the progress flag from the map, stop here

			auto loadoutDataIt = progressToLoadoutDataMapIt->second.find(progress);
			if (loadoutDataIt == progressToLoadoutDataMapIt->second.end()) {
				return;
			}

			char* characterCode = Mem::CharacterCode;
			const char snakeBossSurvivalCharCode[8] = "r_sna_b";

			// Get this for determining the inventory address
			bool isTankerOrSnakeBossSurvival =
				(stage == Stage::Tanker
					|| characterCode == snakeBossSurvivalCharCode);

			// Finally, set all the data for the items, progress, difficulty and alert mode

			SetItemsData(loadoutDataIt->second.WeaponsData, isTankerOrSnakeBossSurvival, true, setStoredItemsToo);
			SetItemsData(loadoutDataIt->second.EquipmentData, isTankerOrSnakeBossSurvival, false, setStoredItemsToo);

			if (loadoutDataIt->second.WeaponToEquip != -1) {
				*Mem::EquippedWeapon = loadoutDataIt->second.WeaponToEquip;
			}
			if (loadoutDataIt->second.EquipmentToEquip != -1) {
				*Mem::EquippedItem = loadoutDataIt->second.EquipmentToEquip;
			}
			if (loadoutDataIt->second.Progress != -1) {
				Mem::SetProgress(loadoutDataIt->second.Progress);
			}
			if (loadoutDataIt->second.Difficulty != 0) {
				*Mem::Difficulty = loadoutDataIt->second.Difficulty;
			}
			AlertModeToSet = loadoutDataIt->second.AlertMode;

			// Turns on the desired flags
			for (const auto& flagAddressToORMask : loadoutDataIt->second.FlagAddressToORMaskMap) {
				*flagAddressToORMask.first |= flagAddressToORMask.second;
			}

			// Repair/remove the desired cameras
			for (const auto& camStatusAddressOffsetToStatus : loadoutDataIt->second.CamStatusAddressOffsetToStatusMap) {

				// Calculate the camera statuses' addresses

				static std::uint8_t* camStatusAddressStart = (std::uint8_t*)0x118DBD7; // (Deck-C cam status)
				std::uint8_t* camStatusAddress = (camStatusAddressStart + camStatusAddressOffsetToStatus.first);
				std::uint8_t* camStatus2Address;
				switch (camStatusAddressOffsetToStatus.first) {
					case 0:
						camStatus2Address = camStatusAddress + 56;
						break;
					case 950:
						camStatus2Address = camStatusAddress + 5;
						break;
					default:
						camStatus2Address = camStatusAddress + 3;
				}

				// If true, make the camera not broken
				if (camStatusAddressOffsetToStatus.second) {
					*camStatusAddress = 0;
					*camStatus2Address = 0;
				}
				// Else, basically make the camera blown up
				else {
					*camStatusAddress = 144; // This can be other values as well but 144 seems to work
					*camStatus2Address = 2; // 1 - regular broken camera, 2 - blown up camera
				}
			}

		catch_mgs2(Category, "884CA0")
	}

	// Before/after running main GCL for stage
	// Better to change some data here (like alert mode)
	tFUN_Int_Int oFUN_004e4090;
	int hkFUN_004e4090(int param_1) {

		int result = oFUN_004e4090(param_1);

		try_mgs2
			// 255 indicates to not change the alert mode
			if (AlertModeToSet == 255) {
				return result;
			}

			AlertModeManager::SetStoredAlertMode(AlertModeToSet);

			// Do this so as not to change alert mode with the next load
			// unless we want to
			AlertModeToSet = 255;
			
		catch_mgs2(Category, "4E4090");

		return result;
	}

	static bool NewGameInfoCallback() {
		return !StageToProgressToLoadoutMap.empty();
	}

	void Run(CSimpleIniA& ini) {

		// If this mod is already being used by another mod, do not parse the .ini file

		if (!StageToProgressToLoadoutMap.empty()) {
			return;
		}

		// If no data has been loaded from the ini file or the mod is disabled
		// do not go further
		if (ini.IsEmpty() || (!ini.GetBoolValue(Category, "Enabled", false))) {
			return;
		}

		mem::PatchSet patchSet = mem::PatchSet{
			// Set the chaff timer to 5s
			mem::Patch((void*)0x957349, "\x2C\x01")
		};
		patchSet.Patch();

		bool needToHookForMainGCL = false;

		// Warning on starting a new game if loadout data was parsed
		NewGameInfo::AddWarning("Event Loadout", &NewGameInfoCallback);

		// Define the regex pattern to match sections like [EventLoadout.Stage.Flag]
		std::regex inventorySectionRegex(R"(^EventLoadout\.[^.]+\.\d+$)", std::regex_constants::icase);

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
			if (!TrySetNumFromStr(progressFlag, loadoutSectionStr.substr(dotPos2 + 1))) {
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

				// Check if we want to set something other than item data (equipped item[s], progress, difficulty, alert mode)
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
					TrySetNumFromStr(loadoutData.Progress, value);
					continue;
				}
				const std::set<std::string> difficultyStrings = {"difficulty", "diff"};
				if (difficultyStrings.contains(sectionKeyStr)) {
					loadoutData.Difficulty = NameToDifficultyMap.find(value)->second;
					continue;
				}
				const static std::set<std::string> alertLevelStrings = {"alertmode", "alertlevel"};
				if (alertLevelStrings.contains(sectionKeyStr)) {
					needToHookForMainGCL = true;
					if (TrySetNumFromStr(loadoutData.AlertMode, value)) {
						continue;
					}
					loadoutData.AlertMode = AlertModeManager::GetAlertModeNumFromStr(value);
					continue;
				}

				// Sets the data for the desired flags to turn on (flags are to be written in binary)

				auto NameToFlagAddressIt = NameToFlagAddressMap.find(sectionKeyStr);
				if (NameToFlagAddressIt != NameToFlagAddressMap.end()) {
					loadoutData.FlagAddressToORMaskMap[NameToFlagAddressIt->second] = std::stoi(value, nullptr, 2);
					continue;
				}

				// Sets the data for cameras to enable/disable

				auto NameToCamAddressOffsetIt = NameToCamStatusAddressOffsetMap.find(sectionKeyStr);
				if (NameToCamAddressOffsetIt != NameToCamStatusAddressOffsetMap.end()) {
					loadoutData.CamStatusAddressOffsetToStatusMap[NameToCamAddressOffsetIt->second] = ini.GetBoolFromChar(value.c_str());
				}

				// Start setting data for an inventory item

				ItemData itemData;
				size_t slashPos = value.find('/');

				// If the string value has no slash, set the item amount to the retrieved value
				if (slashPos == std::string::npos) {
					if (!TrySetNumFromStr(itemData.Amount, value)) {
						continue;
					}
				}
				// Else, the value is of form A/B
				// Set the item amount to A and max capacity to B
				// If A or B is missing, use the default value (which indicates to not change the item amount/capacity)
				else {
					std::string amountStr = value.substr(0, slashPos);
					if (amountStr != "") {
						if (!TrySetNumFromStr(itemData.Amount, amountStr)) {
							continue;
						}
					}

					std::string capacityStr = value.substr(slashPos + 1);
					if (capacityStr != "") {
						if (!TrySetNumFromStr(itemData.Capacity, capacityStr)) {
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

		// Hook function that activates before/after running main GCL for stage
		// but only if there is a need
		if (needToHookForMainGCL) {
			oFUN_004e4090 = (tFUN_Int_Int)mem::TrampHook32((BYTE*)0x4E4090, (BYTE*)hkFUN_004e4090, 8);
		}
	}

	void Run(stage_to_progress_to_loadout_u_map stageToProgressToLoadoutMap, bool needToHookForMainGCL, bool resetAmmoAfterSpecificFights)
	{
		StageToProgressToLoadoutMap = stageToProgressToLoadoutMap;

		ResetAmmoAfterSpecificFights = resetAmmoAfterSpecificFights;

		// Hook function that activates on load
		oFUN_00884ca0 = (tFUN_Void)mem::TrampHook32((BYTE*)0x884CA0, (BYTE*)hkFUN_00884ca0, 6);

		// Hook function that activates before/after running main GCL for stage
		// but only if there is a need
		if (needToHookForMainGCL) {
			oFUN_004e4090 = (tFUN_Int_Int)mem::TrampHook32((BYTE*)0x4E4090, (BYTE*)hkFUN_004e4090, 8);
		}
	}
}
