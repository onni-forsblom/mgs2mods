// A mod by Lucky Rapidflower (2025) for bmn's ASI plugin
#include "MGS2.framework.h"
#include "MGS2.EventLoadout.h"
#include "MGS2.Pickup.h"

namespace MGS2::Challenge {
	const char* Category = "Challenge";

	TextConfig ChallengeConfig{ false,  318, 180, Center, 0xFF5500 };

	// Activates on title screen, displays text
	tFUN_Void_Int oFUN_00744a40;
	void _cdecl hkFUN_00744a40(int param_1) {
		oFUN_00744a40(param_1);
		ChallengeConfig.Draw();
	}

	void Run(CSimpleIniA& ini) {
		// If no data has been loaded from the ini file or the mod is disabled
		// do not go further
		if (ini.IsEmpty() || (!ini.GetBoolValue(Category, "Enabled", false))) {
			return;
		}

		NewGameInfo::AddWarning("Challenge");

		ChallengeConfig.Content = "Lucky's Remix\nv0.1";

		// Prevent clipping into the engine room
		ExploitFix::Run();

		// EventLoadout

		// let's have a look at your equipment cutscene
		std::pair<short, EventLoadout::LoadoutData> tanker6loadout; 
		tanker6loadout.first = 6;
		tanker6loadout.second.WeaponsData = {
			{1, 3, 15}, // M9
			{2, -1, 15}, // USP
			{16, 3} // magazines
		};

		// tanker gameplay start
		std::pair<short, EventLoadout::LoadoutData> tanker14loadout;
		tanker14loadout.first = 14;
		tanker14loadout.second.WeaponsData = {
			{1, 2} // M9
		};
		tanker14loadout.second.FlagAddressToORMaskMap = {
			{(std::uint8_t*)0x118DB21, 0b11000000} // tanker flags to check off a few guard cutscenes
		};

		// after defeating Olga
		std::pair<short, EventLoadout::LoadoutData> tanker29loadout;
		tanker29loadout.first = 29;
		tanker29loadout.second.FlagAddressToORMaskMap = {
			{(std::uint8_t*)0x118E4E4, 0b10} // engine room flags to make the guard layout the same as revisits
		};

		EventLoadout::stage_to_progress_to_loadout_u_map stageToProgressToLoadoutMap = {
			{
				Tanker,
				{
					tanker6loadout,
					tanker14loadout,
					tanker29loadout
				}
			}
		};

		EventLoadout::Run(stageToProgressToLoadoutMap, false);

		// Pickup

		// Default pickups

		std::pair<Pickup::DefaultPickupIdentifiers, Pickup::DefaultPickupData> m9Ammo; 
		m9Ammo.first = { 1, Pickup::PickupCategory::Ammo };
		m9Ammo.second.Amount = 2;

		std::pair<Pickup::DefaultPickupIdentifiers, Pickup::DefaultPickupData> uspAmmo;
		uspAmmo.first = { 2, Pickup::PickupCategory::Ammo };
		uspAmmo.second.Amount = 3;

		std::pair<Pickup::DefaultPickupIdentifiers, Pickup::DefaultPickupData> chaff;
		chaff.first = { 10, Pickup::PickupCategory::StandaloneWeapon };
		chaff.second.Amount = 1;

		Pickup::default_pickup_ids_to_data_map defaultPickupIdentifiersToDataMap = {
			m9Ammo, uspAmmo, chaff
		};

		// Pickup instances

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> tankerStartChaff;
		tankerStartChaff.first = {"w00a",10000,2224,-500 };
		tankerStartChaff.second.X = 7498;
		tankerStartChaff.second.Z = 4474;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> deckAM9Ammo;
		deckAM9Ammo.first = { "w01a",4500,417,3550};
		deckAM9Ammo.second.X = -10750;
		deckAM9Ammo.second.Z = -11176;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> crewLoungeM9ammo;
		crewLoungeM9ammo.first = { "w01f",9000,217,-18500};
		crewLoungeM9ammo.second.X = 7137;
		crewLoungeM9ammo.second.Y = 0;
		crewLoungeM9ammo.second.Z = -17824;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> crewLoungeUSPammo;
		crewLoungeUSPammo.first = {"w01f",-12500,-4783,-12500};
		crewLoungeUSPammo.second.X = 7;
		crewLoungeUSPammo.second.Y = 4;
		crewLoungeUSPammo.second.Z = -15900;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> deckBM9ammo;
		deckBM9ammo.first = {"w01b",11000,3217,-14750};
		deckBM9ammo.second.X = 2376;
		deckBM9ammo.second.Z = 5500;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> deckCChaff;
		deckCChaff.first = {"w01c",-1500,6424,-18160};
		deckCChaff.second.X = -4719;
		deckCChaff.second.Y = 5924;
		deckCChaff.second.Z = -12769;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> deckDM9ammoPantry;
		deckDM9ammoPantry.first = {"w01d",7700,9217,-11850};
		deckDM9ammoPantry.second.X = 9448;
		deckDM9ammoPantry.second.Z = -19326;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> deckDbox1;
		deckDbox1.first = {"w01d",10000,9353,-10000};
		deckDbox1.second.X = 10266;
		deckDbox1.second.Z = -19326;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> deckDUSPammoCamera;
		deckDUSPammoCamera.first = {"w01d",-2500,9217,-11875};
		deckDUSPammoCamera.second.X = -1649;
		deckDUSPammoCamera.second.Y = 9017;
		deckDUSPammoCamera.second.Z = -12970;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> deckDUSPammoTable;
		deckDUSPammoTable.first = {"w01d",-9250,9217,-13000};
		deckDUSPammoTable.second.X = -9050;
		deckDUSPammoTable.second.Z = -11443;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> deckDM9ammoTable;
		deckDM9ammoTable.first = {"w01d",-4750,9217,-13000};
		deckDM9ammoTable.second.X = -9550;
		deckDM9ammoTable.second.Z = -11443;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> deckEUSPammo;
		deckEUSPammo.first = {"w01e",4817,12217,-18682};
		deckEUSPammo.second.DefaultPickupDataReplacement.SpawnType = Pickup::NeverSpawn;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> olgaM9AmmoBetweenBoxes;
		olgaM9AmmoBetweenBoxes.first = {"w00b",-11500,12217,-13500};
		olgaM9AmmoBetweenBoxes.second.DefaultPickupDataReplacement.SpawnType = Pickup::NeverSpawn;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> olgaM9AmmoSouth;
		olgaM9AmmoSouth.first = {"w00b",-6600,12217,-19250};
		olgaM9AmmoSouth.second.X = -12231;
		olgaM9AmmoSouth.second.Y = 11917;
		olgaM9AmmoSouth.second.Z = -10476;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> olgaRationVE;
		olgaRationVE.first = {"w00b",-11250,12221,-10750};
		olgaRationVE.second.X = -4550;
		olgaRationVE.second.Z = -9500;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> navDeckRationNearOlga;
		navDeckRationNearOlga.first = {"w00c",-11500,12221,-18750};
		navDeckRationNearOlga.second.X = 474;
		navDeckRationNearOlga.second.Z = -9526;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> navDeckUSPSuppressor;
		navDeckUSPSuppressor.first = {"w00c",-750,24226,-13250};
		navDeckUSPSuppressor.second.X = -11502;
		navDeckUSPSuppressor.second.Y = 12174;
		navDeckUSPSuppressor.second.Z = -18149;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> navDeckWetBox;
		navDeckWetBox.first = {"w00c",18000,13103,-15000};
		navDeckWetBox.second.X = 12869;
		navDeckWetBox.second.Z = -13778;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> vulcanRavenRoomUSPammo;
		vulcanRavenRoomUSPammo.first = {"w02a",16224,-3782,-22618};
		vulcanRavenRoomUSPammo.second.DefaultPickupDataReplacement.SpawnType = Pickup::NeverSpawn;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> engineRoomM9ammo;
		engineRoomM9ammo.first = {"w02a",7000,-12783,-19500};
		engineRoomM9ammo.second.X = -5500;
		engineRoomM9ammo.second.Z = -21500;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> engineRoomBtmFloorUSPammo;
		engineRoomBtmFloorUSPammo.first = {"w02a",-5500,-12783,-21500};
		engineRoomBtmFloorUSPammo.second.X = 12500;
		engineRoomBtmFloorUSPammo.second.Z = -526;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> engineRoomMidFloorUSPammo;
		engineRoomMidFloorUSPammo.first = {"w02a",-6500,-8783,-10000};
		engineRoomMidFloorUSPammo.second.X = 26;
		engineRoomMidFloorUSPammo.second.Z = 5476;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> engineRoomLasersWestUSPammo;
		engineRoomLasersWestUSPammo.first = {"w02a",-16000,-3783,-22250};
		engineRoomLasersWestUSPammo.second.DefaultPickupDataReplacement.SpawnType = Pickup::NeverSpawn;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> engineRoomLasersEastUSPammo;
		engineRoomLasersEastUSPammo.first = {"w02a",-10000,-3783,-22500};
		engineRoomLasersEastUSPammo.second.DefaultPickupDataReplacement.Amount = 1;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> deck2MarineUSPammo;
		deck2MarineUSPammo.first = {"w03a",-14000,-4783,-68500};
		deck2MarineUSPammo.second.DefaultPickupDataReplacement.SpawnType = Pickup::NeverSpawn;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> deck2junctionUSPammo;
		deck2junctionUSPammo.first = {"w03a",-17500,-4783,-125750};
		deck2junctionUSPammo.second.DefaultPickupDataReplacement.SpawnType = Pickup::NeverSpawn;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> deck2horizontalHallUSPammo;
		deck2horizontalHallUSPammo.first = {"w03a",-4000,-4783,-128000};
		deck2horizontalHallUSPammo.second.DefaultPickupDataReplacement.SpawnType = Pickup::NeverSpawn;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> deck2nearStarboardUSPammo;
		deck2nearStarboardUSPammo.first = {"w03a",10500,-4783,-126500};
		deck2nearStarboardUSPammo.second.DefaultPickupDataReplacement.SpawnType = Pickup::NeverSpawn;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> deck2starboardM9ammo;
		deck2starboardM9ammo.first = {"w03b",12300,-4783,-128750};
		deck2starboardM9ammo.second.DefaultPickupDataReplacement.Amount = 8;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> deck2starboardPipesUSPammo;
		deck2starboardPipesUSPammo.first = {"w03b",12607,-4783,-100250};
		deck2starboardPipesUSPammo.second.DefaultPickupDataReplacement.SpawnType = Pickup::NeverSpawn;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> deck2starboardNorthUSPammo;
		deck2starboardNorthUSPammo.first = {"w03b",15750,-4783,-128500};
		deck2starboardNorthUSPammo.second.DefaultPickupDataReplacement.SpawnType = Pickup::NeverSpawn;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> guardRushNorthUSPammo;
		guardRushNorthUSPammo.first = {"w03b",17250,-4783,-86000};
		guardRushNorthUSPammo.second.DefaultPickupDataReplacement.SpawnType = Pickup::NeverSpawn;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> guardRushSouthUSPammo;
		guardRushSouthUSPammo.first = {"w03b",17250,-4783,-87000};
		guardRushSouthUSPammo.second.DefaultPickupDataReplacement.Amount = 5;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> guardRushRation;
		guardRushRation.first = {"w03b",16250,-4779,-87000};
		guardRushRation.second.DefaultPickupDataReplacement.SpawnType = Pickup::NeverSpawn;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> hold1entranceM9ammo;
		hold1entranceM9ammo.first = {"w04a",7000,-4783,1750};
		hold1entranceM9ammo.second.DefaultPickupDataReplacement.SpawnType = Pickup::NeverSpawn;

		Pickup::pickup_instance_ids_to_data_map pickupInstanceIdentifiersToDataMap = {
			tankerStartChaff,
			deckAM9Ammo,
			crewLoungeM9ammo,
			crewLoungeUSPammo,
			deckBM9ammo,
			deckCChaff,
			deckDM9ammoPantry,
			deckDbox1,
			deckDUSPammoCamera,
			deckDUSPammoTable,
			deckDM9ammoTable,
			deckEUSPammo,
			olgaM9AmmoBetweenBoxes,
			olgaM9AmmoSouth,
			olgaRationVE,
			navDeckRationNearOlga,
			navDeckUSPSuppressor,
			navDeckWetBox,
			vulcanRavenRoomUSPammo,
			engineRoomM9ammo,
			engineRoomBtmFloorUSPammo,
			engineRoomMidFloorUSPammo,
			engineRoomLasersWestUSPammo,
			engineRoomLasersEastUSPammo,
			deck2MarineUSPammo,
			deck2junctionUSPammo,
			deck2horizontalHallUSPammo,
			deck2nearStarboardUSPammo,
			deck2starboardM9ammo,
			deck2starboardPipesUSPammo,
			deck2starboardNorthUSPammo,
			guardRushNorthUSPammo,
			guardRushSouthUSPammo,
			guardRushRation,
			hold1entranceM9ammo
		};

		Pickup::Run(defaultPickupIdentifiersToDataMap, pickupInstanceIdentifiersToDataMap);

		// Hook function to display text on title screen
		oFUN_00744a40 = (tFUN_Void_Int)mem::TrampHook32((BYTE*)0x744A40, (BYTE*)hkFUN_00744a40, 6);
	}
}
