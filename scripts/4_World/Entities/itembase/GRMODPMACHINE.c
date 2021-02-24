class GRMODPMACHINE extends Inventory_Base
{
    override bool CanPutIntoHands ( EntityAI player ) 
	{
        return false;
    }

    override bool CanPutInCargo( EntityAI parent )
	{
        return false;
    }
};