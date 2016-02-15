#pragma once

#include "ChatFrame/FaceCtrl.h"

class CFaceSelDlg : public WindowImplBase
{
public:
	CFaceSelDlg(void);
	~CFaceSelDlg(void);

public:
	LPCTSTR GetWindowClassName() const;	
	virtual void InitWindow();
	virtual CDuiString GetSkinFile();
	virtual CDuiString GetSkinFolder();
	virtual UILIB_RESOURCETYPE GetResourceType() const;
	virtual CControlUI* CreateControl(LPCTSTR pstrClass);
	virtual LRESULT ResponseDefaultKeyEvent(WPARAM wParam);
	virtual void OnFinalMessage(HWND hWnd);
	virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

public:
	int			GetSelFaceId();
	int			GetSelFaceIndex();
	CDuiString	GetSelFaceFileName();
	void		SetNotifyWnd(HWND hWnd);
	CFaceList&	GetFaceList() { return m_faceList; }
protected:
	void Notify(TNotifyUI& msg);

private:
	CFaceCtrl*	m_pFaceCtrl;
	CFaceList	m_faceList;
	int			m_nSelFaceId;
	int			m_nSelFaceIndex;
	CDuiString	m_strSelFaceFileName;
	HWND		m_hNotifyWnd;
};
