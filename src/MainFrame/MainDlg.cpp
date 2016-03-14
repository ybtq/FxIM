#include "stdafx.h"
#include "Resource.h"
#include "Utils/UICrack.h"
#include "Utils/BLOWFISH.H"
#include "MainFrame/MainDlg.h"
#include "MainFrame/SkinDlg.h"
#include "MainFrame/ShareMng.h"
#include "MainFrame/SettingDlg.h"
#include "ControlEx/UIMenu.h"

MapChatDlg CMainDlg::m_mapChatDlg;

CMainDlg::CMainDlg()
 : m_pFriendList(NULL)
 , m_pLabelNickName(NULL)
 , m_pLabelDescription(NULL)
 , m_pLabelAvatar(NULL)
 , m_bMsgJumpTimer(FALSE)
 , m_pMsgJumpHost(NULL)
{
	Config* pCfg = Singleton<Config>::getInstance();
	m_nPortNo = htons(pCfg->nPort);
}

CMainDlg::~CMainDlg()
{
}

LPCTSTR CMainDlg::GetWindowClassName() const
{
	return kWindowClassName;
}

void CMainDlg::OnFinalMessage(HWND hWnd)
{
	// 在此处TaskBar(NIM_DELETE);虽然执行了，但并没有删除成功。
	//TaskBar(NIM_DELETE);
	RemoveObserver();
	WindowImplBase::OnFinalMessage(hWnd);
	PostQuitMessage(0);
}


CDuiString CMainDlg::GetSkinFile()
{
	return _T("xmls\\dlg_main.xml");
}

CDuiString CMainDlg::GetSkinFolder()
{
	return  _T("skin");
}

UILIB_RESOURCETYPE CMainDlg::GetResourceType() const
{
	return UILIB_FILE;
}

CControlUI*	CMainDlg::CreateControl(LPCTSTR pstrClass)
{
	if (_tcsicmp(pstrClass, _T("FriendList")) == 0)
	{
		return new CFriendsUI();
	}
	return NULL;
}

void CMainDlg::InitWindow()
{
	CSkinDlg::AddReceiver(this);
	Config* pCfg = Singleton<Config>::getInstance();
	ASSERT(pCfg);

	SetControlBackGround(m_PaintManager.FindControl(_T("bg")), pCfg->szBgImage, pCfg->dwBgColor);

	m_hIcon = LoadIcon(m_PaintManager.GetInstance(), MAKEINTRESOURCE(IDI_FXIM));
	m_hIconNull = LoadIcon(m_PaintManager.GetInstance(), MAKEINTRESOURCE(IDI_NULL));		// 注意null图标不能是1bit的，不然null图标设置不会成功，也就是托盘图标不会闪动
	TaskBar(NIM_ADD, m_hIcon, _T("FxIM"));

	InitCrypt();
	CMsgMng* pMsgMng = Singleton<CMsgMng>::getInstance();
	pMsgMng->AsyncSelect(m_hWnd);
	BroadcastEntry();

	m_pFriendList = static_cast<CFriendsUI*>(m_PaintManager.FindControl(_T("friends")));
	m_pLabelAvatar = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("label_avatar")));
	m_pLabelNickName = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("label_nickname")));
	m_pLabelDescription = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("label_description")));
	ASSERT(m_pFriendList);
	ASSERT(m_pLabelAvatar);
	ASSERT(m_pLabelNickName);
	ASSERT(m_pLabelDescription);

	UpdateInfo();
}

void CMainDlg::Close(UINT nRet)
{
	BroadcastMsg(IM_BR_EXIT);
	TaskBar(NIM_DELETE);

	int						cnt;
	MapChatDlg::iterator	iter;
	CChatDlg*				pChatDlg = NULL;

	for (iter = m_mapChatDlg.begin(); iter != m_mapChatDlg.end(); iter++)
	{
		pChatDlg = iter->second;
		if (pChatDlg != NULL) {
			pChatDlg->Close();		// CChatDlg::OnFinalMessage Will delete itself.
		}
	}
	m_mapChatDlg.clear();

	// free msg.
	MsgBuf*	msg;
	for (cnt = 0; cnt < (int)m_vecMsgBuf.size(); cnt++)
	{
		msg = m_vecMsgBuf[cnt];
		if (msg) {
			delete msg;
			msg = NULL;
		}
	}
	m_vecMsgBuf.clear();

	__super::Close(nRet);
}


LRESULT CMainDlg::HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	switch (uMsg)
	{
	case WM_UDPEVENT:
		OnUdpEvent(uMsg, wParam, lParam, bHandled);
		break;
	case WM_TCPEVENT:
		OnTcpEvent(uMsg, wParam, lParam, bHandled);
		break;
	case WM_TASKBARNOTIFY:
		OnTaskbarNotify(uMsg, wParam, lParam, bHandled);
		break;
	case WM_MENUCLICK:
		OnMenuClick(uMsg, wParam, lParam, bHandled);
		break;
	case WM_TIMER:
		OnTimer(uMsg, wParam, lParam, bHandled);
		break;
	case WM_RECVOBJEVENT:
		OnRecvObjEvent(uMsg, wParam, lParam, bHandled);
		break;
	}
	return 0;
}

void CMainDlg::Notify(TNotifyUI& msg)
{
	CDuiString strSenderName = msg.pSender->GetName();
	if (_tcscmp(msg.sType, DUI_MSGTYPE_CLICK) == 0)
	{
		if (_tcscmp(strSenderName, kButtonSysCloseName) == 0)
		{
			Close();
		}
		else if (_tcscmp(strSenderName, kButtonSysMinName) == 0)
		{
#if defined(UNDER_CE)
			::ShowWindow(m_hWnd, SW_MINIMIZE);
#else
			SendMessage(WM_SYSCOMMAND, SC_MINIMIZE, 0);
#endif
		}
		else if (_tcscmp(strSenderName, _T("btn_main_menu")) == 0)
		{
			CMenuWnd*	pMenu = new CMenuWnd(m_hWnd);
			
			if (pMenu != NULL)
			{
				RECT		rcBtn = msg.pSender->GetPos();
				POINT		pt = {rcBtn.left, rcBtn.top};
				STRINGorID	xml(_T("xmls\\menu_taskbar.xml"));

				::ClientToScreen(m_hWnd, &pt);		
				pMenu->Init(NULL, xml, _T("xml"), pt, eMenuAlignment_Top | eMenuAlignment_Right);
			}
		}
		else if (_tcscmp(strSenderName, _T("btn_skin")) == 0)
		{
			CSkinDlg*	pSkinDlg = new CSkinDlg;
			RECT		rcBtn = msg.pSender->GetPos();
			POINT		pt = {rcBtn.left, rcBtn.bottom};

			ClientToScreen(m_hWnd, &pt);
			if (!IsWindow(pSkinDlg->GetHWND()))
			{
				pSkinDlg->Create(NULL, _T(""), UI_WNDSTYLE_FRAME & ~WS_MAXIMIZEBOX, WS_EX_TOOLWINDOW, 
					0, 0, 0, 0);
				::SetWindowPos(pSkinDlg->GetHWND(), NULL, pt.x, pt.y, 0, 0, SWP_NOSIZE);
			}
		}
	}
	else if (_tcscmp(msg.sType, DUI_MSGTYPE_ITEMACTIVATE) == 0)
	{
		if (_tcscmp(strSenderName, _T("friendnode")) == 0)
		{	
			Host*			pHost = NULL;			
			CFriendNodeUI*	pFriendNode = static_cast<CFriendNodeUI*>(msg.pSender);
			if (pFriendNode != NULL && (pHost = pFriendNode->GetHost()) != NULL)
			{
				ShowChatDlg(pHost, TRUE);
			}
		}
	}
	else if (_tcscmp(msg.sType, DUI_MSGTYPE_MENU) == 0)
	{
		if (_tcscmp(strSenderName, _T("friendnode")) == 0)
		{
			CMenuWnd*	pMenu = new CMenuWnd(m_hWnd);

			if (pMenu != NULL)
			{
				CPoint		pt;
				STRINGorID	xml(_T("xmls\\menu_friendnode.xml"));

				::GetCursorPos(&pt);		
				pMenu->Init(NULL, xml, _T("xml"), pt);
			}
		}
	}
// 	else if (_tcscmp(msg.sType, DUI_MSGTYPE_TIMER) == 0)
// 	{
// 		MessageBox(m_hWnd, "fd", "fdf", MB_OKCANCEL);
// 	}
}


