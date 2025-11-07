#pragma once

namespace MGS2::EventLoadout {

	enum ToggleableActor : std::uint8_t {
		DeckCCam,
		DeckANWLocker,
		ParcelRoomNWcam,
		ParcelRoomSWcam,
		ParcelRoomSEcam,
		Shell1Core1FSouth1stLockerFromWest,
		Shell1Core1FSouth2ndLockerFromWest,
		Shell1Core1FSouth3rdLockerFromWest,
		Shell1Core1FSouth3rdLockerFromEast,
		Shell1Core1FSouth2ndLockerFromEast,
		Shell1Core1FSouth1stLockerFromEast,
		Shell1Core1FSouthCam,
		Shell2Core1FNWTurret,
		Shell2Core1FSWTurret,
		ArsenalGearStomachLocker
	};

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
		std::unordered_map<ToggleableActor, bool> ToggleableActorToStatusMap;
	};

	typedef std::unordered_map<Stage, std::unordered_map<short, LoadoutData>> stage_to_progress_to_loadout_u_map;

	// For running EventLoadout from other mods
	void Run(stage_to_progress_to_loadout_u_map stageToProgressToLoadoutMap, bool needToHookForMainGCL = true, bool resetAmmoAfterSpecificFights = false,
		bool spawnGuardsAfterPrez = false);
}