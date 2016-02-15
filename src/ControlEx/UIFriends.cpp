
#include "stdafx.h"
#include <math.h>
#include "ControlEx/UIFriends.hpp"

namespace DuiLib
{
const TCHAR kFriendNodeUIClassName[]					= _T("FriendNodeUI");
const TCHAR kFriendNodeUIInterfaceName[]				= _T("FriendNode");
const TCHAR kFriendGroupUIClassName[]					= _T("FriendGroupUI");
const TCHAR kFriendGroupUIInterfaceName[]				= _T("FriendGroup");

const TCHAR	kFriendNodeTabLayoutControlName[]			= _T("tab_friend");
const TCHAR kFriendNodeLabelAvatarControlName[]			= _T("label_avatar");
const TCHAR kFriendNodeLableNickNameControlName[]		= _T("lable_nickname");
const TCHAR kFriendNodeLableDescriptionControlName[]	= _T("lable_description");
const TCHAR kFriendNodeLabelAvatar2ControlName[]		= _T("label_avatar2");
const TCHAR kFriendNodeLableNickName2ControlName[]		= _T("lable_nickname2");
const TCHAR kFriendNodeLableDescription2ControlName[]	= _T("lable_description2");

const int kFriendNodeNormalHeight						= 26;
const int kFriendNodeSelectedHeight						= 52;

//////////////////////////////////////////////////////////////////////////
// CFriendsUI
CFriendsUI::CFriendsUI()
 : m_lDelayDeltaY(0)
 , m_lDelayNumber(0)
 , m_lDelayLeft(0)
 , m_bJumpTimer(false)
{
	
}

CFriendsUI::~CFriendsUI()
{

}

double CFriendsUI::CalculateDelay(double state)
{
	return pow(state, 2);
}

CControlUI* CFriendsUI::CreateControl(LPCTSTR pstrClass)
{
	if (_tcsicmp(pstrClass, kFriendGroupUIInterfaceName) == 0)
	{
		return new CFriendGroupUI();
	}
	else if (_tcsicmp(pstrClass, kFriendNodeUIInterfaceName) == 0)
	{
		return new CFriendNodeUI();
	}
	return NULL;
}

void CFriendsUI::DoEvent(TEventUI& event)
{
	if (!IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND)
	{
		if (m_pParent != NULL)
			m_pParent->DoEvent(event);
		else
			CVerticalLayoutUI::DoEvent(event);
		return;
	}

	if (event.Type == UIEVENT_TIMER)
	{
		DUI__Trace(_T("CFriendsUI::DoEvent UIEVENT_TIMER"));
		if (event.wParam == SCROLL_TIMERID)
		{
			if (m_lDelayLeft > 0)
			{
				--m_lDelayLeft;
				SIZE sz = GetScrollPos();
				LONG lDeltaY =  (LONG)(CalculateDelay((double)m_lDelayLeft / m_lDelayNumber) * m_lDelayDeltaY);
				if ((lDeltaY > 0 && sz.cy != 0)  || (lDeltaY < 0 && sz.cy != GetScrollRange().cy ))
				{
					sz.cy -= lDeltaY;
					SetScrollPos(sz);
					return;
				}
			}
			m_lDelayDeltaY = 0;
			m_lDelayNumber = 0;
			m_lDelayLeft = 0;
			m_pManager->KillTimer(this, SCROLL_TIMERID);
			return;
		}
		else if (event.wParam == FRIENDNODEJUMP_TIMERID)
		{
			CFriendNodeUI*	pFriendNode;
			for (int cnt = 0; cnt < m_arrayJump.GetSize(); cnt++)
			{
				pFriendNode = reinterpret_cast<CFriendNodeUI*>(m_arrayJump[cnt]);

				if (pFriendNode) {
					pFriendNode->DoJump();
				}				
			}
		}
	}
	else if (event.Type == UIEVENT_SCROLLWHEEL)
	{
		DUI__Trace(_T("CFriendsUI::DoEvent UIEVENT_SCROLLWHEEL"));
		LONG lDeltaY = 0;
		if (m_lDelayNumber > 0)
			lDeltaY =  (LONG)(CalculateDelay((double)m_lDelayLeft / m_lDelayNumber) * m_lDelayDeltaY);
		switch (LOWORD(event.wParam))
		{
		case SB_LINEUP:
			if (m_lDelayDeltaY >= 0)
				m_lDelayDeltaY = lDeltaY + 8;
			else
				m_lDelayDeltaY = lDeltaY + 12;
			break;
		case SB_LINEDOWN:
			if (m_lDelayDeltaY <= 0)
				m_lDelayDeltaY = lDeltaY - 8;
			else
				m_lDelayDeltaY = lDeltaY - 12;
			break;
		}
		if
			(m_lDelayDeltaY > 100) m_lDelayDeltaY = 100;
		else if
			(m_lDelayDeltaY < -100) m_lDelayDeltaY = -100;

		m_lDelayNumber = (DWORD)sqrt((double)abs(m_lDelayDeltaY)) * 5;
		m_lDelayLeft = m_lDelayNumber;
		m_pManager->SetTimer(this, SCROLL_TIMERID, 50U);
		return;
	}

	CTreeViewUI::DoEvent(event);
}

bool CFriendsUI::AddFriendGroup(LPCTSTR pstrGroup, int nIndex)
{
	CFriendGroupUI*	pFriendGroup = NULL;
	CDialogBuilder	dlgBuilder;

	if (!dlgBuilder.GetMarkup()->IsValid())
	{
		pFriendGroup = static_cast<CFriendGroupUI*>(dlgBuilder.Create(_T("xmls\\friend_group.xml"), 
			0, this, m_pManager));
	}
	else {
		pFriendGroup = static_cast<CFriendGroupUI*>(dlgBuilder.Create(this, m_pManager));
	}
	if (pFriendGroup == NULL)
	{
		return false;
	}

	pFriendGroup->SetName(pstrGroup);
	pFriendGroup->SetItemText(pstrGroup);

	if (nIndex == -1) {
		if (Add(pFriendGroup)) {
			return false;
		}
	}
	else {
		if (AddAt(pFriendGroup, nIndex)) {
			return false;
		}		
	}
	return true;
}

bool CFriendsUI::AddFriendNode(Host* pHost, int nIndex)
{
	CFriendNodeUI*	pFriendNode = NULL;
	CDialogBuilder	dlgBuilder;

	HostSet			*hostSet = pHost->hostSet;

	CDuiString		strGroupNameEx = (hostSet && *hostSet->groupNameEx) ? 
						hostSet->groupNameEx : DEFAULTGROUPNAMEEX_STR;

	CFriendGroupUI*	pFriendGroup;
	
	if (!dlgBuilder.GetMarkup()->IsValid())
	{
		pFriendNode = static_cast<CFriendNodeUI*>(dlgBuilder.Create(_T("xmls\\friend_node.xml"), 
			0, this, m_pManager));
	}
	else {
		pFriendNode = static_cast<CFriendNodeUI*>(dlgBuilder.Create(0, m_pManager));
	}
	if (pFriendNode == NULL) {
		return false;
	}

	pFriendNode->SetTag((UINT_PTR)pHost);
	pHost->tag = (UINT)pFriendNode;

	UpdateFriendNode(pHost);

	pFriendGroup = static_cast<CFriendGroupUI*>(m_pManager->FindSubControlByName(this, strGroupNameEx));
	if (pFriendGroup == NULL )
	{
		AddFriendGroup(strGroupNameEx);
		pFriendGroup = static_cast<CFriendGroupUI*>(m_pManager->FindSubControlByName(this, strGroupNameEx));
	}	
	if (pFriendGroup == NULL)
	{
		delete pFriendNode;
		return false;
	}

	if (nIndex == -1)
	{
		if (!pFriendGroup->Add(pFriendNode))
		{
			delete pFriendNode;
			return false;
		}
	}
	else {
		if (!pFriendGroup->AddAt(pFriendNode, nIndex))
		{
			delete pFriendNode;
			return false;
		}
	}

	// CTreeNodeUI在AddAt或Add的过程中会使Folder按钮可见，隐藏之
	pFriendNode->SetVisibleFolderBtn(false);
	pFriendNode->SetVisibleCheckBtn(false);

	return true;
}

bool CFriendsUI::UpdateFriendNode(Host* pHost)
{
	ASSERT(pHost != NULL);
	CFriendNodeUI* pFriendNode = reinterpret_cast<CFriendNodeUI*>(pHost->tag);
	
	if (pFriendNode == NULL) {
		return false;
	}

	/*
	CButtonUI*	pButtonAvatar = static_cast<CButtonUI*>(m_pManager->FindSubControlByName(pFriendNode, kFriendNodeButtonAvatarControlName));
	CButtonUI*	pButtonAvatar2 = static_cast<CButtonUI*>(m_pManager->FindSubControlByName(pFriendNode, kFriendNodeButtonAvatar2ControlName));
	if (pButtonAvatar != NULL && pButtonAvatar2 != NULL)
	{
		if (*pHost->avatar != 0) {
			pButtonAvatar->SetBkImage(pHost->avatar);
			pButtonAvatar2->SetBkImage(pHost->avatar);
		}
	}*/
	
	CLabelUI*	pLabelAvatar = static_cast<CLabelUI*>(m_pManager->FindSubControlByName(pFriendNode, kFriendNodeLabelAvatarControlName));
	CLabelUI*	pLabelAvatar2 = static_cast<CLabelUI*>(m_pManager->FindSubControlByName(pFriendNode, kFriendNodeLabelAvatar2ControlName));
	if (pLabelAvatar != NULL && pLabelAvatar2 != NULL)
	{
		if (*pHost->avatar != 0) {
			pLabelAvatar->SetBkImage(pHost->avatar);
			pLabelAvatar2->SetBkImage(pHost->avatar);
		}
	}

	CLabelUI*	pLabelNickName = static_cast<CLabelUI*>(m_pManager->FindSubControlByName(pFriendNode, kFriendNodeLableNickNameControlName));
	CLabelUI*	pLabelNickName2 = static_cast<CLabelUI*>(m_pManager->FindSubControlByName(pFriendNode, kFriendNodeLableNickName2ControlName));
	if (pLabelNickName != NULL && pLabelNickName2 != NULL)
	{
		TCHAR  szNickName[MAX_PATH];
		wsprintf(szNickName, _T("%s (%s)"), pHost->NickNameEx(), pHost->hostSub.hostName);
		pLabelNickName->SetText(szNickName);
		pLabelNickName2->SetText(szNickName);
	}

	CLabelUI*	pLabelDescription = static_cast<CLabelUI*>(m_pManager->FindSubControlByName(pFriendNode, kFriendNodeLableDescriptionControlName));
	CLabelUI*	pLabelDescription2 = static_cast<CLabelUI*>(m_pManager->FindSubControlByName(pFriendNode, kFriendNodeLableDescription2ControlName));
	if (pLabelDescription != NULL && pLabelDescription2 != NULL)
	{
		IN_ADDR addr;
		addr.S_un.S_addr = pHost->hostSub.addr;
		char* szAddr = inet_ntoa(addr);
		pLabelDescription->SetText(szAddr);
		pLabelDescription2->SetText(szAddr);
	}
	
	return true;
}

bool CFriendsUI::DeleteFriendNode(Host* pHost)
{
	ASSERT(pHost != NULL);
	CFriendNodeUI* pFriendNode = reinterpret_cast<CFriendNodeUI*>(pHost->tag);

	if (pFriendNode != NULL)
	{
		return CFriendsUI::Remove(pFriendNode);
	}
	return false;
}

void CFriendsUI::SetFriendNodeJump(Host* pHost, bool bJump)
{
	CFriendNodeUI* pFriendNode = reinterpret_cast<CFriendNodeUI*>(pHost->tag);

	//pFriendNode->SetJump(bJump);
	int		nIdx = m_arrayJump.Find(pFriendNode);
	if (bJump && nIdx < 0)
	{
		if (!m_bJumpTimer) {
			m_pManager->SetTimer(this, FRIENDNODEJUMP_TIMERID, 500);
			m_bJumpTimer = true;
		}
		m_arrayJump.Add(pFriendNode);
	}
	else if (!bJump && nIdx > 0)
	{
		m_arrayJump.Remove(nIdx);
		if (m_arrayJump.IsEmpty()) {
			m_pManager->KillTimer(this, FRIENDNODEJUMP_TIMERID);
			m_bJumpTimer = false;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
// CFriendGroupUI
CFriendGroupUI::CFriendGroupUI()
{

}

LPCTSTR CFriendGroupUI::GetClass() const
{
	return _T("TreeNodeUI"); // kFriendGroupUIClassName;
}

LPVOID CFriendGroupUI::GetInterface(LPCTSTR pstrName)
{
	if (_tcsicmp(pstrName, kFriendGroupUIInterfaceName) == 0)
	{
		return static_cast<CFriendGroupUI*>(this);
	}
	return CTreeNodeUI::GetInterface(pstrName);
}

void CFriendGroupUI::DoEvent(TEventUI& event)
{
	if( !IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND ) 
	{
		if( m_pOwner != NULL ) m_pOwner->DoEvent(event);
		else CContainerUI::DoEvent(event);
		return;
	}

	if( event.Type == UIEVENT_BUTTONDOWN)
	{
		return;
	}	
	else if( event.Type == UIEVENT_CONTEXTMENU )
	{
		if( IsContextMenuUsed() ) {
			m_pManager->SendNotify(this, DUI_MSGTYPE_MENU, event.wParam, event.lParam);
		}
	}
	else
	{
		CTreeNodeUI::DoEvent(event);
	}
}

//////////////////////////////////////////////////////////////////////////
// CFriendNodeUI
CFriendNodeUI::CFriendNodeUI()
 : m_pFriendTab(NULL)
 , m_pLabelAvatar(NULL)
 , m_pLabelAvatar2(NULL)
 , m_bJump(false)
 , m_bFlag(false)
{


}

CFriendNodeUI::~CFriendNodeUI()
{
}

LPCTSTR CFriendNodeUI::GetClass() const
{
	return _T("TreeNodeUI");// kFriendNodeUIClassName;
}

LPVOID CFriendNodeUI::GetInterface(LPCTSTR pstrName)
{
	if (_tcsicmp(pstrName, kFriendNodeUIInterfaceName) == 0)
	{
		return static_cast<CFriendNodeUI*>(this);
	}
	return CTreeNodeUI::GetInterface(pstrName);
}

void CFriendNodeUI::DoInit()
{
	m_pFriendTab = static_cast<CTabLayoutUI*>(m_pManager->FindSubControlByName(this, kFriendNodeTabLayoutControlName));

	m_pLabelAvatar = static_cast<CLabelUI*>(m_pManager->FindSubControlByName(this, kFriendNodeLabelAvatarControlName));
	m_pLabelAvatar2 = static_cast<CLabelUI*>(m_pManager->FindSubControlByName(this, kFriendNodeLabelAvatar2ControlName));
	
	if (m_pLabelAvatar) {
		//m_rcPosLogo = m_pLabelAvatar->GetPos();
		m_cXYAvatar = m_pLabelAvatar->GetFixedXY();
	}
	if (m_pLabelAvatar2) {
		//m_rcPosLogo2 = m_pLabelAvatar2->GetPos();
		m_cXYAvatar2 = m_pLabelAvatar2->GetFixedXY();
	}
}

bool CFriendNodeUI::Activate()
{
	if( !CContainerUI::Activate() ) return false;
	if( m_pManager != NULL ) m_pManager->SendNotify(this, DUI_MSGTYPE_ITEMACTIVATE);

	return true;
}

void CFriendNodeUI::DoEvent(TEventUI& event)
{
	if( !IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND ) 
	{
		if( m_pOwner != NULL ) m_pOwner->DoEvent(event);
		else CContainerUI::DoEvent(event);
		return;
	}

	if( event.Type == UIEVENT_CONTEXTMENU )
	{
		if( IsContextMenuUsed() ) {
			m_pManager->SendNotify(this, DUI_MSGTYPE_MENU, event.wParam, event.lParam);
		}
	}
	else
	{
		CTreeNodeUI::DoEvent(event);
	}
}

bool CFriendNodeUI::Select(bool bSelect /* = true */)
{
	if (m_pFriendTab != NULL)
	{
		if (bSelect) 
		{
			m_pFriendTab->SelectItem(1);
			SetFixedHeight(kFriendNodeSelectedHeight);
			m_pFriendTab->SetFixedHeight(kFriendNodeSelectedHeight);
		}
		else {
			m_pFriendTab->SelectItem(0);
			SetFixedHeight(kFriendNodeNormalHeight);
			m_pFriendTab->SetFixedHeight(kFriendNodeNormalHeight);
		}
	}
	return __super::Select(bSelect);
}

Host* CFriendNodeUI::GetHost()
{
	return reinterpret_cast<Host*>(GetTag());
}

void CFriendNodeUI::SetJump(bool bJump)
{
	m_bJump = bJump;
	if (!m_bJump) 
	{
		if (m_pLabelAvatar) m_pLabelAvatar->SetFixedXY(m_cXYAvatar);
		if (m_pLabelAvatar2) m_pLabelAvatar->SetFixedXY(m_cXYAvatar2);
	}
}

void CFriendNodeUI::DoJump()
{
	if (m_pFriendTab != NULL)
	{
		// 普通模式
		int			nOffset;
		SIZE		szXY;
		if (m_pFriendTab->GetCurSel() == 0)
		{
 			nOffset = m_bFlag ? 1 : -1;
			szXY = m_cXYAvatar;
			szXY.cx += nOffset;
			szXY.cy += nOffset;
			if (m_pLabelAvatar) m_pLabelAvatar->SetFixedXY(szXY);	//SetPos(rcNewPos);
		}
		else {
			nOffset = m_bFlag ? 2 : -2;
			szXY = m_cXYAvatar2;
			szXY.cx += nOffset;
			szXY.cy += nOffset;
			if (m_pLabelAvatar2) m_pLabelAvatar2->SetFixedXY(szXY);	//SetPos(rcNewPos);
		}
		m_bFlag = !m_bFlag;
	}
}

} // namespace DuiLib