BOOL CMainDlg::TaskBar(DWORD nimMode, HICON hIcon /* = NULL */, LPCTSTR lpszTip /* = NULL */)
{
	NOTIFYICONDATA nid;

	nid.cbSize = sizeof(nid);
	nid.hIcon = hIcon;
	nid.hWnd = m_hWnd;
	nid.uCallbackMessage = WM_TASKBARNOTIFY;
	nid.uFlags = NIF_MESSAGE| (hIcon ? NIF_ICON : 0)| (lpszTip ? NIF_TIP : 0);
	nid.uID = IM_DEFAULT_PORT;
	if(lpszTip != NULL) { 
		ZeroMemory(nid.szTip, sizeof(nid.szTip)); 
		_tcsncpy(nid.szTip, lpszTip, sizeof(nid.szTip) / sizeof(TCHAR) - 1); 
	}

	return ::Shell_NotifyIcon(nimMode, &nid);
}

BOOL CMainDlg::InitCrypt()
{
	Config* pCfg = Singleton<Config>::getInstance();
	pCfg->hCsp = NULL;
	pCfg->hSmallCsp = NULL;
	if (pCfg->privEncryptType	== PRIV_BLOB_DPAPI) {
		pCfg->privEncryptType = PRIV_BLOB_RAW;
	}

	InitCryptCore();

#define MAX_RETRY	3
	int	cnt = 0;
	while (pCfg->hCsp && pCfg->pubKey.key == NULL || pCfg->hSmallCsp && pCfg->smallPubKey.key == NULL)
	{
		if (++cnt > MAX_RETRY)
			break;
		if (pCfg->hCsp)
			CryptReleaseContext(pCfg->hCsp, 0), pCfg->hCsp = NULL;
		if (pCfg->hSmallCsp)
			CryptReleaseContext(pCfg->hSmallCsp, 0), pCfg->hSmallCsp = NULL;
		::Sleep(1000);
		InitCryptCore(cnt == MAX_RETRY ? KEY_DIAG : 0);
	}
	if (cnt > MAX_RETRY || pCfg->pubKey.key == NULL && pCfg->smallPubKey.key == NULL)
	{
		if (MessageBox(NULL, "RSA failed. Create New RSA key?", "msg", MB_OKCANCEL) == IDOK)
			InitCryptCore(KEY_REBUILD|KEY_DIAG);
	}
	return pCfg->pubKey.key || pCfg->smallPubKey.key;
}

BOOL CMainDlg::InitCryptCore(int ctl_flg /* = 0*/)
{
	BYTE	data[MAX_BUF] = {0};
	int		len = sizeof(data);
	Config* pCfg = Singleton<Config>::getInstance();

	// RSA
	SetupRSAKey(1024, ctl_flg);
	SetupRSAKey(512, ctl_flg);

	// Self Check 1024bit
	if (pCfg->pubKey.key)
	{
		BOOL		ret = FALSE;
		HCRYPTKEY	hExKey = 0;

		pCfg->pubKey.KeyBlob(data, sizeof(data), &len);
		if (CryptImportKey(pCfg->hCsp, data, len, 0, 0, &hExKey))
		{
			len = 128/8;
			if (CryptEncrypt(hExKey, 0, TRUE, 0, data, (DWORD *)&len, MAX_BUF))
				ret = TRUE;
			else if (ctl_flg & KEY_DIAG)
				odprintf(_T("CryptEncrypt test1024 failed, error 0x%08X"), GetLastError());
			CryptDestroyKey(hExKey);
		}
		else if (ctl_flg & KEY_DIAG) 
			odprintf(_T("CryptImportKey test1024 failed, error 0x%08X"), GetLastError());

		if (ret) {
			ret = FALSE;
			if (CryptDecrypt(pCfg->hPrivKey, 0, TRUE, 0, (BYTE *)data, (DWORD *)&len))
				ret = TRUE;
			else if (ctl_flg & KEY_DIAG) 
				odprintf(_T("CryptDecrypt test1024 failed, error 0x%08X"), GetLastError());
		}

		if (ret == FALSE)
			pCfg->pubKey.ReSet();
	}

	// Self Check 512bit
	if (pCfg->smallPubKey.key) 
	{
		BOOL		ret = FALSE;
		BYTE		tmp[MAX_BUF] = {0};
		DWORD		tmplen = MAX_BUF / 2;
		HCRYPTKEY	hKey = 0, hExKey = 0;

		pCfg->smallPubKey.KeyBlob(data, sizeof(data), &len);
		if (CryptImportKey(pCfg->hSmallCsp, data, len, 0, 0, &hExKey))
		{
			if (CryptGenKey(pCfg->hSmallCsp, CALG_RC2, CRYPT_EXPORTABLE, &hKey))
			{
				CryptExportKey(hKey, hExKey, SIMPLEBLOB, 0, NULL, (DWORD *)&len);
				if (CryptExportKey(hKey, hExKey, SIMPLEBLOB, 0, data, (DWORD *)&len)) 
				{
					if (CryptEncrypt(hKey, 0, TRUE, 0, tmp, &tmplen, MAX_BUF))
						ret = TRUE;
					else if (ctl_flg & KEY_DIAG) 
						odprintf(_T("CryptEncrypt test512 failed, error 0x%08X"), GetLastError());
				}
				else if (ctl_flg & KEY_DIAG) 
					odprintf(_T("CryptExportKey test512 failed, error 0x%08X"), GetLastError());
				CryptDestroyKey(hKey);
			}
			else if (ctl_flg & KEY_DIAG) 
				odprintf(_T("CryptGenKey test512 failed, error 0x%08X"), GetLastError());
			CryptDestroyKey(hExKey);
		}
		else if (ctl_flg & KEY_DIAG)
			odprintf(_T("CryptImportKey test512 failed, error 0x%08X"), GetLastError());

		if (ret) {
			ret = FALSE;
			if (CryptImportKey(pCfg->hSmallCsp, data, len, pCfg->hSmallPrivKey, 0, &hKey)) {
				if (CryptDecrypt(hKey, 0, TRUE, 0, (BYTE *)tmp, (DWORD *)&tmplen))
					ret = TRUE;
				else if (ctl_flg & KEY_DIAG) 
					odprintf(_T("CryptDecrypt test512 failed, error 0x%08X"), GetLastError());
				CryptDestroyKey(hKey);
			}
			else if (ctl_flg & KEY_DIAG)
				odprintf(_T("CryptImportKey test512 failed, error 0x%08X"), GetLastError());
		}

		if (ret == FALSE)
			pCfg->smallPubKey.ReSet();
	}
	//调试时不知为什么总出现如下错误（5），后辛苦找出来了，原来原因在于hPrivKey hSmallPrivKey没有赋值
	//First-chance exception in Messager.exe (ADVAPI32.DLL): 0xC0000005: Access Violation
	if (pCfg->pubKey.key == NULL && pCfg->hPrivKey)		//
		CryptDestroyKey(pCfg->hPrivKey), pCfg->hPrivKey = NULL;
	if (pCfg->smallPubKey.key == NULL && pCfg->hSmallPrivKey)
		CryptDestroyKey(pCfg->hSmallPrivKey), pCfg->hSmallPrivKey = NULL;

	return	pCfg->pubKey.key || pCfg->smallPubKey.key;
}

