#include "StdAfx.h"
#include "Utils/UICrack.h"
#include "ChatFrame/FaceSelDlg.h"

CFaceSelDlg::CFaceSelDlg(void)
{
	m_nSelFaceId = -1;
	m_nSelFaceIndex = -1;
	m_strSelFaceFileName = _T("");
	m_hNotifyWnd = NULL;

	CDuiString	strDefaultFaceConfig = CPaintManagerUI::GetInstancePath() + _T("face\\FaceConfig.xml");
	m_faceList.LoadConfigFile(strDefaultFaceConfig);
}

CFaceSelDlg::~CFaceSelDlg(void)
{
}

LPCTSTR CFaceSelDlg::GetWindowClassName() const
{
	return kWindowClassName;
}

void CFaceSelDlg::InitWindow()
{
	m_pFaceCtrl = static_cast<CFaceCtrl*>(m_PaintManager.FindControl(_T("facectrl1")));

	//m_pFaceCtrl->SetBgColor(RGB(255, 255, 255));
	m_pFaceCtrl->SetLineColor(RGB(223, 230, 246));
	m_pFaceCtrl->SetFocusBorderColor(RGB(0, 0, 255));
	m_pFaceCtrl->SetZoomBorderColor(RGB(0, 138, 255));
	m_pFaceCtrl->SetRowAndCol(8, 15);
	m_pFaceCtrl->SetItemSize(28, 28);
	m_pFaceCtrl->SetZoomSize(84, 84);
	m_pFaceCtrl->SetFaceList(&m_faceList);
	m_pFaceCtrl->SetCurPage(0);

	m_nSelFaceId = -1;
	m_nSelFaceIndex = -1;
	m_strSelFaceFileName = _T("");
}

CDuiString CFaceSelDlg::GetSkinFile()
{
	return _T("xmls\\dlg_facesel.xml");
}

CDuiString CFaceSelDlg::GetSkinFolder()
{
	return  _T("skin");
}

UILIB_RESOURCETYPE CFaceSelDlg::GetResourceType() const
{
	return UILIB_FILE;
}

CControlUI* CFaceSelDlg::CreateControl(LPCTSTR pstrClass)
{
	if (_tcsicmp(pstrClass, _T("FaceCtrl")) == 0) {
		return new CFaceCtrl();
	}
	return NULL;
}

LRESULT CFaceSelDlg::ResponseDefaultKeyEvent(WPARAM wParam)
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

void CFaceSelDlg::OnFinalMessage(HWND hWnd)
{
	WindowImplBase::OnFinalMessage(hWnd);
}

LRESULT CFaceSelDlg::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_ACTIVATE)
	{
		if (WA_INACTIVE == (UINT)LOWORD(wParam))
		{
			// Post message WM_SHOWWINDOW failed, why?
			// ::PostMessage(m_hWnd, WM_SHOWWINDOW, FALSE, NULL);
			ShowWindow(false);
		}
	}
	return __super::HandleMessage(uMsg, wParam, lParam);
}

LRESULT CFaceSelDlg::HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	return 0;
}

int CFaceSelDlg::GetSelFaceId()
{
	return m_nSelFaceId;
}

int CFaceSelDlg::GetSelFaceIndex()
{
	return m_nSelFaceIndex;
}

CDuiString CFaceSelDlg::GetSelFaceFileName()
{
	return m_strSelFaceFileName;
}

void CFaceSelDlg::Notify(TNotifyUI& msg)
{
	if (msg.sType == _T("click"))
	{
		if (msg.pSender == m_pFaceCtrl)
		{
			int nSelIndex = (int)msg.lParam;
			CFaceInfo * lpFaceInfo = m_pFaceCtrl->GetFaceInfo(nSelIndex);
			if (lpFaceInfo != NULL)
			{
				m_nSelFaceId = lpFaceInfo->m_nId;
				m_nSelFaceIndex = lpFaceInfo->m_nIndex;
				m_strSelFaceFileName = lpFaceInfo->m_strFileName.data();
			}
			::PostMessage(m_hNotifyWnd, FACE_CTRL_SEL, NULL, NULL);
			ShowWindow(false);
		}
	}
}

void CFaceSelDlg::SetNotifyWnd(HWND hWnd)
{
	m_hNotifyWnd = hWnd;
}