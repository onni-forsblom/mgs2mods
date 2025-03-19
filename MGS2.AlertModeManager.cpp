#include "MGS2.AlertModeManager.h"

namespace MGS2::AlertModeManager {

	static const std::unordered_map<std::string, AlertMode::Enum> NameToAlertModeMap{
		{"infiltration", AlertMode::Infiltration},
		{"alert", AlertMode::Alert},
		{"evasion", AlertMode::Evasion},
		{"caution", AlertMode::Caution},
	};

	int GetAlertModeNumFromStr(std::string alertModeStr)
	{
		return NameToAlertModeMap.find(alertModeStr)->second;
	}

	void SetStoredAlertMode(int newAlertMode, bool canDemoteAlertMode)
	{
		// If the stored alert mode is more severe than the one we want to set
		// do not change the alert mode (e.g. do not demote from alert to caution)
		if (!canDemoteAlertMode
			&& AreaAlertMode != AlertMode::Infiltration
			&& AreaAlertMode < newAlertMode) {
			return;
		}

		// Set the stored alert mode
		StoredAlertMode = newAlertMode;

		// If we want to set caution mode,
		// set the caution time that is stored between area loads too
		if (newAlertMode == AlertMode::Caution) {
			StoredCautionTime = InitialCautionTime;
		}
	}

	bool CanActivateAlertMode()
	{
		// Taken from bmn's caution mod

		const char* areaCode = (char*)0x118ADEC;
		const char* characterCode = (char*)0x118C374;
		short tankerProgress = *(short*)0x118D93C;
		short plantProgress = *(short*)0x118D912;

		// reached guard rush
		if (strcmp(characterCode, "r_tnk0") == 0) {
			if (tankerProgress >= 31) {
				return false;
			}
		}
		// in plant
		else if (plantProgress != 0) {
			if (
				(plantProgress <= 21) || // in dock
				(plantProgress == 379) || // asc colon 1
				(plantProgress == 154) || // ames
				(strcmp(areaCode, "w25a") == 0) || // before harrier
				(strcmp(areaCode, "w25b") == 0) || // after harrier
				(plantProgress >= 382) ||
				false // this should be pointless
				) {
				return false;
			}
		}
		return true;
	}
}