#ifndef _SKINDLG_H_
#define _SKINDLG_H_

#include "Utils/Config.h"
#include "ControlEx/skin_change_event.hpp"

class CSkinDlg : public WindowImplBase
{
public:
	CSkinDlg();
	~CSkinDlg();

public:
	LPCTSTR GetWindowClassName() const;
	virtual void OnFinalMessage(HWND hWnd);
	virtual CDuiString GetSkinFile();
	virtual CDuiString GetSkinFolder();
	virtual UILIB_RESOURCETYPE GetResourceType() const;
	virtual void InitWindow();
	void	Notify(TNotifyUI& msg);
	void	SetSkin(LPCTSTR pszImage, DWORD dwColor);
public:
	static	void AddReceiver(SkinChangedReceiver* receiver);
protected:
	virtual LRESULT OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
private:
	CTabLayoutUI*	m_pTLayoutSkin;
	CSliderUI*		m_pSliderColorR;
	CSliderUI*		m_pSliderColorG;
	CSliderUI*		m_pSliderColorB;

	static SkinChangedObserver	m_skinObserver;
};
#endif // _SKINDLG_H_