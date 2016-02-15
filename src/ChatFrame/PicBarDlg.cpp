#include "StdAfx.h"
#include "PicBarDlg.h"

CPicBarDlg::CPicBarDlg(void)
{
	m_pControl = NULL;
	m_pAddFaceBtn = m_pSaveAsBtn = m_pEditFaceBtn = m_pSearchFaceBtn = m_pShareFaceBtn = NULL;
}

CPicBarDlg::~CPicBarDlg(void)
{
}

LPCTSTR CPicBarDlg::GetWindowClassName() const
{
	return _T("DUI_WINDOW");
}

void CPicBarDlg::Init()
{
	m_pAddFaceBtn = static_cast<CButtonExUI*>(paint_manager_.FindControl(_T("btnAddFace")));
	m_pSaveAsBtn = static_cast<CButtonExUI*>(paint_manager_.FindControl(_T("btnSaveAs")));
	m_pEditFaceBtn = static_cast<CButtonExUI*>(paint_manager_.FindControl(_T("btnEditFace")));
	m_pSearchFaceBtn = static_cast<CButtonExUI*>(paint_manager_.FindControl(_T("btnSearchFace")));
	m_pShareFaceBtn = static_cast<CButtonExUI*>(paint_manager_.FindControl(_T("btnShareFace")));
}

tString CPicBarDlg::GetSkinFile()
{
	return _T("PicBarDlg.xml");
}

tString CPicBarDlg::GetSkinFolder()
{
	return tString(CPaintManagerUI::GetInstancePath()) + _T("skin\\");
}

CControlUI* CPicBarDlg::CreateControl(LPCTSTR pstrClass)
{
	if (_tcsicmp(pstrClass, _T("ButtonEx")) == 0)
		return new CButtonExUI;
	return NULL;
}

LRESULT CPicBarDlg::ResponseDefaultKeyEvent(WPARAM wParam)
{
	if (wParam == VK_RETURN)
	{
		return FALSE;
	}
	else if (wParam == VK_ESCAPE)
	{
		return TRUE;
	}
	return FALSE;
}

void CPicBarDlg::OnFinalMessage(HWND hWnd)
{
	WindowImplBase::OnFinalMessage(hWnd);
}

LRESULT CPicBarDlg::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
// 	if (uMsg == WM_ACTIVATE)
// 	{
// 		if (WA_INACTIVE == (UINT)LOWORD(wParam))
// 			::PostMessage(m_hWnd, WM_CLOSE, NULL, NULL);
// 	}
// 	else if (uMsg == WM_CLOSE)
// 	{
// 		//::PostMessage(::GetParent(m_hWnd), FACE_CTRL_SEL, NULL, NULL);
// 		::DestroyWindow(m_hWnd);
// 		return 0;
// 	}
	return __super::HandleMessage(uMsg, wParam, lParam);
}

LRESULT CPicBarDlg::HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	return 0;
}

void CPicBarDlg::SetCallBackCtrl(CControlUI * pControl)
{
	m_pControl = pControl;
}

void CPicBarDlg::Notify(TNotifyUI& msg)
{
	if (msg.sType == _T("click"))
	{
		if (msg.pSender == m_pSaveAsBtn)
			OnBtn_SaveAs(msg);
	}
}

void CPicBarDlg::OnBtn_SaveAs(TNotifyUI& msg)
{
	::ShowWindow(m_hWnd, SW_HIDE);
	if (m_pControl != NULL)
	{
		m_pControl->SetUserData(_T("Menu_SaveAs"));
		m_pControl->GetManager()->SendNotify(m_pControl, _T("menu_msg"), NULL, (LPARAM)m_pControl);
	}
}