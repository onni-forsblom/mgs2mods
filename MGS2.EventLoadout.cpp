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
	// Basically item amounts recorded at the start of the fight and reset to post-battle
	std::vector<ItemData> FightStartItemData;

	// If we want to spawn the version of Shell 2 core with guards after meeting the president
	static bool SpawnGuardsAfterPrez = false;
	const char* Shell2Core1FPreEmmaAreaCode = "w31a";
	const char* Shell2Core1FEmmaAreaCode = "w31d";
	const int PostPrezCodecEndProgress = 241;

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
		{"shell2core1fflags", (std::uint8_t*)0x118E077},
		{"emmaescortflags", (std::uint8_t*)0x118E01A}
	};

	static const std::unordered_map<std::string, ToggleableActor> NameToToggleableActorMap{
		{"deckccam", DeckCCam},
		{"deckanwlocker", DeckANWLocker},
		{"parcelroomnwcam", ParcelRoomNWcam},
		{"parcelroomswcam", ParcelRoomSWcam},
		{"parcelroomsecam", ParcelRoomSEcam},
		{"shell1core1fsouth1stlockerfromwest", Shell1Core1FSouth1stLockerFromWest},
		{"shell1core1fsouth2ndlockerfromwest", Shell1Core1FSouth2ndLockerFromWest},
		{"shell1core1fsouth3rdlockerfromwest", Shell1Core1FSouth3rdLockerFromWest},
		{"shell1core1fsouth3rdlockerfromeast", Shell1Core1FSouth3rdLockerFromEast},
		{"shell1core1fsouth2ndlockerfromeast", Shell1Core1FSouth2ndLockerFromEast},
		{"shell1core1fsouth1stlockerfromeast", Shell1Core1FSouth1stLockerFromEast},
		{"shell1core1fsouthcam", Shell1Core1FSouthCam},
		{"arsenalgearstomachlocker", ArsenalGearStomachLocker}
	};

	struct ToggleableActorData {
		short Status1AddressOffset;
		std::int8_t Status2AddressOffset;
		bool InverseToggle; // If true, true/yes/on means setting value to 0
		ToggleableActorData(short status1AddressOffset, std::int8_t status2AddressOffset, bool inverseToggle = false) :
			Status1AddressOffset(status1AddressOffset), Status2AddressOffset(status2AddressOffset), InverseToggle(inverseToggle) {}
	};

	static const std::unordered_map<ToggleableActor, ToggleableActorData> ToggleableActorToDataMap{
		{DeckCCam, {0, 56, true}},
		{DeckANWLocker,{1+32,-32}},
		{ParcelRoomNWcam, {610, 3, true}},
		{ParcelRoomSWcam, {611, 3, true}},
		{ParcelRoomSEcam, {612, 3, true}},
		{Shell1Core1FSouth1stLockerFromWest,{913+27, -27}},
		{Shell1Core1FSouth2ndLockerFromWest,{917+24, -24}},
		{Shell1Core1FSouth3rdLockerFromWest,{921+21, -21}},
		{Shell1Core1FSouth3rdLockerFromEast,{925+18, -18}},
		{Shell1Core1FSouth2ndLockerFromEast,{929+15, -15}},
		{Shell1Core1FSouth1stLockerFromEast,{933+12, -12}},
		{Shell1Core1FSouthCam, {950, 5, true}},
		{ArsenalGearStomachLocker,{1341-4, 4}}
	};

	struct LocationIdentifier {
		const char* AreaCode;
		int X;
		int Y;
		int Z;
		LocationIdentifier(const char* areaCode, int x, int y, int z) : AreaCode(areaCode), X(x), Y(y), Z(z) {}
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

	static void SetItemsData(std::vector<ItemData> itemsData, bool isWeapons = true, bool setStoredItemsToo = false /* item values that are stored between area loads / reset to on continue */) {
		
		// Get either the weapon or equipment inventory as desired
		short* inventoryPtr = isWeapons ? ((short*)*Mem::WeaponData) : ((short*)*Mem::ItemData);

		// The value to add to the item amount address to get the item max capacity address
		size_t maxOffset = isWeapons ? Mem::WeaponMaxOffset : Mem::ItemMaxOffset;

		for (auto &itemData : itemsData) {
			const int maxItemNumber = 40; // No items beyond this number
			if (itemData.Number > maxItemNumber) {
				continue;
			}

			// Set the quantity for the item in the inventory
			// -2 indicates we do not want to change the value

			if (itemData.Amount != -2) {
				inventoryPtr[itemData.Number] = itemData.Amount;

				if (setStoredItemsToo) {
					const int storedItemOffset = 0x1598;
					short* storedInventoryPtr = (short*)((char*)inventoryPtr + storedItemOffset);
					storedInventoryPtr[itemData.Number] = itemData.Amount;
				}
			}

			// Calculate the max capacity address for the item and set the value
			// -1 indicates we do not want to change the value
			if (itemData.Capacity != -1) {
				short* inventoryCapacityPtr = (short*)((char*)inventoryPtr + maxOffset);
				inventoryCapacityPtr[itemData.Number] = itemData.Capacity;
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

		const int marginOfError = 5;

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

	/// <param name="weaponNumbers">The number of each weapon which ammo amount we want to reset after a fight</param>
	static void StoreFightStartAmmoData(const std::vector<int>& weaponNumbers) {
		// (Make sure there is only the necessary item data)
		FightStartItemData.clear();

		for (int weaponNumber : weaponNumbers) {
			ItemData itemData;
			itemData.Number = weaponNumber;
			itemData.Amount = ((short*)*Mem::WeaponData)[weaponNumber]; // The current ammo amount for the desired weapon
			FightStartItemData.push_back(itemData);
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

			if (ResetAmmoAfterSpecificFights){
				if (stage == Stage::Plant) {
					switch (progress)
					{
					// Fatman fight
					case 118:
						// Store the ammo amount for the Socom and M9
						// (could make an enum for weapons/equipment but this will do for now)
						StoreFightStartAmmoData(std::vector<int>{1,3});
						break;
					// First Vamp fight
					case 253:
						// Store the ammo amount for M9, socom, RGB6, AK and M4
						StoreFightStartAmmoData(std::vector<int>{1, 3, 5, 15, 18});
						break;
					// After Fatman
					case 120:
					// After Vamp
					case 257:
						// Reset ammo to pre-fight (including the amount stored between area loads)
						if (!FightStartItemData.empty()) {
							SetItemsData(FightStartItemData, true, true);
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

			// Finally, set all the data for the items, progress, difficulty and alert mode

			SetItemsData(loadoutDataIt->second.WeaponsData, true, setStoredItemsToo);
			SetItemsData(loadoutDataIt->second.EquipmentData, false, setStoredItemsToo);

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

			for (const auto& toggleableActorToStatus : loadoutDataIt->second.ToggleableActorToStatusMap) {

				// Calculate the actor statuses' addresses

				static std::uint8_t* toggleableActorStatusAddressStart = (std::uint8_t*)0x118DBD7; // (Deck-C cam status)
				ToggleableActorData toggleableActorData = ToggleableActorToDataMap.at(toggleableActorToStatus.first);
				std::uint8_t* toggleableActorStatus1Address = (toggleableActorStatusAddressStart + toggleableActorData.Status1AddressOffset);
				std::uint8_t* toggleableActorStatus2Address = toggleableActorStatus1Address + toggleableActorData.Status2AddressOffset;

				*toggleableActorStatus1Address = 144; // can be other values too but this seems to work, basically needed for the change below to apply

				bool setActive = toggleableActorData.InverseToggle ? !toggleableActorToStatus.second : toggleableActorToStatus.second;
				*toggleableActorStatus2Address = setActive ? 2 : 0; // 0 - closed locker / functioning camera, 2 - open locker / blown up camera
			}

		catch_mgs2(Category, "884CA0")
	}

	// Before/after running main GCL for stage
	// Better to change some data here (like alert mode)
	tFUN_Int_Int oFUN_004e4090;
	int hkFUN_004e4090(int param_1) {

		int result = oFUN_004e4090(param_1);

		try_mgs2

			// When spawning the version of Shell 2 core with guards right after meeting the prez
			// make sure game data is loaded correctly
			if (SpawnGuardsAfterPrez
				&& Mem::Progress() == PostPrezCodecEndProgress // Check that the progress flag is correct
				&& (std::strcmp(Mem::AreaCode, Shell2Core1FEmmaAreaCode) == 0)) // Check that we are in Shell 2 core 1F
			{
				*(std::uint8_t*)0xF6DE8B = 64; // For getting guard collision etc.
				*(std::uint8_t*)0xEDE76F = 64; // For getting radar map data
				*(std::uint8_t*)0xDB704B = 64; // For loading map graphics after loading a save
			}

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
	
	// After writing area code (can change the code here and load another area)
	tFUN_Void oFUN_0089180C;
	void __cdecl hkFUN_0089180C() {

		try_mgs2

			// If we want to use the version of shell 2 core with guards after meeting the president...
			if (SpawnGuardsAfterPrez
			&& Mem::Progress() >= PostPrezCodecEndProgress // check that we have actually met the prez
			&& (strcmp(Mem::AreaCode, Shell2Core1FPreEmmaAreaCode) == 0) // check if the game is trying to load the first version of shell 2 core
				) {
				// then use the version of shell 2 core with guards
				strcpy(Mem::AreaCode, Shell2Core1FEmmaAreaCode);
			}
			
		catch_mgs2(Category, "891620");
		
		oFUN_0089180C();
		
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

				// Sets the data for toggleable actors

				auto nameToToggleableActorIt = NameToToggleableActorMap.find(sectionKeyStr);
				if (nameToToggleableActorIt != NameToToggleableActorMap.end()) {
					loadoutData.ToggleableActorToStatusMap[nameToToggleableActorIt->second] = ini.GetBoolFromChar(value.c_str());
					continue;
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

	void Run(stage_to_progress_to_loadout_u_map stageToProgressToLoadoutMap, bool needToHookForMainGCL, bool resetAmmoAfterSpecificFights,
		bool spawnGuardsAfterPrez)
	{
		StageToProgressToLoadoutMap = stageToProgressToLoadoutMap;

		ResetAmmoAfterSpecificFights = resetAmmoAfterSpecificFights;

		SpawnGuardsAfterPrez = spawnGuardsAfterPrez;

		// Hook function that activates on load
		oFUN_00884ca0 = (tFUN_Void)mem::TrampHook32((BYTE*)0x884CA0, (BYTE*)hkFUN_00884ca0, 6);

		// Hook function that activates after writing area code if we want to spawn guards after meeting the prez
		if (SpawnGuardsAfterPrez) {
			oFUN_0089180C = (tFUN_Void)mem::TrampHook32((BYTE*)0x89180C, (BYTE*)hkFUN_0089180C, 5);
		}
		
		// Hook function that activates before/after running main GCL for stage
		// but only if there is a need
		if (needToHookForMainGCL
			|| SpawnGuardsAfterPrez) {
			oFUN_004e4090 = (tFUN_Int_Int)mem::TrampHook32((BYTE*)0x4E4090, (BYTE*)hkFUN_004e4090, 8);
		}
	}
}