#pragma once

namespace MGS2::Pickup {
	static const int UndefId = 255;

	// Item categories
	// (could share with item rando)
	enum PickupCategory : std::uint8_t {
		Item,
		Ammo, // bullets basically
		StandaloneWeapon // weapons, grenades etc.
	};

	enum PickupSpawnType : std::uint8_t {
		Regular,
		Timed, // Item will despawn after a while
		NeverSpawn
	};

	struct PickupAddresses {
		int& Id;
		int& Category;
		int& Amount;
		PickupAddresses(int param_1)
			: Id(*(int*)(param_1 + 0x44)), Category(*(int*)(param_1 + 0x40)), Amount(*(int*)(param_1 + 0x48)) {
		}
	};

	struct DefaultPickupIdentifiers {
		std::uint8_t Id = UndefId;
		PickupCategory Category = PickupCategory::Item;

		// Define the < operator for ordering
		bool operator<(const DefaultPickupIdentifiers& other) const {
			if (Category != other.Category) {
				return Category < other.Category;
			}
			return Id < other.Id;
		}
	};

	struct DefaultPickupData {
		PickupSpawnType SpawnType = PickupSpawnType::Regular;
		short Amount = SHRT_MIN;
		DefaultPickupIdentifiers NewPickupIdentifiers;
	};

	struct PickupInstanceIdentifiers {
		std::string AreaCode;
		int X;
		int Y;
		int Z;

		// Define the < operator for ordering
		bool operator<(const PickupInstanceIdentifiers& other) const {
			if (AreaCode != other.AreaCode) {
				return AreaCode < other.AreaCode;
			}
			if (X != other.X) {
				return X < other.X;
			}
			if (Y != other.Y) {
				return Y < other.Y;
			}
			return Z < other.Z;
		}
	};

	struct PickupInstanceData {
		float X = std::numeric_limits<float>::lowest();
		float Y = std::numeric_limits<float>::lowest();
		float Z = std::numeric_limits<float>::lowest();
		DefaultPickupData DefaultPickupDataReplacement;
	};

	typedef std::map<DefaultPickupIdentifiers, DefaultPickupData> default_pickup_ids_to_data_map;
	typedef std::map<PickupInstanceIdentifiers, PickupInstanceData> pickup_instance_ids_to_data_map;

	// For running Pickup from other mods
	void Run(default_pickup_ids_to_data_map defaultPickupIdentifiersToDataMap, pickup_instance_ids_to_data_map pickupInstanceIdentifiersToDataMap);
}