BOOL CMainDlg::SetupRSAKey(int bitLen, int ctl_flg /* = 0 */)
{
	BOOL		isSmall = (bitLen == 512) ? TRUE : FALSE;
	BYTE		data[MAX_BUF] = {0};
	char		contName[MAX_PATH] = {0};
	int			len = sizeof(data);
	Config*		pCfg = Singleton<Config>::getInstance();
	CMsgMng*	pMsgMng = Singleton<CMsgMng>::getInstance();
	HCRYPTPROV&	hCsp = isSmall ? pCfg->hSmallCsp : pCfg->hCsp;
	HCRYPTKEY&	hPrivKey = isSmall ? pCfg->hSmallPrivKey : pCfg->hPrivKey;
	PubKey&		pubKey = isSmall ? pCfg->smallPubKey : pCfg->pubKey;
	const char	*csp_name = isSmall ? MS_DEF_PROV : MS_ENHANCED_PROV;

	int	cap = isSmall ? IM_RSA_512 | IM_RC2_40 : IM_RSA_1024 | IM_BLOWFISH_128;
	int	SmallAcqFlags[] = { CRYPT_MACHINE_KEYSET, 0, CRYPT_NEWKEYSET|CRYPT_MACHINE_KEYSET, CRYPT_NEWKEYSET, -1 };
	int	BigAcqFlags[] = { CRYPT_MACHINE_KEYSET, CRYPT_NEWKEYSET|CRYPT_MACHINE_KEYSET, -1 };
	int	*AcqFlgs = isSmall ? SmallAcqFlags : BigAcqFlags;

	wsprintf(contName, "fxim.rsa%d.%s", bitLen, pMsgMng->GetLocalHostSub()->userName);

	// rebuild
	if ((ctl_flg & KEY_REBUILD) && hCsp && pubKey.key == NULL)
	{
		CryptReleaseContext(hCsp, 0), hCsp = NULL;
		if (!CryptAcquireContext(&hCsp, contName, csp_name, PROV_RSA_FULL, CRYPT_DELETEKEYSET|CRYPT_MACHINE_KEYSET))
			if (ctl_flg & KEY_DIAG)
				odprintf(_T("CryptAcquireContext(destroy) failed, error 0x%08X"), GetLastError());
		CryptAcquireContext(&hCsp, contName, csp_name, PROV_RSA_FULL, CRYPT_DELETEKEYSET);
	}

	// open key cotainer
	for (int i=0; AcqFlgs[i] != -1; i++)
	{
		hCsp = NULL;
		if (CryptAcquireContext(&hCsp, contName, csp_name, PROV_RSA_FULL, AcqFlgs[i]))
			break;
	}
	if (hCsp == NULL)
	{
		if (isSmall && (ctl_flg & KEY_DIAG))
			odprintf(_T("CryptAcquireContext failed, error 0x%08X"), GetLastError());
		return	FALSE;
	}

	ZeroMemory(data, len);
	//import (only 1024bit)
	if (!isSmall && pCfg->privBlob)
	{
		//载入PrivBlob
		if (LoadPrivBlob(data, &len) && !CryptImportKey(hCsp, data, len, 0, CRYPT_EXPORTABLE, &hPrivKey))
		{	// import is fail...
			if (ctl_flg & KEY_DIAG)
				odprintf(_T("CryptImportKey(blob) failed, error 0x%08X"), GetLastError());
			//
			CryptReleaseContext(hCsp, 0), hCsp = NULL;
			CryptAcquireContext(&hCsp, contName, csp_name, PROV_RSA_FULL, CRYPT_MACHINE_KEYSET);
			if (hCsp == NULL && (ctl_flg & KEY_DIAG)) {
				odprintf(_T("CryptImportKey failed, and CryptAcquireContext failed, error 0x%08X"), GetLastError());
				return FALSE;
			}
		}
		if (hPrivKey == NULL)
		{
			if (pCfg->privEncryptType == PRIV_BLOB_USER && pCfg->privEncryptSeed)
			{
				::ExitProcess(0xffffffff);		//
			}
			if (pCfg->privBlob)
			{
				delete [] pCfg->privBlob;
			} 
			pCfg->privBlob = NULL;
		}
	}

	// or 512bit
	if (hPrivKey == NULL)
	{
		if (!CryptGetUserKey(hCsp, AT_KEYEXCHANGE, &hPrivKey)
			&& !CryptGenKey(hCsp, CALG_RSA_KEYX, CRYPT_EXPORTABLE, &hPrivKey))
		{
			if (ctl_flg & KEY_DIAG)	{
				odprintf(_T("CryptGenKey failed, error 0x%08X"), GetLastError());
			}
		}
	}

	//export
	if (CryptExportKey(hPrivKey, 0, PUBLICKEYBLOB, 0, data, (DWORD *)&len))
	{
		pubKey.SetByBlob(data, cap);        //将公钥放进Pubkey类中
	}
	else if (ctl_flg & KEY_DIAG) {
		odprintf(_T("CryptExportKey failed, error 0x%08X"), GetLastError());
	}

	if (!isSmall && pCfg->privBlob == NULL && hPrivKey)
	{
		len = sizeof(data);
		if (CryptExportKey(hPrivKey, 0, PRIVATEKEYBLOB, 0, data, (DWORD *)&len))
			StorePrivBlob(data, len);      //存储私钥
	}
	return	TRUE;
}


BOOL CMainDlg::LoadPrivBlob(BYTE *rawBlob, int *rawBlobLen)
{
	Config* pCfg = Singleton<Config>::getInstance();
	if (pCfg->privBlob == NULL)
		return	FALSE;

	BYTE	key[MAX_BUF];

	if (pCfg->privEncryptType == PRIV_BLOB_RAW)
	{
		memcpy(rawBlob, pCfg->privBlob, *rawBlobLen = pCfg->privBlobLen);
		return	TRUE;
	}
	else if (pCfg->privEncryptType == PRIV_BLOB_USER)
	{
		if (pCfg->privEncryptSeed == NULL)
		{
			return	FALSE;
		}

		while (1)
		{
			CBlowFish	bl(key, strlen((char *)key));
			if (bl.Decrypt(pCfg->privEncryptSeed, key, pCfg->privEncryptSeedLen) == PRIV_SEED_LEN
				&& memcmp(key, PRIV_SEED_HEADER, PRIV_SEED_HEADER_LEN) == 0)
			{
				break;
			}
		}
	}
	else if (pCfg->privEncryptType == PRIV_BLOB_DPAPI)
	{
		if (pCfg->privEncryptSeed == NULL)
			return	FALSE;

		DATA_BLOB	in = { pCfg->privEncryptSeedLen, pCfg->privEncryptSeed }, out;
		if (CryptUnprotectData(&in, 0, 0, 0, 0, CRYPTPROTECT_LOCAL_MACHINE|CRYPTPROTECT_UI_FORBIDDEN, &out) == FALSE)
			return	FALSE;
		memcpy(key, out.pbData, out.cbData);
		::LocalFree(out.pbData);

		if (out.cbData != PRIV_SEED_LEN)
			return	FALSE;
	}
	else{
		return	FALSE;
	}

	CBlowFish	bl(key + PRIV_SEED_HEADER_LEN, 128 / 8);
	return (*rawBlobLen = bl.Decrypt(pCfg->privBlob, rawBlob, pCfg->privBlobLen)) != 0;
}


