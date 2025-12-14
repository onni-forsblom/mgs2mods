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

		ChallengeConfig.Content = "Lucky's Remix";

		// Set up the alert mode mechanics
		AlertModeMechanics::Run(true, true, true, false, 3);

		mem::PatchSet patchSet = mem::PatchSet{
			// Set the chaff timer to 5s
			{mem::Patch((void*)0x957349, "\x2C\x01")},

			// Set guard tranq sleep time to 20s
			{mem::Patch((void*)0xF6000C, "\xB0\x04")},

			// Remove the code that changes guard tranq sleep time by difficulty
			{mem::Patch((void*)0x42DF7D, "\x90\x90\x90\x90\x90")},

			// Set guard KO time to 5s
			{mem::Patch((void*)0xF60004, "\x2C\x01")},

			// Remove the code that changes guard KO time by difficulty
			{mem::Patch((void*)0x42DF8B, "\x90\x90\x90\x90\x90")},

			// Remove lines of code that overwrite stats stored at section start
			// (this way, using a continue will have those stats reset to what they were at the checkpoint)
			{mem::Patch((void*)0x877E7E, "\x90\x90\x90\x90\x90")}, // ammo used
			{mem::Patch((void*)0x877E90, "\x90\x90\x90\x90\x90")}, // alerts
			{mem::Patch((void*)0x877EA4, "\x90\x90\x90\x90\x90")}, // kills
			{mem::Patch((void*)0x877EB8, "\x90\x90\x90\x90\x90")}, // damage taken
			{mem::Patch((void*)0x877ECD, "\x90\x90\x90\x90\x90")}, // rations used
			{mem::Patch((void*)0x877EE1, "\x90\x90\x90\x90\x90")}, // clearing escapes
			{mem::Patch((void*)0x877F1B, "\x90\x90\x90\x90\x90")}, // mech kills

			// Change the ranking system to count continues as three saves when calculating #1 rank
			{mem::Patch((void*)0x945BDF, "\x6B\xC0\x03\x03\x41\x20\x83\xF8\x08\x7F\x37\x83\x79\x24\x00\x75\x31", 17)},

			// Change the ranking system to count continues as three saves when calculating #2 rank
			{mem::Patch((void*)0x945C42, "\x6B\xD2\x03\x03\x51\x20\x83\xFA\x10\x7F\x21\x83\x79\x24\x00\x75\x1B", 17)},

			// Change the ranking system to ignore continues when calculating #3 rank
			{mem::Patch((void*)0x945C88, "\x90\x90\x90\x90\x90\x90\x90")},

			// Change the ranking system to ignore continues when calculating #4 rank
			{mem::Patch((void*)0x945CC1, "\x90\x90\x90\x90\x90\x90\x90")}
		};
		patchSet.Patch();

		// Prevent clipping into the engine room, Vamp non-lethal exploit and coolant/d.mic vent upclip
		ExploitFix::Run();

		// EventLoadout

		// Tanker

		// "let's have a look at your equipment" cutscene
		std::pair<short, EventLoadout::LoadoutData> tanker6loadout; 
		tanker6loadout.first = 6;
		tanker6loadout.second.WeaponsData = {
			{1, 3, 15}, // M9
			{2, -1, 15}, // USP
			{16, 3} // magazines
		};

		tanker6loadout.second.EquipmentData = {
			{1, 0, 2} // set ration max amount to 2
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

		// Plant

		// Plant gameplay start
		std::pair<short, EventLoadout::LoadoutData> plant9loadout;
		plant9loadout.first = 9;
		plant9loadout.second.WeaponsData = {
			{16, 3}, // magazines
			{21, 2}, // books

			// Set only the max capacity for these weapons
			{1, -1, 15}, // M9
			{3, -1, 15}, // socom
			{15, -1, 30}, // AK
			{18, -1, 30}, // M4
			{4, -1, 20}, // PSG1
			{19, -1, 15}, // PSG1-T
			{7, -1, 20}, // Stinger
			{6, -1, 20}, // Nikita
			{5, -1, 18} // RGB6
		};

		plant9loadout.second.EquipmentData = {
			{1, 0, 2} // set ration max amount to 2
		};

		// BC bridge after Fortune (should set this after the Vamp cutscene but there are problems with that)
		std::pair<short, EventLoadout::LoadoutData> plant62loadout;
		plant62loadout.first = 62;
		plant62loadout.second.WeaponsData = {
			{3, 1, 15} // socom
		};

		// Fortune fight end (fix all parcel room cameras)
		std::pair<short, EventLoadout::LoadoutData> plant115loadout;
		plant115loadout.first = 115;
		plant115loadout.second.ToggleableActorToStatusMap = {
			{EventLoadout::ParcelRoomNWcam, true},
			{EventLoadout::ParcelRoomSWcam, true},
			{EventLoadout::ParcelRoomSEcam, true}
		};

		// Final bomb countdown (fix all parcel room cameras and remove the shell 1 core 1f southmost camera)
		std::pair<short, EventLoadout::LoadoutData> plant120loadout;
		plant120loadout.first = 120;
		plant120loadout.second.ToggleableActorToStatusMap = {
			{EventLoadout::ParcelRoomNWcam, true},
			{EventLoadout::ParcelRoomSWcam, true},
			{EventLoadout::ParcelRoomSEcam, true},
			{EventLoadout::Shell1Core1FSouthCam, false}
		};

		// Outside hostage room after Ames
		std::pair<short, EventLoadout::LoadoutData> plant176loadout;
		plant176loadout.first = 176;
		plant176loadout.second.EquipmentData = {
			{6, 0} // remove the BDU
		};
		plant176loadout.second.ToggleableActorToStatusMap = {
			// Fix all parcel room cameras
			{EventLoadout::ParcelRoomNWcam, true},
			{EventLoadout::ParcelRoomSWcam, true},
			{EventLoadout::ParcelRoomSEcam, true},
			// Open some of the shell 1 core 1f south lockers that may have new items
			{EventLoadout::Shell1Core1FSouth3rdLockerFromWest, true},
			{EventLoadout::Shell1Core1FSouth2ndLockerFromEast, true}
		};

		// Shell 1 escape (constant caution)
		std::pair<short, EventLoadout::LoadoutData> plant180loadout;
		plant180loadout.first = 180;
		plant180loadout.second.AlertMode = AlertMode::Caution;

		// Harrier end (remove stinger ammo)
		std::pair<short, EventLoadout::LoadoutData> plant190loadout;
		plant190loadout.first = 190;
		plant190loadout.second.WeaponsData = {
			{7, 0}
		};

		// After prez codec cutscene (fix the turrets in Shell 2 core)
		std::pair<short, EventLoadout::LoadoutData> plant231loadout;
		plant231loadout.first = 231;
		plant231loadout.second.ToggleableActorToStatusMap = {
			{EventLoadout::Shell2Core1FNWTurret, true},
			{EventLoadout::Shell2Core1FSWTurret, true}
		};

		// Gameplay after codecs after president
		std::pair<short, EventLoadout::LoadoutData> plant241loadout;
		plant241loadout.first = 241;
		plant241loadout.second.FlagAddressToORMaskMap ={ // basically, make sure all guards are around for Shell 2 core
			{(std::uint8_t*)0x118E077, 0b11111011}, 
			{(std::uint8_t*)0x118E01A, 0b10100000}
		};

		// Gameplay starting from second Emma swim
		std::pair<short, EventLoadout::LoadoutData> plant297loadout;
		plant297loadout.first = 297;
		plant297loadout.second.FlagAddressToORMaskMap = { // basically, make sure all guards are around for Shell 2 core
			{(std::uint8_t*)0x118E077, 0b11111011},
			{(std::uint8_t*)0x118E01A, 0b10100000}
		};

		// Vamp 2 fight end (fix all parcel room cameras)
		std::pair<short, EventLoadout::LoadoutData> plant318loadout;
		plant318loadout.first = 318;
		plant318loadout.second.ToggleableActorToStatusMap = {
			{EventLoadout::ParcelRoomNWcam, true},
			{EventLoadout::ParcelRoomSWcam, true},
			{EventLoadout::ParcelRoomSEcam, true}
		};
		

		// (Also fix the parcel room cameras for the Emma countdown)

		EventLoadout::stage_to_progress_to_loadout_u_map stageToProgressToLoadoutMap = {
			{
				Tanker,
				{
					tanker6loadout,
					tanker14loadout,
					tanker29loadout
				}
			},
			{
				Plant,
				{
					plant9loadout,
					plant62loadout,
					plant115loadout,
					plant120loadout,
					plant176loadout,
					plant180loadout,
					plant190loadout,
					plant231loadout,
					plant241loadout,
					plant297loadout,
					plant318loadout
				}
			}
		};

		EventLoadout::Run(stageToProgressToLoadoutMap, true, true, true, true);

		// Pickup

		// Default pickups

		// ammo

		std::pair<Pickup::DefaultPickupIdentifiers, Pickup::DefaultPickupData> m9Ammo; 
		m9Ammo.first = { 1, Pickup::PickupCategory::Ammo };
		m9Ammo.second.Amount = 2;

		std::pair<Pickup::DefaultPickupIdentifiers, Pickup::DefaultPickupData> uspAmmo;
		uspAmmo.first = { 2, Pickup::PickupCategory::Ammo };
		uspAmmo.second.Amount = 3;

		std::pair<Pickup::DefaultPickupIdentifiers, Pickup::DefaultPickupData> socomAmmo;
		socomAmmo.first = { 3, Pickup::PickupCategory::Ammo };
		socomAmmo.second.Amount = 3;

		std::pair<Pickup::DefaultPickupIdentifiers, Pickup::DefaultPickupData> akAmmo;
		akAmmo.first = { 15, Pickup::PickupCategory::Ammo };
		akAmmo.second.Amount = 3;

		std::pair<Pickup::DefaultPickupIdentifiers, Pickup::DefaultPickupData> m4Ammo;
		m4Ammo.first = { 18, Pickup::PickupCategory::Ammo };
		m4Ammo.second.Amount = 3;

		std::pair<Pickup::DefaultPickupIdentifiers, Pickup::DefaultPickupData> psg1Ammo;
		psg1Ammo.first = { 4, Pickup::PickupCategory::Ammo };
		psg1Ammo.second.Amount = 3;

		std::pair<Pickup::DefaultPickupIdentifiers, Pickup::DefaultPickupData> psg1tAmmo;
		psg1tAmmo.first = { 19, Pickup::PickupCategory::Ammo };
		psg1tAmmo.second.Amount = 2;

		std::pair<Pickup::DefaultPickupIdentifiers, Pickup::DefaultPickupData> nikitaAmmo;
		nikitaAmmo.first = { 6, Pickup::PickupCategory::Ammo };
		nikitaAmmo.second.Amount = 3;

		std::pair<Pickup::DefaultPickupIdentifiers, Pickup::DefaultPickupData> stingerAmmo;
		stingerAmmo.first = { 7, Pickup::PickupCategory::Ammo };
		stingerAmmo.second.Amount = 3;

		std::pair<Pickup::DefaultPickupIdentifiers, Pickup::DefaultPickupData> rgb6Ammo;
		rgb6Ammo.first = { 5, Pickup::PickupCategory::Ammo };
		rgb6Ammo.second.Amount = 3;

		// weapons

		std::pair<Pickup::DefaultPickupIdentifiers, Pickup::DefaultPickupData> m9;
		m9.first = { 1, Pickup::PickupCategory::StandaloneWeapon };
		m9.second.Amount = 1;

		std::pair<Pickup::DefaultPickupIdentifiers, Pickup::DefaultPickupData> ak;
		ak.first = { 15, Pickup::PickupCategory::StandaloneWeapon };
		ak.second.Amount = 0;

		std::pair<Pickup::DefaultPickupIdentifiers, Pickup::DefaultPickupData> psg1;
		psg1.first = { 4, Pickup::PickupCategory::StandaloneWeapon };
		psg1.second.Amount = 0;

		std::pair<Pickup::DefaultPickupIdentifiers, Pickup::DefaultPickupData> nikita;
		nikita.first = { 6, Pickup::PickupCategory::StandaloneWeapon };
		nikita.second.Amount = 0;

		std::pair<Pickup::DefaultPickupIdentifiers, Pickup::DefaultPickupData> m4;
		m4.first = { 18, Pickup::PickupCategory::StandaloneWeapon };
		m4.second.Amount = 3;

		std::pair<Pickup::DefaultPickupIdentifiers, Pickup::DefaultPickupData> psg1t;
		psg1t.first = { 19, Pickup::PickupCategory::StandaloneWeapon };
		psg1t.second.Amount = 0;

		std::pair<Pickup::DefaultPickupIdentifiers, Pickup::DefaultPickupData> rgb6;
		rgb6.first = { 5, Pickup::PickupCategory::StandaloneWeapon };
		rgb6.second.Amount = 3;

		// grenades etc.

		std::pair<Pickup::DefaultPickupIdentifiers, Pickup::DefaultPickupData> chaff;
		chaff.first = { 10, Pickup::PickupCategory::StandaloneWeapon };
		chaff.second.Amount = 1;

		std::pair<Pickup::DefaultPickupIdentifiers, Pickup::DefaultPickupData> stun;
		stun.first = { 11, Pickup::PickupCategory::StandaloneWeapon };
		stun.second.Amount = 1;

		Pickup::default_pickup_ids_to_data_map defaultPickupIdentifiersToDataMap = {
			m9Ammo, uspAmmo, socomAmmo, akAmmo, m4Ammo, psg1Ammo, psg1tAmmo, nikitaAmmo, stingerAmmo, rgb6Ammo,
			m9, ak, psg1, nikita, m4, psg1t, rgb6,
			chaff, stun
		};

		// Pickup instances

		// Tanker

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> tankerStartChaff;
		tankerStartChaff.first = {"w00a",10000,2224,-500 };
		tankerStartChaff.second.DefaultPickupDataReplacement.MinProgress = 30;
		//tankerStartChaff.second.X = 7498;
		//tankerStartChaff.second.Z = 4474;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> deckAM9Ammo;
		deckAM9Ammo.first = { "w01a",4500,417,3550};
		//deckAM9Ammo.second.X = -10750;
		//deckAM9Ammo.second.Z = -11176;
		deckAM9Ammo.second.DefaultPickupDataReplacement.SpawnType = Pickup::NeverSpawn;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> crewLoungeM9ammo;
		crewLoungeM9ammo.first = { "w01f",9000,217,-18500};
		//crewLoungeM9ammo.second.X = -8726;
		//crewLoungeM9ammo.second.Z = -18294;
		crewLoungeM9ammo.second.X = -7805;
		crewLoungeM9ammo.second.Z = -18294;
		/*crewLoungeM9ammo.second.X = 7137;
		crewLoungeM9ammo.second.Y = 0;
		crewLoungeM9ammo.second.Z = -17824;*/

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> crewLoungeUSPammo;
		crewLoungeUSPammo.first = {"w01f",-12500,-4783,-12500};
		crewLoungeUSPammo.second.X = 7;
		crewLoungeUSPammo.second.Y = 4;
		crewLoungeUSPammo.second.Z = -15900;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> deckBUSPammo;
		deckBUSPammo.first = { "w01b",1067,3417,5500 };
		deckBUSPammo.second.DefaultPickupDataReplacement.MinProgress = 30;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> deckBM9ammo;
		deckBM9ammo.first = {"w01b",11000,3217,-14750};
		deckBM9ammo.second.DefaultPickupDataReplacement.MinProgress = 30;
		//deckBM9ammo.second.X = 2376;
		//deckBM9ammo.second.Z = 5500;

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
		deckDUSPammoTable.second.DefaultPickupDataReplacement.MinProgress = 30;
		//deckDUSPammoTable.second.X = -9050;
		deckDUSPammoTable.second.X = -9250;
		deckDUSPammoTable.second.Z = -11443;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> deckDM9ammoTable;
		deckDM9ammoTable.first = {"w01d",-4750,9217,-13000};
		deckDM9ammoTable.second.X = -1649;
		deckDM9ammoTable.second.Y = 9567;
		deckDM9ammoTable.second.Z = -12970;
		//deckDM9ammoTable.second.X = -9550;
		//deckDM9ammoTable.second.Z = -11443;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> deckEUSPammo;
		deckEUSPammo.first = {"w01e",4817,12217,-18682};
		deckEUSPammo.second.DefaultPickupDataReplacement.MinProgress = 30;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> olgaM9AmmoBetweenBoxes;
		olgaM9AmmoBetweenBoxes.first = {"w00b",-11500,12217,-13500};
		olgaM9AmmoBetweenBoxes.second.DefaultPickupDataReplacement.SpawnType = Pickup::NeverSpawn;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> olgaM9AmmoSouth;
		olgaM9AmmoSouth.first = {"w00b",-6600,12217,-19250};
		olgaM9AmmoSouth.second.DefaultPickupDataReplacement.Amount = 5;
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
		engineRoomM9ammo.second.DefaultPickupDataReplacement.SpawnType = Pickup::NeverSpawn;
		//engineRoomM9ammo.second.X = -5500;
		//engineRoomM9ammo.second.Z = -21500;

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
		deck2starboardM9ammo.second.DefaultPickupDataReplacement.Amount = 10;

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

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> hold12fsouthwestM9ammo;
		hold12fsouthwestM9ammo.first = { "w04a",-11000,-8783,-500 };
		hold12fsouthwestM9ammo.second.DefaultPickupDataReplacement.SpawnType = Pickup::NeverSpawn;

		// Plant

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> deepSeaDockEZM9ammo1;
		deepSeaDockEZM9ammo1.first = { "w11a",-8625,-43783,14375 };
		deepSeaDockEZM9ammo1.second.DefaultPickupDataReplacement.SpawnType = Pickup::NeverSpawn;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> deepSeaDockEZM9ammo2;
		deepSeaDockEZM9ammo2.first = { "w11a",-7750,-43783,15125 };
		deepSeaDockEZM9ammo2.second.DefaultPickupDataReplacement.SpawnType = Pickup::NeverSpawn;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> deepSeaDockVEM9;
		deepSeaDockVEM9.first = { "w11a",-18000,-45774,20250 };
		deepSeaDockVEM9.second.DefaultPickupDataReplacement.SpawnType = Pickup::NeverSpawn;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> strutARoofEZM9;
		strutARoofEZM9.first = { "w12a",-4800,5226,-12175 };
		strutARoofEZM9.second.DefaultPickupDataReplacement.SpawnType = Pickup::NeverSpawn;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> strutARoofChaff;
		strutARoofChaff.first = { "w12a",6000,5224,-13000 };
		strutARoofChaff.second.DefaultPickupDataReplacement.MinProgress = 106;
		strutARoofChaff.second.X = -6480;
		strutARoofChaff.second.Z = -12185;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> strutBSocomAmmoNearNode;
		strutBSocomAmmoNearNode.first = { "w14a",-48750,-1783,-24500};
		strutBSocomAmmoNearNode.second.DefaultPickupDataReplacement.MinProgress = 63;
		strutBSocomAmmoNearNode.second.X = -43975;
		strutBSocomAmmoNearNode.second.Z = -29910;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> strutBSocomAmmoInLocker;
		strutBSocomAmmoInLocker.first = {"w14a",-50500,417,-43125};
		strutBSocomAmmoInLocker.second.DefaultPickupDataReplacement.MinProgress = 102;
		strutBSocomAmmoInLocker.second.X = -55515;
		strutBSocomAmmoInLocker.second.Y = 118;
		strutBSocomAmmoInLocker.second.Z = -33449;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> strutBM9Ammo;
		strutBM9Ammo.first = { "w14a",-46625,218,-34875 };
		strutBM9Ammo.second.DefaultPickupDataReplacement.MinProgress = 63;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> bcBridgeChaff;
		bcBridgeChaff.first = {"w15a",-47750,224,-60000};
		bcBridgeChaff.second.DefaultPickupDataReplacement.MinProgress = 106;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> diningHallSocomAmmo;
		diningHallSocomAmmo.first = {"w16a",-60500,217,-85875};
		diningHallSocomAmmo.second.DefaultPickupDataReplacement.MinProgress = 106;
		diningHallSocomAmmo.second.X = -52015;
		diningHallSocomAmmo.second.Z = -78025;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> diningHallM9Ammo;
		diningHallM9Ammo.first = {"w16a",-43750,217,-85875};
		diningHallM9Ammo.second.DefaultPickupDataReplacement.MinProgress = 106;
		diningHallM9Ammo.second.X = -57400;
		diningHallM9Ammo.second.Z = -82300;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> pumpRoomEastStairsM9ammo;
		pumpRoomEastStairsM9ammo.first = {"w12b",6250,5217,-7875};
		pumpRoomEastStairsM9ammo.second.DefaultPickupDataReplacement.SpawnType = Pickup::NeverSpawn;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> pumpRoomEastLockerSocomAmmo;
		pumpRoomEastLockerSocomAmmo.first = {"w12b",-9500,417,-4250};
		pumpRoomEastLockerSocomAmmo.second.DefaultPickupDataReplacement.MinProgress = 155;
		pumpRoomEastLockerSocomAmmo.second.X = -10525;
		pumpRoomEastLockerSocomAmmo.second.Y = 118;
		pumpRoomEastLockerSocomAmmo.second.Z = -2515;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> pumpRoomSouthRoomSocomAmmo;
		pumpRoomSouthRoomSocomAmmo.first = {"w12b",-8500,217,1000};
		pumpRoomSouthRoomSocomAmmo.second.DefaultPickupDataReplacement.MinProgress = 102;
		pumpRoomSouthRoomSocomAmmo.second.X = -9525;
		pumpRoomSouthRoomSocomAmmo.second.Z = 1725;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> pumpRoomUnderDeskM9Ammo;
		pumpRoomUnderDeskM9Ammo.first = {"w12b",0,217,-6250};
		pumpRoomUnderDeskM9Ammo.second.DefaultPickupDataReplacement.MinProgress = 155;
		pumpRoomUnderDeskM9Ammo.second.Z = -2940;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> faBridgeChaff;
		faBridgeChaff.first = { "w23a",-13000,-4276,2000 };
		faBridgeChaff.second.X = 7;
		faBridgeChaff.second.Z = 2626;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> warehouseM9;
		warehouseM9.first = { "w22a",55250,226,-23250 };
		warehouseM9.second.X = 56240;
		warehouseM9.second.Z = -21490;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> warehouseM9roomNorthM9ammo;
		warehouseM9roomNorthM9ammo.first = { "w22a",56500,217,-23750 };
		warehouseM9roomNorthM9ammo.second.X = 55595;
		warehouseM9roomNorthM9ammo.second.Z = -20785;
		warehouseM9roomNorthM9ammo.second.DefaultPickupDataReplacement.MinProgress = 117;
		warehouseM9roomNorthM9ammo.second.DefaultPickupDataReplacement.MaxProgress = 154;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> warehouseM9roomEastM9ammo;
		warehouseM9roomEastM9ammo.first = { "w22a",57500,217,-22250 };
		warehouseM9roomEastM9ammo.second.DefaultPickupDataReplacement.MinProgress = 182;
		//warehouseM9roomEastM9ammo.second.X = 56850;
		//warehouseM9roomEastM9ammo.second.Z = -20785;
		//warehouseM9roomEastM9ammo.second.DefaultPickupDataReplacement.MinProgress = 155;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> warehouseSocomAmmoOutM9RoomVent;
		warehouseSocomAmmoOutM9RoomVent.first = { "w22a",63000,217,-25000 };
		warehouseSocomAmmoOutM9RoomVent.second.DefaultPickupDataReplacement.SpawnType = Pickup::NeverSpawn;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> warehouseStun;
		warehouseStun.first = { "w22a",52500,-4776,-33250 };
		warehouseStun.second.Y = -2970;
		warehouseStun.second.Z = -31985;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> warehouseChaff;
		warehouseChaff.first = { "w22a",51000,-4776,-18625 };
		warehouseChaff.second.X = 46470;
		warehouseChaff.second.Z = -29490;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> warehouseBtmFloorEastBoxesSocomAmmo;
		warehouseBtmFloorEastBoxesSocomAmmo.first = { "w22a",58625,-3783,-28500 };
		warehouseBtmFloorEastBoxesSocomAmmo.second.X = 57520;
		warehouseBtmFloorEastBoxesSocomAmmo.second.Y = -2783;
		warehouseBtmFloorEastBoxesSocomAmmo.second.Z = -28540;
		warehouseBtmFloorEastBoxesSocomAmmo.second.DefaultPickupDataReplacement.MaxProgress = 119;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> warehouseBtmFloorEastBoxesM9Ammo;
		warehouseBtmFloorEastBoxesM9Ammo.first = { "w22a",59000,-2783,-26000 };
		warehouseBtmFloorEastBoxesM9Ammo.second.X = 47530;
		warehouseBtmFloorEastBoxesM9Ammo.second.Y = -2783;
		warehouseBtmFloorEastBoxesM9Ammo.second.Z = -24975;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> warehouseBtmFloorSoutheastRoomM9Ammo;
		warehouseBtmFloorSoutheastRoomM9Ammo.first = { "w22a",57500,-4783,-23250 };
		warehouseBtmFloorSoutheastRoomM9Ammo.second.X = 56953;
		//warehouseBtmFloorSoutheastRoomM9Ammo.second.X = 55526;
		//warehouseBtmFloorSoutheastRoomM9Ammo.second.DefaultPickupDataReplacement.MinProgress = 100;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> warehouseSocomSuppressor;
		warehouseSocomSuppressor.first = { "w22a",57625,-4774,-20375 };
		warehouseSocomSuppressor.second.X = 55526;
		warehouseSocomSuppressor.second.Z = -21708;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> warehouseMineDetector;
		warehouseMineDetector.first = { "w22a",55375,-4547,-23625 };
		warehouseMineDetector.second.X = 57625;
		warehouseMineDetector.second.Z = -20375;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> warehouseBtmFloorSouthwestRoomBook;
		warehouseBtmFloorSouthwestRoomBook.first = { "w22a",45560,-4622,-21500 };
		warehouseBtmFloorSouthwestRoomBook.second.X = 48250;
		warehouseBtmFloorSouthwestRoomBook.second.Z = -23940;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> warehouseBtmFloorSouthwestRoomPentaz;
		warehouseBtmFloorSouthwestRoomPentaz.first = { "w22a",48250,-4622,-23940 };
		warehouseBtmFloorSouthwestRoomPentaz.second.X = 45560;
		warehouseBtmFloorSouthwestRoomPentaz.second.Z = -21500;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> heliportBox3;
		heliportBox3.first = {"w20b",51500,11853,-84400};
		heliportBox3.second.X = 44230;
		heliportBox3.second.Z = -94360;
		heliportBox3.second.DefaultPickupDataReplacement.MaxProgress = 106;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> heliportStun;
		heliportStun.first = { "w20b",47000,11724,-96250 };
		heliportStun.second.X = 64440;
		heliportStun.second.Z = -90675;
		heliportStun.second.DefaultPickupDataReplacement.MaxProgress = 106;
		//heliportStun.second.DefaultPickupDataReplacement.MinProgress = 180;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> parcelRoomBox5;
		parcelRoomBox5.first = { "w20a",57500,1353,-94500 };
		parcelRoomBox5.second.DefaultPickupDataReplacement.MinProgress = 98;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> parcelRoomStun;
		parcelRoomStun.first = {"w20a",51500,224,-92000};
		//parcelRoomStun.second.DefaultPickupDataReplacement.MinProgress = 155;
		parcelRoomStun.second.DefaultPickupDataReplacement.MinProgress = 98;
		parcelRoomStun.second.DefaultPickupDataReplacement.MaxProgress = 186;
		parcelRoomStun.second.X = 50000;
		parcelRoomStun.second.Z = -96485;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> parcelRoomNorthEastSocomAmmo;
		parcelRoomNorthEastSocomAmmo.first = { "w20a",56000,217,-100000 };
		//parcelRoomNorthEastSocomAmmo.second.DefaultPickupDataReplacement.MinProgress = 98;
		parcelRoomNorthEastSocomAmmo.second.DefaultPickupDataReplacement.MinProgress = 155;
		parcelRoomNorthEastSocomAmmo.second.DefaultPickupDataReplacement.MaxProgress = 186;
		parcelRoomNorthEastSocomAmmo.second.X = 51500;
		parcelRoomNorthEastSocomAmmo.second.Z = -92000;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> parcelRoomNorthWestSocomAmmo;
		parcelRoomNorthWestSocomAmmo.first = { "w20a",48000,217,-100000 };
		parcelRoomNorthWestSocomAmmo.second.DefaultPickupDataReplacement.MinProgress = 98;
		parcelRoomNorthWestSocomAmmo.second.DefaultPickupDataReplacement.MaxProgress = 186;
		parcelRoomNorthWestSocomAmmo.second.X = 49995;
		parcelRoomNorthWestSocomAmmo.second.Z = -80530;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> parcelRoomM9Ammo;
		parcelRoomM9Ammo.first = {"w20a",52000,217,-87500};
		//parcelRoomM9Ammo.second.DefaultPickupDataReplacement.MinProgress = 119;
		parcelRoomM9Ammo.second.DefaultPickupDataReplacement.MinProgress = 98;
		parcelRoomM9Ammo.second.DefaultPickupDataReplacement.MaxProgress = 186;
		parcelRoomM9Ammo.second.X = 50475;
		parcelRoomM9Ammo.second.Z = -91025;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> parcelRoomM4Ammo;
		parcelRoomM4Ammo.first = { "w20a",53500,217,-84750 };
		parcelRoomM4Ammo.second.DefaultPickupDataReplacement.MaxProgress = 186;
		parcelRoomM4Ammo.second.X = 51560;
		parcelRoomM4Ammo.second.Z = -94560;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> parcelRoomEmmaCountdownStingerAmmo;
		parcelRoomEmmaCountdownStingerAmmo.first = { "w20a",50000,309,-96500 };
		parcelRoomEmmaCountdownStingerAmmo.second.X = 49995;
		parcelRoomEmmaCountdownStingerAmmo.second.Z = -80530;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> parcelRoomEmmaCountdownSocomAmmo;
		parcelRoomEmmaCountdownSocomAmmo.first = { "w20a",53000,-4283,-93000 };
		parcelRoomEmmaCountdownSocomAmmo.second.DefaultPickupDataReplacement.SpawnType = Pickup::NeverSpawn;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> sedimentPoolM9Ammo;
		sedimentPoolM9Ammo.first = {"w18a",-6500,-3783,-133500};
		sedimentPoolM9Ammo.second.DefaultPickupDataReplacement.MinProgress = 102;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> sedimentPoolSocomAmmo;
		sedimentPoolSocomAmmo.first = {"w18a",-8500,-783,-109500};
		sedimentPoolSocomAmmo.second.DefaultPickupDataReplacement.MinProgress = 102;
		sedimentPoolSocomAmmo.second.X = 1250;
		sedimentPoolSocomAmmo.second.Z = -119250;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> sedimentPoolPSG1Ammo;
		sedimentPoolPSG1Ammo.first = { "w18a",8500,-783,-109500 };
		sedimentPoolPSG1Ammo.second.X = -8500;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> sedimentPoolRation;
		sedimentPoolRation.first = { "w18a",1250,-778,-119250};
		sedimentPoolRation.second.X = 8500;
		sedimentPoolRation.second.Z = -109500;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> deBridgeSocomAmmo;
		deBridgeSocomAmmo.first = { "w19a",12750,-4283,3500};
		deBridgeSocomAmmo.second.Z = 2000;
		deBridgeSocomAmmo.second.DefaultPickupDataReplacement.MinProgress = 106;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> deBridgeStun;
		deBridgeStun.first = { "w19a",13250,-4276,2750};
		deBridgeStun.second.X = -9400;
		deBridgeStun.second.Z = 2400;
		deBridgeStun.second.DefaultPickupDataReplacement.MinProgress = 106;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> deepSeaDockSocomAmmo1;
		deepSeaDockSocomAmmo1.first = {"w11a",500,-44783,-500};
		deepSeaDockSocomAmmo1.second.DefaultPickupDataReplacement.SpawnType = Pickup::NeverSpawn;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> deepSeaDockSocomAmmo2;
		deepSeaDockSocomAmmo2.first = { "w11a",5500,-44780,-1000};
		deepSeaDockSocomAmmo2.second.DefaultPickupDataReplacement.SpawnType = Pickup::NeverSpawn;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> deepSeaDockSocomAmmo3;
		deepSeaDockSocomAmmo3.first = { "w11a",5500,-44779,-1000};
		deepSeaDockSocomAmmo3.second.DefaultPickupDataReplacement.SpawnType = Pickup::NeverSpawn;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> strutAroofM9ammo;
		strutAroofM9ammo.first = {"w12a",-8000,5217,-2500};
		strutAroofM9ammo.second.X = 8525;
		strutAroofM9ammo.second.Z = -3225;
		strutAroofM9ammo.second.DefaultPickupDataReplacement.MinProgress = 117;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> heliportPreFatmanSocomAmmo;
		heliportPreFatmanSocomAmmo.first = { "w20b",59000,11717,-80500 };
		heliportPreFatmanSocomAmmo.second.DefaultPickupDataReplacement.SpawnType = Pickup::NeverSpawn;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> fatmanM9ammo;
		fatmanM9ammo.first = {"w20b",66250,11717,-103500};
		fatmanM9ammo.second.X = 47025;
		fatmanM9ammo.second.Y = 11517;
		fatmanM9ammo.second.Z = -92845;
		fatmanM9ammo.second.DefaultPickupDataReplacement.Amount = 15;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> fatmanSocomAmmo1;
		fatmanSocomAmmo1.first = {"w20b",49375,11717,-101750};
		fatmanSocomAmmo1.second.DefaultPickupDataReplacement.Amount = 12;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> fatmanSocomAmmo2;
		fatmanSocomAmmo2.first = {"w20b",63625,11717,-90750};
		fatmanSocomAmmo2.second.DefaultPickupDataReplacement.Amount = 12;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> fatmanSocomAmmo3;
		fatmanSocomAmmo3.first = {"w20b",46000,11717,-81875};
		fatmanSocomAmmo3.second.DefaultPickupDataReplacement.Amount = 12;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> warehouseAK;
		warehouseAK.first = { "w22a",57125,-4601,-44250 };
		warehouseAK.second.X = 46760;
		warehouseAK.second.Z = -37775;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> warehouseSouthWestAKammoAKroom;
		warehouseSouthWestAKammoAKroom.first = { "w22a",50000,-4783,-36250 };
		warehouseSouthWestAKammoAKroom.second.DefaultPickupDataReplacement.MinProgress = 155;
		warehouseSouthWestAKammoAKroom.second.X = 46760;
		warehouseSouthWestAKammoAKroom.second.Z = -37775;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> warehouseSocomAmmoAKroom;
		warehouseSocomAmmoAKroom.first = { "w22a",51250,-4783,-44375 };
		warehouseSocomAmmoAKroom.second.DefaultPickupDataReplacement.MinProgress = 155;
		warehouseSocomAmmoAKroom.second.X = 47505;
		warehouseSocomAmmoAKroom.second.Z = -37775;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> warehouseWesternmostM4ammoAKroom;
		warehouseWesternmostM4ammoAKroom.first = { "w22a",56750,-4783,-40500 };
		warehouseWesternmostM4ammoAKroom.second.X = 46875;
		warehouseWesternmostM4ammoAKroom.second.Z = -37030;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> warehouseSouthEastAKammoAKroom;
		warehouseSouthEastAKammoAKroom.first = { "w22a",54000,-4783,-36250};
		warehouseSouthEastAKammoAKroom.second.DefaultPickupDataReplacement.SpawnType = Pickup::NeverSpawn;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> warehouseNorthWestAKammoAKroom;
		warehouseNorthWestAKammoAKroom.first = { "w22a",46875,-4783,-44250 };
		warehouseNorthWestAKammoAKroom.second.DefaultPickupDataReplacement.SpawnType = Pickup::NeverSpawn;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> warehouseM4;
		warehouseM4.first = { "w22a",43750,399,-32000 };
		warehouseM4.second.Z = -30110;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> warehouseM4ammoM4roomLocker;
		warehouseM4ammoM4roomLocker.first = { "w22a",41560,417,-28500 };
		warehouseM4ammoM4roomLocker.second.DefaultPickupDataReplacement.SpawnType = Pickup::NeverSpawn;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> warehouseM4ammoWestOfM4;
		warehouseM4ammoWestOfM4.first = { "w22a",42500,217,-31500 };
		warehouseM4ammoWestOfM4.second.DefaultPickupDataReplacement.SpawnType = Pickup::NeverSpawn;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> efBridgeAKammo;
		efBridgeAKammo.first = {"w21a",23000,-1283,-61250};
		efBridgeAKammo.second.DefaultPickupDataReplacement.SpawnType = Pickup::NeverSpawn;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> shell1CoreSocomSuppressor;
		shell1CoreSocomSuppressor.first = {"w24a",2250,426,-45825};
		shell1CoreSocomSuppressor.second.DefaultPickupDataReplacement.MinProgress = 155;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> shell1Core1fChaff;
		shell1Core1fChaff.first = { "w24a",-1000,1224,-52000 };
		shell1Core1fChaff.second.DefaultPickupDataReplacement.MinProgress = 155;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> shell1Core1fM9ammoLocker;
		shell1Core1fM9ammoLocker.first = { "w24a",-3750,417,-51250 };
		shell1Core1fM9ammoLocker.second.DefaultPickupDataReplacement.SpawnType = Pickup::NeverSpawn;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> shell1Core1fM4ammoLocker;
		shell1Core1fM4ammoLocker.first = { "w24a",-3750,417,-53250 };
		shell1Core1fM4ammoLocker.second.X = 250;
		shell1Core1fM4ammoLocker.second.Z = -45825;
		shell1Core1fM4ammoLocker.second.DefaultPickupDataReplacement.MinProgress = 155;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> shell1Core1fSocomAmmoWestEnd;
		shell1Core1fSocomAmmoWestEnd.first = { "w24a",-18500,-1282,-55750 };
		shell1Core1fSocomAmmoWestEnd.second.X = -9500;
		shell1Core1fSocomAmmoWestEnd.second.Y = 30;
		shell1Core1fSocomAmmoWestEnd.second.Z = -56020;
		shell1Core1fSocomAmmoWestEnd.second.DefaultPickupDataReplacement.MinProgress = 155;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> computerRoomEastLockerM9ammo;
		computerRoomEastLockerM9ammo.first = {"w24d",8625,-15583,-53000};
		computerRoomEastLockerM9ammo.second.X = -4770;
		computerRoomEastLockerM9ammo.second.Y = -15683;
		computerRoomEastLockerM9ammo.second.Z = -64300;
		computerRoomEastLockerM9ammo.second.DefaultPickupDataReplacement.MinProgress = 155;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> computerRoomWestLockerSocomAmmo;
		computerRoomWestLockerSocomAmmo.first = {"w24d",-8625,-15583,-50000};
		computerRoomWestLockerSocomAmmo.second.X = -5350;
		computerRoomWestLockerSocomAmmo.second.Y = -15683;
		computerRoomWestLockerSocomAmmo.second.Z = -64300;
		computerRoomWestLockerSocomAmmo.second.DefaultPickupDataReplacement.MinProgress = 155;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> computerRoomAKammoNode;
		computerRoomAKammoNode.first = { "w24d",4500,-15783,-55000 };
		computerRoomAKammoNode.second.DefaultPickupDataReplacement.SpawnType = Pickup::NeverSpawn;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> computerRoomM4ammo;
		computerRoomM4ammo.first = { "w24d",-8625,-15583,-52000 };
		computerRoomM4ammo.second.DefaultPickupDataReplacement.SpawnType = Pickup::NeverSpawn;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> shell1CoreB1Stun;
		shell1CoreB1Stun.first = {"w24b",-10500,-4076,-69100};
		shell1CoreB1Stun.second.X = -4725;
		shell1CoreB1Stun.second.Y = -4176;
		shell1CoreB1Stun.second.Z = -69445;
		shell1CoreB1Stun.second.DefaultPickupDataReplacement.MinProgress = 155;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> shell1CoreB1SocomAmmo;
		shell1CoreB1SocomAmmo.first = { "w24b",-8500,-4083,-69100 };
		shell1CoreB1SocomAmmo.second.X = -10520;
		shell1CoreB1SocomAmmo.second.Y = -4183;
		shell1CoreB1SocomAmmo.second.Z = -73525;
		shell1CoreB1SocomAmmo.second.DefaultPickupDataReplacement.MinProgress = 155;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> shell1CoreB1M4ammo;
		shell1CoreB1M4ammo.first = {"w24b",-11250,-4281,-64000};
		shell1CoreB1M4ammo.second.DefaultPickupDataReplacement.SpawnType = Pickup::NeverSpawn;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> shell1Core1fSocomAmmoAfterAmes;
		shell1Core1fSocomAmmoAfterAmes.first = {"w24a",17250,-1282,-55750};
		shell1Core1fSocomAmmoAfterAmes.second.DefaultPickupDataReplacement.SpawnType = Pickup::NeverSpawn;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> shell1Core1fM9ammoAfterAmes;
		shell1Core1fM9ammoAfterAmes.first = {"w24a",15750,-1282,-55750};
		shell1Core1fM9ammoAfterAmes.second.X = 11276;
		shell1Core1fM9ammoAfterAmes.second.Y = 30;
		shell1Core1fM9ammoAfterAmes.second.Z = -50515;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> warehousePSG1;
		warehousePSG1.first = {"w22a",60000,399,-32000};
		warehousePSG1.second.X = 60250;
		warehousePSG1.second.Z = -29995;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> warehousePSG1T;
		warehousePSG1T.first = { "w22a",66750,399,-31000 };
		warehousePSG1T.second.X = 63705;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> warehousePSG1ammoPSG1roomLocker;
		warehousePSG1ammoPSG1roomLocker.first = { "w22a",58060,417,-31250 };
		warehousePSG1ammoPSG1roomLocker.second.DefaultPickupDataReplacement.SpawnType = Pickup::NeverSpawn;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> warehousePSG1ammoPSG1roomOutsideLocker;
		warehousePSG1ammoPSG1roomOutsideLocker.first = { "w22a",58250,217,-30250 };
		warehousePSG1ammoPSG1roomOutsideLocker.second.DefaultPickupDataReplacement.SpawnType = Pickup::NeverSpawn;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> warehouseM4ammoAfterAmes1;
		warehouseM4ammoAfterAmes1.first = { "w22a",45500,217,-28250 };
		warehouseM4ammoAfterAmes1.second.DefaultPickupDataReplacement.SpawnType = Pickup::NeverSpawn;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> warehouseM4ammoAfterAmes2;
		warehouseM4ammoAfterAmes2.first = { "w22a",44500,217,-31000 };
		warehouseM4ammoAfterAmes2.second.DefaultPickupDataReplacement.SpawnType = Pickup::NeverSpawn;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> warehousePSG1ammoM4room;
		warehousePSG1ammoM4room.first = { "w22a",38250,217,-31000 };
		warehousePSG1ammoM4room.second.DefaultPickupDataReplacement.SpawnType = Pickup::NeverSpawn;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> warehouseRGB6;
		warehouseRGB6.first = { "w22a",45875,453,-39375 };
		warehouseRGB6.second.X = 48050;
		warehouseRGB6.second.Z = -38275;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> warehouseRGB6ammoRGB6roomNE;
		warehouseRGB6ammoRGB6roomNE.first = { "w22a",49750,309,-40250 };
		warehouseRGB6ammoRGB6roomNE.second.DefaultPickupDataReplacement.SpawnType = Pickup::NeverSpawn;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> warehouseRGB6ammoRGB6roomBox;
		warehouseRGB6ammoRGB6roomBox.first = { "w22a",47000,1309,-38250 };
		warehouseRGB6ammoRGB6roomBox.second.DefaultPickupDataReplacement.SpawnType = Pickup::NeverSpawn;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> warehousePSG1TammoRGB6roomLocker;
		warehousePSG1TammoRGB6roomLocker.first = {"w22a",49940,417,-37250};
		warehousePSG1TammoRGB6roomLocker.second.Z = -38300;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> warehouseSocomAmmoGrenadeRoom;
		warehouseSocomAmmoGrenadeRoom.first = { "w22a",49500,217,-19750 };
		warehouseSocomAmmoGrenadeRoom.second.DefaultPickupDataReplacement.SpawnType = Pickup::NeverSpawn;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> shell12bridgePSG1ammo;
		shell12bridgePSG1ammo.first = {"w25a",3000,717,-137000};
		shell12bridgePSG1ammo.second.DefaultPickupDataReplacement.Amount = 1;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> shell12bridgeSocomAmmo;
		shell12bridgeSocomAmmo.first = { "w25a",-1500,717,-137000 };
		shell12bridgeSocomAmmo.second.DefaultPickupDataReplacement.Amount = 1;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> harrierStingerAmmo;
		harrierStingerAmmo.first = { "w25a",-2125,-691,-145000 };
		harrierStingerAmmo.second.DefaultPickupDataReplacement.Amount = 10;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> postHarrierDownstairPSG1ammo;
		postHarrierDownstairPSG1ammo.first = {"w25b",-1500,-5564,-156000};
		postHarrierDownstairPSG1ammo.second.DefaultPickupDataReplacement.SpawnType = Pickup::NeverSpawn;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> postHarrierUpstairPSG1ammo;
		postHarrierUpstairPSG1ammo.first = { "w25b",499,-3472,-163597 };
		postHarrierUpstairPSG1ammo.second.DefaultPickupDataReplacement.SpawnType = Pickup::NeverSpawn;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> klBridgeDayAKammo;
		klBridgeDayAKammo.first = { "w25c",59000,-2782,-226500};
		klBridgeDayAKammo.second.DefaultPickupDataReplacement.SpawnType = Pickup::NeverSpawn;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> klBridgeDaySocomAmmo;
		klBridgeDaySocomAmmo.first = { "w25c",53250,217,-227000 };
		klBridgeDaySocomAmmo.second.DefaultPickupDataReplacement.SpawnType = Pickup::NeverSpawn;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> klBridgeDayChaff;
		klBridgeDayChaff.first = { "w25c",53500,224,-253000 };
		klBridgeDayChaff.second.DefaultPickupDataReplacement.SpawnType = Pickup::NeverSpawn;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> klBridgeDayPSG1ammo;
		klBridgeDayPSG1ammo.first = { "w25c",50500,217,-253000 };
		klBridgeDayPSG1ammo.second.DefaultPickupDataReplacement.SpawnType = Pickup::NeverSpawn;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> shell2core1fPrePrezSocomAmmo;
		shell2core1fPrePrezSocomAmmo.first = { "w31a",3000,-783,-228500};
		shell2core1fPrePrezSocomAmmo.second.DefaultPickupDataReplacement.SpawnType = Pickup::NeverSpawn;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> shell2core1fPrePrezM9Ammo;
		shell2core1fPrePrezM9Ammo.first = { "w31a",-6500,-1783,-228500 };
		shell2core1fPrePrezM9Ammo.second.DefaultPickupDataReplacement.SpawnType = Pickup::NeverSpawn;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> shell2core1fPrePrezChaff;
		shell2core1fPrePrezChaff.first = { "w31a",-18000,225,-236000 };
		shell2core1fPrePrezChaff.second.DefaultPickupDataReplacement.SpawnType = Pickup::NeverSpawn;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> shell2core1fPrePrezM4;
		shell2core1fPrePrezM4.first = { "w31a",-18000,400,-234000 };
		shell2core1fPrePrezM4.second.DefaultPickupDataReplacement.SpawnType = Pickup::NeverSpawn;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> shell2core1fPrePrezM4ammoNearNode;
		shell2core1fPrePrezM4ammoNearNode.first = { "w31a",-4500,217,-243750 };
		shell2core1fPrePrezM4ammoNearNode.second.DefaultPickupDataReplacement.SpawnType = Pickup::NeverSpawn;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> shell2core1fPrePrezM4ammoNorthEast;
		shell2core1fPrePrezM4ammoNorthEast.first = { "w31a",8500,218,-252000 };
		shell2core1fPrePrezM4ammoNorthEast.second.DefaultPickupDataReplacement.SpawnType = Pickup::NeverSpawn;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> shell2core1fPrePrezRGB6;
		shell2core1fPrePrezRGB6.first = { "w31a",6500,454,-252000 };
		shell2core1fPrePrezRGB6.second.DefaultPickupDataReplacement.SpawnType = Pickup::NeverSpawn;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> shell2core1fPrePrezNikitaAmmoEastStairs;
		shell2core1fPrePrezNikitaAmmoEastStairs.first = { "w31a",6500,-1691,-228500 };
		shell2core1fPrePrezNikitaAmmoEastStairs.second.DefaultPickupDataReplacement.SpawnType = Pickup::NeverSpawn;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> shell2core1fPreEmmaPrezRoomSocomAmmo;
		shell2core1fPreEmmaPrezRoomSocomAmmo.first = { "w31a",625,217,-246250 };
		shell2core1fPreEmmaPrezRoomSocomAmmo.second.DefaultPickupDataReplacement.SpawnType = Pickup::NeverSpawn;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> shell2core1fPreEmmaPrezRoomStingerAmmo;
		shell2core1fPreEmmaPrezRoomStingerAmmo.first = { "w31a",-2000,559,-238250 };
		shell2core1fPreEmmaPrezRoomStingerAmmo.second.DefaultPickupDataReplacement.SpawnType = Pickup::NeverSpawn;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> shell2core1fPreEmmaPrezRoomBook;
		shell2core1fPreEmmaPrezRoomBook.first = { "w31a",-3750,428,-237750 };
		shell2core1fPreEmmaPrezRoomBook.second.X = -525;
		shell2core1fPreEmmaPrezRoomBook.second.Z = -238245;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> shell2core1fPostPrezRGB6ammo;
		shell2core1fPostPrezRGB6ammo.first = { "w31a",-3750,559,-237750 };
		shell2core1fPostPrezRGB6ammo.second.X = -5500;
		shell2core1fPostPrezRGB6ammo.second.Y = 1309;
		shell2core1fPostPrezRGB6ammo.second.Z = -231750;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> swimming2minesM4ammo;
		swimming2minesM4ammo.first = { "w31b",9500,-10283,-237500};
		swimming2minesM4ammo.second.DefaultPickupDataReplacement.MinProgress = 298;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> swimmingPSG1t;
		swimmingPSG1t.first = { "w31b",-2000,-10101,-223000 };
		swimmingPSG1t.second.X = 11480;
		swimmingPSG1t.second.Y = -12476;
		swimmingPSG1t.second.Z = -235985;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> swimmingRGB6;
		swimmingRGB6.first = { "w31b",-1500,-10047,-249500 };
		swimmingRGB6.second.X = 530;
		swimmingRGB6.second.Y = -11876;
		swimmingRGB6.second.Z = -236495;
		swimmingRGB6.second.DefaultPickupDataReplacement.Amount = 0;
		swimmingRGB6.second.DefaultPickupDataReplacement.MinProgress = 257;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> swimmingStingerammo;
		swimmingStingerammo.first = { "w31b",-2000,-10191,-223000 };
		swimmingStingerammo.second.DefaultPickupDataReplacement.SpawnType = Pickup::NeverSpawn;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> swimmingEmmaElevatorM4ammo;
		swimmingEmmaElevatorM4ammo.first = { "w31b",-2750,-3783,-250750 };
		swimmingEmmaElevatorM4ammo.second.DefaultPickupDataReplacement.SpawnType = Pickup::NeverSpawn;

		// remember area code w31a here (not w31d)
		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> shell2core1fEmmaSocomAmmoNorthWest;
		shell2core1fEmmaSocomAmmoNorthWest.first = { "w31a",-11500,218,-252000};
		shell2core1fEmmaSocomAmmoNorthWest.second.DefaultPickupDataReplacement.MinProgress = 298;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> shell2core1fEmmaSocomAmmoWest;
		shell2core1fEmmaSocomAmmoWest.first = { "w31a",-18000,218,-234000 };
		shell2core1fEmmaSocomAmmoWest.second.DefaultPickupDataReplacement.MinProgress = 298;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> shell2core1fEmmaM9ammo;
		shell2core1fEmmaM9ammo.first = { "w31a",-2000,217,-245500 };
		shell2core1fEmmaM9ammo.second.DefaultPickupDataReplacement.MinProgress = 298;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> shell2core1fEmmaPSG1ammo;
		shell2core1fEmmaPSG1ammo.first = { "w31a",4750,217,-244500 };
		shell2core1fEmmaPSG1ammo.second.DefaultPickupDataReplacement.MinProgress = 298;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> shell2core1fEmmaAKammo;
		shell2core1fEmmaAKammo.first = { "w31a",7500,217,-243500 };
		shell2core1fEmmaAKammo.second.DefaultPickupDataReplacement.MinProgress = 298;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> shell2core1fEmmaPSG1Tammo;
		shell2core1fEmmaPSG1Tammo.first = { "w31a",-2000,467,-238250 };
		shell2core1fEmmaPSG1Tammo.second.DefaultPickupDataReplacement.MinProgress = 298;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> vampSocomAmmo;
		vampSocomAmmo.first = { "w31c",-14000,-6033,-235000};
		vampSocomAmmo.second.DefaultPickupDataReplacement.Amount = 12;
		vampSocomAmmo.second.DefaultPickupDataReplacement.MaxProgress = 253;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> vampM9Ammo;
		vampM9Ammo.first = { "w31c",-14000,-6033,-243250 };
		vampM9Ammo.second.DefaultPickupDataReplacement.Amount = 15;
		vampM9Ammo.second.DefaultPickupDataReplacement.MaxProgress = 253;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> vampSouthRifleAmmo;
		vampSouthRifleAmmo.first = { "w31c",-6000,-6033,-241500 };
		vampSouthRifleAmmo.second.DefaultPickupDataReplacement.Amount = 30;
		vampSouthRifleAmmo.second.DefaultPickupDataReplacement.MaxProgress = 253;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> vampNorthRifleAmmo;
		vampNorthRifleAmmo.first = { "w31c",-6000,-6033,-242500 };
		vampNorthRifleAmmo.second.DefaultPickupDataReplacement.Amount = 30;
		vampNorthRifleAmmo.second.DefaultPickupDataReplacement.MaxProgress = 253;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> vampRGB6Ammo;
		vampRGB6Ammo.first = { "w31c",-4750,-5941,-244500 };
		vampRGB6Ammo.second.DefaultPickupDataReplacement.Amount = 6;
		vampRGB6Ammo.second.DefaultPickupDataReplacement.MaxProgress = 253;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> postVampSwimAKammo;
		postVampSwimAKammo.first = { "w31c",-18000,-10533,-252500 };
		postVampSwimAKammo.second.DefaultPickupDataReplacement.SpawnType = Pickup::NeverSpawn;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> postVampSwimPSG1Tammo;
		postVampSwimPSG1Tammo.first = { "w31c",-6250,-10533,-246750 };
		postVampSwimPSG1Tammo.second.DefaultPickupDataReplacement.SpawnType = Pickup::NeverSpawn;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> klBridgeEmmaChaff;
		klBridgeEmmaChaff.first = { "w25c",23000,-1276,-238500};
		klBridgeEmmaChaff.second.DefaultPickupDataReplacement.MinProgress = 298;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> klBridgeEmmaPSG1ammo;
		klBridgeEmmaPSG1ammo.first = { "w25c",23000,-1283,-241500 };
		klBridgeEmmaPSG1ammo.second.DefaultPickupDataReplacement.MinProgress = 298;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> klBridgeEmmaSocomAmmo;
		klBridgeEmmaSocomAmmo.first = { "w25c",59000,-2780,-253000 };
		klBridgeEmmaSocomAmmo.second.X = 58750;
		klBridgeEmmaSocomAmmo.second.Z = -243990;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> klBridgeEmmaM4Ammo;
		klBridgeEmmaM4Ammo.first = { "w25c",58750,-2780,-236000 };
		klBridgeEmmaM4Ammo.second.Z = -247985;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> sewageTreatmentSocomAmmo;
		sewageTreatmentSocomAmmo.first = { "w28a",56750,218,-222250};
		sewageTreatmentSocomAmmo.second.DefaultPickupDataReplacement.SpawnType = Pickup::NeverSpawn;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> emmaSnipingPSG1ammo;
		emmaSnipingPSG1ammo.first = { "w32a",54500,-38283,-204000};
		emmaSnipingPSG1ammo.second.DefaultPickupDataReplacement.Amount = 20;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> emmaSnipingPSG1Tammo;
		emmaSnipingPSG1Tammo.first = { "w32a",57500,-38283,-208000 };
		emmaSnipingPSG1Tammo.second.DefaultPickupDataReplacement.Amount = 5;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> efBridgeEmmaBleedChaff;
		efBridgeEmmaBleedChaff.first = { "w21a",58250,224,-45250};
		efBridgeEmmaBleedChaff.second.X = 54725;
		efBridgeEmmaBleedChaff.second.Z = -55000;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> rayArenaStingerAmmo;
		rayArenaStingerAmmo.first = { "w46a",8000,4309,-12000};
		rayArenaStingerAmmo.second.DefaultPickupDataReplacement.Amount = 20;
		rayArenaStingerAmmo.second.X = 0;
		rayArenaStingerAmmo.second.Z = 0;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> rayArenaStingerAmmo2;
		rayArenaStingerAmmo2.first = { "w46a",-8000,4309,12000 };
		rayArenaStingerAmmo2.second.DefaultPickupDataReplacement.SpawnType = Pickup::NeverSpawn;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> rayArenaStingerAmmo3;
		rayArenaStingerAmmo3.first = { "w46a",8000,4309,12000 };
		rayArenaStingerAmmo3.second.DefaultPickupDataReplacement.SpawnType = Pickup::NeverSpawn;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> rayArenaStingerAmmo4;
		rayArenaStingerAmmo4.first = { "w46a",14000,4309,0 };
		rayArenaStingerAmmo4.second.DefaultPickupDataReplacement.SpawnType = Pickup::NeverSpawn;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> rayArenaStingerAmmo5;
		rayArenaStingerAmmo5.first = { "w46a",-14000,4309,0 };
		rayArenaStingerAmmo5.second.DefaultPickupDataReplacement.SpawnType = Pickup::NeverSpawn;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> rayArenaStingerAmmo6;
		rayArenaStingerAmmo6.first = { "w46a",-8000,4309,-12000 };
		rayArenaStingerAmmo6.second.DefaultPickupDataReplacement.SpawnType = Pickup::NeverSpawn;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> rayArenaRation;
		rayArenaRation.first = { "w46a",0,4221,0 };
		rayArenaRation.second.DefaultPickupDataReplacement.SpawnType = Pickup::NeverSpawn;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> solidusRation;
		solidusRation.first = { "w61a",1000,18001,21000};
		solidusRation.second.DefaultPickupDataReplacement.SpawnType = Pickup::NeverSpawn;

		std::pair<Pickup::PickupInstanceIdentifiers, Pickup::PickupInstanceData> solidusRation2;
		solidusRation2.first = { "w61a",12250,15526,3500 };
		solidusRation2.second.DefaultPickupDataReplacement.SpawnType = Pickup::NeverSpawn;

		Pickup::pickup_instance_ids_to_data_map pickupInstanceIdentifiersToDataMap = {
			// Tanker
			tankerStartChaff,
			deckAM9Ammo,
			crewLoungeM9ammo,crewLoungeUSPammo,
			deckBM9ammo,deckBUSPammo,
			deckCChaff,
			deckDM9ammoPantry,deckDbox1,deckDUSPammoCamera,deckDUSPammoTable,deckDM9ammoTable,deckEUSPammo,
			olgaM9AmmoBetweenBoxes,olgaM9AmmoSouth,olgaRationVE,
			navDeckRationNearOlga,navDeckUSPSuppressor,navDeckWetBox,
			vulcanRavenRoomUSPammo,engineRoomM9ammo,engineRoomBtmFloorUSPammo,engineRoomMidFloorUSPammo,engineRoomLasersWestUSPammo,engineRoomLasersEastUSPammo,
			deck2MarineUSPammo,deck2junctionUSPammo,deck2horizontalHallUSPammo,deck2nearStarboardUSPammo,deck2starboardM9ammo,deck2starboardPipesUSPammo,
			deck2starboardNorthUSPammo,
			guardRushNorthUSPammo,guardRushSouthUSPammo,guardRushRation,
			hold1entranceM9ammo,hold12fsouthwestM9ammo,
			// Plant
			deepSeaDockEZM9ammo1,deepSeaDockEZM9ammo2,deepSeaDockVEM9,
			strutARoofEZM9,strutARoofChaff,strutAroofM9ammo,
			strutBSocomAmmoNearNode,strutBSocomAmmoInLocker,strutBM9Ammo,
			bcBridgeChaff,
			diningHallSocomAmmo,diningHallM9Ammo,
			pumpRoomEastStairsM9ammo,pumpRoomEastLockerSocomAmmo,pumpRoomSouthRoomSocomAmmo,pumpRoomUnderDeskM9Ammo,
			faBridgeChaff,
			warehouseM9,warehouseM9roomEastM9ammo,warehouseM9roomNorthM9ammo,
			warehouseSocomAmmoOutM9RoomVent,warehouseStun,warehouseChaff,warehouseBtmFloorEastBoxesSocomAmmo,
			warehouseBtmFloorEastBoxesM9Ammo,warehouseBtmFloorSoutheastRoomM9Ammo,warehouseSocomSuppressor,warehouseMineDetector,
			warehouseBtmFloorSouthwestRoomBook, warehouseBtmFloorSouthwestRoomPentaz,
			heliportBox3,heliportStun,
			parcelRoomBox5,parcelRoomStun,parcelRoomNorthEastSocomAmmo,parcelRoomNorthWestSocomAmmo,parcelRoomM9Ammo,parcelRoomM4Ammo,
			sedimentPoolM9Ammo,sedimentPoolSocomAmmo,sedimentPoolRation,sedimentPoolPSG1Ammo,
			deBridgeSocomAmmo,deBridgeStun,
			deepSeaDockSocomAmmo1,deepSeaDockSocomAmmo2,deepSeaDockSocomAmmo3,
			heliportPreFatmanSocomAmmo,
			fatmanM9ammo,fatmanSocomAmmo1,fatmanSocomAmmo2,fatmanSocomAmmo3,
			warehouseAK, warehouseSouthWestAKammoAKroom, warehouseSocomAmmoAKroom, warehouseWesternmostM4ammoAKroom,
			warehouseSouthEastAKammoAKroom, warehouseNorthWestAKammoAKroom,
			warehouseM4,warehouseM4ammoM4roomLocker,warehouseM4ammoWestOfM4,
			efBridgeAKammo,
			shell1CoreSocomSuppressor,shell1Core1fChaff,shell1Core1fM9ammoLocker,shell1Core1fM4ammoLocker,
			shell1Core1fSocomAmmoWestEnd, 
			computerRoomEastLockerM9ammo,computerRoomWestLockerSocomAmmo,computerRoomAKammoNode,computerRoomM4ammo,
			shell1CoreB1Stun,shell1CoreB1SocomAmmo,shell1CoreB1M4ammo,
			shell1Core1fSocomAmmoAfterAmes,shell1Core1fM9ammoAfterAmes,
			warehousePSG1,warehousePSG1T,warehousePSG1ammoPSG1roomLocker,warehousePSG1ammoPSG1roomOutsideLocker,
			warehouseM4ammoAfterAmes1,warehouseM4ammoAfterAmes2,warehousePSG1ammoM4room,
			warehouseRGB6,warehouseRGB6ammoRGB6roomNE,warehouseRGB6ammoRGB6roomBox,warehousePSG1TammoRGB6roomLocker,
			warehouseSocomAmmoGrenadeRoom,
			shell12bridgePSG1ammo,shell12bridgeSocomAmmo,
			harrierStingerAmmo,
			postHarrierDownstairPSG1ammo, postHarrierUpstairPSG1ammo,
			klBridgeDayAKammo, klBridgeDaySocomAmmo, klBridgeDayChaff, klBridgeDayPSG1ammo,
			shell2core1fPrePrezSocomAmmo, shell2core1fPrePrezM9Ammo, shell2core1fPrePrezChaff, shell2core1fPrePrezM4,shell2core1fPrePrezM4ammoNearNode,
			shell2core1fPrePrezM4ammoNorthEast, shell2core1fPrePrezRGB6, shell2core1fPrePrezNikitaAmmoEastStairs,shell2core1fPreEmmaPrezRoomSocomAmmo,
			shell2core1fPreEmmaPrezRoomStingerAmmo,shell2core1fPreEmmaPrezRoomBook,shell2core1fPostPrezRGB6ammo,
			swimming2minesM4ammo, swimmingPSG1t, swimmingRGB6, swimmingStingerammo, swimmingEmmaElevatorM4ammo,
			shell2core1fEmmaSocomAmmoNorthWest, shell2core1fEmmaSocomAmmoWest, shell2core1fEmmaM9ammo,shell2core1fEmmaAKammo,
			shell2core1fEmmaPSG1Tammo, shell2core1fEmmaPSG1ammo,
			vampSocomAmmo, vampM9Ammo, vampSouthRifleAmmo, vampNorthRifleAmmo, vampRGB6Ammo,
			postVampSwimAKammo, postVampSwimPSG1Tammo,
			klBridgeEmmaChaff, klBridgeEmmaPSG1ammo,klBridgeEmmaSocomAmmo,klBridgeEmmaM4Ammo,
			sewageTreatmentSocomAmmo,
			emmaSnipingPSG1ammo, emmaSnipingPSG1Tammo,
			parcelRoomEmmaCountdownSocomAmmo,parcelRoomEmmaCountdownStingerAmmo,
			efBridgeEmmaBleedChaff,
			rayArenaStingerAmmo, rayArenaStingerAmmo2, rayArenaStingerAmmo3, rayArenaStingerAmmo4, rayArenaStingerAmmo5,
			rayArenaStingerAmmo6,rayArenaRation,
			solidusRation, solidusRation2
		};

		Pickup::Run(defaultPickupIdentifiersToDataMap, pickupInstanceIdentifiersToDataMap, 1, true);

		// Hook function to display text on title screen
		oFUN_00744a40 = (tFUN_Void_Int)mem::TrampHook32((BYTE*)0x744A40, (BYTE*)hkFUN_00744a40, 6);
	}
}
