class DepositaryMenu extends UIScriptedMenu
{
    private TextListboxWidget                       m_CarList;
    private ButtonWidget                            m_BtnParkIn;
    private ButtonWidget                            m_BtnParkOut;
	private ButtonWidget							m_BtnYes;
	private ButtonWidget							m_BtnNo;
    private TextWidget                              m_MessageHeadline;
	private TextWidget                              m_OnYouMoney;
	private TextWidget                              m_CostToParkOut;
	private TextWidget								m_YesNoBoxHeadline;
	private TextWidget								m_YesNoBoxBodyText;
	
    private MultilineTextWidget                     m_MessageBody;
    private ItemPreviewWidget                       m_ItemPreviewWidget;
    private Widget              	                m_MessageWidget;
	private Widget									m_YesNoWidget;
	private ImageWidget								m_MainHud;
    
    protected EntityAI                              previewItem;
    bool                                    		m_MenuActiv;
    private bool                                    m_isPayWithBankinActiv = false;
	private bool									m_WillPayWithBank;
    private int                                     m_LastRowIndex;
    private int                                     m_MaxVehicleStore;
    private int                                     m_PreviewWidgetRotationX;
	private int                                     m_PreviewWidgetRotationY;
    private int                                     m_GarageID;
	private int 									m_CostsToParkOutVehicle;
	private int 									m_ActionID;
    private float                                   m_UiUpdateTimer = 0;
	private float									m_UiCooldownTimer = 0;
    private vector                                  m_GaragePos;
    private string                                  m_serverName;
    ref array< ref VehicleData >                    vehicleData;
    ref array<string>                               m_CarListBoxItemsClassnames;
	ref array<int>                                  m_CarListBoxIndexID;
    private vector                                  m_PreviewWidgetOrientation;

    void DepositaryMenu(int GarageID)
    {
        //ToDo: Register RPC
        if(GetGame().IsClient() && !GetGame().IsServer())
        {
            GetRPCManager().AddRPC("Depositary_System", "RequestResponse", this, SingleplayerExecutionType.Client);
            GetRPCManager().AddRPC("Depositary_System", "ConfigResponse", this, SingleplayerExecutionType.Client);
            GetRPCManager().AddRPC("Depositary_System", "UI_MessageRequest", this, SingleplayerExecutionType.Client);
            GetRPCManager().AddRPC("Depositary_System", "UI_QuitRequest", this, SingleplayerExecutionType.Client);
            m_GarageID = GarageID;
            vehicleData = new ref array<ref VehicleData>;
            m_CarListBoxItemsClassnames = new array<string>;
		    m_CarListBoxIndexID = new array<int>;
        }
    }
    override Widget Init()
    {
        
        layoutRoot              = GetGame().GetWorkspace().CreateWidgets( "Depositary_System/gui/GarageMenu.layout" );
        m_BtnParkIn             = ButtonWidget.Cast( layoutRoot.FindAnyWidget( "Btn_ParkIn" ) );
        m_BtnParkOut            = ButtonWidget.Cast( layoutRoot.FindAnyWidget( "Btn_ParkOut" ) );
		m_BtnYes				= ButtonWidget.Cast(layoutRoot.FindAnyWidget("Btn_Yes") );
		m_BtnNo					= ButtonWidget.Cast(layoutRoot.FindAnyWidget("Btn_No") );
        m_CarList               = TextListboxWidget.Cast(layoutRoot.FindAnyWidget("GarageList") );
        m_MessageWidget         = Widget.Cast(layoutRoot.FindAnyWidget("MessageWidget") );
		m_YesNoWidget     		= Widget.Cast(layoutRoot.FindAnyWidget("PanelYesNo") );
		m_MainHud				= ImageWidget.Cast(layoutRoot.FindAnyWidget("LayoutBackground") ); 
        m_MessageHeadline       = TextWidget.Cast(layoutRoot.FindAnyWidget("STATEOFMESSAGE") );
		m_OnYouMoney			= TextWidget.Cast(layoutRoot.FindAnyWidget("ONYouLabel") );
		m_CostToParkOut			= TextWidget.Cast(layoutRoot.FindAnyWidget("PriceToParkOutVehicle") );
		m_YesNoBoxHeadline		= TextWidget.Cast(layoutRoot.FindAnyWidget("HeadlineYesNo") );
        m_MessageBody           = MultilineTextWidget.Cast(layoutRoot.FindAnyWidget("ERRORBOX") );
		m_YesNoBoxBodyText      = MultilineTextWidget.Cast(layoutRoot.FindAnyWidget("InfoBoxYesNo") );
        m_MenuActiv = true;
        m_CarList.SelectRow(0);
		UpdateUI();
        layoutRoot.Show(false);
        return layoutRoot;
    }