//存储
BOOL CMainDlg::StorePrivBlob(BYTE *rawBlob, int rawBlobLen)
{
	Config* pCfg = Singleton<Config>::getInstance();
	delete [] pCfg->privBlob;
	pCfg->privBlob = NULL;
	delete [] pCfg->privEncryptSeed;
	pCfg->privEncryptSeed = NULL;

	pCfg->privBlobLen = pCfg->privEncryptSeedLen = 0;

	BYTE	data[MAX_BUF], *encodeBlob = data;

	if (pCfg->privEncryptType == PRIV_BLOB_RAW)
	{
		encodeBlob = rawBlob;
		pCfg->privBlobLen = rawBlobLen;
	}
	else
	{
		BYTE	seed[PRIV_SEED_LEN], *seedCore = seed + PRIV_SEED_HEADER_LEN;
		// seed
		memcpy(seed, PRIV_SEED_HEADER, PRIV_SEED_HEADER_LEN);
		CryptGenRandom(pCfg->hCsp ? pCfg->hCsp : pCfg->hSmallCsp, 128/8, seedCore);

		if (pCfg->privEncryptType == PRIV_BLOB_USER)
		{
			CBlowFish	bl(data, strlen((char *)data));
			pCfg->privEncryptSeedLen = bl.Encrypt(seed, data, PRIV_SEED_LEN);
			pCfg->privEncryptSeed = new BYTE [pCfg->privEncryptSeedLen];
			memcpy(pCfg->privEncryptSeed, data, pCfg->privEncryptSeedLen);
		}
		else if (pCfg->privEncryptType == PRIV_BLOB_DPAPI)
		{
			DATA_BLOB in = { PRIV_SEED_LEN, seed }, out;
			if (!CryptProtectData(&in, L"IM", 0, 0, 0, CRYPTPROTECT_LOCAL_MACHINE|CRYPTPROTECT_UI_FORBIDDEN, &out))
				return	FALSE;
			pCfg->privEncryptSeed = new BYTE [pCfg->privEncryptSeedLen = out.cbData];
			memcpy(pCfg->privEncryptSeed, out.pbData, out.cbData);
			::LocalFree(out.pbData);
		}
		else
		{
			return	FALSE;
		}
		// seed
		CBlowFish	bl(seedCore, 128/8);
		pCfg->privBlobLen = bl.Encrypt(rawBlob, encodeBlob, rawBlobLen);
	}

	pCfg->privBlob = new BYTE [pCfg->privBlobLen];
	memcpy(pCfg->privBlob, encodeBlob, pCfg->privBlobLen);

	pCfg->WriteConfig(CONFIG_CRYPT);

	return	TRUE;
}

void CMainDlg::BroadcastMsg(ULONG mode, const char* msg, const char* exMsg)
{
	Config*			pCfg = Singleton<Config>::getInstance();
	CMsgMng*		pMsgMng = Singleton<CMsgMng>::getInstance();
	TBroadcastObj	*brobj;

#if 0
	for (brobj=pCfg->broadcastList.Top(); brobj; brobj=pCfg->broadcastList.Next(brobj))
	{
		BroadcastMsgSub(brobj->Addr(pCfg->nResolveOpt & RS_REALTIME), 
			m_nPortNo, IM_NOOPERATION, NULL);
	}
	// INADDR_BROADCAST equal (~0)
	BroadcastMsgSub(INADDR_BROADCAST, m_nPortNo, IM_NOOPERATION, NULL);

	Sleep(pCfg->nDelayTime);
#endif

	UINT command = mode | HostStatus();   //GET_OPT(command)

	// local network broadcast
	pMsgMng->Send(INADDR_BROADCAST, m_nPortNo, command, msg, exMsg);

	//链表广播
	for (brobj=pCfg->broadcastList.Top(); brobj; brobj=pCfg->broadcastList.Next(brobj))
	{
		BroadcastMsgSub(brobj->Addr(), m_nPortNo, command, msg, exMsg);
	}

	//拨号连接，拨号上网用户请勾选 [拨号连接]
	for (AddrObj *obj = (AddrObj *)pCfg->dialUpList.TopObj(); obj;
		obj = (AddrObj *)pCfg->dialUpList.NextObj(obj))
	{
		// obj->portNo已是网络字节序，不需要用htons转换
		BroadcastMsgSub(obj->addr, obj->portNo, command, msg, exMsg);
	}
}

void CMainDlg::BroadcastMsgSub(ULONG addr, USHORT portNo, ULONG command, 
							   const char* msg, const char* exMsg)
{
#if 0
	Config*		pCfg = Singleton<Config>::getInstance();
	CMsgMng*	pMsgMng = Singleton<CMsgMng>::getInstance();

	pMsgMng->Send(addr, portNo, command, msg, exMsg);

	if (command == IM_NOOPERATION)
	{
		pMsgMng->Send(addr, portNo, command);
	}
	else
	{
		pMsgMng->Send(addr, portNo, command | (pCfg->bDialUpCheck ? IM_DIALUPOPT : 0), 
			msg, exMsg);
	}
#endif
	CMsgMng*	pMsgMng = Singleton<CMsgMng>::getInstance();

	pMsgMng->Send(addr, portNo, command, msg, exMsg);
}

void CMainDlg::BroadcastEntry()
{
	Config*		pCfg = Singleton<Config>::getInstance();
	
	BroadcastMsg(IM_BR_ENTRY, GetNickNameEx(), pCfg->szGroupName);
	if (pCfg->bExtendEntry)
	{
		::SetTimer(m_hWnd, IM_ENTRY_TIMER, IM_ENTRYMINSEC * 1000, NULL);
	}
}

ULONG CMainDlg::HostStatus(void)
{
	Config*		pCfg = Singleton<Config>::getInstance();
	CMsgMng*	pMsgMng = Singleton<CMsgMng>::getInstance();
	return	(pCfg->bDialUpCheck ? IM_DIALUPOPT : 0) 
		| (pCfg->bAbsenceCheck ? IM_ABSENCEOPT : 0) 
		| (pMsgMng->IsAvailableTcp() ? IM_FILEATTACHOPT : 0) 
		| (pCfg->pubKey.Key() || pCfg->smallPubKey.Key() ? IM_ENCRYPTOPT : 0)
		| IM_FXIMOPT;
}

char* CMainDlg::GetNickNameEx()
{
	static char buf[MAX_LISTBUF];
	Config*		pCfg = Singleton<Config>::getInstance();
	CMsgMng*	pMsgMng = Singleton<CMsgMng>::getInstance();
	/*if (pCfg->AbsenceCheck && *pCfg->AbsenceHead[pCfg->AbsenceChoice])
		wsprintf(buf, "%s[%s]", *pCfg->NickNameStr ? pCfg->NickNameStr : pMsgMng->GetLocalHostSub()->userName,
			pCfg->AbsenceHead[pCfg->AbsenceChoice]);
	else*/
	strcpy(buf, *pCfg->szNickName ? pCfg->szNickName : pMsgMng->GetLocalHostSub()->userName);

	return	buf;
}

BOOL CMainDlg::Receive(SkinChangedParam param)
{
	return SetControlBackGround(m_PaintManager.FindControl(_T("bg")), param.bgimage, param.bkcolor);
}

LRESULT CMainDlg::OnTaskbarNotify(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	switch(LOWORD(lParam))
	{
	case WM_LBUTTONUP:		
		if (m_pMsgJumpHost != NULL)
		{
			ShowChatDlg(m_pMsgJumpHost, TRUE);		
		}		
		break;
	case WM_LBUTTONDBLCLK:
		if (::IsWindow(m_hWnd))
		{
			SendMessage(WM_SYSCOMMAND, ::IsIconic(m_hWnd) ? SC_RESTORE : SC_MINIMIZE);
		}
		break;
	case WM_RBUTTONUP:
		{
			CMenuWnd*	pMenu = new CMenuWnd(m_hWnd);

			if (pMenu != NULL)
			{
				CPoint		pt;
				STRINGorID	xml(_T("xmls\\menu_taskbar.xml"));

				::GetCursorPos(&pt);		
				pMenu->Init(NULL, xml, _T("xml"), pt, eMenuAlignment_Top | eMenuAlignment_Right);
			}
		}		
		break;
	}
	bHandled = TRUE;
	return 0;
}

