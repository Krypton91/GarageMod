class VehicleData
{
    int indexID;
	int VehiclesHash;
    int GarageID;
    float EngineHealth;
    float FuelAmmount;
    string VehiclesName;
    ref array<ref VehicleCargo> m_Cargo;
	
    void VehicleData(string VHName, int indexid, int VHHash, int garageID, /*ref TStringArray cargo*/ ref array<ref VehicleCargo> this_cargo, float engine_health, float fuel_ammount)
    {
        indexID = indexid;
		VehiclesHash = VHHash;
        GarageID = garageID;
        VehiclesName = VHName;
		m_Cargo = this_cargo;
        EngineHealth = engine_health;
        FuelAmmount = fuel_ammount;
        //CargoItems = cargo;
		
    }
};
//FOR ITEMS IN VEHICLE CARGO.
class VehicleCargo
{
	string ItemName;
	int VehicleCargoAmmount;
	int KeyHash;
    float Health;
	
	void VehicleCargo(string CargoItemName, int CargoItemAmmount, float health, int keysHash = 0)
	{
		ItemName = CargoItemName;
		VehicleCargoAmmount = CargoItemAmmount;
        Health = health;
		KeyHash = keysHash;
	}
}
class DepositaryData
{
    const protected static string m_ProfileDIR = "$profile:";
    const protected static string m_PlayerDataDIR = "Depositary_System/PlayerData";
    protected string m_SteamID;
    protected string m_PlayerName;
    ref array< ref VehicleData > vehicleData;

    void DepositaryData(string plainID = "", string username = "")
    {
        m_SteamID = plainID;
        m_PlayerName = username;
        vehicleData = new ref array<ref VehicleData>;
    }

    protected static DepositaryData CreateDefaultPlayerData(string id, string username = "")
	{
        
		DepositaryData playerData = new DepositaryData(id, username);
		
		SavePlayerData(playerData);
		
		return playerData;
	}
    static void SavePlayerData(DepositaryData playerData, string username = "")
	{
		if(!playerData)
		{
			return;
		}
		
		if(playerData.GetID() == "")
		{
			return;
		}
		
		if(username != "")
		{
			playerData.SetUsername(username);
		}
		
		JsonFileLoader<DepositaryData>.JsonSaveFile(m_ProfileDIR + m_PlayerDataDIR + "/" + playerData.GetID() + ".json", playerData);
	}
    string GetID()
	{
		return m_SteamID;
	}
	string GetUsername()
	{
		return m_PlayerName;
	}
    VehicleData GetVehicleData(int index)
    {
        for(int i = 0; i < vehicleData.Count(); i++)
        {
            if(vehicleData[i].indexID == index)
                return vehicleData[i];
            
        }
        return null;
    }
    void SortVehicleIDs()
    {
        for(int i = 0; i < vehicleData.Count(); i++)
        {
            vehicleData[i].indexID = i;
        }
    }
    int GetNextVehicleIndex()
    {
        if(vehicleData.Count() == 0)
            return 0;
        SortVehicleIDs();
        return vehicleData.Count();
    }
    void SetID(string id)
	{
		m_SteamID = id;
	}
	void SetUsername(string playerUsername)
	{
		m_PlayerName = playerUsername;
	}
    void InsertNewVehicle(string VHName, int indexid, int VHHash, int GID,/*ref TStringArray cargo*/ ref array<ref VehicleCargo> this_cargo, float EngineHealth, float tank_fuelAmmount)
    {
		//string VHName, int indexid, int VHHash, ref TStringArray cargo, int VHKeyHash
        vehicleData.Insert(new ref VehicleData(VHName, indexid, VHHash, GID, this_cargo, EngineHealth, tank_fuelAmmount));
        
    }
	int AmountOfParkedVehicles()
	{
		int parkedvehicles = 0;
		for(int i = 0; i < vehicleData.Count(); i++)
		{
			parkedvehicles++;
		}
		return parkedvehicles;
	}
    static DepositaryData LoadPlayerData(string PlainID, string username = "")
    {
        if(PlainID == "")
        {
            return null;
        }

        DepositaryData playerData = new DepositaryData();

        if(!FileExist(m_ProfileDIR + m_PlayerDataDIR + "/"))
            MakeDirectory(m_ProfileDIR + m_PlayerDataDIR + "/");
        
        if(FileExist(m_ProfileDIR + m_PlayerDataDIR + "/" + PlainID + ".json"))
        {
            JsonFileLoader<DepositaryData>.JsonLoadFile(m_ProfileDIR + m_PlayerDataDIR + "/" + PlainID + ".json", playerData);
            //User Changed name, no problem on file Load it will change to aktuell name.
            if(username != "")
            {
                playerData.SetUsername(username);
            }
        }
        else
        {
            playerData = CreateDefaultPlayerData(PlainID, username);
        }
        return playerData;
    }
};