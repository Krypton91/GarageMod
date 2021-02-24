////////////////////////////////////////////////////////////////////
//DeRap: Depositary_System\config.bin
//Produced from mikero's Dos Tools Dll version 7.64
//https://mikero.bytex.digital/Downloads
//'now' is Wed Feb 24 16:31:57 2021 : 'file' last modified on Tue Jan 19 14:07:30 2021
////////////////////////////////////////////////////////////////////

#define _ARMA_

class CfgPatches
{
	class Depositary_System
	{
		units[] = {};
		weapons[] = {};
		requiredVersion = 0.1;
		requiredAddons[] = {"DZ_Data"};
	};
};
class CfgMods
{
	class Depositary_System
	{
		dir = "Depositary_System";
		picture = "";
		action = "";
		hideName = 1;
		hidePicture = 1;
		name = "Depositary System";
		credits = "DeutscheBohrmaschine.de";
		author = "Krypton91";
		authorID = "0";
		version = "1.0";
		extra = 0;
		type = "mod";
		dependencies[] = {"Game","World","Mission"};
		class defs
		{
			class imageSets
			{
				files[] = {"Depositary_System/gui/Images/Overlay.imageset"};
			};
			class gameScriptModule
			{
				value = "";
				files[] = {"Depositary_System/scripts/Defines","Depositary_System/scripts/3_Game"};
			};
			class worldScriptModule
			{
				value = "";
				files[] = {"Depositary_System/scripts/Defines","Depositary_System/scripts/4_World"};
			};
			class missionScriptModule
			{
				value = "";
				files[] = {"Depositary_System/scripts/Defines","Depositary_System/scripts/5_Mission"};
			};
		};
	};
};
class CfgVehicles
{
	class HouseNoDestruct;
	class GarageMod_ParkingArea1: HouseNoDestruct
	{
		scope = 1;
		model = "\Depositary_System\models\ParkingArea1.p3d";
	};
	class GarageMod_ParkingPositionHolo: HouseNoDestruct
	{
		scope = 1;
		model = "\Depositary_System\models\P_Arrow.p3d";
	};
	class GarageMod_ParkingMachine: HouseNoDestruct
	{
		scope = 1;
		model = "\Depositary_System\models\ParkingMachine.p3d";
	};
};
