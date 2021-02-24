const protected static string m_AdminProfileDIR = "$profile:";
const protected static string m_AdminConfigDIR = "Depositary_System";
const protected static string m_AdminConfigPath = m_ProfileDIR + m_ConfigDIR + "/" + "AdminConfig.json";
class Depositary_AdminConfig
{

    ref TStringArray Admins;
	ref TStringArray VehicleBlackList;
    //Constructor Load Config there.
    void Depositary_AdminConfig()
    {
        Admins = new TStringArray();
        VehicleBlackList = new TStringArray();
    }

    void LoadDefaultSettings()
    {
		Admins.Insert("76561198796326626");
        VehicleBlackList.Insert("Hatchback_02");

        SaveSettings();
    }

    void SaveSettings()
    {
       if (!FileExist(m_AdminProfileDIR + m_AdminConfigDIR + "/"))
			MakeDirectory(m_AdminProfileDIR + m_AdminConfigDIR + "/");

         JsonFileLoader<Depositary_AdminConfig>.JsonSaveFile(m_AdminConfigPath, this);
    }

    static ref Depositary_AdminConfig Load()
    {
        ref Depositary_AdminConfig settings = new Depositary_AdminConfig();

        if(FileExist(m_AdminConfigPath))
        {
            JsonFileLoader<Depositary_AdminConfig>.JsonLoadFile(m_AdminConfigPath, settings);
        }
        else
        {
            settings.LoadDefaultSettings();
        }
        return settings;
    }
};