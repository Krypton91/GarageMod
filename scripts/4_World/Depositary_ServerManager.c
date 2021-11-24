class Depositary_ServerManager
{
    protected static ref Depositary_ServerManager   m_ServerManager;
	private bool 									m_IsParkInDisabled;
	const string									m_HologrammClassname = "GarageMod_ParkingPositionHolo";
    protected ref Depositary_Config                 m_Settings;
	protected ref Depositary_AdminConfig            m_AdminList;
	ref array< ref NPCDummyClass > 					m_DummyClasses;
	autoptr TStringArray 							m_MuchCarKeyKeyNames = new TStringArray();
	autoptr TStringArray 							m_TraderKeyNames = new TStringArray();

    void Depositary_ServerManager()
    {
		//For better mod support!
        InitGarageServerside();
    }

	void ~Depositary_ServerManager()
    {
		GetGame().GetCallQueue(CALL_CATEGORY_GUI).Remove(this.SetIsParkinDisabled);
    }

	EntityAI GetParentInventoryItem(EntityAI item)
	{
		if (!item)
			return NULL;

		InventoryLocation inventoryLocation = new InventoryLocation;
		item.GetInventory().GetCurrentInventoryLocation(inventoryLocation);
		return inventoryLocation.GetParent();
	}

	ItemCargoCoordinates GetItemCargoCoordinates(EntityAI item)
	{
		int row = -1;
		int col = -1;

		CargoBase parentCargo = GetParentInventoryItem(item).GetInventory().GetCargo();

		if(parentCargo)
		{
			int index = parentCargo.FindEntityInCargo(item);
		}
		else
		{
			return new ItemCargoCoordinates(row, col, false);
		}
		
		parentCargo.GetItemRowCol(index, row, col);
		return new ItemCargoCoordinates(row, col, item.GetInventory().GetFlipCargo());
	}

	void InitGarageServerside()
	{
		m_Settings  = Depositary_Config.Load();
		m_AdminList = Depositary_AdminConfig.Load();
        if(GetGame().IsServer())
        {
            GetRPCManager().AddRPC("Depositary_System", "GarageDataRequest", this, SingleplayerExecutionType.Server);
            GetRPCManager().AddRPC("Depositary_System", "VehicleParkinRequest", this, SingleplayerExecutionType.Server);
            GetRPCManager().AddRPC("Depositary_System", "VehicleParkOutRequest", this, SingleplayerExecutionType.Server);
			GetRPCManager().AddRPC("Depositary_System", "FULLCONFIGREQ", this, SingleplayerExecutionType.Server);
			GetRPCManager().AddRPC("Depositary_System", "GetVehicleKey", this, SingleplayerExecutionType.Server);
			m_DummyClasses =  new ref array<ref NPCDummyClass>;

			SpawnNPCs();

			if(m_Settings.ServerRestartPeriods != -1){
				int hourToLock = m_Settings.ServerRestartPeriods * 3600000;
				int MinutesToLock = hourToLock / 60 - 5;
				int FinalTime = MinutesToLock * 60;
				GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(this.SetIsParkinDisabled, FinalTime, false);
			}

			if(m_Settings.IsLoggingActiv){
				GetGarageLogger().Log("[ServerManager] -> Restart Periods of server are : " + m_Settings.ServerRestartPeriods + " Info: to Disable the parkin function befor an restart go to config and change ServerRestartPeriods to -1! \n Server Will Lock Parking in: " + FinalTime + " Minutes!");
				GetGarageLogger().Log("[ServerManager] -> Parkin Function will be disabled 5 Minutes befor!");
			}
			#ifdef MUCHCARKEY
			m_MuchCarKeyKeyNames = {"MCK_CarKey_Blue", "MCK_CarKey_Green", "MCK_CarKey_Red", "MCK_CarKey_White", "MCK_CarKey_Yellow", "MCK_CarKey_Base"};
			#else
			m_TraderKeyNames = {"VehicleKeyRed", "VehicleKeyBlack", "VehicleKeyGrayCyan", "VehicleKeyYellow", "VehicleKeyPurple"};
			#endif
        }
	}

	void SetIsParkinDisabled()
	{
		GetGarageLogger().Log("[ServerManager] -> Parkin Function was locked....");
		m_IsParkInDisabled = true;
	}

    string isVehicleSpawnFree(vector VehiclesSpawnPos, vector VehiclesSpawnOri)
	{
		vector size = "3 5 9";
		array<Object> excluded_objects = new array<Object>;
		array<Object> nearby_objects = new array<Object>;

		GetGame().IsBoxColliding( VehiclesSpawnPos, VehiclesSpawnOri, size, excluded_objects, nearby_objects);
		if (nearby_objects.Count() > 0)
		{
			string objsName = nearby_objects.Get(0).GetType();
			int index = m_Settings.ObjectWhiteList.Find(objsName);
			if(index == -1)
			{
				if(objsName == "")
					return "FREE";
				return objsName;
			}
		}
		return "FREE";
	}

    //////////////////////////////////////////////////SERVERSIDE RPC////////////////////////////////////////////////////////
	void FULLCONFIGREQ(CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target)
	{
		if(type == CallType.Server)
        {
			GetRPCManager().SendRPC("Depositary_System", "FULLCONFIGRES", new Param6<float, bool, int, int, bool, ref array<ref Currency>>(m_Settings.CooldownForGarage, m_Settings.KeepInventory, m_Settings.CostsToParkInVehicle, m_Settings.CostsToParkOutVehicle, m_Settings.CanPayWithBankAccount, m_Settings.CurrencyConfig), true, sender);
			GetRPCManager().SendRPC("Depositary_System", "NPCDUMMYCLASSES", new Param2<ref array<ref NPCDummyClass>, bool>(m_DummyClasses, isSenderAdmin(sender.GetPlainId())), true, sender);
		}
	}
    
    vector GetVehiclesSpawnOriByGarageID(int garageID)
    {
        local vector spawn_Pos = "0 0 0";
        for(int i = 0; i < m_Settings.NPCConfig.Count(); i++)
        {
            if(m_Settings.NPCConfig[i].GarageID == garageID)
            {
                //MATCH FOUND RETRUN AND BUILD VECTOR
                spawn_Pos[0] = m_Settings.NPCConfig[i].CARSpawnYaw;
                spawn_Pos[1] = m_Settings.NPCConfig[i].CARSpawnPitch;
                spawn_Pos[2] = m_Settings.NPCConfig[i].CARSpawnRoll;
            }
        }
        return spawn_Pos;
    }

    private PlayerBase GetPlayerBySteamID(string steamid)
    {
        array<Man> players = new array<Man>;
        GetGame().GetPlayers(players);
        for(int i = 0; i < players.Count(); i++)
        {
            if(players.Get(i).GetIdentity().GetPlainId() == steamid)
            {
                return PlayerBase.Cast(players.Get(i));
            }
        }
        return null;
    }

    void VehicleParkOutRequest(CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target)
    {
        Param3<int, int, bool> params;
        if(!ctx.Read(params)) return;
        if(type == CallType.Server)
        {
            DepositaryData playerData = DepositaryData.LoadPlayerData(sender.GetPlainId(), sender.GetName());
            if(playerData)
            {
                PlayerBase player = PlayerBase.Cast(GetPlayerBySteamID(sender.GetPlainId()));
				if(!player) return;

                if(!CanPayCosts(player, m_Settings.CostsToParkOutVehicle))
                {
                    GetRPCManager().SendRPC("Depositary_System", "UI_MessageRequest", new Param3<string, string, int>("#garage_UI_Message_ERROR","You dont have enought money in your Inventory! " + m_Settings.CostsToParkOutVehicle, 1), true, sender);
                    return;
                }
                else
                {
                    ref array<ref VehicleData> vehicleData = playerData.vehicleData;
                    for(int i = 0; i < vehicleData.Count(); i++)
                    {
                        if(vehicleData[i].indexID == params.param2)
                        {
                            EntityAI vehicle;
							int garageID = params.param1;
                            if(garageID == -1)
                                return;
							vector vehicle_Spawn;
							vector vehicle_ori;
							string vehiclesSpawnState;
							if(garageID == 6876578756)
							{
								if(!vehicleData[i].SpawnPos || !vehicleData[i].SpawnOri)
								{
									GetRPCManager().SendRPC("Depositary_System", "UI_MessageRequest", new Param3<string, string, int>("#garage_UI_Message_ERROR","This Vehicle can not parked out here!", 1), true, sender);
									return;
								}
								vehicle_Spawn = vehicleData[i].SpawnPos;
								vehicle_ori = vehicleData[i].SpawnOri;
								float distance = vector.Distance(vehicle_Spawn, player.GetPosition());
								if(distance && distance >= 10)
								{
									GetRPCManager().SendRPC("Depositary_System", "UI_MessageRequest", new Param3<string, string, int>("#garage_UI_Message_ERROR","This Vehicle can not parked out here!", 1), true, sender);
									return;
								}
								vehiclesSpawnState = "FREE";

							}
							else
							{
								vehicle_Spawn = GetVehicleSpawnPosByGarageID(garageID);
								vehicle_ori = GetVehiclesSpawnOriByGarageID(garageID);
								vehiclesSpawnState = isVehicleSpawnFree(vehicle_Spawn, vehicle_ori);
							}

							if(!m_Settings.IsGarageGlobal && garageID != vehicleData[i].GarageID && vehicleData[i].GarageID != -1)
							{
								GetRPCManager().SendRPC("Depositary_System", "UI_MessageRequest", new Param3<string, string, int>("ERROR","This Vehicle cant parked out here!", 1), true, sender);
                    			return;
							}
                            if(vehiclesSpawnState == "FREE")
                            {
								RemoveCurrencyFromPlayer(player, m_Settings.CostsToParkOutVehicle);
								ref array<Man> m_Players = new array<Man>;
								GetGame().GetWorld().GetPlayerList(m_Players);
								PlayerBase currentPlayer;
                                vehicle = EntityAI.Cast(GetGame().CreateObject(vehicleData[i].VehiclesName, vehicle_Spawn));
								if(vehicleData[i].EngineHealth)
									vehicle.SetHealth(vehicleData[i].EngineHealth);
								//vehicle.SetPosition(vehicle_Spawn);
								vehicle.SetOrientation(vehicle_ori);
								vehicle.SetDirection(vehicle.GetDirection());
								
								for (int pl = 0; pl < m_Players.Count(); pl++)
								{
									currentPlayer = PlayerBase.Cast(m_Players.Get(pl));

									if ( !currentPlayer )
										continue;

									GetGame().RPCSingleParam(currentPlayer, TRPCs.RPC_SYNC_OBJECT_ORIENTATION, new Param2<Object, vector>( vehicle, vehicle_ori ), true, currentPlayer.GetIdentity());
								}
                                if(vehicle)
                                {
									//THIS CASE IS ONLY IF MUCHCARKEY IS USED!
									#ifdef MuchCarKey
									ParkOutWithMuchCarKey(vehicle, vehicleData, i, player, playerData);
									#else
									ParkOutWithTrader(vehicle, vehicleData, i, player, playerData);
									#endif
								}
                            }
                            else
                            {
                                GetRPCManager().SendRPC("Depositary_System", "UI_MessageRequest", new Param3<string, string, int>("#garage_UI_Message_ERROR","Vehicle Spawn was blocked by: " + vehiclesSpawnState, 1), true, sender);
                            }
                        }
                    }
                }
            }
        }
    }

	void GetVehicleKey(CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target)
	{
		Param1<int> params;
        if(!ctx.Read(params)) return;
        if(type == CallType.Server)
        {
			PlayerBase player = GetPlayerBySteamID(sender.GetPlainId());
			if(!player) return;

			DepositaryData playerData = DepositaryData.LoadPlayerData(sender.GetPlainId(), sender.GetName());
			if(playerData)
			{
				
				ref VehicleData foundVehicle;
				for(int i = 0; i < playerData.vehicleData.Count(); i++)
				{
					if(playerData.vehicleData.Get(i).indexID == params.param1)
					{
						foundVehicle = playerData.vehicleData.Get(i);
						break;
					}
				}

				if(foundVehicle)
				{
					if(CanPayCosts(player, m_Settings.CostsToBuyVehicleKey))
					{
						#ifdef MUCHCARKEY
						MCK_CarKey_Base spawnedVehicleKey = MCK_CarKey_Base.Cast(player.GetHumanInventory().CreateInInventory(m_MuchCarKeyKeyNames.GetRandomElement()));
						if(spawnedVehicleKey)
						{
							RemoveCurrencyFromPlayer(player, m_Settings.CostsToBuyVehicleKey);
							spawnedVehicleKey.SetNewMCKId(foundVehicle.VehiclesHash);
							GetRPCManager().SendRPC("Depositary_System", "UI_MessageRequest", new Param3<string, string, int>("#garage_UI_Message_SUCESSFULLY","Vehicle Key is in your Inventory!", 2), true, sender);
						}
						else
						{
							Error("Cant Cast class to vehicle!");
						}
						#else
						VehicleKeyBase spawnedVehicleKey = VehicleKeyBase.Cast(player.GetHumanInventory().CreateInInventory(m_TraderKeyNames.GetRandomElement()));
						if(spawnedVehicleKey)
						{
							RemoveCurrencyFromPlayer(player, m_Settings.CostsToBuyVehicleKey);
							spawnedVehicleKey.SetNewHash(foundVehicle.VehiclesHash);
							GetRPCManager().SendRPC("Depositary_System", "UI_MessageRequest", new Param3<string, string, int>("#garage_UI_Message_SUCESSFULLY","Vehicle Key is in your Inventory!", 2), true, sender);
						}
						else
						{
							Error("Cant Cast class to vehicle!");
						}
						#endif
					}
					else
					{
						GetRPCManager().SendRPC("Depositary_System", "UI_MessageRequest", new Param3<string, string, int>("#garage_UI_Message_ERROR","#garage_UI_Message_NotEnoughCashOnPlayer", 1), true, sender);
					}
				}
				else
				{
					Error("Cant find vehicle!");
				}
			}
		}
	}

#ifdef MuchCarKey
	
	void ParkOutWithMuchCarKey_NEW01(EntityAI vehicle, ref array<ref VehicleData> vehicleData, int i, PlayerBase player, DepositaryData playerData) {

		MCK_CarKey_Base vehicleKey;

		ref VehicleCargo vehicleCargoPiece;
		int slotID;

		map<int, EntityAI> items = new map<int, EntityAI>();
		EntityAI parentItem;
		EntityAI childItem;
		Barrel_ColorBase barrel;

		for (int n = 0; n < vehicleData[i].m_Cargo.Count(); n++)
		{
			vehicleCargoPiece = vehicleData[i].m_Cargo[n];
			parentItem = items.Get(vehicleCargoPiece.GetOldParentID());

			if (!parentItem)
				parentItem = vehicle;

			if (vehicleCargoPiece.IsAttachment())
			{
				slotID = InventorySlots.GetSlotIdFromString(vehicleCargoPiece.GetAttachmentSlotName());
				childItem = parentItem.GetInventory().CreateAttachmentEx(vehicleCargoPiece.ItemName, slotID);

				if (Class.CastTo(barrel, childItem))
					barrel.Open();
			}
			else
			{
				slotID = parentItem.GetInventory().GetCargo().GetOwnerCargoIndex();
				childItem = parentItem.GetInventory().CreateEntityInCargoEx(vehicleCargoPiece.ItemName, slotID, vehicleCargoPiece.GetItemCargoCoordinateRow(), vehicleCargoPiece.GetItemCargoCoordinateCol(), vehicleCargoPiece.IsFlipped());

				if (Class.CastTo(vehicleKey, childItem))
				{
					vehicleKey.SetNewMCKId(vehicleCargoPiece.KeyHash);
				}
			}

			//
			// Item could neither be created as attachment nor in cargo. The only reason I can think of is
			// when the vehicle cargo size or a container cargo size has been changed after parking the vehicle in.
			//
			if (!childItem)
			{
				childItem = ItemBase.Cast(player.SpawnEntityOnGroundPos(vehicleCargoPiece.ItemName, player.GetPosition()));
				Param1<string> message = new Param1<string>("Vehicle inventory was full. The rest of the items was spawned on the ground.");
				GetGame().RPCSingleParam(player, ERPCs.RPC_USER_ACTION_MESSAGE, message, true, player.GetIdentity());
			}

			if (vehicleCargoPiece.Health && m_Settings.SaveDamage)
				childItem.SetHealth(vehicleCargoPiece.Health);

			SetItemAmount(childItem, vehicleCargoPiece.VehicleCargoAmmount);
			items.Insert(vehicleCargoPiece.GetOldID(), childItem);
		}
	}

	void ParkOutWithMuchCarKey(EntityAI vehicle, ref array<ref VehicleData> vehicleData, int i, PlayerBase player, DepositaryData playerData)
	{
		if(DepositaryData.m_FormatVersion == DepositaryDataFormatVersion.NEW01)
		{
			ParkOutWithMuchCarKey_NEW01(vehicle, vehicleData, i, player, playerData);
		}
		else
		{
			MCK_CarKey_Base vehicleKey;
			for (int n = 0; n < vehicleData[i].m_Cargo.Count(); n++)
			{
				if (!canCreateItemInVehicleInventory(vehicle, vehicleData[i].m_Cargo[n].ItemName, vehicleData[i].m_Cargo[n].VehicleCargoAmmount))
				{
					ItemBase playersInvItem = ItemBase.Cast(player.SpawnEntityOnGroundPos(vehicleData[i].m_Cargo[n].ItemName, player.GetPosition()));
					if (vehicleData[i].m_Cargo[n].Health && m_Settings.SaveDamage)
						playersInvItem.SetHealth(vehicleData[i].m_Cargo[n].Health);
					Param1<string> msgRp0 = new Param1<string>("Vehicle Inventory was full rest of items spawned on ground!!");
					GetGame().RPCSingleParam(player, ERPCs.RPC_USER_ACTION_MESSAGE, msgRp0, true, player.GetIdentity());
				}
				else
				{
					ItemBase item = ItemBase.Cast(CreateItemInVehicleInventory(vehicle, vehicleData[i].m_Cargo[n].ItemName, vehicleData[i].m_Cargo[n].VehicleCargoAmmount, player));
					if (vehicleData[i].m_Cargo[n].Health && m_Settings.SaveDamage)
						item.SetHealth(vehicleData[i].m_Cargo[n].Health);
					if (Class.CastTo(vehicleKey, item))
					{
						//WE NO KNOW ITS AN KEY FROM HELKIANA.
						local int KeysHashCode = 0;
						KeysHashCode = vehicleData[i].m_Cargo[n].KeyHash;
						if (KeysHashCode != 0)
						{
							//Database has the entry
							vehicleKey.SetNewMCKId(KeysHashCode);
						}
					}
				}
			}
		}

		Car car;
        Class.CastTo(car, vehicle);
        if (car)
        {
			if(vehicleData[i].FuelAmmount && m_Settings.SaveDamage)
			{
				car.Fill( CarFluid.FUEL, vehicleData[i].FuelAmmount);
			}
			else
			{
				car.Fill( CarFluid.FUEL, car.GetFluidCapacity( CarFluid.FUEL ));
			}
            car.Fill( CarFluid.OIL, car.GetFluidCapacity( CarFluid.OIL ));
            car.Fill( CarFluid.BRAKE, car.GetFluidCapacity( CarFluid.BRAKE ));
            car.Fill( CarFluid.COOLANT, car.GetFluidCapacity( CarFluid.COOLANT ));

            car.Fill( CarFluid.USER1, car.GetFluidCapacity( CarFluid.USER1 ));
            car.Fill( CarFluid.USER2, car.GetFluidCapacity( CarFluid.USER2 ));
            car.Fill( CarFluid.USER3, car.GetFluidCapacity( CarFluid.USER3 ));
            car.Fill( CarFluid.USER4, car.GetFluidCapacity( CarFluid.USER4 ));
										
			CarScript carScript;
			if(Class.CastTo(carScript, vehicle))
			{
											
				if(vehicleData[i].VehiclesHash != 0)
				{
					carScript.m_HasCKAssigned = true;
					carScript.m_CarKeyId = vehicleData[i].VehiclesHash;
				}

				carScript.SynchronizeValues();
				if(player.m_Trader_IsInSafezone)
				{
					carScript.m_Trader_IsInSafezone = true;
					car.SetAllowDamage(false);
				}
			}
        }

        playerData.vehicleData.Remove(i);
        playerData.SortVehicleIDs();
        playerData.SavePlayerData(playerData, player.GetIdentity().GetName());

		if(m_Settings.IsLoggingActiv)
			GetGarageLogger().LogLine(player.GetIdentity().GetName(), player.GetIdentity().GetPlainId(), player.GetPosition(), "Parked out :" + vehicle.GetType());
		
        GetRPCManager().SendRPC("Depositary_System", "UI_QuitRequest", null, true, player.GetIdentity());
	}
#else
	void ParkOutWithTrader(EntityAI vehicle, ref array<ref VehicleData> vehicleData, int i, PlayerBase player,  DepositaryData playerData)
	{
		VehicleKeyBase vehicleKey;
		for(int n = 0; n < vehicleData[i].m_Cargo.Count(); n++)
		{
			if(!canCreateItemInVehicleInventory(vehicle, vehicleData[i].m_Cargo[n].ItemName, vehicleData[i].m_Cargo[n].VehicleCargoAmmount))
			{
				ItemBase PlayersInvItem = player.SpawnEntityOnGroundPos(vehicleData[i].m_Cargo[n].ItemName, player.GetPosition());
				if(vehicleData[i].m_Cargo[n].Health && m_Settings.SaveDamage)
					PlayersInvItem.SetHealth(vehicleData[i].m_Cargo[n].Health);
				Param1<string> msgRp0 = new Param1<string>( "Vehicle Inventory was full rest of items spawned on ground!!" );
				GetGame().RPCSingleParam(player, ERPCs.RPC_USER_ACTION_MESSAGE, msgRp0, true, player.GetIdentity());
			}
			else
			{
				ItemBase item = ItemBase.Cast(CreateItemInVehicleInventory(vehicle, vehicleData[i].m_Cargo[n].ItemName, vehicleData[i].m_Cargo[n].VehicleCargoAmmount, player));
				if(vehicleData[i].m_Cargo[n].Health && m_Settings.SaveDamage)
					item.SetHealth(vehicleData[i].m_Cargo[n].Health);
				if(Class.CastTo(vehicleKey, item))
				{
					//WE NO KNOW ITS AN KEY FROM JONES. Danke Für deine Arbeit Brudi <3
					local int KeysHashCode = 0;
					KeysHashCode = vehicleData[i].m_Cargo[n].KeyHash;
					if(KeysHashCode != 0)
					{
						vehicleKey.SetNewHash(KeysHashCode);
					}
				}
			}
		}
		Car car;
		Class.CastTo(car, vehicle);
		if (car)
		{
			if(vehicleData[i].FuelAmmount && m_Settings.SaveDamage)
			{
				car.Fill( CarFluid.FUEL, vehicleData[i].FuelAmmount);
			}
			else
			{
				car.Fill( CarFluid.FUEL, car.GetFluidCapacity( CarFluid.FUEL ));
			}
			car.Fill( CarFluid.OIL, car.GetFluidCapacity( CarFluid.OIL ));
			car.Fill( CarFluid.BRAKE, car.GetFluidCapacity( CarFluid.BRAKE ));
			car.Fill( CarFluid.COOLANT, car.GetFluidCapacity( CarFluid.COOLANT ));

			car.Fill( CarFluid.USER1, car.GetFluidCapacity( CarFluid.USER1 ));
			car.Fill( CarFluid.USER2, car.GetFluidCapacity( CarFluid.USER2 ));
			car.Fill( CarFluid.USER3, car.GetFluidCapacity( CarFluid.USER3 ));
			car.Fill( CarFluid.USER4, car.GetFluidCapacity( CarFluid.USER4 ));
			
			CarScript carScript;
			if(Class.CastTo(carScript, vehicle))
			{
				if(vehicleData[i].VehiclesHash != 0)
				{
					carScript.m_Trader_HasKey = true;
					carScript.m_Trader_VehicleKeyHash = vehicleData[i].VehiclesHash;
					
				}

				carScript.SynchronizeValues();
				if(player.m_Trader_IsInSafezone)
				{
					carScript.m_Trader_IsInSafezone = true;
					car.SetAllowDamage(false);
				}
			}
		}
		playerData.vehicleData.Remove(i);
		playerData.SortVehicleIDs();
		playerData.SavePlayerData(playerData, player.GetIdentity().GetName());
		if(m_Settings.IsLoggingActiv)
			GetGarageLogger().LogLine(player.GetIdentity().GetName(), player.GetIdentity().GetPlainId(), player.GetPosition(), " Parked out :" + vehicle.GetType());
		GetRPCManager().SendRPC("Depositary_System", "UI_QuitRequest", null, true, player.GetIdentity());
	}
	#endif

    void VehicleParkinRequest(CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target)
    {
        Param2<int, bool> params;
        if(!ctx.Read(params)) return;
        if(type == CallType.Server)
        {
			if(m_IsParkInDisabled){
				GetRPCManager().SendRPC("Depositary_System", "UI_MessageRequest", new Param3<string, string, int>("#garage_UI_Message_ERROR","PARKIN FUNCTION IS DISABLED (RESTART IS CLOSE)!" + m_Settings.CostsToParkInVehicle, 1), true, sender);
                 return;
			}

            DepositaryData playerData = DepositaryData.LoadPlayerData(sender.GetPlainId(), sender.GetName());
            if(playerData)
            {
                PlayerBase player = PlayerBase.Cast(GetPlayerBySteamID(sender.GetPlainId()));
				if(!player) return;

				if(!CanPayCosts(player, m_Settings.CostsToParkInVehicle))
				{
						GetRPCManager().SendRPC("Depositary_System", "UI_MessageRequest", new Param3<string, string, int>("#garage_UI_Message_ERROR","You dont have enough money in your Inventory!" + m_Settings.CostsToParkInVehicle, 1), true, sender);
                    	return;
				}

				if(playerData.AmountOfParkedVehicles() + 1 <= m_Settings.MaxVehiclesToStore || isSenderAdmin(sender.GetPlainId()))
				{
					Object car;
					if(params.param1 == 6876578756)
					{
						car = GetVehicleParkPosWithCords(player.GetPosition(), player.GetOrientation());
					}
					else
					{
	                	car = GetVehicleParkPosWithGarageID(params.param1);
					}
	                if(car)
	                {
						if(isVehicleBlackListet(car.GetType()))
						{
							GetRPCManager().SendRPC("Depositary_System", "UI_MessageRequest", new Param3<string, string, int>("#garage_UI_Message_ERROR","Sorry but this Vehicle is Blacklistet by an Admin!", 1), true, sender);
                    		return;
						}
						#ifdef MuchCarKey
						ParkInWithMuchCarKey(car, player, sender, playerData, params.param1);
						#else
						ParkinWithTrader(car, player, sender, playerData, params.param1);
						#endif
	                }
					else
					{
						GetRPCManager().SendRPC("Depositary_System", "UI_MessageRequest", new Param3<string, string, int>("#garage_UI_Message_ERROR","No Vehicle in Park Pos found!", 1), true, sender);
					}
                }
				else
				{
					GetRPCManager().SendRPC("Depositary_System", "UI_MessageRequest", new Param3<string, string, int>("#garage_UI_Message_ERROR","Your Garage is already full!", 1), true, sender);
				}
            }
        }
    }
#ifdef MuchCarKey
	void ParkInWithMuchCarKey(Object car, PlayerBase player, PlayerIdentity sender, DepositaryData playerData, int GarageID)
	{
		ref array<ref VehicleCargo> Cargo = new array<ref VehicleCargo>;
		int VehicleHash = 0;
	    array<EntityAI> items = new array<EntityAI>();
	    EntityAI vehicle;
	    if( Class.CastTo(vehicle, car))
	    {
			CarScript carScript = CarScript.Cast(vehicle);
			
			if(carScript && carScript.m_Trader_LastDriverId != sender.GetId() || carScript.m_Trader_LastDriverId == string.Empty)
			{
				//WAS NOT LAST DRIVER!!
				GetRPCManager().SendRPC("Depositary_System", "UI_MessageRequest", new Param3<string, string, int>("#garage_UI_Message_ERROR","#garage_UI_Message_WasNotLastDriver", 1), true, sender);
				return;
			}
			
			//Todo add here money to deduct currency.
			RemoveCurrencyFromPlayer(player, m_Settings.CostsToParkInVehicle);
	        vehicle.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER,items);
			VehicleHash = carScript.m_CarKeyId;
	        Cargo.Clear();
			int insertIndex = playerData.GetNextVehicleIndex();
			if(m_Settings.KeepInventory == false)
			{
				//Attachments from Car.
				if(vehicle.GetInventory())
				{
					GameInventory inv = vehicle.GetInventory();
					int attachmentCount = inv.AttachmentCount();
					for(int y = 0; y < attachmentCount; y++)
					{
						EntityAI vhAttachment = inv.GetAttachmentFromIndex(y);
						if(vhAttachment.IsRuined())
								continue;
						Cargo.Insert(new VehicleCargo(vhAttachment.GetType(), vhAttachment.GetQuantity(), vhAttachment.GetHealth()));
										
					}
				}
			}
			else
			{
				int attachmentSlotID;
				string attachmentSlotName;

				for(int i = 0; i < items.Count(); i++)
		        {
					ItemBase item = ItemBase.Cast(items[i]);
					if(!item)
						continue;
					if(item.GetType() == vehicle.GetType())
						continue;
					if(item.IsRuined())
						continue;

					MCK_CarKey_Base key;
					VehicleCargo vehicleCargo;

					if (m_Settings.KeepInventory == true)
					{
						if(Class.CastTo(key, item))
						{
							vehicleCargo = new VehicleCargo(item.GetType(), GetItemAmount(item), item.GetHealth(), key.GetMCKId());
						}
						else
						{
							vehicleCargo = new VehicleCargo(item.GetType(), GetItemAmount(item), item.GetHealth());
						}
						
						vehicleCargo.SetOldID(item.GetID());
						vehicleCargo.SetOldParentID(GetParentInventoryItem(item).GetID());
						vehicleCargo.SetItemCargoCoordinateRow(GetItemCargoCoordinates(item).GetRow());
						vehicleCargo.SetItemCargoCoordinateCol(GetItemCargoCoordinates(item).GetCol());
						vehicleCargo.SetFlipped(GetItemCargoCoordinates(item).IsFlipped());

						if(item.GetInventory().IsAttachment())
						{
							item.GetInventory().GetCurrentAttachmentSlotInfo(attachmentSlotID, attachmentSlotName)
							vehicleCargo.SetAttachmentSlotID(attachmentSlotID);
							vehicleCargo.SetAttachmentSlotName(attachmentSlotName);
						}

						vehicleCargo.SetAttachment(item.GetInventory().IsAttachment());

						Cargo.Insert(vehicleCargo);
					}
		        }
			}

			if(m_Settings.IsLoggingActiv)
				GetGarageLogger().LogLine(sender.GetName(), sender.GetPlainId(), player.GetPosition(), "Parked in : " + vehicle.GetType() + " " + Cargo.Count() + " items stored to DB!");

			if(!m_Settings.IsGarageGlobal)
			{
				//Safe The GarageID to File
				InsertNewVehicle(insertIndex, vehicle.GetType(), VehicleHash, GarageID, Cargo, playerData, sender.GetName(), vehicle.GetHealth(), carScript.GarageGetFuelAmmount(), vehicle.GetID());
			}
			else
			{
				//Insert with GarageID -1 = Global
				InsertNewVehicle(insertIndex, vehicle.GetType(), VehicleHash, -1, Cargo, playerData, sender.GetName(), vehicle.GetHealth(), carScript.GarageGetFuelAmmount(), vehicle.GetID());
			}

			if(GarageID == 6876578756)
			{
				SetVehicleSpawnData(playerData, insertIndex, vehicle.GetPosition(), vehicle.GetOrientation());//Safe on this garage spawn pos & yaw!
			} 

	        GetGame().ObjectDelete(car);
	    }
	}
