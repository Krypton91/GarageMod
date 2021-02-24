class NPCSettings
{
    int GarageID;
    string NPCCharacterName;
    ref TStringArray NPCAttachments;
    float NPCSpawnPosX;
    float NPCSpawnPosY;
    float NPCSpawnPosZ;
    float NPCSpawnYaw;
    float NPCSpawnPitch;
    float NPCSpawnRoll;
    float CARSpawnPosX;
    float CARSpawnPosY;
    float CARSpawnPosZ;
    float CARSpawnYaw;
    float CARSpawnPitch;
    float CARSpawnRoll;

    void NPCSettings(int GID, string Name, ref TStringArray NPCAT, float NPCSpawn1, float NPCSpawn2, float NPCSpawn3, float NPCYaw, float NPCPitch, float NPCRoll, float CARSpawn1, float CARSpawn2, float CARSpawn3, float CarYaw, float CarPitch, float  CarRoll)
    {
       GarageID             = GID;
       NPCCharacterName     = Name;
       NPCAttachments       = NPCAT;
       NPCSpawn1            = NPCSpawnPosX;
       NPCSpawn2            = NPCSpawnPosY;
       NPCSpawn3            = NPCSpawnPosZ;
       NPCSpawnYaw          = NPCYaw;
       NPCSpawnPitch        = NPCPitch;
       NPCSpawnRoll         = NPCRoll;
       CARSpawnPosX         = CARSpawn1;
       CARSpawnPosY         = CARSpawn2;
       CARSpawnPosZ         = CARSpawn3;
       CARSpawnYaw          = CarYaw;
       CARSpawnPitch        = CarPitch;
       CARSpawnRoll         = CarRoll;

    }
};
class NPCDummyClass
{
	int GarageID;
	vector GaragesPos;
	
	void NPCDummyClass(int GID, vector GrgPos)
	{
		GarageID = GID;
		GaragesPos = GrgPos;
	}
}
class Currency
{
    string Currency_Name;
    int  Currency_Amount;
    void Currency(string currencyName, int Currencynumber)
    {
        Currency_Name = currencyName;
        Currency_Amount = Currencynumber;
    }
}
const protected static string m_ProfileDIR = "$profile:";
const protected static string m_ConfigDIR = "Depositary_System";
const protected static string m_ConfigPath = m_ProfileDIR + m_ConfigDIR + "/" + "DepositaryConfig.json";
class Depositary_Config
{

    float CooldownForGarage;
    int MaxVehiclesToStore;
    int CostsToParkInVehicle;
    int CostsToParkOutVehicle;
    int CostsToBuyVehicleKey;
    int CostsToRepairVehicle;
    int ServerRestartPeriods;
    bool CanPayWithBankAccount;
	bool KeepInventory;
    bool IsLoggingActiv;
    bool IsGarageGlobal;
    bool ShowParkinParkOutPos;
    bool SaveDamage;
    ref array< ref NPCSettings > NPCConfig;
    ref array< ref Currency > CurrencyConfig;
	ref TStringArray ObjectWhiteList;
    //Constructor Load Config there.
    void Depositary_Config()
    {
       NPCConfig = new ref array<ref NPCSettings>;
       CurrencyConfig = new ref array<ref Currency>;
    }

    void LoadDefaultSettings()
    {
        CooldownForGarage = 5;
        MaxVehiclesToStore = 5;
        CostsToParkInVehicle  = 1000;
        CostsToParkOutVehicle = 2000;
        CostsToBuyVehicleKey = 2000;
        CostsToRepairVehicle = 4000;
        ServerRestartPeriods = 3;
        CanPayWithBankAccount = true;
		KeepInventory = true;
        IsLoggingActiv = true;
        IsGarageGlobal = true;
        ShowParkinParkOutPos = true;
        SaveDamage = true;
        ref TStringArray NPCsGear = new TStringArray();
		TStringArray NPCsGear2 = new TStringArray();
        ObjectWhiteList = new TStringArray();
		ObjectWhiteList.Insert("Example Object");
        ObjectWhiteList.Insert("Another example Object");
        NPCsGear.Insert("ManSuit_Black");
        NPCsGear.Insert("Jeans_Black");
        NPCsGear.Insert("MilitaryBeret_NZ");
        NPCsGear.Insert("DressShoes_Black");  
		                      //ARRAY     X       Y       Z  YAWPITCHROLL    X   Y       Z       YAW PITCH ROLL
		NPCConfig.Insert(new ref NPCSettings(1, "SurvivorM_Taiki", NPCsGear, 3731, 402.54, 5994.96, 0, 0, 0, 3730.69, 402.016, 5999.32, -129.88, 0, 0));
        
		
		NPCsGear2.Insert("LugWrench");
		NPCsGear2.Insert("WorkingGloves_Black");
		NPCsGear2.Insert("ReflexVest");
		NPCsGear2.Insert("JumpsuitJacket_Blue");
		NPCsGear2.Insert("JumpsuitPants_Blue");
		NPCsGear2.Insert("HikingBootsLow_Blue");
		
        NPCConfig.Insert(new ref NPCSettings(2, "SurvivorM_Taiki", NPCsGear2, 11435.7, 329.584, 11364.1, 24.2163, 0, 0, 11444.6, 329.995, 11361.6, 54.3295, 0, 0));

        CurrencyConfig.Insert(new ref Currency("MoneyRuble1", 1));
        CurrencyConfig.Insert(new ref Currency("MoneyRuble5", 5));
        CurrencyConfig.Insert(new ref Currency("MoneyRuble10", 10));
        CurrencyConfig.Insert(new ref Currency("MoneyRuble25", 25));
        CurrencyConfig.Insert(new ref Currency("MoneyRuble50", 50));
        CurrencyConfig.Insert(new ref Currency("MoneyRuble100", 100));

        SaveSettings();
    }

    void SaveSettings()
    {
       if (!FileExist(m_ProfileDIR + m_ConfigDIR + "/"))
			MakeDirectory(m_ProfileDIR + m_ConfigDIR + "/");

         JsonFileLoader<Depositary_Config>.JsonSaveFile(m_ConfigPath, this);
    }

    static ref Depositary_Config Load()
    {
        ref Depositary_Config settings = new Depositary_Config();

        if(FileExist(m_ConfigPath))
        {
            JsonFileLoader<Depositary_Config>.JsonLoadFile(m_ConfigPath, settings);
        }
        else
        {
            settings.LoadDefaultSettings();
        }
        return settings;
    }
};