// Feixin.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "FxIM.h"
#include "MainFrame/MainDlg.h"
#include "MainFrame/MsgMng.h"
#include "MainFrame/ShareMng.h"
#include "Utils/Config.h"

HWND CreateHiddenWnd(HINSTANCE hInstance)
{
	HWND		hWndHidden;
	TCHAR		szHiddenClass[] = _T("FxIM_Hidden");
	WNDCLASSEX	wcex;

	memset(&wcex, 0, sizeof(wcex));
	wcex.cbSize			= sizeof(WNDCLASSEX);
	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.hInstance		= hInstance;
	wcex.lpszClassName	= szHiddenClass;
	wcex.lpfnWndProc	= ::DefWindowProc;
	RegisterClassEx(&wcex);

	hWndHidden = CreateWindow(szHiddenClass, NULL, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);
	if (hWndHidden == NULL)	{
		odprintf(_T("Create hidden window failed, error %d."), GetLastError());
	}
	ShowWindow(hWndHidden, SW_HIDE);
	return hWndHidden;
}

#if defined(WIN32) && !defined(UNDER_CE)
int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, 
					   LPTSTR /*lpCmdLine*/, int nCmdShow)
#else
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR lpCmdLine, int nCmdShow)
#endif
{
	CPaintManagerUI::SetInstance(hInstance);
	CPaintManagerUI::SetResourcePath(CPaintManagerUI::GetInstancePath());

	HINSTANCE hInstRich = ::LoadLibrary(_T("Riched20.dll"));

	if (!DllRegisterServer(_T("ImageOleCtrl.dll")))
	{
		::MessageBox(NULL, _T("COM组件注册失败！"), _T("提示"), MB_OK);
		return 0;
	}

	::CoInitialize(NULL);
	::OleInitialize(NULL);

	CMsgMng* pMsgMng = Singleton<CMsgMng>::getInstance();
	if (!pMsgMng->WSockInit()) 
	{
		return -1; 
	}

#if defined(WIN32) && !defined(UNDER_CE)
	HRESULT Hr = ::CoInitialize(NULL);
#else
	HRESULT Hr = ::CoInitializeEx(NULL, COINIT_MULTITHREADED);
#endif
	if( FAILED(Hr) ) return 0;

	CMainDlg*	pMainDlg = Singleton<CMainDlg>::getInstance();
	HWND		hWndHidden = CreateHiddenWnd(hInstance);

#if defined(WIN32) && !defined(UNDER_CE)
	pMainDlg->Create(hWndHidden, _T("FxIM"), UI_WNDSTYLE_FRAME, WS_EX_STATICEDGE 
		| WS_EX_APPWINDOW, 0, 0, 600, 800);
#else
	pMainDlg->Create(hWndHidden, _T("FxIM"), UI_WNDSTYLE_FRAME, WS_EX_TOPMOST, 0, 
		0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
#endif
	pMainDlg->CenterWindow();
	ShowInTaskBar(pMainDlg->GetHWND(), FALSE);
	::ShowWindow(pMainDlg->GetHWND(), SW_SHOW);

	CPaintManagerUI::MessageLoop();
	CPaintManagerUI::Term();
	
	if (hWndHidden != NULL && ::IsWindow(hWndHidden)) {
		::DestroyWindow(hWndHidden); 
	}

	::OleUninitialize();
	::CoUninitialize();

	::FreeLibrary(hInstRich);

	return 0;
}