LRESULT CMainDlg::OnMenuClick(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (_tcscmp((LPCTSTR)wParam, _T("menu_open")) == 0)
	{
		ShowWindow(true);
	}
	else if (_tcscmp((LPCTSTR)wParam, _T("menu_close")) == 0) 
	{
		Close();
	}
	else if (_tcscmp((LPCTSTR)wParam, _T("menu_setting")) == 0) 
	{
		ShowSettingDlg();
	}
	else if (_tcscmp((LPCTSTR)wParam, _T("menu_sendmsg")) == 0) 
	{
		Host*			pHost = NULL;
		CControlUI*		pControl = m_pFriendList->GetItemAt(m_pFriendList->GetCurSel());
		CFriendNodeUI*	pFriendNode = static_cast<CFriendNodeUI*>(pControl);
		if (pFriendNode != NULL && (pHost = pFriendNode->GetHost()) != NULL)
		{
			ShowChatDlg(pHost, TRUE);
		}
			
	}
	else if (_tcscmp((LPCTSTR)wParam, _T("menu_sendfile")) == 0) 
	{
		ShareInfo*	shareInfo = NULL;
		if (BrowserFile(m_hWnd, &shareInfo))
		{
			Host*			pHost = NULL;
			CControlUI*		pControl = m_pFriendList->GetItemAt(m_pFriendList->GetCurSel());
			CFriendNodeUI*	pFriendNode = static_cast<CFriendNodeUI*>(pControl);
			CChatDlg*		pChatDlg = NULL;
			if (pFriendNode != NULL && (pHost = pFriendNode->GetHost()) != NULL)
			{
				pChatDlg = ShowChatDlg(pHost, TRUE);
				pChatDlg->SetShareInfo(shareInfo);
			}
		}
	}
	else if (_tcscmp((LPCTSTR)wParam, _T("menu_sendfolder")) == 0) 
	{
		ShareInfo*	shareInfo = NULL;
		if (BrowserFolder(m_hWnd, &shareInfo))
		{
			Host*			pHost = NULL;
			CControlUI*		pControl = m_pFriendList->GetItemAt(m_pFriendList->GetCurSel());
			CFriendNodeUI*	pFriendNode = static_cast<CFriendNodeUI*>(pControl);
			CChatDlg*		pChatDlg = NULL;
			if (pFriendNode != NULL && (pHost = pFriendNode->GetHost()) != NULL)
			{
				pChatDlg = ShowChatDlg(pHost, TRUE);
				pChatDlg->SetShareInfo(shareInfo);
			}
		}
	}
	bHandled = TRUE;
	return 0;
}

LRESULT CMainDlg::OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	// WindowImplBase::HandleMessage的消息处理流程 先是处理WM_CREATE WM_CLOSE等消息，再是HandleCustomMessage，
	// 再是m_PaintManager.MessageHandler，最后才是基类CWindowWnd::HandleMessage
	// 本类中OnTimer从属于HandleCustomMessage，因此bHandled = TRUE;不能放在switch之外（当然也可以对bHandled不赋值），不然控件的WM_TIMER消息不会处理。
	switch (wParam)
	{
	case IM_MSGJUMP_TIMER:
		MsgJump();
		bHandled = TRUE;
		break;
	}
	
	return 0;
}

LRESULT CMainDlg::OnRecvObjEvent(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	RecvFileObj*	recvObj = (RecvFileObj*)wParam;

	if (recvObj == NULL) {
		return -1;
	}
	switch (lParam)
	{
	case CShareMng::TRANS_DONE:
		if (recvObj->fileInfo->exattr == IM_FILE_AVATAR)
		{
			Config*			pCfg = Singleton<Config>::getInstance();
			ConnectInfo*	conInfo = recvObj->conInfo;

			ASSERT(conInfo);
			HostSub hostSub;
			hostSub.addr = conInfo->addr;
			hostSub.portNo = conInfo->port;
			Host * pHost = pCfg->hosts.GetHostByAddr(&hostSub);

			if (pHost == NULL)
			{
				break;
			}
			wsprintfA(pHost->avatar, "%s%s", recvObj->saveDir, recvObj->fileInfo->Fname());
			GenerateAvatarIcon(pHost);
			m_pFriendList->UpdateFriendNode(pHost);
		}
		break;
	}
	bHandled = TRUE;
	return 0;
}

LRESULT CMainDlg::OnUdpEvent(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	MsgBuf		msg;
	CMsgMng*	pMsgMng = Singleton<CMsgMng>::getInstance();
	
	if (HIWORD(lParam) || !pMsgMng->Recv(&msg))  //HIWORD(lParam) is error message
	{
		return -1;
	}

	switch(GET_MODE(msg.command))
	{
	case IM_BR_ENTRY:		//上线
		MsgBrEntry(&msg);
		break;
	case IM_ANSENTRY:		//回应上线
		MsgAnsEntry(&msg);
		break;
	case IM_BR_EXIT:
		MsgBrExit(&msg);
		break;
	case IM_SENDMSG:
	case IM_SENDMSGIMAGE:
		MsgSendMsg(&msg);
		break;
	case IM_RECVMSG:
		MsgRecvMsg(&msg);
		break;
	case IM_GETPUBKEY:
		MsgGetPubKey(&msg);
		break;
	case IM_ANSPUBKEY:
		MsgAnsPubKey(&msg);
		break;
	case IM_GETAVATAR:
		MsgGetAvatar(&msg);
		break;
	case IM_ANSAVATAR:
		MsgAnsAvatar(&msg);
		break;
	default:
		break;
	}
	bHandled = TRUE;
	return 0;
}

LRESULT CMainDlg::OnTcpEvent(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CShareMng*	pShareMng = Singleton<CShareMng>::getInstance();
	if (HIWORD(lParam)) { 
		DUI__Trace(_T("CMainDlg::OnTcpEvent error %d"), HIWORD(lParam));
		return -1; 
	}

	switch (LOWORD(lParam))
	{
	case FD_ACCEPT:
		DUI__Trace(_T("CMainDlg::OnTcpEvent: FD_ACCEPT, so AddConnectInfo\n"));
		{
			ConnectInfo*	conInfo = new ConnectInfo;
			CMsgMng*		pMsgMng = Singleton<CMsgMng>::getInstance();
			CShareMng*		pShareMng = Singleton<CShareMng>::getInstance();
			memset(conInfo, 0, sizeof(ConnectInfo));
			if (pMsgMng->Accept(m_hWnd, conInfo) == FALSE
				|| pShareMng->AddConnectInfo(conInfo) == FALSE)
			{
				::closesocket(conInfo->sd);
				delete conInfo;
			}
		}
		break;
	case FD_READ:
		DUI__Trace(_T("CMainDlg::OnTcpEvent: FD_READ, so StartSendFile\n"));
		pShareMng->StartSendFile((SOCKET)wParam, m_hWnd);
		break;
// 	case FD_CONNECT:
// 		//DUI__Trace(_T("CMainDlg::OnTcpEvent: FD_CONNECT, so StartRecvFile\n"));
// 		break;
	case FD_CLOSE:
		DUI__Trace(_T("CMainDlg::OnTcpEvent: FD_CLOSE, so closesocket"));
		{
			SendFileObj* sendObj = NULL;
			RecvFileObj* recvObj = NULL;
			if ((sendObj = pShareMng->GetSendFileObj((SOCKET)wParam)) != NULL)
			{
				pShareMng->EndSendFile(sendObj);
			}
			else if ((recvObj = pShareMng->GetRecvFileObj((SOCKET)wParam)) != NULL) 
			{
				pShareMng->EndRecvFile(recvObj);
			}
		}		
		//::closesocket((SOCKET)wParam);
		break;
	default:
		break;
	}
	return 0;
}


