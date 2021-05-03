class VehicleData
{
    int indexID;
	int VehiclesHash;
    int GarageID;
    float EngineHealth;
    float FuelAmmount;
    int m_oldID;
    string VehiclesName;
    vector SpawnPos;
    vector SpawnOri;
    ref array<ref VehicleCargo> m_Cargo;

    void VehicleData(string VHName, int indexid, int VHHash, int garageID, /*ref TStringArray cargo*/ ref array<ref VehicleCargo> this_cargo, float engine_health, float fuel_ammount, int oldID = 0)
    {
        indexID = indexid;
		VehiclesHash = VHHash;
        GarageID = garageID;
        VehiclesName = VHName;
		m_Cargo = this_cargo;
        EngineHealth = engine_health;
        FuelAmmount = fuel_ammount;
        m_oldID = oldID;
        //CargoItems = cargo;
    }

    void SetSpawnData(vector pos, vector ori)
    {
        SpawnPos = pos;
        SpawnOri = ori;
    }
};

//FOR ITEMS IN VEHICLE CARGO.
class VehicleCargo
{
	string ItemName;
	int VehicleCargoAmmount;
	int KeyHash;
    float Health;
    int m_oldID;
    int m_oldParentID;
    int m_itemCargoCoordinateRow;
    int m_itemCargoCoordinateCol;
    bool m_IsFlipped
    bool m_IsAttachment;
    int m_AttachmentSlotID;
    string m_AttachmentSlotName;

    void VehicleCargo(string CargoItemName, int CargoItemAmmount, float health, int keysHash = 0)
    {
		ItemName = CargoItemName;
		VehicleCargoAmmount = CargoItemAmmount;
        Health = health;
		KeyHash = keysHash;
	}

    void SetOldID(int oldID)
    {
        m_oldID = oldID;
    }

    void SetOldParentID(int oldParentID)
    {
        m_oldParentID = oldParentID;
    }

    void SetItemCargoCoordinateRow(int itemCargoCoordinateRow)
    {
        m_itemCargoCoordinateRow = itemCargoCoordinateRow;
    }

    void SetItemCargoCoordinateCol(int itemCargoCoordinateCol)
    {
        m_itemCargoCoordinateCol = itemCargoCoordinateCol;
    }

    void SetFlipped(bool isFlipped)
    {
        m_IsFlipped = isFlipped;
    }

    void SetAttachment(bool isAttachment)
    {
        m_IsAttachment = isAttachment;
    }

    void SetAttachmentSlotID(int attachmentSlotID)
    {
        m_AttachmentSlotID = attachmentSlotID;
    }

    void SetAttachmentSlotName(string attachmentSlotName)
    {
        m_AttachmentSlotName = attachmentSlotName;
    }

    int GetOldID()
    {
        return m_oldID;
    }

    int GetOldParentID()
    {
        return m_oldParentID;
    }

    int GetItemCargoCoordinateRow()
    {
        return m_itemCargoCoordinateRow;
    }

    int GetItemCargoCoordinateCol()
    {
        return m_itemCargoCoordinateCol;
    }

    bool IsFlipped()
    {
        return m_IsFlipped;
    }

    bool IsAttachment()
    {
        return m_IsAttachment;
    }

    int GetAttachmentSlotID()
    {
        return m_AttachmentSlotID ;
    }

    string GetAttachmentSlotName()
    {
        return m_AttachmentSlotName;
    }
}

enum DepositaryDataFormatVersion
{
    LEGACY,
    NEW01
}

class DepositaryData
{
    const protected static string m_ProfileDIR = "$profile:";
    const protected static string m_PlayerDataDIR = "Depositary_System/PlayerData";
 
    protected string m_SteamID;
    protected string m_PlayerName;
    
    ref array< ref VehicleData > vehicleData;

    static int m_FormatVersion;

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
		
		// JsonFileLoader<DepositaryData>.JsonSaveFile(m_ProfileDIR + m_PlayerDataDIR + "/" + playerData.GetID() + ".json", playerData);

        autoptr FileSerializer file = new FileSerializer();
        if (file.Open(m_ProfileDIR + m_PlayerDataDIR + "/" + playerData.GetID() + ".serialized", FileMode.WRITE))
        {
            file.Write(playerData);
            file.Close();

            // @TODO delete json version
        }
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

    void InsertNewVehicle(string VHName, int indexid, int VHHash, int GID,/*ref TStringArray cargo*/ ref array<ref VehicleCargo> this_cargo, float EngineHealth, float tank_fuelAmmount, int oldID)
    {
		//string VHName, int indexid, int VHHash, ref TStringArray cargo, int VHKeyHash
        vehicleData.Insert(new ref VehicleData(VHName, indexid, VHHash, GID, this_cargo, EngineHealth, tank_fuelAmmount, oldID));
    }

    void SetSpawnData(int index, vector pos, vector ori)
    {
        vehicleData.Get(index).SetSpawnData(pos, ori);
        SavePlayerData(this);
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
        
        if(FileExist(m_ProfileDIR + m_PlayerDataDIR + "/" + PlainID + ".serialized"))
        {
            m_FormatVersion = DepositaryDataFormatVersion.NEW01;

            FileSerializer file = new FileSerializer();

            if (file.Open(m_ProfileDIR + m_PlayerDataDIR + "/" + PlainID + ".serialized", FileMode.READ))
            {
                file.Read(playerData);
                file.Close();
            }
        }
        else if(FileExist(m_ProfileDIR + m_PlayerDataDIR + "/" + PlainID + ".json"))
        {
            m_FormatVersion = DepositaryDataFormatVersion.LEGACY;
            JsonFileLoader<DepositaryData>.JsonLoadFile(m_ProfileDIR + m_PlayerDataDIR + "/" + PlainID + ".json", playerData);
        }
        else
        {
            playerData = CreateDefaultPlayerData(PlainID, username);
        }

        if (username != "")
        {
            playerData.SetUsername(username);
        }

        return playerData;
    }
};