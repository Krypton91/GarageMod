class CfgPatches
{
	class Depositary_System
	{
		units[] = {};
		weapons[] = {};
		requiredVersion = 0.1;
		requiredAddons[] = 
		{
			"DZ_Data",
			"JM_CF_Scripts"
		};
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
		version = "1.11";
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
	class Inventory_Base;
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

	

	class GarageMod_ParkingMachineKit: Inventory_Base
	{
		scope=2;
		displayName="Parking Machine Kit";
		descriptionShort="This kit is to make a Garage in your Base!";
		model="\Depositary_System\models\ParkingMachineKit\GarageMod_ParkingMachineKit.p3d";
		allowOwnedCargoManipulation=1;
		hiddenSelections[]=
		{
			"placing",
			"inventory"
		};
		hiddenSelectionsTextures[]=
		{
            "Depositary_System\models\ParkingMachineKit\data\ParkingMachine_co.paa",
			"Depositary_System\models\ParkingMachineKit\data\ParkingMachine_co.paa"
		};
		hologramMaterial="fishnettrap";
		hologramMaterialPath="dz\gear\traps\data";
		overrideDrawArea = 8.0;
		forceFarBubble = "true";
		destroyOnEmpty = 0;
		carveNavmesh = 1;
		heavyItem = 1;
		weight = 10000;
		itemSize[] = {3, 6};
		itemBehaviour = 0;
		physLayer = "item_large";
		lootTag[] = {"Work"};
		class AnimationSources
		{
			class inventory
			{
				source="user";
				animPeriod=0.0099999998;
				initPhase=0;
			};
			class placing
			{
				source="user";
				animPeriod=0.0099999998;
				initPhase=1;
			};
		};
   	};

	class GRMODPMACHINE: Inventory_Base       
	{
		scope=2;
		displayName="Garage";
		descriptionShort="Garage machine to open the Garage!";
		model="Depositary_System\models\ParkingMachine.p3d";
		overrideDrawArea = 8.0;
		forceFarBubble = "true";
		destroyOnEmpty = 0;
		quantityBar = 0;
		varQuantityMax = 0;
		carveNavmesh = 1;
		canBeDigged = 0;
		heavyItem = 1;
		weight = 10000;
		itemSize[] = {10, 15};
		itemBehaviour = 0;
		physLayer = "item_large";
		lootTag[] = {"Work"};
		hiddenSelections[]=
		{
			"zbytek"
		};
		hiddenSelectionsTextures[]=
		{
            "Depositary_System\models\data\ParkingMachine_co.paa"
		};
		class DamageSystem {
            class GlobalHealth {
                class Health {
                    hitpoints = 1000000;
                    healthLevels[] = {{1.0, {"DZ\gear\containers\data\Barrel_green.rvmat"}}, {0.7, {"DZ\gear\containers\data\Barrel_green.rvmat"}}, {0.5, {"DZ\gear\containers\data\Barrel_green_damage.rvmat"}}, {0.3, {"DZ\gear\containers\data\Barrel_green_damage.rvmat"}}, {0.0, {"DZ\gear\containers\data\Barrel_green_destruct.rvmat"}}};
                };
            };

            class GlobalArmor {
                class Projectile {
                    class Health {
                        damage = 0;
                    };

                    class Blood {
                        damage = 0;
                    };

                    class Shock {
                        damage = 0;
                    };
                };
            };
        };
  	};
};
