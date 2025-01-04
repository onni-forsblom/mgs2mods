#include "MGS2.framework.h"
#include <array>
#include <vector>
#include "MGS2.InventoryData.h"

namespace MGS2::EquipShortcuts {
	const char* Category = "EquipShortcuts";

	const char RESERVEMODE_NONE = 0;
	const char RESERVEMODE_NORMAL = 1;
	const char RESERVEMODE_PREVIOUS = 2;
	const char RESERVEMODE_UNEQUIP = 3;
	const char RESERVEMODE_ITEM = 4;

	const size_t WEAPON_COUNT = 21;
	const size_t ITEM_COUNT = 40;

	struct EquipGroup {
		std::vector<short> Items;
		bool Unequip = false;
		char ReserveMode = RESERVEMODE_NORMAL; // 0 = none, 1 = normal, 2 = previous, 3 = unequip, 4 = item
		short ReserveItem = 0;

		EquipGroup(std::vector<short> items) {
			Items = items;
		}

		short NewReserveItem(short equippedItem, short reserveItem) const {
			char reserveMode = ReserveMode;
			if (reserveMode == RESERVEMODE_NORMAL) {
				return -1;
				unsigned short* ppCurrentData = *(unsigned short**)Mem::CurrentData0;
				reserveMode = ((ppCurrentData[3] & 0x200) == 0x200) ? RESERVEMODE_PREVIOUS : RESERVEMODE_UNEQUIP;
			}

			switch (reserveMode) {
			case RESERVEMODE_NONE:
				return reserveItem;
			case RESERVEMODE_PREVIOUS:
				return equippedItem;
			case RESERVEMODE_ITEM:
				return ReserveItem;
			default:
				return 0;
			}
		}
	};

	bool Active = true;
	bool Unequip = false;
	char ReserveMode = RESERVEMODE_NORMAL; // 0 = no change, 1 = normal, 2 = previous, 3 = unequip, 4 = item

	std::array<EquipGroup*, 10> WeaponGroups = { {
		new EquipGroup({1, 2, 3}),
		new EquipGroup({15, 18}),
		new EquipGroup({17, 16}),
		new EquipGroup({6, 5}),
		new EquipGroup({7}),
		new EquipGroup({8, 21}),
		new EquipGroup({9, 14}),
		new EquipGroup({11}),
		new EquipGroup({10}),
		new EquipGroup({4, 19})
	} };
	std::array<short, 6> ZeroAmmoWeapons = { 8, 9, 10, 11, 16, 17 };

	std::array<EquipGroup*, 10> ItemGroups = { {
		new EquipGroup({1}),
		new EquipGroup({5}),
		new EquipGroup({3, 4}),
		new EquipGroup({13, 12}),
		new EquipGroup({9, 10, 11, 25, 21}),
		new EquipGroup({14, 15}),
		new EquipGroup({16, 22, 23, 26, 27, 24}),
		new EquipGroup({7, 6}),
		new EquipGroup({32, 36, 37, 38, 39, 40}),
		new EquipGroup({8})
	} };

	std::map<std::string, char> ReserveModeMap{
		{ "nochange", RESERVEMODE_NONE }, { "normal", RESERVEMODE_NORMAL }, { "previous", RESERVEMODE_PREVIOUS }, { "unequip", RESERVEMODE_UNEQUIP }
	};


	bool ChangeWeapon(short weapon, bool reserve) {
		if (!reserve && ((weapon == 0) || ((Unequip) && (*Mem::EquippedWeapon == weapon)))) {
			*Mem::EquippedWeapon = 0;
			return true;
		}

		short* weaponData = (short*)*Mem::WeaponData;
		if (weaponData == 0) {
			return false;
		}

		short noAmmo = (std::find(ZeroAmmoWeapons.begin(), ZeroAmmoWeapons.end(), weapon) == ZeroAmmoWeapons.end()) ? 0 : 1;

		short ammo = weaponData[weapon];
		if (ammo < noAmmo) {
			return false;
		}

		if (reserve) {
			*(char*)0x9AA890 = (char)weapon;
		}
		else {
			*Mem::EquippedWeapon = weapon;
		}
		return true;
	}