    //////////////////////////////////////CLIENT RPC HANDLING//////////////////////////////
    void ConfigResponse(CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target)
	{
		Param2<string, int> params;
        if(!ctx.Read(params)) return;
        if(type == CallType.Client)
		{
            m_MaxVehicleStore = params.param2;
		}
	}
    void RequestResponse(CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target)
	{
		//Print("Response recived!");
        Param1<ref array<ref VehicleData>> params;
        if(!ctx.Read(params)) return;
        if(type == CallType.Client)
		{
			if(layoutRoot != null)
			{
	           vehicleData =  params.param1;
	           m_CarListBoxItemsClassnames.Clear();
	           m_CarListBoxIndexID.Clear();
	           for(int i = 0; i < vehicleData.Count(); i++)
	           {
	               m_CarListBoxItemsClassnames.Insert(vehicleData[i].VehiclesName);
	               m_CarListBoxIndexID.Insert(vehicleData[i].indexID);
	           }
	           ReloadGarage();
			}
		}
    }
    void UI_QuitRequest(CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target)
	{
        if(type == CallType.Client)
		{
			if(layoutRoot)
           		GetGame().GetUIManager().HideScriptedMenu(this);
		}
    }
    protected void RequestParkedVehicles()
	{
		GetRPCManager().SendRPC("Depositary_System", "GarageDataRequest", null, true);
	}
    void SetHeadlineColor(int color)
    {
        m_MessageHeadline.SetColor(color);
    }
    void UI_MessageRequest(CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target)
    {
        Param3<string, string, int> params;
        if(!ctx.Read(params)) return;
        if(type == CallType.Client)
		{
			GarageSystemMessage(params.param1, params.param2, params.param3);
        }
    }
    void handleParkOutRequest(int IndexID, bool PayWithBank = false)
    {
        GetRPCManager().SendRPC("Depositary_System", "VehicleParkOutRequest", new Param3<int, int, bool>(m_GarageID, IndexID, PayWithBank), true);
		m_WillPayWithBank = false;
    }
    private void ReloadGarage()
    {
        m_CarList.ClearItems();

        for(int i = 0; i < m_CarListBoxItemsClassnames.Count(); i++)
        {
            m_CarList.AddItem(getItemDisplayName(m_CarListBoxItemsClassnames.Get(i)), NULL, 0 );
            m_CarList.SetItem( i, "" + m_CarListBoxIndexID.Get(i), 				NULL, 1 );
        }
    }
    protected void handleParkInRequest(bool PayWithBank = false)
    {
        GetRPCManager().SendRPC("Depositary_System", "VehicleParkinRequest", new Param2<int, bool>(m_GarageID, PayWithBank), true);
    }

	override void OnShow()
	{
		RequestParkedVehicles();
		
		super.OnShow();
		
		PPEffects.SetBlurMenu(0.5);

		GetGame().GetInput().ChangeGameFocus(1);
		
		layoutRoot.Show(true);
		
		
		SetFocus( layoutRoot );

		GetGame().GetMission().PlayerControlDisable(INPUT_EXCLUDE_ALL);

        GetGame().GetUIManager().ShowUICursor( true );
	}

