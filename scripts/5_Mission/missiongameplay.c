modded class MissionGameplay
{
    ref Depositary_ClientManager    m_clientManager;
    override void OnKeyRelease(int key)
	{
        super.OnKeyRelease(key);
	
		if ( key == KeyCode.KC_ESCAPE )
		{
            PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
			if(!player)
				return;
            if(hasServerConfig && player.m_DepositaryMenu)
                player.m_DepositaryMenu.m_MenuActiv = false;
        }	
    }
    override void OnInit()
    {
        super.OnInit();
        m_clientManager = new Depositary_ClientManager();
    }
}