void CMainDlg::MsgBrEntry(MsgBuf *msg)
{
	char		buf[MAX_UDPBUF] = {0};
	int			len;
	Config*		pCfg = Singleton<Config>::getInstance();
	CMsgMng*	pMsgMng = Singleton<CMsgMng>::getInstance();

	//回应广播
	len = pMsgMng->MakeMsg(buf, IM_ANSENTRY| HostStatus(), GetNickNameEx(), 
		pCfg->szGroupName);
	pMsgMng->UdpSend(msg->hostSub.addr, msg->hostSub.portNo, buf, len);

	AddHost(&msg->hostSub, msg->command, msg->msgBuf, msg->msgBuf + msg->exOffset);
}

void CMainDlg::MsgAnsEntry(MsgBuf *msg)
{
	AddHost(&msg->hostSub, msg->command, msg->msgBuf, msg->msgBuf + msg->exOffset);
}

void CMainDlg::MsgBrExit(MsgBuf *msg)
{
	Config* pCfg = Singleton<Config>::getInstance();
	Host*	pHost = pCfg->hosts.GetHostByNameAddr(&msg->hostSub);

	if (pHost != NULL && pCfg->hosts.DelHost(pHost))
	{
		m_pFriendList->DeleteFriendNode(pHost);
	}
}

void CMainDlg::MsgSendMsg(MsgBuf *msg)
{
	Config*		pCfg = Singleton<Config>::getInstance();
	Host*		pHost = pCfg->hosts.GetHostByNameAddr(&msg->hostSub);
	CMsgMng*	pMsgMng = Singleton<CMsgMng>::getInstance();
	if (pHost == NULL) {
		return ;
	}

	//将对方发来的消息的包编号回复给对方
	if ((msg->command & IM_SENDCHECKOPT) != 0 )//	&&msg->command & (IM_BROADCASTOPT | IM_AUTORETOPT) != 0)
	{
		pMsgMng->Send(&msg->hostSub, IM_RECVMSG, msg->packetNo);
	}

	CChatDlg*	pChatDlg = m_mapChatDlg[pHost];
	if (pChatDlg == NULL) 
	{
		MsgBuf*	msgNew = new MsgBuf;

		if (msgNew != NULL)
		{
			memset(msgNew, 0, sizeof(MsgBuf));
			msgNew->Init(msg);
			m_vecMsgBuf.push_back(msgNew);
			if (!m_bMsgJumpTimer) {
				::SetTimer(m_hWnd, IM_MSGJUMP_TIMER, 750, NULL);
				m_bMsgJumpTimer = TRUE;
			}
			m_pMsgJumpHost = pCfg->hosts.GetHostByNameAddr(&msg->hostSub);
			if (m_pMsgJumpHost) {
				m_pFriendList->SetFriendNodeJump(m_pMsgJumpHost, true);
			}
		}
	}
	else
	{
		pChatDlg->AddMsgBuf(msg);
		pChatDlg->ShowFlashWindow();
	}
	
	//消息提示
// 	if (pCfg->recvMsgSound && *pCfg->recvMsgSoundFile != 0)
// 	{
// 		//SND_ASYNC 用异步方式播放声音，PlaySound函数在开始播放后立即返回
// 		::PlaySound(pCfg->recvMsgSoundFile, NULL, SND_FILENAME| SND_ASYNC);
// 	}
}

void CMainDlg::MsgRecvMsg(MsgBuf *msg)
{

}

void CMainDlg::MsgGetPubKey(MsgBuf *msg)
{
	int			capa = strtoul(msg->msgBuf, 0, 16);	//strtoul 把字符串转换为一个无符号长整数
	Config*		pCfg = Singleton<Config>::getInstance();
	CMsgMng*	pMsgMng = Singleton<CMsgMng>::getInstance();
	if ((capa &= (pCfg->pubKey.capa | pCfg->smallPubKey.capa)) == 0)
		return;

	PubKey	*pubKey = (capa & IM_RSA_1024) ? &pCfg->pubKey : &pCfg->smallPubKey;

	char	buf[MAX_BUF];

	wsprintf(buf, "%X:%X-", pCfg->pubKey.capa | pCfg->smallPubKey.capa, pubKey->e);

	bin2hexstr_bigendian(pubKey->key, pubKey->keyLen, buf + strlen(buf));

	pMsgMng->Send(&msg->hostSub, IM_ANSPUBKEY, buf);
}

void CMainDlg::MsgAnsPubKey(MsgBuf *msg)
{
	Config* pCfg = Singleton<Config>::getInstance();
	if (pCfg->pubKey.Key() == NULL && pCfg->smallPubKey.Key() == NULL)
		return;

	BYTE	key[MAX_BUF];
	int		key_len, e, capa;
	char	*capa_hex, *e_hex, *key_hex, *p;

	if ((capa_hex = separate_token(msg->msgBuf, ':', &p)) == NULL)
		return;
	if ((e_hex = separate_token(NULL, '-', &p)) == NULL)
		return;
	if ((key_hex = separate_token(NULL, ':', &p)) == NULL)
		return;

	capa = strtoul(capa_hex, 0, 16);
	e = strtoul(e_hex, 0, 16);
	hexstr2bin_bigendian(key_hex, key, sizeof(key), &key_len);

	Host*	pHost = pCfg->hosts.GetHostByNameAddr(&msg->hostSub);
	if (pHost != NULL) {
		CChatDlg*	pChatDlg = m_mapChatDlg[pHost];
		if (pChatDlg) pChatDlg->SendPubKeyNotify(&msg->hostSub, key, key_len, e, capa);
	}
}

void CMainDlg::MsgGetAvatar(MsgBuf *msg)
{
	Config*		pCfg = Singleton<Config>::getInstance();
	CMsgMng*	pMsgMng = Singleton<CMsgMng>::getInstance();
	CShareMng*	pShareMng = Singleton<CShareMng>::getInstance();

	if (*pCfg->szAvatar != 0 && ::PathFileExists(pCfg->szAvatar))
	{
		char	szMsg[MAX_PATH] = {0};
		char	szMsgBuf[MAX_UDPBUF] = {0};
		char	szShare[MAX_PATH] = {0};

#if (defined(UNICODE) || defined(_UNICODE))
		LPCSTR pszAvatarFileNameA = UnicodeToAnsi(PathFindFileName(pCfg->szAvatar));
		if (pszAvatarFileNameA != NULL)
		{
			strncpy(szMsg, pszAvatarFileNameA, MAX_PATH);
			delete pszAvatarFileNameA;
		}
#else
		strncpy(szMsg, ::PathFindFileName(pCfg->szAvatar), MAX_PATH);
#endif
		if (msg->command & IM_GETAVATARFILE)
		{
			ShareInfo*	shareInfo = pShareMng->CreateShare(pMsgMng->MakePacketNo());
#if (defined(UNICODE) || defined(_UNICODE))
			LPCSTR pszAvatarA = UnicodeToAnsi(pCfg->szAvatar);
			if (pszAvatarA != NULL)
			{
				pShareMng->AddFileShare(shareInfo, pszAvatarA, IM_FILE_AVATAR);
				delete pszAvatarA;
			}
#else
			pShareMng->AddFileShare(shareInfo, pCfg->szAvatar, IM_FILE_AVATAR);
#endif
			EncodeShareMsg(szShare, shareInfo, MAX_PATH);

			SendEntry	sendEntry;

			// 此时未必获取了对方的公钥，因此头像文件名信息先不加密。
			sendEntry.status = ST_MAKEMSG;
			sendEntry.command = IM_ANSAVATAR | IM_FILEATTACHOPT;
			sendEntry.host = pCfg->hosts.GetHostByNameAddr(&msg->hostSub);

			// 要保证ShareInfo中的PacketNo和消息中的PacketNo一致，对方接收文件时靠接收消息的PacketNo向发送方请求要传输的文件(夹)
			int len = pMsgMng->MakeMsg(szMsgBuf, shareInfo->packetNo, 
				sendEntry.command, szMsg, szShare);
			pMsgMng->UdpSend(&msg->hostSub, szMsgBuf, len);

			pShareMng->AddHostShare(shareInfo, &sendEntry, 1);
			pShareMng->AddSendFileObjs(shareInfo, m_hWnd);
		}
		else
		{
			pMsgMng->Send(&msg->hostSub, IM_ANSAVATAR, szMsg);
		}
	}
}

