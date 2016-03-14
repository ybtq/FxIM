#include "StdAfx.h"
#include "Utils/UICrack.h"
#include "MainFrame/SkinDlg.h"
#include "MainFrame/SettingDlg.h"
#include "MainFrame/MainDlg.h"

CSettingDlg::CSettingDlg()
 : m_bPersonalChange(FALSE)
 , m_bGeneralChange(FALSE)
 , m_bNetworkChange(FALSE)
 , m_pTabLayoutSetting(NULL)
 , m_pVerticalLayoutGeneral(NULL)
 , m_pLabelAvatar(NULL)
 , m_pEditNickName(NULL)
 , m_pEditGroupName(NULL)
 , m_pVerticalLayoutPersonal(NULL)
 , m_pVerticalLayoutNetwork(NULL)
 , m_pComboAdapter(NULL)
{

}

CSettingDlg::~CSettingDlg()
{

}

LPCTSTR CSettingDlg::GetWindowClassName() const
{
	return kWindowClassName;
}

void CSettingDlg::OnFinalMessage(HWND hWnd)
{
	RemoveObserver();
	CWindowWnd::OnFinalMessage(hWnd);
	delete this;
}

CDuiString CSettingDlg::GetSkinFile()
{
	return _T("xmls\\dlg_setting.xml");
}

CDuiString CSettingDlg::GetSkinFolder()
{
	return _T("skin");
}

UILIB_RESOURCETYPE CSettingDlg::GetResourceType() const
{
	return UILIB_FILE;
}

BOOL CSettingDlg::Receive(SkinChangedParam param)
{
	return SetControlBackGround(m_PaintManager.FindControl(_T("bg")), param.bgimage, param.bkcolor);
}

void CSettingDlg::InitWindow()
{
	Config*	pCfg = Singleton<Config>::getInstance();
	CSkinDlg::AddReceiver(this);
	SetControlBackGround(m_PaintManager.FindControl(_T("bg")), pCfg->szBgImage, pCfg->dwBgColor);
	
	m_pTabLayoutSetting = static_cast<CTabLayoutUI*>(m_PaintManager.FindControl(_T("tablayout_setting")));
	m_pVerticalLayoutPersonal = static_cast<CVerticalLayoutUI*>(m_PaintManager.FindControl(_T("verticallayout_personal")));
	m_pLabelAvatar = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("label_avatar")));
	m_pEditNickName = static_cast<CEditUI*>(m_PaintManager.FindControl(_T("edit_nickname")));
	m_pEditGroupName = static_cast<CEditUI*>(m_PaintManager.FindControl(_T("edit_groupname")));

	ASSERT(m_pLabelAvatar != NULL);
	ASSERT(m_pEditNickName != NULL);
	ASSERT(m_pEditGroupName != NULL);

	if (::PathFileExists(pCfg->szAvatar))
	{
		m_pLabelAvatar->SetBkImage(pCfg->szAvatar);
		m_pLabelAvatar->SetUserData(pCfg->szAvatar);
	}
	else
	{
		m_pLabelAvatar->SetBkImage(DEFAULT_AVATAR);
	}
	m_pEditNickName->SetText(pCfg->szNickName);
	m_pEditGroupName->SetText(pCfg->szGroupName);

	// 常规设置
	m_pVerticalLayoutGeneral = static_cast<CVerticalLayoutUI*>(m_PaintManager.FindControl(_T("verticallayout_general")));
	ASSERT(m_pVerticalLayoutGeneral != NULL);

	// 网络设置
	m_pVerticalLayoutNetwork = static_cast<CVerticalLayoutUI*>(m_PaintManager.FindControl(_T("verticallayout_network")));
	m_pComboAdapter = static_cast<CComboBoxUI*>(m_PaintManager.FindControl(_T("combo_adapter")));
	ASSERT(m_pVerticalLayoutNetwork != NULL);
	ASSERT(m_pComboAdapter != NULL);
	
	InitTabLayoutNetWork();

	// InitTabLayoutNetWork()中选择网卡时会有DUI_MSGTYPE_ITEMSELECT消息，因此初始化完成后，各个标志位恢复其值
	m_bNetworkChange = FALSE;		
	m_bGeneralChange = FALSE;
	m_bPersonalChange = FALSE;
}

