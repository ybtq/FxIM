#include "StdAfx.h"
#include "ControlEx/UIMenu.h"

namespace DuiLib {

/////////////////////////////////////////////////////////////////////////////////////
//
ContextMenuObserver s_context_menu_observer;

// MenuUI
const TCHAR* const kMenuUIClassName = _T("MenuUI");
const TCHAR* const kMenuUIInterfaceName = _T("Menu");

CMenuUI::CMenuUI()
{
	if (GetHeader() != NULL)
		GetHeader()->SetVisible(false);
}

CMenuUI::~CMenuUI()
{}

LPCTSTR CMenuUI::GetClass() const
{
    return kMenuUIClassName;
}

LPVOID CMenuUI::GetInterface(LPCTSTR pstrName)
{
    if( _tcsicmp(pstrName, kMenuUIInterfaceName) == 0 ) return static_cast<CMenuUI*>(this);
    return CListUI::GetInterface(pstrName);
}

void CMenuUI::DoEvent(TEventUI& event)
{
	return __super::DoEvent(event);
}

bool CMenuUI::Add(CControlUI* pControl)
{
	CMenuElementUI* pMenuItem = static_cast<CMenuElementUI*>(pControl->GetInterface(kMenuElementUIInterfaceName));
	if (pMenuItem == NULL)
		return false;

	for (int i = 0; i < pMenuItem->GetCount(); ++i)
	{
		if (pMenuItem->GetItemAt(i)->GetInterface(kMenuElementUIInterfaceName) != NULL)
		{
			(static_cast<CMenuElementUI*>(pMenuItem->GetItemAt(i)->GetInterface(kMenuElementUIInterfaceName)))->SetInternVisible(false);
		}
	}
	return CListUI::Add(pControl);
}

bool CMenuUI::AddAt(CControlUI* pControl, int iIndex)
{
	CMenuElementUI* pMenuItem = static_cast<CMenuElementUI*>(pControl->GetInterface(kMenuElementUIInterfaceName));
	if (pMenuItem == NULL)
		return false;

	CMenuElementUI*	pSubMenuItem = NULL;
	for (int i = 0; i < pMenuItem->GetCount(); ++i)
	{
		pSubMenuItem = static_cast<CMenuElementUI*>(pMenuItem->GetItemAt(i)->GetInterface(kMenuElementUIInterfaceName));
		if (pSubMenuItem != NULL)
		{
			pSubMenuItem->SetInternVisible(false);
		}
	}
	return CListUI::AddAt(pControl, iIndex);
}

int CMenuUI::GetItemIndex(CControlUI* pControl) const
{
	CMenuElementUI* pMenuItem = static_cast<CMenuElementUI*>(pControl->GetInterface(kMenuElementUIInterfaceName));
	if (pMenuItem == NULL)
		return -1;

	return __super::GetItemIndex(pControl);
}

bool CMenuUI::SetItemIndex(CControlUI* pControl, int iIndex)
{
	CMenuElementUI* pMenuItem = static_cast<CMenuElementUI*>(pControl->GetInterface(kMenuElementUIInterfaceName));
	if (pMenuItem == NULL)
		return false;

	return __super::SetItemIndex(pControl, iIndex);
}

bool CMenuUI::Remove(CControlUI* pControl)
{
	CMenuElementUI* pMenuItem = static_cast<CMenuElementUI*>(pControl->GetInterface(kMenuElementUIInterfaceName));
	if (pMenuItem == NULL)
		return false;

	return __super::Remove(pControl);
}

SIZE CMenuUI::EstimateSize(SIZE szAvailable)
{
	int cxFixed = 0;
    int cyFixed = 0;
    for( int it = 0; it < GetCount(); it++ ) {
        CControlUI* pControl = static_cast<CControlUI*>(GetItemAt(it));
        if( !pControl->IsVisible() ) continue;
        SIZE sz = pControl->EstimateSize(szAvailable);
        cyFixed += sz.cy;
		if( cxFixed < sz.cx )
			cxFixed = sz.cx;
    }
	// 增加inset的值，修复窗口高宽计算不正确的Bug。 [10/30/2014 ybt]
	RECT rc = GetInset();
	cxFixed += rc.left + rc.right;
	cyFixed += rc.top + rc.bottom;

    return CSize(cxFixed, cyFixed);
}


/////////////////////////////////////////////////////////////////////////////////////
//
class CMenuBuilderCallback: public IDialogBuilderCallback
{
	CControlUI* CreateControl(LPCTSTR pstrClass)
	{
		if (_tcsicmp(pstrClass, kMenuUIInterfaceName) == 0)
		{
			return new CMenuUI();
		}
		else if (_tcsicmp(pstrClass, kMenuElementUIInterfaceName) == 0)
		{
			return new CMenuElementUI();
		}
		return NULL;
	}
};

HWND CMenuWnd::m_hWndNotify = NULL;
CMenuWnd::CMenuWnd(HWND hParent):
m_hParent(hParent),
m_pOwner(NULL),
m_pLayout(NULL),
m_xml(_T(""))
{}

BOOL CMenuWnd::Receive(ContextMenuParam param)
{
	switch (param.wParam)
	{
	case 1:
		Close();
		break;
	case 2:
		{
			HWND hParent = GetParent(m_hWnd);
			while (hParent != NULL)
			{
				if (hParent == param.hWnd)
				{
					Close();
					break;
				}
				hParent = GetParent(hParent);
			}
		}
		break;
	default:
		break;
	}

	return TRUE;
}

void CMenuWnd::Init(CMenuElementUI* pOwner, STRINGorID xml, LPCTSTR pSkinType, POINT point, DWORD dwAlignment)
{
	m_BasedPoint = point;
    m_pOwner = pOwner;
    m_pLayout = NULL;
	m_dwAlignment = dwAlignment;

	if (pSkinType != NULL)
		m_sType = pSkinType;

	m_xml = xml;

	// 如果是根菜单
	if (m_pOwner == NULL) {
		m_hWndNotify = m_hParent;
	}

	s_context_menu_observer.AddReceiver(this);

	Create((m_pOwner == NULL) ? m_hParent : m_pOwner->GetManager()->GetPaintWindow(), NULL, WS_POPUP, 
		WS_EX_TOOLWINDOW | WS_EX_TOPMOST, CDuiRect());
    ::ShowWindow(m_hWnd, SW_SHOW);

	// HACK: Don't deselect the parent's caption
// 	HWND hWndParent = m_hWnd;
// 	while( ::GetParent(hWndParent) != NULL ) hWndParent = ::GetParent(hWndParent);
// #if defined(WIN32) && !defined(UNDER_CE)
//     ::SendMessage(hWndParent, WM_NCACTIVATE, TRUE, 0L);
// #endif	
}

LPCTSTR CMenuWnd::GetWindowClassName() const
{
    return _T("MenuWnd");
}

void CMenuWnd::OnFinalMessage(HWND hWnd)
{
	RemoveObserver();
	if( m_pOwner != NULL ) {
		for( int i = 0; i < m_pOwner->GetCount(); i++ ) {
			CMenuElementUI*	pMenuElement = static_cast<CMenuElementUI*>(m_pOwner->GetItemAt(i)->GetInterface(kMenuElementUIInterfaceName));
			if( pMenuElement != NULL ) {
				pMenuElement->SetOwner(m_pOwner->GetParent());
				pMenuElement->SetVisible(false);
				pMenuElement->SetInternVisible(false);
			}
		}
		m_pOwner->m_pWindow = NULL;
		m_pOwner->m_uButtonState &= ~ UISTATE_PUSHED;
		m_pOwner->Invalidate();
	}
    delete this;
}

LRESULT CMenuWnd::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if( uMsg == WM_CREATE ) {
		if( m_pOwner != NULL) {
			// 如果是子菜单
			LONG styleValue = ::GetWindowLong(*this, GWL_STYLE);
			styleValue &= ~WS_CAPTION;
			::SetWindowLong(*this, GWL_STYLE, styleValue | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
			RECT rcClient;
			::GetClientRect(*this, &rcClient);
			::SetWindowPos(*this, NULL, rcClient.left, rcClient.top, rcClient.right - rcClient.left, \
				rcClient.bottom - rcClient.top, SWP_FRAMECHANGED);

			m_pm.Init(m_hWnd);
			// The trick is to add the items to the new container. Their owner gets
			// reassigned by this operation - which is why it is important to reassign
			// the items back to the righfull owner/manager when the window closes.
			m_pLayout = new CMenuUI();
			CPaintManagerUI* pParentPM = m_pOwner->GetManager();
			m_pm.UseParentResource(pParentPM);
			
			// CPaintManagerUI类的实现中，有些父窗口属性没有复用，这里加上。
			// 是否透明不能设置，否则子菜单窗口无法显示。[11/5/2014 ybt]
			m_pm.SetRoundCorner(pParentPM->GetRoundCorner().cx, pParentPM->GetRoundCorner().cy);
			m_pm.SetTransparent(pParentPM->GetTransparent());
			
 			m_pLayout->SetManager(&m_pm, NULL, true);
			LPCTSTR pDefaultAttributes = pParentPM->GetDefaultAttributeList(kMenuUIInterfaceName);
			if( pDefaultAttributes ) {
				m_pLayout->ApplyAttributeList(pDefaultAttributes);
			}
			//m_pLayout->SetBkColor(0xFFFFFFFF);
			//m_pLayout->SetBorderColor(0xFF85E4FF);
			//m_pLayout->SetBorderSize(0);
			//m_pLayout->EnableScrollBar();
			m_pLayout->SetAutoDestroy(false);
			
			for( int i = 0; i < m_pOwner->GetCount(); i++ ) {
				if(m_pOwner->GetItemAt(i)->GetInterface(kMenuElementUIInterfaceName) != NULL ){
					(static_cast<CMenuElementUI*>(m_pOwner->GetItemAt(i)))->SetOwner(m_pLayout);
					m_pLayout->Add(static_cast<CControlUI*>(m_pOwner->GetItemAt(i)));
				}
			}
			m_pm.AttachDialog(m_pLayout);
			m_pm.AddNotifier(this);

			// Position the popup window in absolute space
			CDuiRect rcOwner = m_pOwner->GetPos();
			CDuiRect rc = rcOwner;

			int cxFixed = 0;
			int cyFixed = 0;

#if defined(WIN32) && !defined(UNDER_CE)
			MONITORINFO oMonitor = {}; 
			oMonitor.cbSize = sizeof(oMonitor);
			::GetMonitorInfo(::MonitorFromWindow(*this, MONITOR_DEFAULTTOPRIMARY), &oMonitor);
			CDuiRect rcWork = oMonitor.rcWork;
#else
			CDuiRect rcWork;
			GetWindowRect(m_pOwner->GetManager()->GetPaintWindow(), &rcWork);
#endif
			SIZE szAvailable = { rcWork.right - rcWork.left, rcWork.bottom - rcWork.top };

			for( int it = 0; it < m_pOwner->GetCount(); it++ ) {
				if(m_pOwner->GetItemAt(it)->GetInterface(kMenuElementUIInterfaceName) != NULL ){
					CControlUI* pControl = static_cast<CControlUI*>(m_pOwner->GetItemAt(it));
					SIZE sz = pControl->EstimateSize(szAvailable);
					cyFixed += sz.cy;

					if( cxFixed < sz.cx )
						cxFixed = sz.cx;
				}
			}
			
			// 修正子菜单宽高不准确的Bug. [10/30/2014 ybt] 
			RECT rcLayoutInset = m_pLayout->GetInset();
			cxFixed += rcLayoutInset.left + rcLayoutInset.right;
			cyFixed += rcLayoutInset.top + rcLayoutInset.bottom;

			RECT rcWindow;
			GetWindowRect(m_pOwner->GetManager()->GetPaintWindow(), &rcWindow);

			// 优化菜单的显示方式 [10/30/2014 ybt]
			// 设置子菜单的左侧位于父菜单的右侧
			CDuiRect rcWinOwner = rcOwner;
			rcWinOwner.Offset(rcWindow.left, rcWindow.top);			// rcOwner是相对坐标，取得Windows坐标

			//::MapWindowRect(m_pOwner->GetManager()->GetPaintWindow(), HWND_DESKTOP, &rc);
#define X_MENU_OFFSET	5											// 两个菜单之间在x轴上的距离
			rc.left = rcWinOwner.right;
			rc.left += X_MENU_OFFSET;
			rc.right = rc.left + cxFixed;
			rc.top = rcWinOwner.top - rcLayoutInset.top;
			rc.bottom = rc.top + cyFixed;

			// 超出显示器的底部。向上移动
			if( rc.bottom > rcWork.bottom ) {
				// 如果上部空间足够，则全部移往上部
				if (rcWinOwner.bottom + rcLayoutInset.bottom > cyFixed) {
					rc.bottom = rcWinOwner.bottom + rcLayoutInset.bottom;
					rc.top = rc.bottom - cyFixed;
				}
				else {
					rc.top = 0;
					rc.bottom = rc.top + cyFixed;
					if (rc.bottom > rcWork.bottom)
					{
						rc.bottom = rcWork.bottom;
					}					
				}
			}

			// 超出显示器的右边。向左移动
			if (rc.right > rcWork.right) {
				rc.right = rcWinOwner.left - X_MENU_OFFSET;
				rc.left = rc.right - cxFixed;
			}

			// 超出显示器顶部，向下移动
			if( rc.top < rcWork.top ) {
				// 如果下部空间足够，则全部移往下部
				if (rcWinOwner.top - rcLayoutInset.top < cyFixed )
				{
					rc.top = rcWinOwner.top - rcLayoutInset.top;
					rc.bottom = rc.top + cyFixed;
				}
				else {
					rc.bottom = rcWork.bottom;
					rc.top = rc.bottom - cyFixed;
					if (rc.top < 0)	{
						rc.top = 0;
					}
				}
				rc.top = rcWinOwner.top;
				rc.bottom = rc.top + cyFixed;
			}

			// 超出显示器左边，向右移动
			if (rc.left < rcWork.left){
				rc.left = rcWinOwner.right + X_MENU_OFFSET;
				rc.right = rc.left + cxFixed;
			}

			MoveWindow(m_hWnd, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, FALSE);
		}
		else 
		{
			// 根菜单创建
			m_pm.Init(m_hWnd);

			CDialogBuilder builder;
			CMenuBuilderCallback menuCallback;

			CControlUI* pRoot = builder.Create(m_xml, m_sType.GetData(), &menuCallback, &m_pm);
			m_pLayout = static_cast<CMenuUI*>(pRoot);
			m_pm.AttachDialog(pRoot);
			m_pm.AddNotifier(this);

#if defined(WIN32) && !defined(UNDER_CE)
			MONITORINFO oMonitor = {}; 
			oMonitor.cbSize = sizeof(oMonitor);
			::GetMonitorInfo(::MonitorFromWindow(*this, MONITOR_DEFAULTTOPRIMARY), &oMonitor);
			CDuiRect rcWork = oMonitor.rcWork;
#else
			CDuiRect rcWork;
			GetWindowRect(m_pOwner->GetManager()->GetPaintWindow(), &rcWork);
#endif
			SIZE szAvailable = { rcWork.right - rcWork.left, rcWork.bottom - rcWork.top };
			szAvailable = pRoot->EstimateSize(szAvailable);
			m_pm.SetInitSize(szAvailable.cx, szAvailable.cy);

			SIZE szInit = m_pm.GetInitSize();			
			CPoint point = m_BasedPoint;
			CDuiRect rc(point.x, point.y, point.x + szInit.cx, point.y + szInit.cy);
			
			// 修复根菜单会在屏幕之外位置显示的Bug [10/30/2014 ybt]
			if (m_dwAlignment & eMenuAlignment_Bottom)
			{
				rc.top = point.y;
				if (rc.top + szInit.cy > rcWork.bottom) {
					rc.top = rcWork.bottom - szInit.cy;
				}
				rc.bottom = rc.top + szInit.cy;
			}
			if (m_dwAlignment & eMenuAlignment_Right)
			{
				rc.left = point.x;
				if (rc.left + szInit.cx > rcWork.right) {
					rc.left = rcWork.right - szInit.cx;
				}			
				rc.right = rc.left + szInit.cx;
			}
			if (m_dwAlignment & eMenuAlignment_Left) 
			{
				rc.left = point.x - szInit.cx;
				if (rc.left < 0) {
					rc.left = 0;
				}
				rc.right = rc.left + szInit.cx;
			}			
			if (m_dwAlignment & eMenuAlignment_Top)
			{
				rc.top = point.y - szInit.cy;
				if (rc.top < 0) {
					rc.top = 0;
				}
				rc.bottom = rc.top + szInit.cy;
			}

			SetForegroundWindow(m_hWnd);
			MoveWindow(m_hWnd, rc.left, rc.top, rc.GetWidth(), rc.GetHeight(), FALSE);
			SetWindowPos(m_hWnd, HWND_TOPMOST, rc.left, rc.top, rc.GetWidth(), rc.GetHeight(), SWP_SHOWWINDOW);
		}

		return 0;
    }
    else if( uMsg == WM_CLOSE ) {
		if( m_pOwner != NULL )
		{
			m_pOwner->SetManager(m_pOwner->GetManager(), m_pOwner->GetParent(), false);
			m_pOwner->SetPos(m_pOwner->GetPos());
			m_pOwner->SetFocus();
		}
	}
	else if( uMsg == WM_RBUTTONDOWN || uMsg == WM_CONTEXTMENU || uMsg == WM_RBUTTONUP || uMsg == WM_RBUTTONDBLCLK )
	{
		return 0L;
	}
	else if( uMsg == WM_KILLFOCUS )
	{
		HWND hFocusWnd = (HWND)wParam;

		BOOL bInMenuWindowList = FALSE;
		ContextMenuParam param;
		param.hWnd = GetHWND();

		ContextMenuObserver::Iterator<BOOL, ContextMenuParam> iterator(s_context_menu_observer);
		ReceiverImplBase<BOOL, ContextMenuParam>* pReceiver = iterator.next();
		while( pReceiver != NULL ) {
			CMenuWnd* pContextMenu = dynamic_cast<CMenuWnd*>(pReceiver);
			if( pContextMenu != NULL && pContextMenu->GetHWND() ==  hFocusWnd ) {
				bInMenuWindowList = TRUE;
				break;
			}
			pReceiver = iterator.next();
		}

		if( !bInMenuWindowList ) {
			param.wParam = 1;
			s_context_menu_observer.RBroadcast(param);

			return 0;
		}
	}
	else if( uMsg == WM_KEYDOWN)
	{
		if( wParam == VK_ESCAPE)
		{
			Close();
		}
	}
	else if (uMsg == WM_SIZE)
	{
		// 增加WM_SIZE消息处理，以修复roundcorner属性无效的Bug. [11/5/2014 ybt]
		SIZE szRoundCorner = m_pm.GetRoundCorner();
#if defined(WIN32) && !defined(UNDER_CE)
		if( !::IsIconic(*this) && (szRoundCorner.cx != 0 || szRoundCorner.cy != 0) ) {
			CDuiRect rcWnd;
			::GetWindowRect(*this, &rcWnd);
			rcWnd.Offset(-rcWnd.left, -rcWnd.top);
			rcWnd.right++; rcWnd.bottom++;
			HRGN hRgn = ::CreateRoundRectRgn(rcWnd.left, rcWnd.top, rcWnd.right, rcWnd.bottom, szRoundCorner.cx, szRoundCorner.cy);
			::SetWindowRgn(*this, hRgn, TRUE);
			::DeleteObject(hRgn);
		}
#endif
	}

    LRESULT lRes = 0;
    if( m_pm.MessageHandler(uMsg, wParam, lParam, lRes) ) return lRes;
    return CWindowWnd::HandleMessage(uMsg, wParam, lParam);
}

void CMenuWnd::Notify(TNotifyUI& msg)
{	
	if (_tcsicmp(msg.sType, DUI_MSGTYPE_ITEMCLICK) == 0)
	{
		static TCHAR szSender[MAX_PATH] = {0};

		_tcsncpy(szSender, msg.pSender->GetName(), MAX_PATH - 1);
		::PostMessage(m_hWndNotify, WM_MENUCLICK, (WPARAM)szSender, NULL);
	}
}

void CMenuWnd::EnableMenuItem(LPCTSTR lpszName, bool bEnable)
{
	CMenuElementUI* pControl = (CMenuElementUI*)m_pm.FindControl(lpszName);
	if (pControl != NULL)
		pControl->SetEnabled(bEnable);
}

void CMenuWnd::CheckMenuItem(LPCTSTR lpszName, bool bCheck)
{
	CMenuElementUI* pControl = (CMenuElementUI*)m_pm.FindControl(lpszName);
	if (pControl != NULL)
		pControl->SetCheckState(bCheck);
}


/////////////////////////////////////////////////////////////////////////////////////
//

// MenuElementUI
const TCHAR* const kMenuElementUIClassName = _T("MenuElementUI");
const TCHAR* const kMenuElementUIInterfaceName = _T("MenuElement");

CMenuElementUI::CMenuElementUI():
m_pWindow(NULL), m_bHasSubMenu(false),
m_bIsSeparator(false)
{
	m_bMouseChildEnabled = true;
	SetMouseChildEnabled(false);
	m_szIconImage.cx = 16;
	m_szIconImage.cy = 16;
	m_szCheckImage = m_szIconImage;
	m_szArrowImage.cx = 12;
	m_szArrowImage.cy = 12;
	m_bCheck = false;
}

CMenuElementUI::~CMenuElementUI()
{}

LPCTSTR CMenuElementUI::GetClass() const
{
	return kMenuElementUIClassName;
}

LPVOID CMenuElementUI::GetInterface(LPCTSTR pstrName)
{
    if( _tcsicmp(pstrName, kMenuElementUIInterfaceName) == 0 ) return static_cast<CMenuElementUI*>(this);    
    return CListContainerElementUI::GetInterface(pstrName);
}

void CMenuElementUI::DoPaint(HDC hDC, const RECT& rcPaint)
{
    if(!::IntersectRect(&m_rcPaint, &rcPaint, &m_rcItem)) 
		return;
	
	if (m_cxyBorderRound.cx > 0 && m_cxyBorderRound.cy > 0){
		CRenderClip roundClip;
		CRenderClip::GenerateRoundClip(hDC, m_rcPaint, m_rcItem, m_cxyBorderRound.cx, m_cxyBorderRound.cy, roundClip); //绘制圆角
	}

	// 直接画一条线.
	if (m_bIsSeparator) 
	{		
		DWORD dwBkColor1 = 0x00FFFFFF;
		DWORD dwBkColor2 = 0xFF000000;

		CDuiRect rcItemNew = m_rcItem;

		CDuiRect LineRect = m_rcItem;
		LineRect.top = rcItemNew.top + (rcItemNew.bottom - rcItemNew.top - 1) / 2;
		LineRect.bottom = LineRect.top + 1;

		LineRect.right = rcItemNew.left + (rcItemNew.right - rcItemNew.left) / 2;

		CRenderEngine::DrawGradient(hDC, LineRect, GetAdjustColor(dwBkColor1), GetAdjustColor(dwBkColor2), false, 16);

		LineRect.left = LineRect.right;
		LineRect.right = rcItemNew.right;

		CRenderEngine::DrawGradient(hDC, LineRect, GetAdjustColor(dwBkColor2), GetAdjustColor(dwBkColor1), false, 16);

		return;
	}

	// 绘制背景
	CMenuElementUI::DrawItemBk(hDC, m_rcItem);

#define X_IMAGE_OFFSET	5
	// 绘制左边图标.  如果是选中状态，则优先绘制选中图标。
	bool	bCheck = m_bCheck && !m_strCheckImage.IsEmpty();

	if (m_bCheck || !m_strIconImage.IsEmpty())
	{
		SIZE		szImage = bCheck ? m_szCheckImage : m_szIconImage;
		LPCTSTR		pszImage = bCheck ? m_strCheckImage.GetData() : m_strIconImage.GetData();
		int			nOffset = (m_rcItem.bottom - m_rcItem.top - szImage.cy) / 2;

		nOffset = nOffset < 0 ? 0 : nOffset;
		CDuiRect	destRect(X_IMAGE_OFFSET, nOffset, X_IMAGE_OFFSET + szImage.cx, 
						nOffset + szImage.cy);
		CDuiString	strModify;

		strModify.SmallFormat(_T("dest='%d,%d,%d,%d'"), destRect.left, destRect.top, destRect.right, destRect.bottom);

		DrawImage(hDC, pszImage, strModify);
	}

	// 绘制右侧箭头	
	if (m_bHasSubMenu && !m_strArrowImage.IsEmpty()) 
	{
		int			nOffset = (m_rcItem.bottom - m_rcItem.top - m_szArrowImage.cy) / 2;

		nOffset = nOffset < 0 ? 0 : nOffset;
		CDuiRect	destRect(m_rcItem.right - X_IMAGE_OFFSET - m_szArrowImage.cx, nOffset, 
						m_rcItem.right - X_IMAGE_OFFSET, nOffset + m_szArrowImage.cy);
		CDuiString	strModify;

		strModify.SmallFormat(_T("dest='%d,%d,%d,%d'"), destRect.left, destRect.top, destRect.right, destRect.bottom);

		DrawImage(hDC, m_strArrowImage, strModify);		
	}

	// 绘制文本
	CDuiRect textRect = m_rcItem;

	DrawItemText(hDC, textRect);
	
	// 子控件的绘制	
	for (int i = 0; i < GetCount(); ++i){
		if (GetItemAt(i)->GetInterface(kMenuElementUIInterfaceName) == NULL)
			GetItemAt(i)->DoPaint(hDC, rcPaint);
	}
}

void CMenuElementUI::SetArrowImage(LPCTSTR lpszImage)
{
	m_strArrowImage = lpszImage;
	Invalidate();
}

void CMenuElementUI::SetCheckImage(LPCTSTR lpszImage)
{
	m_strCheckImage = lpszImage;
	Invalidate();
}

void CMenuElementUI::SetIconImage(LPCTSTR lpszImage)
{
	m_strIconImage = lpszImage;
	Invalidate();
}

void CMenuElementUI::SetIconSize(int cx, int cy)
{
	m_szIconImage.cx = cx;
	m_szIconImage.cy = cy;
}

void CMenuElementUI::SetArrowSize(int cx, int cy)
{
	m_szArrowImage.cx = cx;
	m_szArrowImage.cy = cy;
}

void CMenuElementUI::SetCheckSize(int cx, int cy)
{
	m_szCheckImage.cx = cx;
	m_szCheckImage.cy = cy;
}

void CMenuElementUI::SetSeparator(bool bSeparator)
{
	m_bIsSeparator = bSeparator;
	SetEnabled(false);
}

void CMenuElementUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
	if (_tcscmp(pstrName, _T("arrowimage")) == 0) {
		SetArrowImage(pstrValue);
	}
	else if (_tcscmp(pstrName, _T("arrowsize")) == 0) {
		int cx, cy;
		LPTSTR pstr = NULL;
		cx = _tcstol(pstrValue, &pstr, 10);  ASSERT(pstr);    
		cy = _tcstol(pstr + 1, &pstr, 10);   ASSERT(pstr); 
		SetArrowSize(cx, cy);
	}
	else if (_tcscmp(pstrName, _T("checkimage")) == 0) {
		SetCheckImage(pstrValue);
	}
	else if (_tcscmp(pstrName, _T("checksize")) == 0) {
		int cx, cy;
		LPTSTR pstr = NULL;
		cx = _tcstol(pstrValue, &pstr, 10);  ASSERT(pstr);    
		cy = _tcstol(pstr + 1, &pstr, 10);   ASSERT(pstr); 
		SetCheckSize(cx, cy);
	}
	else if (_tcscmp(pstrName, _T("iconimage")) == 0) {
		SetIconImage(pstrValue);
	}
	else if (_tcscmp(pstrName, _T("iconsize")) == 0) {
		int cx, cy;
		LPTSTR pstr = NULL;
		cx = _tcstol(pstrValue, &pstr, 10);  ASSERT(pstr);    
		cy = _tcstol(pstr + 1, &pstr, 10);   ASSERT(pstr); 
		SetIconSize(cx, cy);
	}
	else if(_tcscmp(pstrName, _T("separator")) == 0) {
		SetSeparator(_tcscmp(pstrValue, _T("true")) == 0);
	}
	else{
		CListContainerElementUI::SetAttribute(pstrName, pstrValue);
	}
}

