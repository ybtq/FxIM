#pragma once

#include "win_impl_base.h"
#include "ButtonExUI.h"

class CPicBarDlg : public WindowImplBase
{
public:
	CPicBarDlg(void);
	~CPicBarDlg(void);

public:
	LPCTSTR GetWindowClassName() const;	
	virtual void Init();
	virtual tString GetSkinFile();
	virtual tString GetSkinFolder();
	virtual CControlUI* CreateControl(LPCTSTR pstrClass);
	virtual LRESULT ResponseDefaultKeyEvent(WPARAM wParam);
	virtual void OnFinalMessage(HWND hWnd);
	virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

public:
	void SetCallBackCtrl(CControlUI * pControl);

protected:
	void Notify(TNotifyUI& msg);
	void OnBtn_SaveAs(TNotifyUI& msg);

private:
	CControlUI * m_pControl;
	CButtonExUI * m_pAddFaceBtn, * m_pSaveAsBtn, * m_pEditFaceBtn, * m_pSearchFaceBtn, * m_pShareFaceBtn;
};