void CSettingDlg::InitTabLayoutNetWork()
{
	char				szBuf[MAX_PATH];				// PIP_ADAPTER_INFO中的字符串是Ansi
	Config*				pCfg = Singleton<Config>::getInstance();
	PIP_ADAPTER_INFO	pAdapter = pCfg->pIpAdapterInfo;

	// 先将该页显示，不然设置不能成功
	m_pTabLayoutSetting->SelectItem(m_pVerticalLayoutNetwork);	
	while (pAdapter != NULL) 
	{
		wsprintfA(szBuf, "%s (%s)", pAdapter->IpAddressList.IpAddress.String, pAdapter->Description);
		CListLabelElementUI* pListLabel = new CListLabelElementUI;
		if (pListLabel == NULL) {
			continue;
		}
#if (defined(UNICODE) || defined(_UNICODE))
		LPCWSTR pszBufW = AnsiToUnicode(szBuf);
		if (pszBufW != NULL)
		{
			pListLabel->SetText(pszBufW);
			pListLabel->SetToolTip(pszBufW);
			delete pszBufW;
		}
		LPCWSTR pszAdapterNameW = UnicodeToAnsi(pAdapter->AdapterName);
		if (pszAdapterNameW != NULL)
		{
			pListLabel->SetUserData(pszAdapterNameW);
			delete pszAdapterNameW;
		}
#else
		pListLabel->SetText(szBuf);
		pListLabel->SetToolTip(szBuf);
		pListLabel->SetUserData(pAdapter->AdapterName);
#endif				
		m_pComboAdapter->Add(pListLabel);
		if (_tcscmp(pAdapter->AdapterName, pCfg->szAdapterNameUse) == 0) {
			pListLabel->Select();
		}

		pAdapter = pAdapter->Next;
	}
	m_pTabLayoutSetting->SelectItem(m_pVerticalLayoutPersonal);
}

void CSettingDlg::Notify(TNotifyUI& msg)
{
	CDuiString strSenderName = msg.pSender->GetName();
	if (_tcscmp(msg.sType, DUI_MSGTYPE_CLICK) == 0)
	{
		if (_tcscmp(strSenderName, kButtonSysCloseName) == 0
			|| _tcscmp(strSenderName, _T("btn_cancel")) == 0)
		{
			Close();
		}
		else if (_tcscmp(strSenderName, kButtonSysMinName) == 0)
		{
			SendMessage(WM_SYSCOMMAND, SC_MINIMIZE, 0);
		}
		else if (_tcscmp(strSenderName, _T("btn_ok")) == 0)
		{
			OnOK();
		}
		else if (_tcscmp(strSenderName, _T("option_personal")) == 0)
		{
			if (m_pTabLayoutSetting != NULL && m_pVerticalLayoutPersonal != NULL) {
				m_pTabLayoutSetting->SelectItem(m_pVerticalLayoutPersonal);
			}
		}
		else if (_tcscmp(strSenderName, _T("option_general")) == 0)
		{
			if (m_pTabLayoutSetting != NULL && m_pVerticalLayoutGeneral != NULL) {
				m_pTabLayoutSetting->SelectItem(m_pVerticalLayoutGeneral);
			}
		}
		else if (_tcscmp(strSenderName, _T("option_network")) == 0)
		{
			if (m_pTabLayoutSetting != NULL && m_pVerticalLayoutNetwork != NULL) {
				m_pTabLayoutSetting->SelectItem(m_pVerticalLayoutNetwork);
			}
		}
		// General
		else if (_tcscmp(strSenderName, _T("btn_msgtipfile")) == 0)
		{
			
		}
		// Network
		else if (_tcscmp(strSenderName, _T("btn_netsegment_add")) == 0)
		{
			OnBtnNetSegmentAdd();
		}
		else if (_tcscmp(strSenderName, _T("btn_netsegment_del")) == 0)
		{
			OnBtnNetSegmentDel();
		}
		// Personal
		else if (_tcscmp(strSenderName, _T("btn_avatar_border")) == 0)
		{
			OnBrowserAvatarFile();
		}
	}
	// Combo value changed.
	else if (_tcscmp(msg.sType, DUI_MSGTYPE_ITEMSELECT) == 0)
	{
		if (_tcscmp(strSenderName, _T("combo_adapter")) == 0)
		{
			m_bNetworkChange = TRUE;
		}
	}
	// Edit value changed.
	else if (_tcscmp(msg.sType, DUI_MSGTYPE_TEXTCHANGED) == 0)
	{
		if (_tcscmp(strSenderName, _T("edit_nickname")) == 0
			|| _tcscmp(strSenderName, _T("edit_groupname")) == 0)
		{
			m_bPersonalChange = TRUE;
		}
		else if (_tcscmp(strSenderName, _T("edit_msgtipfile")) == 0)
		{
			m_bGeneralChange = TRUE;
		}
	}
}

