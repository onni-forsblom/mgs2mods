// A mod by Lucky Rapidflower (2025) for bmn's ASI plugin
#include "MGS2.framework.h"
#include "MGS2.AlertModeManager.h"
#include "Utils.h"
#include "set"

namespace MGS2::AlertModeMechanics {
	const char* Category = "AlertModeMechanics";
	static bool AlertModeOnExitingAreaWithAlarmedGuard = true;
	static std::string PrevAreaCode = "";
	static std::string PrevRespawnAreaCode = "";
	static int PrevContinueAmount = -1;
	static bool AlertModePersistsOnContinue = true;
	static int AreaStartCautionTime;
	static char AreaStartAlertMode;
	static int NumberOfAreasTiedToCaution = 4;
	std::set<std::string> CautionAreasPassed;
	std::set<std::string> AreaStartCautionAreasPassed;
	static char* RespawnAreaCode = (char*)0x118C384;
	static short& ContinueAmount = *(short*)(*Mem::MainGameStats + 0x132); // (this address is also used by the Stats mod, move to a separate header file?)
	static bool UsedByOtherMod = false;

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

	static void SetAlertModeOnExitingAreaWithAlarmedGuard() {
		if (!AlertModeOnExitingAreaWithAlarmedGuard) {
			return;
		}

		int& enemyCommStatus1 = *(int*)0xA16068;
		int& enemyCommStatus2 = *(int*)0xA1606C;
		int& wasGuardAlarmed = *(int*)0xA164B4;

		// This makes sure an alert carries over
		// if it started after the area transition
		if (AlertModeManager::AlertTime > 0) {
			AlertModeManager::StoredAlertMode = AlertMode::Alert;

			// For preventing unexpected behavior
			AlertModeManager::AlertTime = 0;
		}
		// If a guard was radioing an alert in the previous area,
		// start alert mode 
		else if (enemyCommStatus1 == 2) {
			AlertModeManager::StoredAlertMode = AlertMode::Alert;
			// also increment the alert amount stat, both area and stored variable (this address is also used by the Stats mod, move to a separate header file?)
			*(short*)(*Mem::MainGameStats + 0x142) += 1; // area alert stat
			*(short*)(0x118C49A) += 1; // stored alert stat
		}
		// Else, if an enemy was radioing a caution or alarmed (but not necessarily radioing an alert),
		// start caution mode
		else if ((enemyCommStatus1 > 0 && enemyCommStatus2 > 0) // true when a guard is radioing a caution
			|| wasGuardAlarmed) {
			AlertModeManager::SetStoredAlertMode(AlertMode::Caution);
		}
		// Make sure cautions carry over through area transitions
		else if (AlertModeManager::AreaCautionTime > 0
			&& AlertModeManager::StoredAlertMode == AlertMode::Infiltration) {
			AlertModeManager::StoredAlertMode = AlertMode::Caution;
		}
		// Reset these variables back to their default values
		// so that the previous code does not cause alert modes at undesired times
		enemyCommStatus1 = -1;
		enemyCommStatus2 = 0;
		wasGuardAlarmed = 0;
		AlertModeManager::AreaCautionTime = 0;
	}