	bool ChangeWeaponGroup(short id) {
		if ((id < 0) || (id > 9)) {
			return false;
		}

		EquipGroup* group = WeaponGroups[id];
		if (group == nullptr) {
			return false;
		}

		short equippedWeapon = *Mem::EquippedWeapon;
		std::vector<short> &weaponGroup = group->Items;

		auto it = std::find(weaponGroup.begin(), weaponGroup.end(), equippedWeapon);
		short sLastElement = weaponGroup.back();

		if (group->Unequip && (equippedWeapon == sLastElement)) {
			return ChangeWeapon(0);
		}

		if ((it != weaponGroup.end()) && (it != (weaponGroup.end() - 1))) { // found, and not the last element
			std::rotate(weaponGroup.begin(), it + 1, weaponGroup.end());
		}

		for (short weapon : weaponGroup) {
			if (ChangeWeapon(weapon)) {
				ChangeWeapon(group->NewReserveItem(equippedWeapon, *Mem::ReserveWeapon), true);
				return true;
			}
			if (group->Unequip && (weapon == sLastElement)) {
				return ChangeWeapon(0);
			}
		}

		*Mem::EquippedWeapon = 0;
		return false;
	}

	bool ChangeItem(short item, bool reserve) {
		if (!reserve && ((item == 0) || ((Unequip) && (*Mem::EquippedItem == item)))) {
			*Mem::EquippedItem = 0;
			return true;
		}

		short* itemData = (short*)*Mem::ItemData;
		if (itemData == 0) {
			return false;
		}

		short ammo = itemData[item];
		if (ammo < 1) {
			return false;
		}

		if (reserve) {
			*(char*)0x9AA891 = (char)item;
		}
		else {
			*Mem::EquippedItem = item;
		}
		return true;
	}

	bool ChangeItemGroup(short id) {
		if ((id < 0) || (id > 9)) {
			return false;
		}

		EquipGroup* group = ItemGroups[id];
		if (group == nullptr) {
			return false;
		}

		short equippedItem = *Mem::EquippedItem;
		std::vector<short> &itemGroup = group->Items;
		short sLastElement = itemGroup.back();

		if (group->Unequip && (equippedItem == sLastElement)) {
			return ChangeItem(0);
		}

		auto it = std::find(itemGroup.begin(), itemGroup.end(), equippedItem);
		if ((it != itemGroup.end()) && (it != (itemGroup.end() - 1))) { // found, and not the last element
			std::rotate(itemGroup.begin(), it + 1, itemGroup.end());
		}

		for (short item : itemGroup) {
			if (ChangeItem(item)) {
				ChangeItem(group->NewReserveItem(equippedItem, *Mem::ReserveItem), true);
				return true;
			}
			if (group->Unequip && (item == sLastElement)) {
				return ChangeItem(0);
			}
		}

		*Mem::EquippedItem = 0;
		return false;
	}


	void ChangeWeaponGroup(Actions::Action action) {
		if (Active) {
			ChangeWeaponGroup(action.CastData<short>());
		}
	}

	void ChangeWeapon(Actions::Action action) {
		if (Active) {
			ChangeWeapon(action.CastData<short>());
		}
	}

	void ChangeItemGroup(Actions::Action action) {
		if (Active) {
			ChangeItemGroup(action.CastData<short>());

		}
	}

	void ChangeItem(Actions::Action action) {
		if (Active) {
			ChangeItem(action.CastData<short>());
		}
	}


	void ToggleAction(Actions::Action action) {
		Active = !Active;
		Log::DisplayToggleMessage("Equip Shortcuts", Active);
	}

	bool NewGameInfoCallback() {
		return Active;
	}