void CSettingDlg::OnBtnNetSegmentAdd()
{
	CEditUI*	pEditNetSegment = static_cast<CEditUI*>(m_PaintManager.FindControl(_T("edit_netsegment")));
	CListUI*	pListNetSegment = static_cast<CListUI*>(m_PaintManager.FindControl(_T("list_netsegment")));
	
	ASSERT(pEditNetSegment != NULL && pListNetSegment != NULL);
	
	CDuiString	strText = pEditNetSegment->GetText();
	if (strText.IsEmpty())
	{
		return ;
	}
	//DUI__Trace(_T("inet_addr(strText) = 0x%08X."), inet_addr(strText));
	// inet_addr(strText) == INADDR_NONE 并不能判断，比如输入一个"1"，inet_addr会返回0x01000000 [12/27/2014 ybt]
	if (!IsValidAddrString(strText))
	{
		MessageBox(m_hWnd, _T("请输入正确的IP地址."), APPNAME_STR, MB_OK);
		return ;
	}
	pEditNetSegment->SetText(_T(""));
	CListLabelElementUI* pLableElement = new CListLabelElementUI;
	pLableElement->SetText(strText);
	pListNetSegment->Add(pLableElement);
	m_bNetworkChange = TRUE;
}

void CSettingDlg::OnBtnNetSegmentDel()
{
	CListUI*	pListNetSegment = static_cast<CListUI*>(m_PaintManager.FindControl(_T("list_netsegment")));
	ASSERT(pListNetSegment != NULL);
	if (pListNetSegment->GetCurSel() >= 0)
	{
		pListNetSegment->Remove(pListNetSegment->GetItemAt(pListNetSegment->GetCurSel()));
		m_bNetworkChange = TRUE;
	}
}

