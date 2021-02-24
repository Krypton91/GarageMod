modded class ActionCheckPulse: ActionInteractBase
{
	override bool ActionCondition( PlayerBase player, ActionTarget target, ItemBase item )
	{
		PlayerBase ntarget = PlayerBase.Cast(  target.GetObject() );
		if(ntarget.m_Depositary_IsGarageNPC)
			return false;
		return super.ActionCondition(player, target, item);
	}
}