void CMainDlg::MsgAnsAvatar(MsgBuf *msg)
{
	TCHAR	szAvatar[MAX_PATH] = {0};
	Config*	pCfg = Singleton<Config>::getInstance();

#if (defined(UNICODE) || defined(_UNICODE))
	LPCWSTR pszMsgBufW = AnsiToUnicode(msg->msgBuf);
	if (pszMsgBufW != NULL)
	{
		wsprintfW(szAvatar, L"%s%s", pCfg->szAvatarSaveDir, pszMsgBufW);
		delete pszMsgBufW;
	}
#else
	wsprintfA(szAvatar, "%s%s", pCfg->szAvatarSaveDir, msg->msgBuf);
#endif

	if (::PathFileExists(szAvatar))
	{
		Host* pHost = pCfg->hosts.GetHostByNameAddr(&msg->hostSub);
		if (pHost == NULL)
		{
			return ;
		}
#if (defined(UNICODE) || defined(_UNICODE))
		LPCSTR pszAvatarA = AnsiToUnicode(szAvatar);
		if (pszAvatarA != NULL)
		{
			strncpy(pHost->avatar, pszAvatarA, MAX_PATH);
			delete pszAvatarA;
		}
#else
		strncpy(pHost->avatar, szAvatar, MAX_PATH);
#endif
		GenerateAvatarIcon(pHost);
		m_pFriendList->UpdateFriendNode(pHost);
	}
	else if (!(msg->command & IM_FILEATTACHOPT))
	{
		Singleton<CMsgMng>::getInstance()->Send(&msg->hostSub, IM_GETAVATAR | IM_GETAVATARFILE);
	}
	else
	{
		ShareInfo*	shareInfo = DecodeShareMsg(msg->msgBuf + msg->exOffset);
		CShareMng*	pShareMng = Singleton<CShareMng>::getInstance();
		if (shareInfo == NULL)
		{
			return ;
		}
		shareInfo->packetNo = msg->packetNo;
		pShareMng->AddRecvFileObjs(shareInfo, m_hWnd, m_hWnd, 
			msg->hostSub.addr, msg->hostSub.portNo);
		pShareMng->SaveRecvFileObjs(shareInfo->packetNo, pCfg->szAvatarSaveDir);
		// 和SendFileObj不一样，RecvFileObj的fileInfo项并不是指向ShareInfo中的某个fileInfo，而是自行申请了一个空间
		// 在接收完成后，会自行删除。 因此这里可以释放ShareInfo
		FreeDecodeShareMsg(shareInfo);
	}	
}


// void CMainDlg::MsgSendImage(MsgBuf* msg)
// {
// 	Host* pHost = pCfg->hosts.GetHostByNameAddr(&msg->hostSub);
// 	if (pHost == NULL) {
// 		return ;
// 	}
// 	
// 	ShareInfo* shareInfo = DecodeShareMsg(msg->msgBuf + msg->exOffset);
// 
// 	if (shareInfo != NULL)
// 	{
// 		shareInfo->packetNo = msg->packetNo;			// DecodeShareMsg没有给ShareInfo的packetNo赋值
// 		pShareMng->AddRecvFileObjs(shareInfo, m_hWnd, msg->hostSub.addr, msg->hostSub.portNo);
// 		//AddRecvFileTrans(shareInfo, msg->hostSub.addr, msg->hostSub.portNo);
// 		FreeDecodeShareMsg(shareInfo);
// 	}
// }


void CMainDlg::AddHost(HostSub *hostSub, ULONG command, char *nickName, char *groupName)
{
	time_t	nowTime	= time(NULL);		
	Host*	pHost = NULL;
	Config* pCfg = Singleton<Config>::getInstance();

	pHost = pCfg->hosts.GetHostByNameAddr(hostSub);
	if (pHost != NULL)
	{
		strncpy(pHost->nickName, nickName, MAX_NAMEBUF - 1);
		strncpy(pHost->groupName, groupName, MAX_NAMEBUF - 1);
		m_pFriendList->UpdateFriendNode(pHost);
		return ;
	}

	pHost = new Host;
	ASSERT(pHost != NULL);

	pHost->Init(hostSub, command, nowTime, nickName, groupName);
	// 如果不是本软件用户，则设置飞鸽的头像
	if (!(pHost->hostStatus & IM_FXIMOPT))
	{
		strcpy(pHost->avatar, IPMSG_AVATAR);
	}
	else if (*pHost->avatar == 0) {
		strcpy(pHost->avatar, DEFAULT_AVATAR);

		// 如果对方是FxIM用户，请求对方头像文件名
		Singleton<CMsgMng>::getInstance()->Send(hostSub, IM_GETAVATAR);
	}

	GenerateAvatarIcon(pHost);

	if (!pCfg->hosts.AddHost(pHost))
	{
		delete pHost;
	}

	m_pFriendList->AddFriendNode(pHost);
}

void CMainDlg::GenerateAvatarIcon(Host* pHost)
{
	ASSERT(pHost);
	// 下面这种方法不能成功，GetImage返回NULL
	// const TImageInfo* data = m_pFriendList->GetManager()->GetImage(pHost->logo);
	// pHost->hIconAvatar = BitmapToIcon(data->hBitmap, data->nX, data->nY);

	// 转化头像为Icon [12/12/2014 ybt] 
	STRINGorID	sBitmap(pHost->avatar);
	TImageInfo* data = CRenderEngine::LoadImage(sBitmap);
	if (data == NULL)
	{
		return ;
	}
	HICON iconAvatar = BitmapToIcon(data->hBitmap, data->nX, data->nY);
	if (iconAvatar != NULL)
	{
		if (pHost->hIconAvatar != NULL)
		{
			::DeleteObject(pHost->hIconAvatar);
		}
		pHost->hIconAvatar = iconAvatar;
		::DeleteObject(data->hBitmap);
	}	
	delete data;	
}