    override void OnHide()
	{
		super.OnHide();

		PPEffects.SetBlurMenu(0);

		GetGame().GetInput().ResetGameFocus();

		if ( previewItem ) 
		{
			GetGame().ObjectDelete( previewItem );
		}

        layoutRoot.Show(false);

		GetGame().GetMission().PlayerControlEnable(false);

        GetGame().GetUIManager().ShowUICursor( false );

		Close();
    }
    override void Update(float timeslice)
	{
       super.Update(timeslice);

		if(m_UiCooldownTimer > 0)
			m_UiCooldownTimer -= timeslice;
		
        if (m_UiUpdateTimer >= 0.05)
		{
            local int row_index = m_CarList.GetSelectedRow();
            if(row_index != m_LastRowIndex)
            {
                m_LastRowIndex = row_index;
                if(!m_CarListBoxItemsClassnames.Get(row_index))
				{
					m_UiUpdateTimer = 0;
					return;
				}
                string itemType = m_CarListBoxItemsClassnames.Get(row_index);
                UpdateItemPreview(itemType);
            }
        }
        else
		{
			m_UiUpdateTimer = m_UiUpdateTimer + timeslice;
		}
        if(!m_MenuActiv)
                GetGame().GetUIManager().HideScriptedMenu(this);
    }
    bool OnMouseButtonDown(Widget w, int x, int y, int button)
	{
		super.OnMouseButtonDown(w, x, y, button);
		
		if (w == m_ItemPreviewWidget)
		{
			GetGame().GetDragQueue().Call(this, "UpdateRotation");
			g_Game.GetMousePos(m_PreviewWidgetRotationX, m_PreviewWidgetRotationY);
			return true;
		}
		return false;
	}
    void UpdateRotation(int mouse_x, int mouse_y, bool is_dragging)
	{
		vector o = m_PreviewWidgetOrientation;
		o[0] = o[0] + (m_PreviewWidgetRotationY - mouse_y);
		o[1] = o[1] - (m_PreviewWidgetRotationX - mouse_x);
		
		m_ItemPreviewWidget.SetModelOrientation( o );
		
		if (!is_dragging)
		{
			m_PreviewWidgetOrientation = o;
		}
	}
	protected void BuildMessage(string headline, string MessageIndex)
	{
		if(!m_MessageWidget.IsVisible())
		{
			GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(this.DestroyUIMessage, 5 * 1000, false);
			m_MessageWidget.Show(true);
			m_MessageHeadline.SetText(headline);
			m_MessageBody.SetText(MessageIndex);
		}
		else
		{
			GetGame().GetCallQueue(CALL_CATEGORY_GUI).Remove(this.DestroyUIMessage);
			m_MessageHeadline.SetText(headline);
			
			m_MessageBody.SetText(MessageIndex);
			
			GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(this.DestroyUIMessage, 5 * 1000, false);
		}
		
	}
	protected void GarageSystemMessage(string messageHeadline, string MessageBody, int message_type)
	{
		switch(message_type)
		{
			//ERROR
			case 1:
				SetHeadlineColor(ARGB(255, 255, 0, 0));
				break;
			case 2:
				SetHeadlineColor(ARGB(255, 34, 171, 13));
				break;
			case 3:
				SetHeadlineColor(ARGB(255, 255, 227, 31));
				break;
		}
		BuildMessage(messageHeadline, MessageBody);
	}
    void UpdateItemPreview(string itemType)
    {
        if ( !m_ItemPreviewWidget )
			{
				Widget preview_frame = layoutRoot.FindAnyWidget("CarPreview");

				if ( preview_frame ) 
				{
					float width;
					float height;
					preview_frame.GetSize(width, height);
					m_ItemPreviewWidget = ItemPreviewWidget.Cast( GetGame().GetWorkspace().CreateWidget(ItemPreviewWidgetTypeID, 0, 0, 1, 1, WidgetFlags.VISIBLE, ARGB(255, 255, 255, 255), 10, preview_frame) );
				}
			}

			if ( previewItem  )
				GetGame().ObjectDelete( previewItem  );

			previewItem  =  EntityAI.Cast(GetGame().CreateObject( itemType, "0 0 0", true, false, true ));
			m_ItemPreviewWidget.SetItem( previewItem  );
			m_ItemPreviewWidget.SetModelPosition( Vector(0,0,0.5) );

			float itemx, itemy;		
			m_ItemPreviewWidget.GetPos(itemx, itemy);

			m_ItemPreviewWidget.SetSize( 1.5, 1.5 );

			// align to center 
			m_ItemPreviewWidget.SetPos( -0.225, -0.225 );
    }
    override bool OnClick( Widget w, int x, int y, int button )
	{
        super.OnClick(w, x, y, button);
		
			
        local int row_index = m_CarList.GetSelectedRow();

        PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
		

        if(w == m_BtnParkIn)
        {
			if(m_UiCooldownTimer > 0)
			{
				GarageSystemMessage("#garage_UI_Message_ToFast", "#garage_UI_Message_ClickToFast", 3);
				return true;
			}
			#ifdef DC_BANKING
			if(NeedPayWithBankAccount(1) && !m_isPayWithBankinActiv && Depositary_ClientManager.CanPayWithBankAccount)
			{
				m_ActionID = 2;
				handleYesNoAction();
				return true;
			}
			#endif
			if(!Depositary_ClientManager.KeepInventory)
			{
				m_ActionID = 3;
				handleYesNoAction();
				return true;
			}
			
			else
			{
				m_UiCooldownTimer = Depositary_ClientManager.CooldownForGarage;
	            handleParkInRequest();
				HideYesNoMessage();
				UpdateUI();
				RequestParkedVehicles();
	            return true;
			}
        }
        if(w == m_BtnParkOut)
        {
			if(m_UiCooldownTimer > 0)
			{
				GarageSystemMessage("#garage_UI_Message_ToFast", "#garage_UI_Message_ClickToFast", 3);
				return true;
			}
			if(m_CarList.GetSelectedRow() == -1)
			{
				GarageSystemMessage("#garage_UI_Message_WARNING", "#garage_UI_Message_NoCarSelected", 3);
				return true;
			}
			#ifdef DC_BANKING
			if(NeedPayWithBankAccount(2) && !m_isPayWithBankinActiv && Depositary_ClientManager.CanPayWithBankAccount)
			{
				m_ActionID = 1;
				handleYesNoAction();
			}
			#endif
			else
			{
				HideYesNoMessage();
				m_UiCooldownTimer = Depositary_ClientManager.CooldownForGarage;
	            handleParkOutRequest(row_index);
				RequestParkedVehicles();
	            return true;
			}
        }
		if(w == m_BtnNo)
		{
			HideYesNoMessage();
			return true;
		}
		if(w == m_BtnYes)
		{
			//player.MessageStatus("Will mit bank zahlen!");
			m_UiCooldownTimer = Depositary_ClientManager.CooldownForGarage;
			handleYesButtonClick(row_index);
			RequestParkedVehicles();
			//HideYesNoMessage();
			return true;
		}
        return false;
    }
    string getItemDisplayName(string itemClassname)
	{
		TStringArray itemInfos = new TStringArray;
		
		string cfg = "CfgVehicles " + itemClassname + " displayName";
		string displayName;
		GetGame().ConfigGetText(cfg, displayName);
	
		if (displayName == "")
		{
			cfg = "CfgAmmo " + itemClassname + " displayName";
			GetGame().ConfigGetText(cfg, displayName);
		}
		
		if (displayName == "")
		{
			cfg = "CfgMagazines " + itemClassname + " displayName";
			GetGame().ConfigGetText(cfg, displayName);
		}
		
		if (displayName == "")
		{
			cfg = "cfgWeapons " + itemClassname + " displayName";
			GetGame().ConfigGetText(cfg, displayName);
		}
	
		if (displayName == "")
		{
			cfg = "CfgNonAIVehicles " + itemClassname + " displayName";
			GetGame().ConfigGetText(cfg, displayName);
		}
		
		
		if (displayName != "")
			return TrimUntPrefix(displayName);
		else
			return itemClassname;
	}
	protected void handleYesButtonClick(int row_index)
	{
		if(m_ActionID == 2)
		{
			handleParkInRequest(true);
			HideYesNoMessage();
		}
		if(m_ActionID == 1)
		{
			if(m_CarList.GetSelectedRow() == -1)
			{
				GarageSystemMessage("#garage_UI_Message_WARNING", "#garage_UI_Message_NoCarSelected", 3);
				return;
			}
			handleParkOutRequest(row_index, true);
		}
		if(m_ActionID == 3)
		{
			//TODO Check again currency on Player ask him for banking.
			if(NeedPayWithBankAccount(1) && !m_isPayWithBankinActiv && Depositary_ClientManager.CanPayWithBankAccount)
			{
				m_ActionID = 2;
				handleYesNoAction();
			}
			else
			{
				HideYesNoMessage();
				handleParkInRequest();
			}
		}
	}
	protected bool NeedPayWithBankAccount(int action)
	{
		if(action == 1)
		{
			if(getPlayerCurrencyAmount() < Depositary_ClientManager.CostsToParkInVehicle)
			{
				return true;
			}
		}
		else
		{
			if(getPlayerCurrencyAmount() < Depositary_ClientManager.CostsToParkOutVehicle)
			{
				return true;
			}
		}
		return false;
	}
    protected void UpdateUI()
    {
		m_CostToParkOut.SetText(Depositary_ClientManager.CostsToParkOutVehicle.ToString());
		int MoneyOnPlayer = getPlayerCurrencyAmount();
		m_OnYouMoney.SetText(MoneyOnPlayer.ToString());
		
    }
	int getPlayerCurrencyAmount() // duplicate
	{
		PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
		
		int currencyAmount = 0;
		
		array<EntityAI> itemsArray = new array<EntityAI>;
		player.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, itemsArray);