#else
	void ParkinWithTrader(Object car, PlayerBase player, PlayerIdentity sender, DepositaryData playerData, int GarageID)
	{
		ref array<ref VehicleCargo> Cargo = new array<ref VehicleCargo>;
		int VehicleHash = 0;
	    array<EntityAI> items = new array<EntityAI>();
	    EntityAI vehicle;
		CarScript carScript;
	    if( Class.CastTo(vehicle, car))
	    {
			carScript = CarScript.Cast(vehicle);
			if(carScript && carScript.m_Trader_LastDriverId != sender.GetId() || carScript.m_Trader_LastDriverId == string.Empty)
			{
				//WAS NOT LAST DRIVER!!
				GetRPCManager().SendRPC("Depositary_System", "UI_MessageRequest", new Param3<string, string, int>("#garage_UI_Message_ERROR","#garage_UI_Message_WasNotLastDriver", 1), true, sender);
				return;
			}

			//Todo Add here system to deduct currency.
			RemoveCurrencyFromPlayer(player, m_Settings.CostsToParkInVehicle);
	        vehicle.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER,items);
			VehicleHash = carScript.m_Trader_VehicleKeyHash;
	        Cargo.Clear();
			int insertIndex = playerData.GetNextVehicleIndex();
			if(m_Settings.KeepInventory == false)
			{
				//Attachments from Car.
				if(vehicle.GetInventory())
				{
					GameInventory inv = vehicle.GetInventory();
					int attachmentCount = inv.AttachmentCount();
					for(int y = 0; y < attachmentCount; y++)
					{
						EntityAI vhAttachment = inv.GetAttachmentFromIndex(y);
						if(vhAttachment.IsRuined())
								continue;
							Cargo.Insert(new VehicleCargo(vhAttachment.GetType(), vhAttachment.GetQuantity(), vhAttachment.GetHealth()));			
					}
				}
			}
			else
			{
		        for(int i = 0; i < items.Count(); i++)
		        {
					ItemBase item = ItemBase.Cast(items[i]);
						if(!item)
							continue;
						if(item.GetType() == vehicle.GetType())
							continue;
						if(item.IsRuined())
							continue;

						VehicleKeyBase key;
						if(Class.CastTo(key, item) && m_Settings.KeepInventory == true)
						{
							Cargo.Insert(new VehicleCargo(item.GetType(), GetItemAmount(item), item.GetHealth(), key.GetHash()));
						}
						else
						{
							if(m_Settings.KeepInventory == true)
								Cargo.Insert(new VehicleCargo(item.GetType(), GetItemAmount(item), item.GetHealth()));
						}
		            }
				}
				if(m_Settings.IsLoggingActiv)
					GetGarageLogger().LogLine(sender.GetName(), sender.GetPlainId(), player.GetPosition(), "Parked in : " + vehicle.GetType() + " " + Cargo.Count() + " items stored to DB!");
				if(!m_Settings.IsGarageGlobal)
				{
					//Safe The GarageID to File
					InsertNewVehicle(insertIndex, vehicle.GetType(), VehicleHash, GarageID, Cargo, playerData, sender.GetName(), vehicle.GetHealth(), carScript.GarageGetFuelAmmount(), vehicle.GetID());
				}
				else
				{
					//Insert with GarageID -1 = Global
					InsertNewVehicle(insertIndex, vehicle.GetType(), VehicleHash, -1, Cargo, playerData, sender.GetName(), vehicle.GetHealth(), carScript.GarageGetFuelAmmount(), vehicle.GetID());
				}
				if(GarageID == 6876578756)
				{
					SetVehicleSpawnData(playerData, insertIndex, vehicle.GetPosition(), vehicle.GetOrientation());//Safe on this garage spawn pos & yaw!
				}

	            GetGame().ObjectDelete(car);
	    }
	}
