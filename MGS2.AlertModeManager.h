#pragma once
#include "MGS2.framework.h"

namespace MGS2::AlertModeManager {
	static char& StoredAlertMode = *(char*)0x118AEDC;
	static char& AreaAlertMode = *(char*)0x118AEDA;

	static int& StoredCautionTime = *(int*)0xF6DE10;
	static int& AreaCautionTime = *(int*)0xA160C8;

	static int& InitialCautionTime = *(int*)0xF60000;

	// Area? alert time (is set again on area load?)
	static int& AlertTime = *(int*)0xA160C0;

	static const mem::Patch CautionTimerFreezePatch = mem::Patch((void*)0x42CFF9, "\x90\x90\x90\x90\x90\x90");

	int GetAlertModeNumFromStr(std::string alertModeStr);

	// Sets the alert mode that is stored between areas. Does not demote alert mode by default.
	// When setting to caution, sets the caution timer too.
	void SetStoredAlertMode(int newAlertMode, bool canDemoteAlertMode = false);

	// Return false if the player is in an area in the main game
	// where any alert mode should not be activated
	bool CanActivateAlertMode();
}