void CMenuElementUI::DrawItemText(HDC hDC, const RECT& rcItem)
{
    if( m_sText.IsEmpty() ) return;

    if( m_pOwner == NULL ) return;
    TListInfoUI* pInfo = m_pOwner->GetListInfo();
    DWORD iTextColor = pInfo->dwTextColor;
    if( (m_uButtonState & UISTATE_HOT) != 0 ) {
        iTextColor = pInfo->dwHotTextColor;
    }
    if( IsSelected() ) {
        iTextColor = pInfo->dwSelectedTextColor;
    }
    if( !IsEnabled() ) {
        iTextColor = pInfo->dwDisabledTextColor;
    }
    int nLinks = 0;
    RECT rcText = rcItem;
    rcText.left += pInfo->rcTextPadding.left;
    rcText.right -= pInfo->rcTextPadding.right;
    rcText.top += pInfo->rcTextPadding.top;
    rcText.bottom -= pInfo->rcTextPadding.bottom;

    if( pInfo->bShowHtml )
        CRenderEngine::DrawHtmlText(hDC, m_pManager, rcText, m_sText, iTextColor, \
        NULL, NULL, nLinks, DT_SINGLELINE | pInfo->uTextStyle);
    else
        CRenderEngine::DrawText(hDC, m_pManager, rcText, m_sText, iTextColor, \
        pInfo->nFont, DT_SINGLELINE | pInfo->uTextStyle);
}


