#ifndef __UIMENU_H__
#define __UIMENU_H__

/*
// UI菜单类 [10/31/2014 ybt]
基于Duilib的原MenuDemo中的菜单类修改，参考Redrain的思路：http://blog.csdn.net/zhuhongshu/article/details/38253297。
1、修正菜单项几处显示位置不正确的Bug
2、简化菜单项左侧图标和箭头(如果有子菜单的话)的使用
3、修正菜单窗口圆角设置失效的问题。
4、增加安全的按钮消息及其它一些小细节的优化
*/

#ifdef _MSC_VER
#pragma once
#endif

#define WM_MENUCLICK	(WM_USER + 10)  //用来接收按钮单击的消息  

#include "ControlEx/observer_impl_base.hpp"

namespace DuiLib {

/////////////////////////////////////////////////////////////////////////////////////
//
struct ContextMenuParam
{
	// 1: remove all
	// 2: remove the sub menu
	WPARAM wParam;
	HWND hWnd;
};

enum MenuAlignment
{
	eMenuAlignment_Left = 1 << 1,
	eMenuAlignment_Top = 1 << 2,
	eMenuAlignment_Right = 1 << 3,
	eMenuAlignment_Bottom = 1 << 4,
};

typedef class ObserverImpl<BOOL, ContextMenuParam> ContextMenuObserver;
typedef class ReceiverImpl<BOOL, ContextMenuParam> ContextMenuReceiver;

extern ContextMenuObserver s_context_menu_observer;

// MenuUI
extern const TCHAR* const kMenuUIClassName;// = _T("MenuUI");
extern const TCHAR* const kMenuUIInterfaceName;// = _T("Menu");

class CListUI;
class CMenuUI : public CListUI
{
public:
	CMenuUI();
	~CMenuUI();

    LPCTSTR GetClass() const;
    LPVOID GetInterface(LPCTSTR pstrName);

	virtual void DoEvent(TEventUI& event);

    virtual bool Add(CControlUI* pControl);
    virtual bool AddAt(CControlUI* pControl, int iIndex);

    virtual int GetItemIndex(CControlUI* pControl) const;
    virtual bool SetItemIndex(CControlUI* pControl, int iIndex);
    virtual bool Remove(CControlUI* pControl);

	SIZE EstimateSize(SIZE szAvailable);
};

/////////////////////////////////////////////////////////////////////////////////////
//

// MenuElementUI
extern const TCHAR* const kMenuElementUIClassName;// = _T("MenuElementUI");
extern const TCHAR* const kMenuElementUIInterfaceName;// = _T("MenuElement);

class CMenuElementUI;
class CMenuWnd : public CWindowWnd, public ContextMenuReceiver, public INotifyUI
{
public:
	CMenuWnd(HWND hParent = NULL);
    void	Init(CMenuElementUI* pOwner, STRINGorID xml, LPCTSTR pSkinType, POINT point, DWORD dwAlignment = eMenuAlignment_Right | eMenuAlignment_Bottom);
    LPCTSTR GetWindowClassName() const;
    void	OnFinalMessage(HWND hWnd);

    virtual	LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
	void	Notify(TNotifyUI& msg);

	BOOL	Receive(ContextMenuParam param);

	void	EnableMenuItem(LPCTSTR lpszName, bool bEnable);
	void	CheckMenuItem(LPCTSTR lpszName, bool bCheck);
public:
	HWND				m_hParent;
	POINT				m_BasedPoint;
	STRINGorID			m_xml;
	CDuiString			m_sType;
    CPaintManagerUI		m_pm;
    CMenuElementUI*		m_pOwner;
    CMenuUI*			m_pLayout;
	DWORD				m_dwAlignment;
	static	HWND		m_hWndNotify;				// WM_MENUCLICK消息接受窗口
};

class CListContainerElementUI;
class CMenuElementUI : public CListContainerElementUI
{
	friend CMenuWnd;
public:
    CMenuElementUI();
	~CMenuElementUI();

    LPCTSTR GetClass() const;
    LPVOID GetInterface(LPCTSTR pstrName);

    void DoPaint(HDC hDC, const RECT& rcPaint);

	void DrawItemText(HDC hDC, const RECT& rcItem);

	SIZE EstimateSize(SIZE szAvailable);

	bool Activate();

	void DoEvent(TEventUI& event);

	CMenuWnd* GetMenuWnd();

	void CreateMenuWnd();
	void DoInit();
	void SetAllSubMenuItemVisible(bool bVisible = true);

	void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
	void SetIconImage(LPCTSTR lpszImage);
	void SetArrowImage(LPCTSTR lpszImage);
	void SetCheckImage(LPCTSTR lpszImage);
	void SetIconSize(int cx, int cy);
	void SetArrowSize(int cx, int cy);
	void SetCheckSize(int cx, int cy);
	void SetSeparator(bool bSeparator = true);

	void SetCheckState(bool bCheck);
protected:
	CMenuWnd* m_pWindow;				// 子菜单窗口指针

private:
	CDuiString	m_strIconImage;
	CDuiString	m_strArrowImage;
	CDuiString	m_strCheckImage;
	SIZE		m_szIconImage;
	SIZE		m_szArrowImage;
	SIZE		m_szCheckImage;
	bool		m_bHasSubMenu;
	bool		m_bIsSeparator;
	bool		m_bCheck;
};

} // namespace DuiLib

#endif // __UIMENU_H__
