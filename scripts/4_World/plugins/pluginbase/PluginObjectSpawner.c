class ObjectSpawner extends PluginBase
{
    protected const string  m_ObjectsDIR = "$profile:/Depositary_System/Objects/";
    protected const string  m_ObjectsFile = "GarageObjects.obj";
    FileHandle  m_FileHandler;

    void ObjectSpawner()
    {
        if(!FileExist(m_ObjectsDIR))
        {
            MakeDirectory(m_ObjectsDIR);
        }

        if(FileExist(m_ObjectsDIR + m_ObjectsFile))
        {
            //LoadTheFIle
        }
        else
        {
            //CreateDefaultObjFile
			CreateDefaultObjFile();
        }
    }

    void CreateDefaultObjFile()
    {
		MakeDirectory(m_ObjectsDIR + m_ObjectsFile);
        m_FileHandler = OpenFile(m_ObjectsDIR + m_ObjectsFile, FileMode.WRITE);
		if ( m_FileHandler != 0 )
		{
			//SpawnObject( "bldr_Platform2_Block", "3729.850098 398.942230 5998.458496", "48.000004 0.000000 -2.900001" );
			FPrintln(m_FileHandler,  "\"bldr_Platform2_Block\" \" 3729.850098 398.942230 5998.458496\" \" 48.000004 0.000000 -2.900001\"");
		}
    }
}