SIZE CMenuElementUI::EstimateSize(SIZE szAvailable)
{
	SIZE cXY = {0};
	for( int it = 0; it < GetCount(); it++ ) {
		CControlUI* pControl = static_cast<CControlUI*>(GetItemAt(it));
		if( !pControl->IsVisible() ) continue;
		SIZE sz = pControl->EstimateSize(szAvailable);
		cXY.cy += sz.cy;
		if( cXY.cx < sz.cx )
			cXY.cx = sz.cx;
	}
	
	if (m_bIsSeparator) {
#define SEPARATOR_HEIGHT	10			// 分隔条高度
		cXY.cy = SEPARATOR_HEIGHT;
		return cXY;
	}

	// 高度没有设置，则根据文本信息来获取菜单项的高度
	if(cXY.cy == 0) {
		TListInfoUI* pInfo = m_pOwner->GetListInfo();

		DWORD iTextColor = pInfo->dwTextColor;
		if( (m_uButtonState & UISTATE_HOT) != 0 ) {
			iTextColor = pInfo->dwHotTextColor;
		}
		if( IsSelected() ) {
			iTextColor = pInfo->dwSelectedTextColor;
		}
		if( !IsEnabled() ) {
			iTextColor = pInfo->dwDisabledTextColor;
		}

		RECT rcText = { 0, 0, max(szAvailable.cx, m_cxyFixed.cx), 9999 };
		rcText.left += pInfo->rcTextPadding.left;

		rcText.right -= pInfo->rcTextPadding.right;
		
		// 通过DT_CALCRECT获取绘制宽度/高度
		if( pInfo->bShowHtml ) {   
			int nLinks = 0;
			CRenderEngine::DrawHtmlText(m_pManager->GetPaintDC(), m_pManager, rcText, m_sText, iTextColor, NULL, NULL, nLinks, DT_CALCRECT | pInfo->uTextStyle);
		}
		else {
			CRenderEngine::DrawText(m_pManager->GetPaintDC(), m_pManager, rcText, m_sText, iTextColor, pInfo->nFont, DT_CALCRECT | pInfo->uTextStyle);
		}
		
		// 设置高度宽度
		cXY.cx = rcText.right - rcText.left + pInfo->rcTextPadding.left + pInfo->rcTextPadding.right + 20;
		cXY.cy = rcText.bottom - rcText.top + pInfo->rcTextPadding.top + pInfo->rcTextPadding.bottom;
	}

	// 如果有固定高度则使用固定高度
	if( m_cxyFixed.cy != 0 ) 
		cXY.cy = m_cxyFixed.cy;

	return cXY;
}

