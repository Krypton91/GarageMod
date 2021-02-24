modded class PluginManager
{
	//=================================
	// Init
	//=================================
	override void Init()
	{	
		//----------------------------------------------------------------------
		// Register modules
		//----------------------------------------------------------------------
		//				Module Class Name 						Client	Server
		//----------------------------------------------------------------------
		if (GetGame().IsServer() && GetGame().IsMultiplayer())
		{
			RegisterPlugin( "GarageLogger",						false, 	true );
		}
		super.Init();
	}
};