// #include "Scripts\Classes\Gui\ChatLine.c"

/*!
	channel type, possible values
	CCSystem(1)
	CCAdmin(2)
	CCDirect(4)
	CCMegaphone(8)
	CCTransmitter(16)
	CCPublicAddressSystem(32)
	CCBattlEye(64)
*/
modded class Chat
{
	ref TStringArray m_CommandParams;
	void Chat()
	{
		m_CommandParams = new TStringArray();
	}
	override void Add ( ChatMessageEventParams params )
	{
		int name_lenght = params.param2.Length( );
		int text_lenght = params.param3.Length( );
		int total_lenght = text_lenght + name_lenght;
		//@Param 1 = PlayerName
		//@Param 3 = ChatInput
		string ChatInput = params.param3;
		if(ChatInput.Contains("&Garage"))
		{
			m_CommandParams = GenerateCommandArray(ChatInput);
			handleGarageSystemCommands(m_CommandParams);
		}
		else
		{
			super.Add(params);
		}
	}
	protected void handleGarageSystemCommands(ref TStringArray args)
	{
		PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
		vector pos;
		vector p_dir;
		int argLengh = args.Count();
		if(!Depositary_ClientManager.IsGarageAdmin)
		{
			player.MessageStatus("[Garage MOD] Permission rejected!");
			return;
		}
		if(argLengh >= 2)
		{
			if(argLengh == 2)
			{
				if(args[1] == "help")
				{
					SendHelpMessage(player);
				}
				if(args[1] == "POS")
				{
					pos = player.GetPosition();
					GetGame().CopyToClipboard("X: " + pos[0] + " Y: " + pos[1] + " Z: " + pos[2]);
					player.MessageStatus("[Garage MOD] Position of Player Copyed to Clipboard!");
				}
				if(args[1] == "YAW")
				{
					p_dir = player.GetOrientation();
					GetGame().CopyToClipboard("YAW: " + p_dir[0] + " PITCH: " + p_dir[1] + " ROLL: " + p_dir[2]);
					player.MessageStatus("[Garage MOD] Orientation of Player Copyed to Clipboard!");
				}
				else
				{
					player.MessageStatus("[Garage MOD] Unkown Command, use &Garage help!");
				}
			}
		}
		else
		{
			SendHelpMessage(player);
		}
	}
	protected void SendHelpMessage(PlayerBase player)
	{
		player.MessageStatus("[Garage MOD] -------- Avaible Commands --------");
		player.MessageStatus("[Garage MOD] help -Shows this meessage!");
		player.MessageStatus("[Garage MOD] POS  -Copy The Position of your Player into Clipboard!");
		player.MessageStatus("[Garage MOD] YAW  -Copy The Direction of your Player into Clipboard!");
	}
	
	//!Returns an Array from our chat input!
	TStringArray GenerateCommandArray(string chatInput)
    {
        ref TStringArray commandParams = new TStringArray();
        chatInput.Split(" ", commandParams);
		return commandParams;
    }
	/*
	bool StartsWith(string text, string withWhat)
	{
		if(withWhat.Length() > 1) 
			return false; 
		if(text.Get(0) == withWhat)
			return true;
		return false;
	}
	*/
}
