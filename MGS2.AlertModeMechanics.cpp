// A mod by Lucky Rapidflower (2025) for bmn's ASI plugin
#include "MGS2.framework.h"
#include "MGS2.AlertModeManager.h"
#include "Utils.h"
#include "set"

namespace MGS2::AlertModeMechanics {
	const char* Category = "AlertModeMechanics";
	static bool AlertModeOnExitingAreaWithAlarmedGuard = true;
	static std::string PrevAreaCode = "";
	static bool AlertModePersistsOnContinue = true;
	static int AreaStartCautionTime;
	static int NumberOfAreasTiedToCaution = 4;
	std::set<std::string> CautionAreasPassed;

	// If we want to tie caution mode to areas instead of a timer,
	// keep track of areas passed in caution here
	// and reduce the caution bar with every new area passed
	static void SetCautionModeLeft() {

		char& storedAlertMode = AlertModeManager::StoredAlertMode;

		// <1 indicates to not tie caution to areas instead of a timer
		// Also do nothing if caution mode is not active
		if (NumberOfAreasTiedToCaution < 1
			|| storedAlertMode != AlertMode::Caution) {
			return;
		}

		const char* areaCode = Mem::AreaCode;

		int& storedCautionTime = AlertModeManager::StoredCautionTime;

		const int& InitialCautionTime = AlertModeManager::InitialCautionTime;

		// If the caution mode just started in the area,
		// clear the set of areas passed in caution
		if (storedCautionTime == InitialCautionTime) {
			CautionAreasPassed.clear();
		}

		// If the player returns to a caution area they already passed,
		// remove it from the set
		if (CautionAreasPassed.contains(areaCode)) {
			CautionAreasPassed.erase(areaCode);
		}
		// Else, insert the previous area into the set 
		else {
			CautionAreasPassed.insert(PrevAreaCode);
		}

		// Set the caution bar (/time) depending on how many areas have been passed in caution
		// and how many need to be passed to clear the caution
		storedCautionTime = InitialCautionTime - (int)(InitialCautionTime * CautionAreasPassed.size() / NumberOfAreasTiedToCaution);
		if (storedCautionTime == 0) {
			storedAlertMode = AlertMode::Infiltration;
		}
	}

	// On load
	tFUN_Void oFUN_00884ca0;
	void __cdecl hkFUN_00884ca0() {

		oFUN_00884ca0();

		try_mgs2

			if (Mem::Stage() == None
				|| !AlertModeManager::CanActivateAlertMode()) {
				return;
			}

			const char* areaCode = Mem::AreaCode;

			char& storedAlertMode = AlertModeManager::StoredAlertMode;

			int& storedCautionTime = AlertModeManager::StoredCautionTime;

			if (PrevAreaCode != Mem::AreaCode) {

				if (AlertModeOnExitingAreaWithAlarmedGuard){

					int& enemyCommStatus1 = *(int*)0xA16068;
					int& enemyCommStatus2 = *(int*)0xA1606C;
					int& wasGuardAlarmed = *(int*)0xA164B4;

					// This makes sure an alert carries over
					// if it started after the area transition
					if (AlertModeManager::AlertTime > 0) {
						storedAlertMode = AlertMode::Alert;

						// For preventing unexpected behavior
						AlertModeManager::AlertTime = 0;
					}
					// If a guard was radioing an alert in the previous area,
					// start alert mode 
					else if (enemyCommStatus1 == 2) {
						storedAlertMode = AlertMode::Alert;
						// also increment the alert amount stat (this address is also used by the Stats mod, move to a separate header file?)
						*(short*)(*Mem::MainGameStats + 0x142) += 1;
					}
					// Else, if an enemy was radioing a caution or alarmed (but not necessarily radioing an alert),
					// start caution mode
					else if ( (enemyCommStatus1 > 0 && enemyCommStatus2 > 0) // true when a guard is radioing a caution
						||  wasGuardAlarmed) {
						AlertModeManager::SetStoredAlertMode(AlertMode::Caution);
					}

					// Reset these variables back to their default value
					// because the game sometimes does not
					enemyCommStatus1 = -1;
					enemyCommStatus2 = 0;
					wasGuardAlarmed = 0;
				}

				SetCautionModeLeft();

				// If we want the caution timer to reset back to what it was at the start of an area on continue,
				// store the caution mode time here
				AreaStartCautionTime = storedCautionTime;
			}
			// If we want caution time to reset back to area start on continue,
			// set it here when the same area is loaded
			else if (AlertModePersistsOnContinue
				&& storedAlertMode == AlertMode::Caution) {
				// Make sure there is enough time left in the caution timer,
				// so that the player is not immediately rushed by returning attack team guards
				storedCautionTime = (AreaStartCautionTime < 120) ? 120 : AreaStartCautionTime;
			}
			PrevAreaCode = Mem::AreaCode;

		catch_mgs2(Category, "884CA0")
	}

	void Run(CSimpleIniA& ini) {
		// If no data has been loaded from the ini file or the mod is disabled
		// do not go further
		if (ini.IsEmpty() || (!ini.GetBoolValue(Category, "Enabled", false))) {
			return;
		}

		NewGameInfo::AddWarning("Alert Mode Mechanics");

		if (ini.GetBoolValue(Category, "NoExplosionInstaAlert", true)) {
			mem::PatchSet{
				// Remove (one part of the) code that causes instant alert on explosion
				mem::Patch((void*)0x42C74E, "\x90\x90\x90\x90\x90"),
				// Replace (one part of the) code that causes instant alert on explosion
				// with code that causes a guard to radio in an alert
				mem::Patch((void*)0x485EDA, "\xE8\x01\x0B\x00\x00"),
				// Replace (one part of the) code that causes instant alert after guard recovers from explosion shock wave
				// with code that causes a guard to radio in an alert
				mem::Patch((void*)0x485EE2, "\xE8\xF9\x0A\x00\x00")
			}.Patch();
		}

		bool HookToOnLoad = false;

		AlertModeOnExitingAreaWithAlarmedGuard = ini.GetBoolValue(Category, "AlertModeOnExitingAreaWithAlarmedGuard", true);
		if (AlertModeOnExitingAreaWithAlarmedGuard) {
			HookToOnLoad = true;
		}

		AlertModePersistsOnContinue = ini.GetBoolValue(Category, "AlertModePersistsOnContinue", true);
		if (AlertModePersistsOnContinue){
			mem::PatchSet{
				// Remove the part of the code that resets the alert mode on continue
				mem::Patch((void*)0x877E0E, "\x90\x90\x90\x90\x90\x90\x90\x90\x90")
			}.Patch();
			HookToOnLoad = true;
		}

		NumberOfAreasTiedToCaution = ini.GetLongValue(Category, "NumberOfAreasTiedToCaution", 4);
		if (NumberOfAreasTiedToCaution > 0) {
			mem::PatchSet({
				// Remove the part of the code that ticks down the caution time
				mem::Patch((void*)0x42CFF9, "\x90\x90\x90\x90\x90\x90"),
				}).Patch();
			HookToOnLoad = true;
		}

		if (HookToOnLoad) {
			// Hook function that activates on load
			oFUN_00884ca0 = (tFUN_Void)mem::TrampHook32((BYTE*)0x884CA0, (BYTE*)hkFUN_00884ca0, 6);
		}
	}
}