		ItemBase item;
		
		for (int i = 0; i < itemsArray.Count(); i++)
		{
			Class.CastTo(item, itemsArray.Get(i));

			if (!item)
				continue;

			for (int j = 0; j < Depositary_ClientManager.currencyData.Count(); j++)
			{
				if(item.GetType() == Depositary_ClientManager.currencyData[j].Currency_Name)
				{
					currencyAmount += getItemAmount(item) * Depositary_ClientManager.currencyData[j].Currency_Amount;
				}
			}
		}
		
		return currencyAmount;
	}
	protected void handleYesNoAction()
	{
		switch(m_ActionID)
		{
			case 1:
				BuildYesNoMessage("#garage_UI_Message_ParkingOutWithBank", "#garage_UI_Message_NotEnoughCashOnPlayer"); 
				break;
			case 2:
				BuildYesNoMessage("#garage_UI_Message_ParkingInWithBank", "#garage_UI_Message_NotEnoughCashOnPlayer"); 
				break;
			case 3:
				BuildYesNoMessage("#garage_UI_Message_NoInventory", "#garage_UI_Message_WarningInvWipe");
				break;
		}
	}
	protected void BuildYesNoMessage(string Headline, string BodyMessage)
	{
		m_YesNoWidget.Show(true);
		m_YesNoBoxHeadline.SetText(Headline);
		m_YesNoBoxBodyText.SetText(BodyMessage);
		if(m_MainHud.IsVisible())
		{
			m_MainHud.Show(false);
			if(previewItem)
				GetGame().ObjectDelete(previewItem);
		}
	}
	int getItemAmount(ItemBase item)
	{
		Magazine mgzn = Magazine.Cast(item);
				
		int itemAmount = 0;
		if( item.IsMagazine() )
		{
			itemAmount = mgzn.GetAmmoCount();
		}
		else
		{
			itemAmount = QuantityConversions.GetItemQuantity(item);
		}
		
		return itemAmount;
	}
	void ShowYesNoMessage()
	{
		if(!m_YesNoWidget.IsVisible())
		{
			m_YesNoWidget.Show(true);
			m_isPayWithBankinActiv = true;
		}
	}
	void HideYesNoMessage()
	{
		if(m_YesNoWidget.IsVisible())
		{
			m_YesNoWidget.Show(false);
			m_isPayWithBankinActiv = false;
			if(!m_MainHud.IsVisible())
				m_MainHud.Show(true);
		}
	}
    void DestroyUIMessage()
    {
        if(m_MessageWidget)
        {
            m_MessageWidget.Show(false);
            m_MessageHeadline.SetText("");
            m_MessageBody.SetText("");
        }
    }
    string TrimUntPrefix(string str)
	{
		str.Replace("$UNT$", "");
		return str;
	}
};