class GarageModStaticObjectParkingNew extends Inventory_Base
{
    protected int hash = 0;
    void GarageModStaticObjectParkingNew()
    {
        if(GetGame().IsServer())
        {
            if(GetGarageModConfig().ParkingMachineLifeTimeIsUnlimited)
            {
                //SetLifetime again to 999999 
            }
        }
    }

    int GetGarageNumber()
    {
        return hash;
    }

    void VehicleKeyBase()
    {
        RegisterNetSyncVariableInt( "hash", 0, int.MAX - 1);
    }

    override void OnStoreSave( ParamsWriteContext ctx )
	{   
		super.OnStoreSave( ctx );

        ctx.Write( hash );
	}
	
	override bool OnStoreLoad( ParamsReadContext ctx, int version )
	{
		if ( !super.OnStoreLoad( ctx, version ) )
			return false;
		
		if ( !ctx.Read( hash ) )
		{
            hash = 0;
		}

        Synchronize();
		
		return true;
	}

    int GenerateNewHash()
    {
        if ( GetGame().IsServer() )
        {
            generateHash();
        }

        return hash;
    }

    int SetNewHash(int newHash)
    {
        if ( GetGame().IsServer() )
        {
            hash = newHash;
            Synchronize();
        }

        return hash;
    }

    int GetHash()
    {
        return hash;
    }

    protected void generateHash()
    {
        if (hash <= 0)
        {
            hash = Math.RandomIntInclusive(1, int.MAX - 1);
        }

        Synchronize();
    }

    protected void Synchronize()
	{
		if (GetGame().IsServer())
        {
			SetSynchDirty();
        }
    }
}