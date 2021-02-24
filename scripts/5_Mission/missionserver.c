modded class MissionServer
{
    void MissionServer()
    {
		Depositary_ServerManager.CleanInstance();
        Depositary_ServerManager.GetInstance();
    }
};