modded class PlayerBase
{
    bool m_Depositary_IsGarageNPC = false;
    int  m_GarageID = -1;
    ref DepositaryMenu m_DepositaryMenu;

    override void Init()
    {
        super.Init();

        RegisterNetSyncVariableBool("m_Depositary_IsGarageNPC");
        RegisterNetSyncVariableInt("m_GarageID");
    }
#ifdef DAYZ_1_09
    override void SetActions()
	{
		super.SetActions();

        AddAction(ActionOpenGarageMenu);
	}
#else
    override void SetActions(out TInputActionMap InputActionMap)
	{
        super.SetActions(InputActionMap);

         AddAction(ActionOpenGarageMenu, InputActionMap);
    }
#endif
}