void CSettingDlg::OnBrowserAvatarFile()
{
	TCHAR szFiles[MAX_BUF] = {0};
	TCHAR szFilter[] = _T("图像文件(*.bmp;*.jpg;*.jpeg;*.gif;*.png)\0*.bmp;*.jpg;*.jpeg;*.gif;*.png\0\0");

	OPENFILENAME ofn;      
	ZeroMemory(&ofn, sizeof(ofn));

	ofn.Flags			  = OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY | OFN_PATHMUSTEXIST 
								| OFN_ALLOWMULTISELECT | OFN_EXPLORER;
	ofn.lStructSize       = sizeof(ofn);
	ofn.hwndOwner         = m_hWnd;
	ofn.lpstrFile         = szFiles;
	ofn.nMaxFile          = sizeof(szFiles);
	ofn.lpstrFilter       = szFilter;
	ofn.nFilterIndex      = 1;
	ofn.lpstrFileTitle    = NULL;
	ofn.nMaxFileTitle     = 0;
	ofn.lpstrInitialDir   = NULL;
	ofn.lpstrDefExt		  = NULL;

	if (GetOpenFileName(&ofn) == FALSE) {
		return;
	}

	TCHAR	szMd5[MAX_MD5] = {0};
	TCHAR	szAvatarFile[MAX_NAMEBUF] = {0};
	TCHAR	szAvatarPath[MAX_PATH] = {0};
	Config*	pCfg = Singleton<Config>::getInstance();
	if (!MD5File(szMd5, ofn.lpstrFile))
	{
		return ;
	}
	wsprintf(szAvatarFile, _T("%s%s"), szMd5, ::PathFindExtension(ofn.lpstrFile));
	wsprintf(szAvatarPath, _T("%s%s"), pCfg->szAvatarSaveDir, szAvatarFile);
	if (!::CopyFile(ofn.lpstrFile, szAvatarPath, FALSE))
	{
		return ;
	}

	m_pLabelAvatar->SetBkImage(szAvatarPath);
	m_pLabelAvatar->SetUserData(szAvatarPath);
	m_bPersonalChange = TRUE;
}

void CSettingDlg::OnOK()
{
	int		nWriteFlag = 0;
	bool	bNeedRestart = false;
	bool	bAvatarChanged = false;
	Config*	pCfg = Singleton<Config>::getInstance();

	// 个人设置
	if (m_bPersonalChange)
	{
		CDuiString strNickName = m_pEditNickName->GetText();
		if (_tcscmp(strNickName, pCfg->szNickName) != 0) {
			_tcsncpy(pCfg->szNickName, strNickName, MAX_NAMEBUF - 1);
			nWriteFlag |= CONFIG_PERSONAL;
		}
		CDuiString strGroupName = m_pEditGroupName->GetText();
		if (_tcscmp(strGroupName, pCfg->szGroupName) != 0) {
			_tcsncpy(pCfg->szGroupName, strGroupName, MAX_NAMEBUF - 1);
			nWriteFlag |= CONFIG_PERSONAL;
		}
		CDuiString strAvatarPath = m_pLabelAvatar->GetUserData();
		if (_tcscmp(strAvatarPath, pCfg->szAvatar) != 0)
		{
			_tcsncpy(pCfg->szAvatar, strAvatarPath, MAX_PATH - 1);
			nWriteFlag |= CONFIG_PERSONAL;
			bAvatarChanged = true;
		}
	}	

	// 网络设置
	if (m_bNetworkChange)
	{
		CControlUI* pControl = m_pComboAdapter->GetItemAt(m_pComboAdapter->GetCurSel());
		CDuiString strAdapterName = pControl->GetUserData();
		if (_tcscmp(strAdapterName, pCfg->szAdapterNameUse) != 0) {
			_tcsncpy(pCfg->szAdapterNameUse, strAdapterName, MAX_PATH - 1);
			bNeedRestart = true;
			nWriteFlag |= CONFIG_NETWORK;
		}
	}	

	if (nWriteFlag != 0) {
		pCfg->WriteConfig(nWriteFlag);
	}
	CMainDlg* pMainDlg = Singleton<CMainDlg>::getInstance();
	if (m_bPersonalChange || m_bNetworkChange)
	{
		pMainDlg->UpdateInfo();
	}
	if (bNeedRestart)
	{
		::MessageBox(NULL, _T("需要重启程序"), _T("提示"), MB_OK);
		pMainDlg->RestartApplication();
	}
	if (m_bPersonalChange)
	{
		pMainDlg->BroadcastEntry();
	}
	if (bAvatarChanged)
	{
		pMainDlg->BroadcastMsg(IM_ANSAVATAR, ::PathFindFileNameA(pCfg->szAvatar));
	}
	Close();
}