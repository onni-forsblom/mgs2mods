#pragma once
#include "MGS2.framework.h"

namespace MGS2::InventoryData {
	static std::map<std::string, short> WeaponMap{
		{ "no", 0 }, { "none", 0 }, { "off", 0 }, { "unequip", 0 }, { "0", 0 },
		{ "m9", 1 }, { "1", 1 },
		{ "usp", 2 }, { "2", 2 },
		{ "socom", 3 }, { "3", 3 },
		{ "psg1", 4 }, { "psg-1", 4 }, { "4", 4 },
		{ "rgb6", 5 }, { "5", 5 },
		{ "nikita", 6 }, { "6", 6 },
		{ "stinger", 7 }, { "7", 7 },
		{ "claymore", 8 }, { "mine", 8 }, { "claymore mine", 8 }, { "8", 8 },
		{ "c4", 9 }, { "bomb", 9 }, { "c4 bomb", 9 }, { "9", 9 },
		{ "chaff", 10 }, { "chaff g", 10 }, { "chaff.g", 10 }, { "chaff-g", 10 }, {"chaff grenade", 10 }, {"10", 10 },
		{ "stun", 11 }, { "stun g", 11 }, { "stun.g", 11 }, { "stun-g", 11 }, {"stun grenade", 11 }, {"11", 11 },
		{ "mic", 12 }, { "d.mic", 12 }, { "d-mic", 12 }, { "d mic", 12 }, { "microphone", 12 }, { "12", 12 },
		{ "blade", 13 }, { "sword", 13 }, { "hf-blade", 13 }, { "hf.blade", 13 }, { "hf blade", 13 }, { "13", 13 },
		{ "coolant", 14 }, { "14", 14 },
		{ "ak74u", 15 }, { "ak", 15 }, { "ak-74u", 15 }, { "ak74-u", 15 }, { "aks-74u", 15 }, { "aks74-u", 15 }, { "15", 15 },
		{ "magazine", 16 }, { "16", 16 },
		{ "grenade", 17 }, { "17", 17 },
		{ "m4", 18 }, { "18", 18 },
		{ "psg1t", 19 }, { "psg1-t", 19 }, { "19", 19 },
		{ "20", 20 }, // fake d-mic
		{ "book", 21 }, { "21", 21 }
	};
	static std::map<std::string, short> ItemMap{
		{ "no", 0 }, { "none", 0 }, { "off", 0 }, { "unequip", 0 }, { "0", 0 },
		{ "ration", 1 }, { "1", 1 },
		{ "2", 2 }, // fake scope
		{ "medicine", 3 }, { "meds", 3 }, { "3", 3 },
		{ "bandage", 4 }, { "4", 4 },
		{ "pentazemin", 5 }, { "pentaz", 5 }, { "diazepam", 5 }, { "5", 5 },
		{ "bdu", 6 }, { "battle dress", 6 }, { "camo", 6 }, { "6", 6 },
		{ "armor", 7 }, { "b.armor", 7 }, { "body armor", 7 }, { "armour", 7 }, { "b.armour", 7 }, { "body armour", 7 }, { "7", 7 },
		{ "stealth", 8 }, { "stealth camo", 8 }, { "stealth camouflage", 8 }, { "8", 8 },
		{ "mine.d", 9 }, { "mine d", 9 }, { "mine detector", 9 }, { "9", 9 },
		{ "sensor.a", 10 }, { "sensor a", 10 }, { "10", 10 },
		{ "sensor.b", 11 }, { "sensor b", 11 }, { "11", 11 },
		{ "nvg", 12 }, { "night goggles", 12 }, { "night vision goggles", 12 }, { "night vision", 12 }, { "12", 12 },
		{ "therm.g", 13 }, { "therm g", 13 }, { "thermals", 13 }, { "thermal goggles", 13 }, { "13", 13 },
		{ "scope", 14 }, { "binoculars", 14 }, { "14", 14 },
		{ "d.camera", 15 }, { "d camera", 15 }, { "d cam", 15 }, { "d.cam", 15 }, { "digital cam", 15 }, { "digital camera", 15 }, { "15", 15 },
		{ "box 1", 16 }, { "cardboard box 1", 16 }, { "16", 16 },
		{ "cigs", 17 }, { "cigarettes", 17 }, { "smokes", 17 }, { "17", 17 },
		{ "card", 18 }, { "id card", 18 }, { "18", 18 },
		{ "shaver", 19 }, { "19", 19 },
		{ "phone", 20 }, { "mobile", 20 }, { "cell", 20 }, { "cellphone", 20 }, { "cell phone", 20 }, { "mobile phone", 20 }, { "20", 20 },
		{ "camera", 21 }, { "21", 21 },
		{ "box 2", 22 }, { "cardboard box 2", 22 }, { "22", 22 },
		{ "box 3", 23 }, { "cardboard box 3", 23 }, { "23", 23 },
		{ "wet box", 24 }, { "24", 24 },
		{ "ap.sensor", 25 }, { "ap sensor", 25 }, { "25", 25 },
		{ "box 4", 26 }, { "cardboard box 4", 26 }, { "26", 26 },
		{ "box 5", 27 }, { "cardboard box 5", 27 }, { "27", 27 },
		{ "28", 28 }, // unknown item
		{ "socom.supp", 29 }, { "socom supp", 29 }, { "socom suppressor", 29 }, { "29", 29 },
		{ "ak.supp", 30 }, { "ak supp", 30 }, { "ak suppressor", 30 }, { "30", 30 },
		{ "31", 31 }, // fake camera
		{ "bandana", 32 }, { "32", 32 },
		{ "dogtags", 33 }, { "dog tags", 33 }, { "33", 33 },
		{ "mo disc", 34 }, { "mo.disc", 34 }, { "34", 34 },
		{ "usp.supp", 35 }, { "usp supp", 35 }, { "usp suppressor", 35 }, { "35", 35 },
		{ "infinity wig", 36 }, { "ammo wig", 36 }, { "inf wig", 36 }, { "inf.wig", 36 }, { "36", 36 },
		{ "blue wig", 37 }, { "o2 wig", 37 }, { "37", 37 },
		{ "orange wig", 38 }, { "grip wig", 38 }, { "38", 38 },
		{ "wig c", 39 }, { "39", 39 },
		{ "wig d", 40 }, { "40", 40 }
	};
}