	void ParseReserveMode(CSimpleIniA& ini, const char* category, EquipGroup* group, bool weaponGroup) {
		const char* key = "Reserve";
		char reserveMode = ConfigParser::ParseValueMap(ini, category, key, ReserveModeMap, (char)-1);

		if (reserveMode == -1) {
			short item = ConfigParser::ParseValueMap(ini, category, key, weaponGroup ? MGS2::InventoryData::WeaponMap : MGS2::InventoryData::ItemMap, (short)-1);
			if (item == -1) {
				reserveMode = ReserveMode;
			}
			else {
				reserveMode = RESERVEMODE_ITEM;
				group->ReserveItem = item;
			}
		}

		group->ReserveMode = reserveMode;
	}


	void Run(CSimpleIniA& ini) {
		if (ini.IsEmpty() || (!ini.GetBoolValue(Category, "Enabled", false))) {
			return;
		}

		ReserveMode = ConfigParser::ParseValueMap(ini, Category, "Reserve", ReserveModeMap, ReserveMode);

		for (short i = 0; i < 10; i++) {
			std::string strWeapon = "EquipShortcuts.WeaponGroup." + std::to_string(i);
			const char* ccWeapon = strWeapon.c_str();
			std::list<CSimpleIniA::Entry> lstWeapons;
			if (ini.GetAllValues(ccWeapon, "Weapon", lstWeapons)) {
				std::vector<short> vOverrideGroup;
				for (const CSimpleIniA::Entry& weapon : lstWeapons) {
					short newWeapon = ConfigParser::ParseValueMap(weapon.pItem, MGS2::InventoryData::WeaponMap, (short)-1);
					if (newWeapon != -1) {
						vOverrideGroup.push_back(newWeapon);
					}
				}
				if (!vOverrideGroup.empty()) {
					WeaponGroups[i]->Items = vOverrideGroup;
				}
			}
			WeaponGroups[i]->Unequip = ini.GetBoolValue(ccWeapon, "Unequip", Unequip);
			ParseReserveMode(ini, ccWeapon, WeaponGroups[i], true);
						
			std::string strItem = "EquipShortcuts.ItemGroup." + std::to_string(i);
			const char* ccItem = strItem.c_str();
			std::list<CSimpleIniA::Entry> lstItems;
			if (ini.GetAllValues(ccItem, "Item", lstItems)) {
				std::vector<short> vOverrideGroup;
				for (const CSimpleIniA::Entry& item : lstItems) {
					short newItem = ConfigParser::ParseValueMap(item.pItem, MGS2::InventoryData::ItemMap, (short)-1);
					if (newItem != -1) {
						vOverrideGroup.push_back(newItem);
					}
				}
				if (!vOverrideGroup.empty()) {
					ItemGroups[i]->Items = vOverrideGroup;
				}
			}
			ItemGroups[i]->Unequip = ini.GetBoolValue(ccItem, "Unequip", Unequip);
			ParseReserveMode(ini, ccItem, ItemGroups[i], false);

			Actions::RegisterAction(ini, strWeapon.c_str(), &ChangeWeaponGroup, (void*)i);
			Actions::RegisterAction(ini, strItem.c_str(), &ChangeItemGroup, (void*)i);
		}

		for (short i = 0; i <= 21; i++) {
			std::string strWeapon = "EquipShortcuts.Weapon." + std::to_string(i);
			Actions::RegisterAction(ini, strWeapon.c_str(), &ChangeWeapon, (void*)i);
		}

		for (short i = 0; i <= 40; i++) {
			std::string strItem = "EquipShortcuts.Item." + std::to_string(i);
			Actions::RegisterAction(ini, strItem.c_str(), &ChangeItem, (void*)i);
		}

		Active = ini.GetBoolValue(Category, "Active", Active);
		Actions::RegisterAction(ini, "EquipShortcuts.Toggle", &ToggleAction);

		Unequip = ini.GetBoolValue(Category, "Unequip", Unequip);

		NewGameInfo::AddWarning("Equip Shortcuts", &NewGameInfoCallback);
	}

}