CChatDlg* CMainDlg::ShowChatDlg(Host* pHost, BOOL bOpenIfNotExist)
{
	Config*		pCfg = Singleton<Config>::getInstance();
	CChatDlg*	pChatDlg = NULL;

	pChatDlg = m_mapChatDlg[pHost];
	if (pChatDlg != NULL)
	{
		::ShowWindow(*pChatDlg, SW_SHOWNOACTIVATE);
		return pChatDlg;
	}
	if (!bOpenIfNotExist) {
		return NULL;
	}
	pChatDlg = new CChatDlg(pHost);		

	ASSERT(pChatDlg != NULL);
	m_mapChatDlg[pHost] = pChatDlg;
#if defined(WIN32) && !defined(UNDER_CE)
	pChatDlg->Create(NULL, _T("聊天窗口"), UI_WNDSTYLE_FRAME, 
		WS_EX_STATICEDGE | WS_EX_APPWINDOW, 0, 0, 0, 0);
#else
	pChatDlg->Create(NULL, _T("聊天窗口"), UI_WNDSTYLE_FRAME, 
		WS_EX_TOPMOST, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
#endif
	pChatDlg->CenterWindow();
	::ShowWindow(pChatDlg->GetHWND(), SW_SHOWNOACTIVATE);

	m_pFriendList->SetFriendNodeJump(pHost, false);

	if (m_bMsgJumpTimer)
	{
		VecMsgBuf::iterator	iter;
		MsgBuf*				msg = NULL;
		for (iter = m_vecMsgBuf.begin(); iter != m_vecMsgBuf.end(); )
		{
			msg = *iter;
			if (msg && msg->hostSub.IsSameHostSub(&m_pMsgJumpHost->hostSub))
			{
				pChatDlg->AddMsgBuf(msg);
				iter = m_vecMsgBuf.erase(iter);
				delete msg;
			}
			else {
				iter++;
			}
		}
		if (m_vecMsgBuf.size() == 0)
		{
			KillTimer(m_hWnd, IM_MSGJUMP_TIMER);
			TaskBar(NIM_MODIFY, m_hIcon, _T("FxIM"));
			m_pMsgJumpHost = NULL;
			m_bMsgJumpTimer = FALSE;
		}
		else
		{
			msg = m_vecMsgBuf[m_vecMsgBuf.size() - 1];
			m_pMsgJumpHost = pCfg->hosts.GetHostByNameAddr(&msg->hostSub);
		}
	}

	return pChatDlg;
}

void CMainDlg::ShowSettingDlg()
{
	CSettingDlg* pSettingDlg = new CSettingDlg;
	if (!IsWindow(pSettingDlg->GetHWND())) 
	{
#if defined(WIN32) && !defined(UNDER_CE)
		pSettingDlg->Create(NULL, _T("程序设置"), UI_WNDSTYLE_FRAME & ~WS_MAXIMIZEBOX, 
			WS_EX_STATICEDGE | WS_EX_APPWINDOW, 0, 0, 0, 0);
#else
		pSettingDlg->Create(NULL, _T("程序设置"), UI_WNDSTYLE_FRAME & ~WS_MAXIMIZEBOX,
			WS_EX_TOPMOST, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
#endif
	}
	pSettingDlg->CenterWindow();
	::ShowWindow(pSettingDlg->GetHWND(), SW_SHOWNOACTIVATE);
}

void CMainDlg::UpdateInfo()
{
	Config*		pCfg = Singleton<Config>::getInstance();
	CMsgMng*	pMsgMng = Singleton<CMsgMng>::getInstance();
	
	if (*pCfg->szAvatar != 0 && ::PathFileExists(pCfg->szAvatar))
	{
		m_pLabelAvatar->SetBkImage(pCfg->szAvatar);
	}
	else
	{
		m_pLabelAvatar->SetBkImage(DEFAULT_AVATAR);
	}

	m_pLabelNickName->SetText(GetNickNameEx());
	
	in_addr addr;
	addr.S_un.S_addr = pMsgMng->GetLocalHostSub()->addr;
	m_pLabelDescription->SetText(inet_ntoa(addr));
	
}

void CMainDlg::MsgJump()
{
	static bool bIconNull = false;
	if (m_pMsgJumpHost != NULL)
	{
		/*
		"表达式 : TRUE ? TRUE ? 1 : 2 : 3" = 1
		"表达式 : TRUE ? FALSE ? 1 : 2 : 3" = 2
		"表达式 : FALSE ? TRUE ? 1 : 2 : 3" = 3
		"表达式 : FALSE ? FALSE ? 1 : 2 : 3" = 3
		*/
		HICON	hIcon = bIconNull ? m_pMsgJumpHost->hIconAvatar ? m_pMsgJumpHost->hIconAvatar : m_hIcon : m_hIconNull;
		TCHAR	szTip[MAX_PATH] = {0};
		IN_ADDR	addr;
		addr.S_un.S_addr = m_pMsgJumpHost->hostSub.addr;
		
		wsprintf(szTip, _T("来自%s(%s)的消息"), m_pMsgJumpHost->NickNameEx(), inet_ntoa(addr));
		TaskBar(NIM_MODIFY, hIcon, szTip);
		bIconNull = !bIconNull;
	}
}

BOOL CMainDlg::BrowserFile(HWND hParent, ShareInfo** ppShareInfo)
{
	TCHAR szFiles[MAX_BUF] = {0};
	TCHAR szFilter[] = {
		_T("All Files\0*.*;\0")
	};
	CMsgMng*	pMsgMng = Singleton<CMsgMng>::getInstance();
	CShareMng*	pShareMng = Singleton<CShareMng>::getInstance();

	if (ppShareInfo == NULL) {
		return FALSE;
	}

	OPENFILENAME ofn;      
	ZeroMemory(&ofn, sizeof(ofn));

	ofn.Flags			  = OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY | OFN_PATHMUSTEXIST 
								| OFN_ALLOWMULTISELECT | OFN_EXPLORER;
	ofn.lStructSize       = sizeof(ofn);
	ofn.hwndOwner         = hParent;
	ofn.lpstrFile         = szFiles;
	ofn.nMaxFile          = sizeof(szFiles);
	ofn.lpstrFilter       = szFilter;
	ofn.nFilterIndex      = 1;
	ofn.lpstrFileTitle    = NULL;
	ofn.nMaxFileTitle     = 0;
	ofn.lpstrInitialDir   = NULL;
	ofn.lpstrDefExt		  = NULL;

	if (GetOpenFileName(&ofn) == FALSE) {
		return FALSE;
	}

	if (*ppShareInfo == NULL) {
		*ppShareInfo = pShareMng->CreateShare(pMsgMng->MakePacketNo());
	}

	// 得到所有文件 [12/10/2014 ybt]
	// 单选时，szFiles中存的是一个完整的路径如示例："F:\\Videos\\test1.mp4"，ofn.nFileOffset是文件名的起始索引 
	// 多选时，szFiles中存的值如示例："F:\\Videos\0test1.mp4\0test2.rmvb\0"，ofn.nFileOffset是第一个文件名的起始索引
	TCHAR	szPath[MAX_PATH] = {0};
	LPTSTR	psz = ofn.lpstrFile + ofn.nFileOffset;

	*(psz -1) = _T('\0');	
	while (*psz != 0) 
	{
		wsprintf(szPath, _T("%s\\%s"), ofn.lpstrFile, psz); 	
		psz += _tcslen(psz) + 1;

		pShareMng->AddFileShare(*ppShareInfo, szPath);
	}

	return TRUE;
}

BOOL CMainDlg::BrowserFolder(HWND hParent, ShareInfo** ppShareInfo)
{
	TCHAR		szFolder[MAX_PATH] = {0};
	BROWSEINFO	bInfo;
	CMsgMng*	pMsgMng = Singleton<CMsgMng>::getInstance();
	CShareMng*	pShareMng = Singleton<CShareMng>::getInstance();

	if (ppShareInfo == NULL) {
		return FALSE;
	}

	ZeroMemory(&bInfo, sizeof(bInfo));
	bInfo.hwndOwner = hParent;
	bInfo.pszDisplayName = szFolder;
	bInfo.lpszTitle = _T("请选择你要发送的文件夹");     
	bInfo.ulFlags   = BIF_RETURNONLYFSDIRS|BIF_EDITBOX;

	LPITEMIDLIST lpDlist;					//用来保存返回信息的IDList
	lpDlist = SHBrowseForFolder(&bInfo) ;	//显示选择对话框

	if (lpDlist == NULL) {
		return FALSE;
	}

	SHGetPathFromIDList(lpDlist, szFolder);
	if (*ppShareInfo == NULL) {
		*ppShareInfo = pShareMng->CreateShare(pMsgMng->MakePacketNo());
	}
	pShareMng->AddFileShare(*ppShareInfo, szFolder);
	return TRUE;
}

BOOL CMainDlg::RestartApplication()
{
	STARTUPINFO			si;
	PROCESS_INFORMATION	pi;

	TCHAR szModule[MAX_PATH] = {0};
	TCHAR szCurrentDirectory[MAX_PATH]  = {0};
	::GetModuleFileName(m_PaintManager.GetInstance(), szModule, MAX_PATH);
	_tcscpy(szCurrentDirectory, szModule);
	::PathRemoveFileSpec(szCurrentDirectory);

	memset(&si, 0, sizeof(STARTUPINFO));
	si.cb = sizeof(STARTUPINFO);
	if (!::CreateProcess(szModule, NULL, NULL, NULL, FALSE, 
		 NORMAL_PRIORITY_CLASS, NULL, szCurrentDirectory, &si, &pi))
	{
		return FALSE;
	}
	Close();
	return TRUE;
}