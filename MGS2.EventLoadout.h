#pragma once

namespace MGS2::EventLoadout {

	struct ItemData {
		std::uint8_t Number = 255;
		short Amount = -2;
		short Capacity = -1;
	};

	struct LoadoutData {
		std::vector<ItemData> WeaponsData;
		std::vector<ItemData> EquipmentData;
		short WeaponToEquip = -1;
		short EquipmentToEquip = -1;
		short Progress = -1;
		std::uint8_t Difficulty = 0;
		std::uint8_t AlertMode = 255;
		std::unordered_map<std::uint8_t*, std::uint8_t> FlagAddressToORMaskMap;
		std::unordered_map<short, bool> CamStatusAddressOffsetToStatusMap;
	};

	struct LocationIdentifier {
		const char* AreaCode;
		int X;
		int Y;
		int Z;
		LocationIdentifier(const char* areaCode, int x, int y, int z) : AreaCode(areaCode), X(x), Y(y), Z(z) {}
	};

	typedef std::unordered_map<Stage, std::unordered_map<short, LoadoutData>> stage_to_progress_to_loadout_u_map;

	// For running EventLoadout from other mods
	void Run(stage_to_progress_to_loadout_u_map stageToProgressToLoadoutMap, bool needToHookForMainGCL = true);
}