	// On load
	tFUN_Void oFUN_00884ca0;
	void __cdecl hkFUN_00884ca0() {

		oFUN_00884ca0();

		try_mgs2

			if (Mem::Stage() == None) { // title screen etc.
				// Reset these variables to avoid unintended behavior on starting a game after quitting a game etc.
				PrevAreaCode = "";
				PrevRespawnAreaCode = "";
				PrevContinueAmount = -1;
				CautionAreasPassed.clear();
				AreaStartCautionAreasPassed.clear();
				AreaStartCautionTime = 0;
				AreaStartAlertMode = AlertMode::Infiltration;

				AlertModeManager::AlertTime = 0;
				AlertModeManager::AreaCautionTime = 0;
				return;
			} 

			const char* areaCode = Mem::AreaCode;
			
			if ((strcmp(areaCode, "tales") == 0) // = snake tales "cutscenes"
				|| !AlertModeManager::CanActivateAlertMode()) {
				return;
			}

			char& storedAlertMode = AlertModeManager::StoredAlertMode;

			int& storedCautionTime = AlertModeManager::StoredCautionTime;

			if (// for checking that the area was not reloaded after a continue (enough for normal gameplay including countdown sequences)
				(ContinueAmount == PrevContinueAmount) 
				&& PrevAreaCode != areaCode) // for checking if the area was reloaded without a continue (like with the trainer)
				{

				SetAlertModeOnExitingAreaWithAlarmedGuard();

				SetCautionModeLeft();

				// (For Alert Mode Persists On Continue)
				// Store section start alert mode, caution time and caution areas passed (this last one is for countdown sequences)
				// when respawn area changes
				if (AlertModePersistsOnContinue
					&& RespawnAreaCode != PrevRespawnAreaCode) {
					AreaStartCautionTime = storedCautionTime;
					AreaStartAlertMode = storedAlertMode;
					AreaStartCautionAreasPassed = CautionAreasPassed;
				}
			}
			// Set the alert mode etc.
			// upon reloading a section if we want to
			else if (AlertModePersistsOnContinue) {
				storedAlertMode = AreaStartAlertMode;
				if (storedAlertMode == AlertMode::Caution) {
					// (This is for countdown sequences)
					CautionAreasPassed = AreaStartCautionAreasPassed;
					// Make sure there is enough time left in the caution timer,
					// so that the player is not immediately rushed by returning attack team guards
					storedCautionTime = (AreaStartCautionTime < 120) ? 120 : AreaStartCautionTime;
				}
			}
			PrevAreaCode = areaCode;
			PrevRespawnAreaCode = RespawnAreaCode;
			PrevContinueAmount = ContinueAmount;

		catch_mgs2(Category, "884CA0")
	}

	void Run(CSimpleIniA& ini) {
		// If no data has been loaded from the ini file or the mod is disabled
		// do not go further
		if (UsedByOtherMod || ini.IsEmpty() || (!ini.GetBoolValue(Category, "Enabled", false))) {
			return;
		}

		NewGameInfo::AddWarning("Alert Mode Mechanics");

		Run(
			false,
			ini.GetBoolValue(Category, "NoExplosionInstaAlert", true),
			ini.GetBoolValue(Category, "AlertModeOnExitingAreaWithAlarmedGuard", true),
			ini.GetBoolValue(Category, "AlertModePersistsOnContinue", true),
			ini.GetLongValue(Category, "NumberOfAreasTiedToCaution", 4)
		);
	}

	void Run(bool usedByOtherMod, bool noExplosionInstaAlert, bool alertModeOnExitingAreaWithAlarmedGuard,
		bool alertModePersistsOnContinue, int numberOfAreasTiedToCaution) {

		UsedByOtherMod = usedByOtherMod;

		if (noExplosionInstaAlert) {
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

		AlertModeOnExitingAreaWithAlarmedGuard = alertModeOnExitingAreaWithAlarmedGuard;

		AlertModePersistsOnContinue = alertModePersistsOnContinue;

		NumberOfAreasTiedToCaution = numberOfAreasTiedToCaution;
		if (NumberOfAreasTiedToCaution > 0) {
			mem::PatchSet({
				// Remove the part of the code that ticks down the caution time
				AlertModeManager::CautionTimerFreezePatch,
				}).Patch();
		}

		if (AlertModeOnExitingAreaWithAlarmedGuard
			|| AlertModePersistsOnContinue
			|| NumberOfAreasTiedToCaution > 0) {
			// Hook function that activates on load
			oFUN_00884ca0 = (tFUN_Void)mem::TrampHook32((BYTE*)0x884CA0, (BYTE*)hkFUN_00884ca0, 6);
		}
	}
}
