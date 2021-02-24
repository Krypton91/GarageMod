class Depositary_ClientManager
{
    static float CooldownForGarage;
    static int CostsToParkInVehicle;
    static int CostsToParkOutVehicle;
    static bool CanPayWithBankAccount;
	static bool KeepInventory;
	static bool IsGarageAdmin = false;
	static ref array< ref NPCDummyClass > m_DUMMYS;
	static ref array< ref Currency > currencyData;
	
    void Depositary_ClientManager()
    {
		currencyData = new ref array<ref Currency>;
		m_DUMMYS =  new ref array<ref NPCDummyClass>;
        if(GetGame().IsClient() && !GetGame().IsServer())
        {
            GetRPCManager().AddRPC("Depositary_System", "FULLCONFIGRES", this, SingleplayerExecutionType.Client);
            GetRPCManager().AddRPC("Depositary_System", "NPCDUMMYCLASSES", this, SingleplayerExecutionType.Client);
            GetRPCManager().SendRPC("Depositary_System", "FULLCONFIGREQ", null, true);
        }
    }
    /**
        Server -> Client Config Sync Start when mission Start!
		Response Handling from Server.
    **/
    void FULLCONFIGRES(CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target)
	{
		Param6<float, bool, int, int, bool, ref array<ref Currency>> data;
        if(!ctx.Read(data)) return;
        if(type == CallType.Client)
		{
            //Print("[GarageSystem] Server Config Recived!");
            CooldownForGarage = data.param1;
            //Print("[GarageSystem] Cooldown for Garage use: " + data.param1);
			KeepInventory = data.param2;
            //Print("[GarageSystem] Max Vehicles to Store: " + data.param2);
			CostsToParkInVehicle = data.param3;
            //Print("[GarageSystem] Costs to park a Vehicle: " + data.param3);
			CostsToParkOutVehicle = data.param4;
            //Print("[GarageSystem] Costs to park out a Vehicle: " + data.param4);
			CanPayWithBankAccount = data.param5;
            //Print("[GarageSystem] Can pay with bank account: " + data.param5);
			currencyData = data.param6;
			//for(int i = 0; i < currencyData.Count(); i++)
			//{
				//Print("[Garage System] Currency Name: " + currencyData[i].Currency_Name);
				//Print("[Garage System] Currency Ammount: " + currencyData[i].Currency_Amount);
			//}
			hasServerConfig = true;
		}
	}
	void NPCDUMMYCLASSES(CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target)
	{
		Param2<ref array<ref NPCDummyClass>, bool> data;
		if(!ctx.Read(data))return;
		if(type == CallType.Client)
		{
			m_DUMMYS = data.param1;
			IsGarageAdmin = data.param2;
		}
	}
}