#endif
	//!Returns if a Vehicle is Blacklistet by an Admin.
	protected bool isVehicleBlackListet(string VehiclesName)
	{
		for(int i = 0; i < m_AdminList.VehicleBlackList.Count(); i++)
		{
			if(VehiclesName == m_AdminList.VehicleBlackList[i])
				return true;
		}
		return false;
	}

	bool CanPayCosts(PlayerBase player, int CostsToCheck)
	{
		if(CostsToCheck > GetCurrencyAmountOnPlayer(player))
		{
			#ifdef ADVANCED_BANKING
			KR_JsonDatabaseHandler playerdata = KR_JsonDatabaseHandler.LoadPlayerData(player.GetIdentity().GetPlainId(), player.GetIdentity().GetName());
			if(playerdata)
			{
				if(playerdata.GetBankCredit() >= CostsToCheck && m_Settings.CanPayWithBankAccount)
					return true;
			}
			#endif
			#ifdef DC_BANKING
			DC_BankingData playerdata = DC_BankingData.LoadPlayerData(player.GetIdentity().GetPlainId(), player.GetIdentity().GetName());
			if(playerdata)
			{
				Print("DC BANKING MODULE WAS LOADET: " + playerdata.GetOwnedCurrency().ToString() + " needed costs: " + CostsToCheck.ToString());
				if(playerdata.GetOwnedCurrency() >= CostsToCheck && m_Settings.CanPayWithBankAccount)
					return true;
			}
			#endif
		}
		else
		{
			return true;
		}
		return false;
	}

    protected void InsertNewVehicle(int index, string VehicleName, int VehicleHash, int GaragenID, /*ref TStringArray cargo*/ref array<ref VehicleCargo> this_cargo, DepositaryData playerData, string AccountName, float EngineHealth, float tank_fuelAmmount, int oldID)
    {
        playerData.InsertNewVehicle(VehicleName, index, VehicleHash, GaragenID, this_cargo, EngineHealth, tank_fuelAmmount, oldID);
        playerData.SavePlayerData(playerData, AccountName);
    }

	void SetVehicleSpawnData(DepositaryData playerdata, int index, vector spawnPos, vector spawnOri)
	{
		playerdata.SetSpawnData(index, spawnPos, spawnOri);
	}

	void GarageDataRequest(CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target)
    {
		if(type == CallType.Server)
        {
			DepositaryData playerData = DepositaryData.LoadPlayerData(sender.GetPlainId(), sender.GetName());
			if(playerData)
			{
			    GetRPCManager().SendRPC("Depositary_System", "RequestResponse", new Param1<ref array<ref VehicleData>>(playerData.vehicleData), true, sender);
			}
        }
    }

    /////////////////////////////////////////////////////HELPER///////////////////////////////////////////////////////////
    void SpawnNPCs()
    {
		//Todo: add an hologram for parkin / park out position here.
        for(int i = 0; i < m_Settings.NPCConfig.Count(); i++)
        {
            vector spawnPos;
            vector objectOrientation;
            spawnPos[0] = m_Settings.NPCConfig[i].NPCSpawnPosX;
            spawnPos[1] = m_Settings.NPCConfig[i].NPCSpawnPosY;
            spawnPos[2] = m_Settings.NPCConfig[i].NPCSpawnPosZ;
            objectOrientation[0] = m_Settings.NPCConfig[i].NPCSpawnYaw;
            objectOrientation[1] = m_Settings.NPCConfig[i].NPCSpawnPitch;
            objectOrientation[2] = m_Settings.NPCConfig[i].NPCSpawnRoll;
            Object obj = Object.Cast(GetGame().CreateObject(m_Settings.NPCConfig[i].NPCCharacterName, spawnPos, false, false, true));
            if(!obj)
                continue;
				if(m_Settings.IsLoggingActiv)
					GetGarageLogger().Log("[GarageSystem] ->  " + m_Settings.NPCConfig[i].NPCCharacterName + " Spawned on: " + spawnPos.ToString());
            obj.SetPosition(spawnPos);
            obj.SetOrientation(objectOrientation);
            PlayerBase man;
            if (Class.CastTo(man, obj))
		    {
			    man.SetAllowDamage(false);
			    man.m_Depositary_IsGarageNPC = true;
				man.m_GarageID = m_Settings.NPCConfig[i].GarageID;
                ref TStringArray items = m_Settings.NPCConfig[i].NPCAttachments;

                for(int n = 0; n < items.Count(); n++)
                {
                    man.GetInventory().CreateInInventory(items[n]);
                    if(m_Settings.IsLoggingActiv)
						GetGarageLogger().Log("[GarageSystem] ->  " + m_Settings.NPCConfig[i].NPCCharacterName + " ItemAttached: " + items[n]);
                }
            }
			else
			{
				vector garage_pos;
				garage_pos[0] = m_Settings.NPCConfig[i].NPCSpawnPosX;
				garage_pos[1] = m_Settings.NPCConfig[i].NPCSpawnPosY;
				garage_pos[2] = m_Settings.NPCConfig[i].NPCSpawnPosZ;
				m_DummyClasses.Insert(new ref NPCDummyClass(m_Settings.NPCConfig[i].GarageID, garage_pos));
				if(m_Settings.IsLoggingActiv)
					GetGarageLogger().Log("[GarageSystem] ->  " + m_Settings.NPCConfig[i].NPCCharacterName + " NPC DUMMY CLASS FOUND!");
			}
			
			//new for Hologramms
			if(m_Settings.ShowParkinParkOutPos)
			{
				//Only do it if serveradmin allowed it.
				GetGarageLogger().Log("[GarageSystem] ->  Hologram Spawn Enabled! spawning!");
				vector HologramPos;
				HologramPos[0] = m_Settings.NPCConfig[i].CARSpawnPosX;
				HologramPos[1] = m_Settings.NPCConfig[i].CARSpawnPosY;
				HologramPos[2] = m_Settings.NPCConfig[i].CARSpawnPosZ;
				vector ParkPostionRoation;
				ParkPostionRoation[0] = m_Settings.NPCConfig[i].CARSpawnYaw;
				ParkPostionRoation[1] = m_Settings.NPCConfig[i].CARSpawnPitch;
				ParkPostionRoation[2] = m_Settings.NPCConfig[i].CARSpawnRoll;
				SpawnHologrammOnPosition(HologramPos, ParkPostionRoation);

			}
			else
			{
				GetGarageLogger().Log("[GarageSystem] ->  Hologram Spawn Disabled skipping!");
			}
        }
    }

	//!Spawns a hologramm for Parkin/Parkout Position!
	protected void SpawnHologrammOnPosition(vector HologramPosition, vector ParkOrientation)
	{
		if(!HologramPosition || !ParkOrientation)
			return;
		
		ItemBase g_hologram = ItemBase.Cast(GetGame().CreateObject(m_HologrammClassname, HologramPosition));
		if(!g_hologram) return;

		g_hologram.SetOrientation(ParkOrientation);

		GetGarageLogger().Log("[GarageSystem] ->  Spawned Hologram on Positon: " + HologramPosition.ToString());
	}

	//!Gets the current Ammount of Money on Player.
    protected int GetCurrencyAmountOnPlayer(PlayerBase player)
	{
		int currencyAmountOnPlayer = 0;
		
		array<EntityAI> inventory = new array<EntityAI>;
		player.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, inventory);
		
		
		ItemBase item;
		for (int i = 0; i < inventory.Count(); i++)
		{
			Class.CastTo(item, inventory.Get(i));
			if (item)
			{
				for (int j = 0; j < m_Settings.CurrencyConfig.Count(); j++)
				{
					if(item.GetType() == m_Settings.CurrencyConfig[j].Currency_Name)
					{
						if(GetItemQuantityMax(item) == 0)
						{
							currencyAmountOnPlayer += m_Settings.CurrencyConfig[j].Currency_Amount;
						}
						else
						{
							currencyAmountOnPlayer += GetItemQuantity(item) * m_Settings.CurrencyConfig[j].Currency_Amount;
						}
					}
				}
			}
		}
		return currencyAmountOnPlayer;
	}

    protected bool RemoveCurrencyFromPlayer(PlayerBase player, int amountToRemove)
	{
		if(GetCurrencyAmountOnPlayer(player) >=  amountToRemove)
		{
			int amountStillNeeded = amountToRemove;
			
			array<EntityAI> inventory = new array<EntityAI>;
			player.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, inventory);
			
			ItemBase item;
			
			int currencyValue;
			int quantityNeeded;
			for(int i = inventory.Count() - 1; i >= 0; i--)
			{
				for(int j = 0; j < m_Settings.CurrencyConfig.Count(); j++)
				{
					if(m_Settings.CurrencyConfig[j].Currency_Name == inventory.Get(i).GetType())
					{
						Class.CastTo(item, inventory.Get(i));
						if(item)
						{
							currencyValue = m_Settings.CurrencyConfig[j].Currency_Amount;
							quantityNeeded = amountStillNeeded / currencyValue;
							if(GetItemQuantityMax(item) == 0)
							{
								GetGame().ObjectDelete(item);
								if(quantityNeeded >= 1)
								{
									amountStillNeeded -= currencyValue;
								}
								else
								{
									AddCurrencyToPlayer(player, currencyValue - amountStillNeeded);							
								}
							}
							else
							{
								if(quantityNeeded >= GetItemQuantity(item))
								{
									amountStillNeeded -= GetItemQuantity(item) * currencyValue;
									GetGame().ObjectDelete(item);
								}
								else
								{
									SetItemAmount(item, GetItemQuantity(item) - quantityNeeded);
									amountStillNeeded -= quantityNeeded * currencyValue;
									
									if(amountStillNeeded < currencyValue)
									{
										if(GetItemQuantity(item) == 1)
										{
											GetGame().ObjectDelete(item);
										}
										else
										{
											SetItemAmount(item, GetItemQuantity(item) - 1);
										}
										AddCurrencyToPlayer(player, currencyValue - amountStillNeeded);
									}
								}
							}
						}
					}
				}
			}
			return true;
		}
		else
		{
			//Pay with bank!
			#ifdef ADVANCED_BANKING
			KR_JsonDatabaseHandler playerdata = KR_JsonDatabaseHandler.LoadPlayerData(player.GetIdentity().GetPlainId(), player.GetIdentity().GetName());
			if(playerdata)
			{
				playerdata.WitdrawMoney(amountToRemove);
				return true;
			}
			return false;
			#endif
			#ifdef DC_BANKING
			DC_BankingData playerdata = DC_BankingData.LoadPlayerData(player.GetIdentity().GetPlainId(), player.GetIdentity().GetName());
			if(playerdata)
			{
				int CurrentAmount = playerdata.GetOwnedCurrency();
				playerdata.SetOwnedCurrency(CurrentAmount - amountToRemove);
				playerdata.SavePlayerData(playerdata);
			}
			return false;
			#endif
		}
		return false;
	}

	//!Returns if Sender is an Admin
	protected bool isSenderAdmin(string streamID)
	{
		for(int i = 0; i < m_AdminList.Admins.Count(); i++)
		{
			if(streamID == m_AdminList.Admins[i])
				return true;
		}
		return false;
	}

	//!This returns the Vehicle Spawn pos from a sepcify Garage.
	protected vector GetVehicleSpawnPosByGarageID(int GarageID)
	{
		local vector vehpos = "0 0 0";
		for(int i = 0; i < m_Settings.NPCConfig.Count(); i++)
		{
			if(m_Settings.NPCConfig[i].GarageID == GarageID)
			{
				vehpos[0] = m_Settings.NPCConfig[i].CARSpawnPosX;
				vehpos[1] = m_Settings.NPCConfig[i].CARSpawnPosY;
				vehpos[2] = m_Settings.NPCConfig[i].CARSpawnPosZ;
			}
		}
		return vehpos;
	}

	//!This returns an vector with the SpawnOri. n from a Garage with the ID.
	protected vector GetVehicleSpawnOriByGarageID(int GarageID)
	{
		local vector vehori = "0 0 0";
		for(int i = 0; i < m_Settings.NPCConfig.Count(); i++)
		{
			if(m_Settings.NPCConfig[i].GarageID == GarageID)
			{
				vehori[0] = m_Settings.NPCConfig[i].CARSpawnYaw;
				vehori[1] = m_Settings.NPCConfig[i].CARSpawnPitch;
				vehori[2] = m_Settings.NPCConfig[i].CARSpawnRoll;
			}
		}
		return vehori;
	}

	//!This Gets the Quantity from an item.
    protected int GetItemQuantity(ItemBase item)
	{
		if(!item)
		{
			return 0;
		}
		
		if(item.IsMagazine())
		{
			Magazine magazineItem = Magazine.Cast(item);
			if(magazineItem)
			{
				return magazineItem.GetAmmoCount();
			}
		}
		return item.GetQuantity();
	}

	//!This adds Currency to Player.
    protected void AddCurrencyToPlayer(PlayerBase player, int amountToAdd)
	{
		int amountStillNeeded = amountToAdd;
		
		int maxQuantity;
		int currencyValue;
		int quantityNeeded;
		int quantityLeftToAdd;
		
		
		for(int i = 0; i < m_Settings.CurrencyConfig.Count(); i++)
		{
			currencyValue = m_Settings.CurrencyConfig[i].Currency_Amount;
			string cursname = m_Settings.CurrencyConfig[i].Currency_Name;
			quantityNeeded = amountStillNeeded / currencyValue;
			if(quantityNeeded > 0)
			{
				quantityLeftToAdd = AddCurrencyToInventory(player, cursname, quantityNeeded);
				amountStillNeeded -= (quantityNeeded - quantityLeftToAdd) * currencyValue;
			}
		}
	}

	//!This adds Currency to Players Inventory.
    protected int AddCurrencyToInventory(PlayerBase player, string Currency_Name, int quantityToAdd)
	{
		if(quantityToAdd <= 0)
		{
			return 0;
		}
		
		int quantityLeftToAdd = quantityToAdd;
		
		array<EntityAI> inventory = new array<EntityAI>;
		player.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, inventory);
		ItemBase currencyItem;
		
		int addableQuantity;
		
		for(int i = 0; i < inventory.Count(); i++) //Add currency to already in the inventory existing currency
		{
			if(inventory.Get(i).GetType() == Currency_Name)
			{
				Class.CastTo(currencyItem, inventory.Get(i));
				if(currencyItem)
				{
					addableQuantity = GetItemQuantityMax(currencyItem) - GetItemQuantity(currencyItem);
					if(addableQuantity > 0)
					{
						if(addableQuantity >= quantityLeftToAdd)
						{
							SetItemAmount(currencyItem, GetItemQuantity(currencyItem) + quantityLeftToAdd);
							quantityLeftToAdd = 0;
						}
						else
						{
							SetItemAmount(currencyItem, GetItemQuantityMax(currencyItem));
							quantityLeftToAdd -= addableQuantity;
						}
					}
					
					if(quantityLeftToAdd == 0)
					{
						return 0;
					}
				}
			}
		}
		
		EntityAI createdCurrencyEntity;
		int currencyItemMaxQuantity;
		InventoryLocation invLocation = new InventoryLocation();
		while(player.GetInventory().FindFirstFreeLocationForNewEntity(Currency_Name, FindInventoryLocationType.CARGO, invLocation)) //Create new currency in the inventory
		{
			createdCurrencyEntity = player.GetHumanInventory().CreateInInventory(Currency_Name);
			Class.CastTo(currencyItem, createdCurrencyEntity);
			if(currencyItem)
			{
				currencyItemMaxQuantity = GetItemQuantityMax(currencyItem);
				if(currencyItemMaxQuantity == 0)
				{
					SetItemAmount(currencyItem, 0);
					quantityLeftToAdd -= 1;
				}
				else
				{
					if(quantityLeftToAdd <= currencyItemMaxQuantity)
					{
						SetItemAmount(currencyItem, quantityLeftToAdd);
						quantityLeftToAdd = 0;
					}
					else
					{
						SetItemAmount(currencyItem, currencyItemMaxQuantity);
						quantityLeftToAdd -= currencyItemMaxQuantity;
					}
				}
				
				if(quantityLeftToAdd == 0)
				{
					return 0;
				}
			}
			else
			{
				break;
			}
		}
		
		if(!player.GetHumanInventory().GetEntityInHands()) //Create new currency in the hands of the player
		{
			createdCurrencyEntity = player.GetHumanInventory().CreateInHands(Currency_Name);
			Class.CastTo(currencyItem, createdCurrencyEntity);
			if(currencyItem)
			{
				currencyItemMaxQuantity = GetItemQuantityMax(currencyItem);
				if(currencyItemMaxQuantity == 0)
				{
					SetItemAmount(currencyItem, 0);
					quantityLeftToAdd -= 1;
				}
				else
				{
					if(quantityLeftToAdd <= currencyItemMaxQuantity)
					{
						SetItemAmount(currencyItem, quantityLeftToAdd);
						quantityLeftToAdd = 0;
					}
					else
					{
						SetItemAmount(currencyItem, currencyItemMaxQuantity);
						quantityLeftToAdd -= currencyItemMaxQuantity;
					}
				}
				
				if(quantityLeftToAdd == 0)
				{
					return 0;
				}
			}
		}
		return quantityLeftToAdd;
	}

	//returns an List which holds item what are merge able.
	array<ItemBase> GetMergeableItemsFromVehicleInventroy(EntityAI vehicle, string itemType, int amount, bool absolute = false)
	{
		array<ItemBase> mergableItems = new array<ItemBase>;

		array<EntityAI> itemsArray = new array<EntityAI>;		
		vehicle.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, itemsArray);

		ItemBase itemToCombine = ItemBase.Cast(GetGame().CreateObject(itemType, "0 0 0"));

		if (!itemToCombine)
			return new array<ItemBase>;

		SetItemAmount(itemToCombine, amount);

		ItemBase item;		
		for (int i = 0; i < itemsArray.Count(); i++)
		{
			Class.CastTo(item, itemsArray.Get(i));

			if (!item)
				continue;

			if (item.IsRuined())
				continue;

			if (itemToCombine.GetType() != item.GetType())
				continue;

			if (item.CanBeCombined(itemToCombine) && GetItemAmount(item) < GetItemMaxQuantity(item.GetType()))
			{
				amount -= GetItemMaxQuantity(item.GetType()) - GetItemAmount(item);
				SetItemAmount(itemToCombine, amount);
				mergableItems.Insert(item);
			}
		}

		GetGame().ObjectDelete(itemToCombine);

		if (absolute && amount > 0)
			return new array<ItemBase>;

		return mergableItems;
	}

	//!returns the max Quanity from an Item.
	int GetItemMaxQuantity(string itemClassname) // duplicate
	{
		TStringArray searching_in = new TStringArray;
		searching_in.Insert( CFG_MAGAZINESPATH  + " " + itemClassname + " count");
		//searching_in.Insert( CFG_WEAPONSPATH );
		searching_in.Insert( CFG_VEHICLESPATH + " " + itemClassname + " varQuantityMax");

		for ( int s = 0; s < searching_in.Count(); ++s )
		{
			string path = searching_in.Get( s );

			if ( GetGame().ConfigIsExisting( path ) )
			{
				return g_Game.ConfigGetInt( path );
			}
		}

		return 0;
	}

	//!Creates an Item in Vehicle
	EntityAI CreateItemInVehicleInventory(EntityAI vehicle, string itemType, int amount, PlayerBase player)
	{		
		EntityAI entity;

		array<ItemBase> mergeableItems = GetMergeableItemsFromVehicleInventroy(vehicle, itemType, amount);

		bool isMergingPossible = mergeableItems.Count() > 0;
		bool itemsAreLeftOverMerging = GetMergeableItemsFromVehicleInventroy(vehicle, itemType, amount, true).Count() == 0;
			if (isMergingPossible)
			{
				if (itemsAreLeftOverMerging)
				{
					entity = vehicle.SpawnEntityOnGroundPos(itemType, vehicle.GetPosition());
				}
				else
				{
					entity = vehicle.SpawnEntityOnGroundPos(itemType, vehicle.GetPosition());
				}
			}
			else
			{
				entity = vehicle.GetInventory().CreateInInventory(itemType);
			}
	
			if (!entity)
				return null;
	
			ItemBase item;
			Class.CastTo(item, entity);
	
			if (!item)
				return null;
			
			SetItemAmount(item, amount);
	
			for (int i = 0; i < mergeableItems.Count(); i++)
			{
				int mergeQuantity;
	
				if (GetItemAmount(mergeableItems.Get(i)) + GetItemAmount(item) <= GetItemMaxQuantity(mergeableItems.Get(i).GetType()))
					mergeQuantity = GetItemAmount(item);
				else
					mergeQuantity = GetItemMaxQuantity(mergeableItems.Get(i).GetType()) - GetItemAmount(mergeableItems.Get(i));
	
	
				SetItemAmount(mergeableItems.Get(i), GetItemAmount(mergeableItems.Get(i)) + mergeQuantity);
				SetItemAmount(item, GetItemAmount(item) - mergeQuantity);
				amount -= mergeQuantity;
	
				if (item)
				{
					if (GetItemAmount(item) <= 0 || amount <= 0)
					{
						GetGame().ObjectDelete(item);
	
						return null;
					}
				}
			}
		return item;
	}

	//!returns true or false if the vehicle inv can not store the item.
	bool canCreateItemInVehicleInventory(EntityAI vehicle, string itemType, int amount)
	{
		array<ItemBase> mergeableItems = GetMergeableItemsFromVehicleInventroy(vehicle, itemType, amount);
		if (mergeableItems.Count() > 0)
			return true;

		ItemBase item = ItemBase.Cast(GetGame().CreateObject(itemType, "0 0 0"));
		if (!item)
			return false;

		SetItemAmount(item, amount);
		if(vehicle.GetInventory().CanAddEntityToInventory(item))
		{
			GetGame().ObjectDelete(item);
			return true;
		}
		GetGame().ObjectDelete(item);

		return false;			
	}

	//!Sets the Ammount of an Item.
	bool SetItemAmount(ItemBase item, int amount)
	{
		if (!item)
			return false;

		if (amount == -1)
			amount = GetItemMaxQuantity(item.GetType());

		if (amount == -3)
			amount = 0;

		if (amount == -4)
			amount = 0;

		if (amount == -5)
			amount = Math.RandomIntInclusive(GetItemMaxQuantity(item.GetType()) * 0.5, GetItemMaxQuantity(item.GetType()));

		Magazine mgzn = Magazine.Cast(item);
				
		if( item.IsMagazine() )
		{
			if (!mgzn)
				return false;

			mgzn.ServerSetAmmoCount(amount);
		}
		else
		{
			item.SetQuantity(amount);
		}

		return true;
	}

	//!this returns you the first object from a park position if something is blocking the position.
    Object GetVehicleParkPosWithGarageID(int garageID)
	{
		vector size = "3 5 9";
		array<Object> excluded_objects = new array<Object>;
		array<Object> nearby_objects = new array<Object>;
		if (GetGame().IsBoxColliding(GetVehicleSpawnPosByGarageID(garageID), GetVehicleSpawnOriByGarageID(garageID), size, excluded_objects, nearby_objects))
		{
			for (int i = 0; i < nearby_objects.Count(); i++)
			{
					bool vehicleIsEmpty = true;

					Transport transport;
					Class.CastTo(transport, nearby_objects.Get(i));
					if (transport)
					{
						int crewSize = transport.CrewSize();
						for (int c = 0; c < crewSize; c++)
						{
							if (transport.CrewMember(c))
								vehicleIsEmpty = false;
						}
					}
					else
					{
						continue;
					}

					if (!vehicleIsEmpty)
						continue;

					return nearby_objects.Get(i);						
			}
		}
		return NULL;
	}

	Object GetVehicleParkPosWithCords(vector Position, vector Orientation)
	{
		vector size = "3 5 9";
		array<Object> excluded_objects = new array<Object>;
		array<Object> nearby_objects = new array<Object>;
		if (GetGame().IsBoxColliding(Position, Orientation, size, excluded_objects, nearby_objects))
		{
			for (int i = 0; i < nearby_objects.Count(); i++)
			{
					bool vehicleIsEmpty = true;

					Transport transport;
					Class.CastTo(transport, nearby_objects.Get(i));
					if (transport)
					{
						int crewSize = transport.CrewSize();
						for (int c = 0; c < crewSize; c++)
						{
							if (transport.CrewMember(c))
								vehicleIsEmpty = false;
						}
					}
					else
					{
						continue;
					}

					if (!vehicleIsEmpty)
						continue;

					return nearby_objects.Get(i);						
			}
		}
		return NULL;
	}

	//!Retrurns the Ammount of the current item.
	int GetItemAmount(ItemBase item) // duplicate
	{
		Magazine mgzn = Magazine.Cast(item);
				
		int itemAmount = 0;
		if( item.IsMagazine() )
		{
			itemAmount = mgzn.GetAmmoCount();
		}
		else
		{
			itemAmount = QuantityConversions.GetItemQuantity(item);
		}
		
		return itemAmount;
	}

    protected int GetItemQuantityMax(ItemBase item)
	{
		if(!item)
		{
			return 0;
		}
		
		if(item.IsMagazine())
		{
			Magazine magazineItem = Magazine.Cast(item);
			if(magazineItem)
			{
				return magazineItem.GetAmmoMax();
			}
		}
		return item.GetQuantityMax();
	}

    /** Instance Handling 
		You can just use ref Depositary_ServerManager m_GarageServerside;
		m_GarageServerside = Depositary_ServerManager.GetInstance();
		Use it: 
			bool isGarageAdmin = m_GarageServerside.IsSenderAdmin("ExampleSteamID");
			this example will return true if the player is an Garage Admin or not.
	**/
    static Depositary_ServerManager GetInstance()
    {
        if(!m_ServerManager)
            m_ServerManager = new Depositary_ServerManager();

        return m_ServerManager;
    }

    static void CleanInstance()
    {
        m_ServerManager = null;
    }
}

/*
slotID = InventorySlots.GetSlotIdFromString("Truck_01_Hood");
InventoryLocation inventoryLocation = new InventoryLocation;
inventoryLocation.SetAttachment(truck, truckHood, slotID);
*/