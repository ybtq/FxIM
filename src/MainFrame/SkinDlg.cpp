#include "StdAfx.h"
#include "Utils/UICrack.h"
#include "MainFrame/SkinDlg.h"


SkinChangedObserver CSkinDlg::m_skinObserver;

CSkinDlg::CSkinDlg()
: m_pTLayoutSkin(NULL)
, m_pSliderColorR(NULL)
, m_pSliderColorG(NULL)
, m_pSliderColorB(NULL)
{
}

CSkinDlg::~CSkinDlg()
{
}

LPCTSTR CSkinDlg::GetWindowClassName() const
{
	return kWindowClassName;
}

void CSkinDlg::OnFinalMessage(HWND hWnd)
{
	WindowImplBase::OnFinalMessage(hWnd);
	delete this;
}


CDuiString CSkinDlg::GetSkinFile()
{
	return _T("xmls\\dlg_skin.xml");
}

CDuiString CSkinDlg::GetSkinFolder()
{
	return  _T("skin");
}

UILIB_RESOURCETYPE CSkinDlg::GetResourceType() const
{
	return UILIB_FILE;
}

void CSkinDlg::AddReceiver(SkinChangedReceiver* receiver)
{
	m_skinObserver.AddReceiver(receiver);
}

void CSkinDlg::InitWindow()
{
	Config*		pCfg = Singleton<Config>::getInstance();
	m_pTLayoutSkin = static_cast<CTabLayoutUI*>(m_PaintManager.FindControl(_T("tlayout_skin")));

	m_pSliderColorR = static_cast<CSliderUI*>(m_PaintManager.FindControl(_T("slider_color_r")));
	m_pSliderColorG = static_cast<CSliderUI*>(m_PaintManager.FindControl(_T("slider_color_g")));
	m_pSliderColorB = static_cast<CSliderUI*>(m_PaintManager.FindControl(_T("slider_color_b")));

	if (m_pSliderColorR) m_pSliderColorR->SetValue(GetRValue(pCfg->dwBgColor));
	if (m_pSliderColorG) m_pSliderColorG->SetValue(GetGValue(pCfg->dwBgColor));
	if (m_pSliderColorB) m_pSliderColorB->SetValue(GetBValue(pCfg->dwBgColor));
}

void CSkinDlg::Notify(TNotifyUI& msg)
{
	Config*		pCfg = Singleton<Config>::getInstance();
	CDuiString  strSenderName = msg.pSender->GetName();
	if (_tcscmp(msg.sType, DUI_MSGTYPE_CLICK) == 0)
	{
		if (_tcscmp(strSenderName, _T("option_color")) == 0)
		{
			if (m_pTLayoutSkin) m_pTLayoutSkin->SelectItem(0);
		}
		else if (_tcscmp(strSenderName, _T("option_image")) == 0)
		{
			if (m_pTLayoutSkin) m_pTLayoutSkin->SelectItem(1);
		}
		else if (_tcsstr(strSenderName, _T("btn_color_")) != 0)
		{
			DWORD dwColor = msg.pSender->GetBkColor();
			if (m_pSliderColorR) m_pSliderColorR->SetValue(static_cast<BYTE>(GetRValue(dwColor)));
			if (m_pSliderColorG) m_pSliderColorG->SetValue(static_cast<BYTE>(GetGValue(dwColor)));
			if (m_pSliderColorB) m_pSliderColorB->SetValue(static_cast<BYTE>(GetBValue(dwColor)));

			SetSkin(NULL, dwColor);
		}
		else if (_tcsstr(strSenderName, _T("btn_image_")) != 0)
		{
			CDuiString	strImage = msg.pSender->GetUserData();

			SetSkin(strImage, pCfg->dwBgColor);
		}
	}
	else if (_tcscmp(msg.sType, DUI_MSGTYPE_VALUECHANGED) == 0)
	{
		if (_tcsstr(strSenderName, _T("slider_color_")) != 0)
		{
			BYTE	red, green, blue;
			if (m_pSliderColorR) red = static_cast<BYTE>(m_pSliderColorR->GetValue());
			if (m_pSliderColorG) green = static_cast<BYTE>(m_pSliderColorG->GetValue());
			if (m_pSliderColorB) blue = static_cast<BYTE>(m_pSliderColorB->GetValue());

			SetSkin(NULL, 0xFF000000 | RGB(red, green, blue));
		}
	}
}

void CSkinDlg::SetSkin(LPCTSTR pszImage, DWORD dwColor)
{
	BOOL				bFlag = FALSE;
	SkinChangedParam	param;
	Config*				pCfg = Singleton<Config>::getInstance();

	if (pszImage != NULL) 
	{
		param.bgimage = pszImage;
		if (_tcsicmp(pCfg->szBgImage, pszImage) != 0) 
		{
			_tcsncpy(pCfg->szBgImage, pszImage, MAX_PATH);
			bFlag = TRUE;
		}		
	}
	else if (*pCfg->szBgImage != 0) {
		*pCfg->szBgImage = 0;
		bFlag = TRUE;
	}
	param.bkcolor = dwColor;
	if (dwColor != pCfg->dwBgColor) 
	{
		pCfg->dwBgColor = dwColor;
		bFlag = TRUE;
	}

	if (bFlag) {
		pCfg->WriteConfig(CONFIG_SKIN);
		m_skinObserver.Broadcast(param);
	}	
}

LRESULT CSkinDlg::OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	Close();
	return 0;
}