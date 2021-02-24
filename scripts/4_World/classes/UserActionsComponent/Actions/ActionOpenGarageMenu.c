class ActionOpenGarageMenu: ActionInteractBase
{
	void ActionOpenGarageMenu()
	{
		m_CommandUID = DayZPlayerConstants.CMD_ACTIONMOD_INTERACTONCE;
		m_StanceMask = DayZPlayerConstants.STANCEMASK_ERECT | DayZPlayerConstants.STANCEMASK_CROUCH;
		m_HUDCursorIcon = CursorIcons.CloseHood;
	}

    override void CreateConditionComponents()  
	{
		m_ConditionTarget = new CCTObject(10);//CCTMan(10);
		m_ConditionItem = new CCINone;
	}

	override string GetText()
	{
		return "Garage";
	}

	override bool ActionCondition( PlayerBase player, ActionTarget target, ItemBase item )
	{
        if (GetGame().IsServer())
            return true;
		bool isGarageObject = false;
		if(Depositary_ClientManager.m_DUMMYS)
		{
			for(int i = 0; i < Depositary_ClientManager.m_DUMMYS.Count(); i++)
			{
				if(target.GetObject().GetPosition() == Depositary_ClientManager.m_DUMMYS[i].GaragesPos)
				{
					isGarageObject = true;
				}
			}
		}
		if(!target || !target.GetObject() || !player || !hasServerConfig)
			return false;		

		PlayerBase ntarget = PlayerBase.Cast(target.GetObject());
		bool isGarageNPCCharacter = false;
		if(ntarget)
			isGarageNPCCharacter = ntarget.m_Depositary_IsGarageNPC;
					
		if (!isGarageNPCCharacter && !isGarageObject)
			return false;

        return true;
	}
    
    override void OnStartClient(ActionData action_data)
    {
        handleOpenGarageMenuOpenRequest(action_data);
    }
	
    void handleOpenGarageMenuOpenRequest(ActionData action_data)
	{
		PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
		if(hasServerConfig)
		{
			
			if ( g_Game.GetUIManager().GetMenu() == NULL )
			{	
				PlayerBase man;
	            if (Class.CastTo(man, action_data.m_Target.GetObject()))
			    {
					GetGame().GetUIManager().CloseAll();
					int GarageID = man.m_GarageID;
					player.m_DepositaryMenu = new DepositaryMenu(GarageID);
			    	player.m_DepositaryMenu.Init();
					GetGame().GetUIManager().ShowScriptedMenu( player.m_DepositaryMenu, NULL );
				}
				else
				{
					for(int i = 0; i < Depositary_ClientManager.m_DUMMYS.Count(); i++)
					{
						if(action_data.m_Target.GetObject().GetPosition() == Depositary_ClientManager.m_DUMMYS[i].GaragesPos)
						{		
							GetGame().GetUIManager().CloseAll();
							GarageID = Depositary_ClientManager.m_DUMMYS[i].GarageID;
							player.m_DepositaryMenu = new DepositaryMenu(GarageID);
			    			player.m_DepositaryMenu.Init();
							GetGame().GetUIManager().ShowScriptedMenu( player.m_DepositaryMenu, NULL );
						}
					}
				}			
			}
		}
		else
		{
			player.MessageStatus("[GarageSystem] Server Config not loadet! Please wait for Response!");
		}
	}
};
