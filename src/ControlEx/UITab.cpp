#include "stdafx.h"
#include "UITab.h"

namespace DuiLib
{



//////////////////////////////////////////////////////////////////////////
CTabUI::CTabUI()
: m_pTabLayout(NULL)
{
}

CTabUI::~CTabUI()
{
	m_mapButtonClose.clear();
}

void CTabUI::DoInit()
{
	__super::DoInit();
// 	LPCTSTR pDefaultAttributes = m_pManager->GetDefaultAttributeList(kMenuUIInterfaceName);
// 	if( pDefaultAttributes ) {
// 		m_pLayout->ApplyAttributeList(pDefaultAttributes);
// 	}
}

void CTabUI::SetTabLayout(CTabLayoutUI* pTabLayout)
{
	m_pTabLayout = pTabLayout;
}

bool CTabUI::AddPage(LPCTSTR pszText, CControlUI* pControlPage, bool bHasCloseBtn)
{
	COptionUI*	pOption = new COptionUI;
	TCHAR		szName[MAX_PATH] = {0};
	
	if (!Add(pOption))
	{
		return false;
	}
	wsprintf(szName, _T("option_panel_%d"), time(NULL));
	pOption->SetName(szName);
	pOption->SetTag((UINT_PTR)pControlPage);
	pOption->SetFloat(true);	
	pOption->SetText(pszText);
	pOption->SetNormalImage(_T("chatdlg\\option_tab_normal.png"));
	pOption->SetHotImage(_T("chatdlg\\option_tab_hover.png"));
	pOption->SetSelectedImage(_T("chatdlg\\option_tab_selected.png"));
	pOption->SetGroup(_T("group_panel"));
	// MakeDelegate的函数返回值必须是bool型 [12/25/2014 ybt]
	pOption->OnNotify += MakeDelegate(this, &CTabUI::OnSelect);
	
	if (bHasCloseBtn)
	{
		CButtonUI*	pButtonClose = new CButtonUI;
		if (!Add(pButtonClose))
		{
			return false;
		}
		wsprintf(szName, _T("btn_panel_close_%d"), time(NULL));
		m_mapButtonClose[pOption] = pButtonClose;
		pButtonClose->SetName(szName);
		pButtonClose->SetFloat(true);
		pButtonClose->SetTag((UINT_PTR)pOption);
		pButtonClose->SetNormalImage(_T("chatdlg\\close_normal.png"));
		pButtonClose->SetHotImage(_T("chatdlg\\close_hover.png"));
		pButtonClose->SetPushedImage(_T("chatdlg\\close_down.png"));
		pButtonClose->SetFixedWidth(15);
		pButtonClose->SetFixedHeight(15);
		

		pButtonClose->OnNotify += MakeDelegate(this, &CTabUI::OnCloseTab);
	}
	if (m_pTabLayout)
	{
		if (m_pTabLayout->FindSubControl(pControlPage->GetName()) == NULL)
		{
			m_pTabLayout->Add(pControlPage);
		}
//		m_pTabLayout->SelectItem(pControlPage);
	}

	AdjustControl();
	return true;
}

void CTabUI::AdjustControl()
{
	int			nIndex, nOptionIndex;
	int			nWidth;
	int			nOffset;
	int			nOptionCount = 0;
	bool		bOnlyOne;
	CDuiRect	rcOption;
	CDuiRect	rcButtonClose;
	COptionUI*	pOption;
	CControlUI*	pControl;
	CButtonUI*	pButtonClose;
	MapControl::iterator	iterOption;

	// Tab控件不是绝对位置，因此m_rcItem的值为空。 [12/25/2014 ybt]
	// height="22" width="150" 参数对应的是m_cxyFixed的值

	for (nIndex = 0; nIndex < GetCount(); nIndex++)
	{
		if (GetItemAt(nIndex)->GetInterface(DUI_CTR_OPTION) != NULL)
		{
			nOptionCount++;
		}
	}
	bOnlyOne = (nOptionCount == 1);
	nWidth = m_cxyFixed.cx / nOptionCount;

	nOptionIndex = 0;
	for (nIndex = 0; nIndex < GetCount(); nIndex++)
	{
		pControl = GetItemAt(nIndex);
		if (pControl->GetInterface(DUI_CTR_OPTION) == NULL)
		{
			continue;
		}		
		pOption = static_cast<COptionUI*>(pControl);
		pOption->SetMouseEnabled(!bOnlyOne);			// 如果只有一个标签，该标签不能点击

		rcOption.left = nWidth * nOptionIndex;
		rcOption.top = 0;	
		if (nIndex == nOptionCount - 1)
		{
			rcOption.right = m_cxyFixed.cx;
		}
		else 
		{
			rcOption.right = nWidth * (nOptionIndex + 1);
		}
		rcOption.bottom = m_cxyFixed.cy;

		SIZE szXY = {rcOption.left >= 0 ? rcOption.left : rcOption.right, rcOption.top >= 0 ? rcOption.top : rcOption.bottom};
		pOption->SetFixedXY(szXY);
		pOption->SetFixedWidth(rcOption.GetWidth());
		pOption->SetFixedHeight(rcOption.GetHeight());
		//pOption->SetVisible(false);
		//pOption->SetPos(rcOption);

		pButtonClose = static_cast<CButtonUI*>(m_mapButtonClose[pOption]);
		if (pButtonClose != NULL)
		{
			nOffset = (rcOption.GetHeight() - pButtonClose->GetFixedHeight()) / 2;
			rcButtonClose.right = rcOption.right - nOffset;
			rcButtonClose.left = rcButtonClose.right - pButtonClose->GetFixedHeight();
			rcButtonClose.top = rcOption.top + nOffset;
			rcButtonClose.bottom = rcOption.bottom - nOffset;
			//pButtonClose->SetPos(rcButtonClose);
			SIZE szXYClose = {rcButtonClose.left >= 0 ? rcButtonClose.left : rcButtonClose.right, rcButtonClose.top >= 0 ? rcButtonClose.top : rcButtonClose.bottom};
			pButtonClose->SetFixedXY(szXYClose);
			//pButtonClose->SetFixedWidth(rcButtonClose.GetWidth());
			//pButtonClose->SetFixedHeight(rcButtonClose.GetHeight());
		}

		nOptionIndex++;
	}
}

bool CTabUI::OnCloseTab(void* param)
{
	TNotifyUI* pMsg = (TNotifyUI*)param;
	if (pMsg->sType == DUI_MSGTYPE_CLICK)
	{
		COptionUI*	pOption = reinterpret_cast<COptionUI*>(pMsg->pSender->GetTag());
		CButtonUI*	pButtonClose = NULL;
		CControlUI*	pControlPage = NULL;
		if (pOption)
		{
			if (m_mapButtonClose.find(pOption) != m_mapButtonClose.end())
			{
				pButtonClose = static_cast<CButtonUI*>(m_mapButtonClose[pOption]);
				if (pButtonClose != NULL)
				{
					Remove(pButtonClose);
					delete pButtonClose;
				}
				m_mapButtonClose.erase(pOption);
			}
			
			if (m_pTabLayout)
			{
				pControlPage = reinterpret_cast<CControlUI*>(pOption->GetTag());
				if (pControlPage)
				{
					m_pTabLayout->Remove(pControlPage);
					delete pControlPage;
				}				
			}
			
			Remove(pOption);
			delete pOption;

			AdjustControl();
		}
	}
	return true;
}

bool CTabUI::OnSelect(void* param)
{
	TNotifyUI* pMsg = (TNotifyUI*)param;
	if (m_pTabLayout && pMsg->sType == DUI_MSGTYPE_CLICK)
	{
		CControlUI*	pControl = reinterpret_cast<CControlUI*>(pMsg->pSender->GetTag());
		m_pTabLayout->SelectItem(pControl);		
	}
	return true;
}

} //namespace DuiLib