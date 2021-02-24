class GarageLogger extends PluginBase
{
    protected ref Depositary_Config                 m_Settings;
    protected const string  m_LogPath = "$profile:/Depositary_System/Logs/";
    string m_currentLogFileName = "";
    FileHandle  m_FileHandler;
    
    //TimeStamps
    int year, month, day, hour, minute, second;
	string sYear, sMonth, sDay, sHour, sMinute, sSecond, currentTime;

    void GarageLogger()
    {
        m_Settings = Depositary_Config.Load();
        if(!m_Settings.IsLoggingActiv)
            return;
        
        CheckLogPath();

        if(m_currentLogFileName == "")
        {
            m_currentLogFileName = "GarageSystem_" + GetTimeForLog() + ".log";
            if(FileExist(m_LogPath + m_currentLogFileName))
            {
                DeleteFile(m_LogPath + m_currentLogFileName);
            }
            //Create File......
            CreateLogFile();
        }
        
    }
    void CreateLogFile()
    {
        if(GetGame().IsServer())
        {
            m_FileHandler = OpenFile(m_LogPath + m_currentLogFileName, FileMode.WRITE);
            if(m_FileHandler != 0)
            {
                WriteFirstLine("[GarageLogger] startet! You can disable that in servers config json!");
            }
        }
    }
    void ~GarageLogger()
    {
        CloseFile(m_FileHandler);
    }
    void CheckLogPath()
    {
        if(!FileExist(m_LogPath))
        {
            MakeDirectory(m_LogPath);
        }
    }
    string GetTimeForLog()
    {
            // Setting Time Variables for new file name
			GetYearMonthDay(year, month, day);
			GetHourMinuteSecond(hour, minute, second);
			
			// Setting String for Time Variables for new file name
			sYear = year.ToString();
			
			sMonth = month.ToString();
			if (sMonth.Length() == 1)
			{
				sMonth = "0" + sMonth;
			}
			
			sDay = day.ToString();
			if (sDay.Length() == 1)
			{
				sDay = "0" + sDay;
			}
			
			sHour = hour.ToString();
			if (sHour.Length() == 1)
			{
				sHour = "0" + sHour;
			}
			
			sMinute = minute.ToString();
			if (sMinute.Length() == 1)
			{
				sMinute = "0" + sMinute;
			}
			
			sSecond = second.ToString();
			if (sSecond.Length() == 1)
			{
				sSecond = "0" + sSecond;
			}
			currentTime = "_" + sYear + "-" + sMonth + "-" + sDay + "_" + sHour + "-" + sMinute + "-" + sSecond;
            return currentTime;

    }
    string GetTimeForLogMessage()
    {
         // Setting Time Variables for new file name
			GetYearMonthDay(year, month, day);
			GetHourMinuteSecond(hour, minute, second);
			
			// Setting String for Time Variables for new file name
			sYear = year.ToString();
			
			sMonth = month.ToString();
			if (sMonth.Length() == 1)
			{
				sMonth = "0" + sMonth;
			}
			
			sDay = day.ToString();
			if (sDay.Length() == 1)
			{
				sDay = "0" + sDay;
			}
			
			sHour = hour.ToString();
			if (sHour.Length() == 1)
			{
				sHour = "0" + sHour;
			}
			
			sMinute = minute.ToString();
			if (sMinute.Length() == 1)
			{
				sMinute = "0" + sMinute;
			}
			
			sSecond = second.ToString();
			if (sSecond.Length() == 1)
			{
				sSecond = "0" + sSecond;
			}
			currentTime = sYear + ":" + sMonth + ":" + sDay + ":" + sHour + ":" + sMinute + ":" + sSecond;
            return currentTime;
    }
    void LogLine(string playername, string steamid, vector playerpos, string reason)
    {
        WriteToFile("Player: " + playername + " with steam64ID< " + steamid + "> " + reason + " on position: " + playerpos);
    }
    void Log(string text)
    {
        WriteToFile(text);
    }
    void WriteFirstLine(string text)
    {
        FPrintln(m_FileHandler, GetTimeForLogMessage() + " | " + text);
    }
    void WriteToFile(string text)
    {
        if(text == "")
            return;
        FPrintln(m_FileHandler, GetTimeForLogMessage() + " | " + text);
    }
};
GarageLogger GetGarageLogger() 
{
	if (GetGame().IsServer())
    {
        return GarageLogger.Cast(GetPlugin(GarageLogger));
    }
	return null;
};