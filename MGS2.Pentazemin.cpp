// A mod by Lucky Rapidflower for bmn's ASI plugin
#include "MGS2.framework.h"

namespace MGS2::Pentazemin {
	const char* Category = "Pentazemin";
	int pentazeminTime = 999966; // default value for this mod

	// Set the pentazemin time on selecting new game
	tFUN_Void_Int oFUN_00756a40;
	void __cdecl hkFUN_00756a40(int param_1) {
		try_mgs2
			* (int*)0xA18B40 = pentazeminTime;
		catch_mgs2(Category, "756A40");

		oFUN_00756a40(param_1);
	}

	// Set the pentazemin time on loading a completed save file
	tFUN_Void_Int oFUN_00758330;
	void __cdecl hkFUN_00758330(int param_1) {
		try_mgs2
			*(int*)0xA18B40 = pentazeminTime;
		catch_mgs2(Category, "758330");

		oFUN_00758330(param_1);
	}

	void Run(CSimpleIniA& ini) {
		// If no data has been loaded from the ini file or the mod is disabled
		// do not go further
		if (ini.IsEmpty() || (!ini.GetBoolValue(Category, "Enabled", false))) {
			return;
		}

		// Try to get the pentazemin time from the ini file and clamp it between a min and a max
		// if the value could not be found, use the default
		const int minPentazeminTime = 0;
		const int maxPentazeminTime = 1000000;
		pentazeminTime = ConfigParser::ParseInteger(ini, Category, "PentazeminTime", pentazeminTime, minPentazeminTime, maxPentazeminTime);

		// Bind the functions that set the pentazemin time at the difficulty select
		oFUN_00756a40 = (tFUN_Void_Int)mem::TrampHook32((BYTE*)0x756A40, (BYTE*)hkFUN_00756a40, 5);
		oFUN_00758330 = (tFUN_Void_Int)mem::TrampHook32((BYTE*)0x758330, (BYTE*)hkFUN_00758330, 5);
	}
}