void CMenuElementUI::DoInit()
{
	// 控件加载完成.判断其是否有子节点
	for( int i = 0; i < GetCount(); ++i ) {
		if( GetItemAt(i)->GetInterface(kMenuElementUIInterfaceName) != NULL ) {
			m_bHasSubMenu = true;
			break;
		}
	}
	
	// 从默认属性列表中加载右侧箭头的属性
	LPCTSTR pDefaultAttributes = m_pManager->GetDefaultAttributeList(kMenuElementUIInterfaceName);
	if (pDefaultAttributes) {
		ApplyAttributeList(pDefaultAttributes);
	}
}

void CMenuElementUI::SetAllSubMenuItemVisible(bool bVisible)
{
	CMenuElementUI*	pMenuItem = NULL;

	for( int i = 0; i < GetCount(); ++i ) {
		pMenuItem = static_cast<CMenuElementUI*>(GetItemAt(i)->GetInterface(kMenuElementUIInterfaceName));
		if( pMenuItem != NULL ){
			pMenuItem->SetVisible(bVisible);
			pMenuItem->SetInternVisible(bVisible);
		}
	}
}

void CMenuElementUI::DoEvent(TEventUI& event)
{
	if(event.Type == UIEVENT_MOUSEENTER){
		CListContainerElementUI::DoEvent(event);
		
		// 如果子菜单项已经创建了,则不进行处理
		if(m_pWindow) 
			return;

		SetAllSubMenuItemVisible();

		// 如果有子菜单则创建之,否则通知有子菜单项的菜单项关闭子菜单
		if(m_bHasSubMenu){
			m_pOwner->SelectItem(GetIndex(), true);
			CreateMenuWnd();
		}
		else
		{
			ContextMenuParam param;
			param.hWnd = m_pManager->GetPaintWindow();
			param.wParam = 2;
			s_context_menu_observer.RBroadcast(param);
			m_pOwner->SelectItem(GetIndex(), true);
		}
		return;
	}

	else if( event.Type == UIEVENT_BUTTONDOWN )
	{
		if( IsEnabled() ){
			CListContainerElementUI::DoEvent(event);

			if( m_pWindow ) return;

			SetAllSubMenuItemVisible();
			if( m_bHasSubMenu )
			{
				CreateMenuWnd();
			}
			else
			{
				ContextMenuParam param;
				param.hWnd = m_pManager->GetPaintWindow();
				param.wParam = 1;
				s_context_menu_observer.RBroadcast(param);
			}
        }
        return;
    }

    CListContainerElementUI::DoEvent(event);
}

bool CMenuElementUI::Activate()
{
	if (CListContainerElementUI::Activate() && m_bSelected)
	{
		if( m_pWindow ) return true;
		SetAllSubMenuItemVisible();
		if (m_bHasSubMenu)
		{
			CreateMenuWnd();
		}
		else
		{
			ContextMenuParam param;
			param.hWnd = m_pManager->GetPaintWindow();
			param.wParam = 1;
			s_context_menu_observer.RBroadcast(param);
		}

		return true;
	}
	return false;
}

CMenuWnd* CMenuElementUI::GetMenuWnd()
{
	return m_pWindow;
}

void CMenuElementUI::CreateMenuWnd()
{
	if( m_pWindow ) return;

	m_pWindow = new CMenuWnd(m_pManager->GetPaintWindow());
	ASSERT(m_pWindow);

	ContextMenuParam param;
	param.hWnd = m_pManager->GetPaintWindow();
	param.wParam = 2;
	s_context_menu_observer.RBroadcast(param);

	m_pWindow->Init(static_cast<CMenuElementUI*>(this), _T(""), _T(""), CPoint());
}

void CMenuElementUI::SetCheckState(bool bCheck)
{
	m_bCheck = bCheck;
}

} // namespace DuiLib
