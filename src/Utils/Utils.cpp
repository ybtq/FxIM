#include "StdAfx.h"
#include <gdiplus.h>
#include <shlobj.h>
#include "Utils/Utils.h"

#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "shell32.lib")

CHAR * UnicodeToUtf8(const WCHAR * lpszStr)
{
	CHAR * lpUtf8;
	int nLen;

	if (NULL == lpszStr)
		return NULL;

	nLen = ::WideCharToMultiByte(CP_UTF8, 0, lpszStr, -1, NULL, 0, NULL, NULL);
	if (0 == nLen)
		return NULL;

	lpUtf8 = new CHAR[nLen + 1];
	if (NULL == lpUtf8)
		return NULL;

	memset(lpUtf8, 0, nLen + 1);
	nLen = ::WideCharToMultiByte(CP_UTF8, 0, lpszStr, -1, lpUtf8, nLen, NULL, NULL);
	if (0 == nLen)
	{
		delete []lpUtf8;
		return NULL;
	}

	return lpUtf8;
}

WCHAR * AnsiToUnicode(const CHAR * lpszStr)
{
	WCHAR * lpUnicode;
	int nLen;

	if (NULL == lpszStr)
		return NULL;

	nLen = ::MultiByteToWideChar(CP_ACP, 0, lpszStr, -1, NULL, 0);
	if (0 == nLen)
		return NULL;

	lpUnicode = new WCHAR[nLen + 1];
	if (NULL == lpUnicode)
		return NULL;

	memset(lpUnicode, 0, sizeof(WCHAR) * (nLen + 1));
	nLen = ::MultiByteToWideChar(CP_ACP, 0, lpszStr, -1, lpUnicode, nLen);
	if (0 == nLen)
	{
		delete []lpUnicode;
		return NULL;
	}

	return lpUnicode;
}

WCHAR * Utf8ToUnicode(const CHAR * lpszStr)
{
	WCHAR * lpUnicode;
	int nLen;

	if (NULL == lpszStr)
		return NULL;

	nLen = ::MultiByteToWideChar(CP_UTF8, 0, lpszStr, -1, NULL, 0);
	if (0 == nLen)
		return NULL;

	lpUnicode = new WCHAR[nLen + 1];
	if (NULL == lpUnicode)
		return NULL;

	memset(lpUnicode, 0, sizeof(WCHAR) * (nLen + 1));
	nLen = ::MultiByteToWideChar(CP_UTF8, 0, lpszStr, -1, lpUnicode, nLen);
	if (0 == nLen)
	{
		delete []lpUnicode;
		return NULL;
	}

	return lpUnicode;
}

CHAR * UnicodeToAnsi(const WCHAR * lpszStr)
{
	CHAR * lpAnsi;
	int nLen;

	if (NULL == lpszStr)
		return NULL;

	nLen = ::WideCharToMultiByte(CP_ACP, 0, lpszStr, -1, NULL, 0, NULL, NULL);
	if (0 == nLen)
		return NULL;

	lpAnsi = new CHAR[nLen + 1];
	if (NULL == lpAnsi)
		return NULL;

	memset(lpAnsi, 0, nLen + 1);
	nLen = ::WideCharToMultiByte(CP_ACP, 0, lpszStr, -1, lpAnsi, nLen, NULL, NULL);
	if (0 == nLen)
	{
		delete []lpAnsi;
		return NULL;
	}

	return lpAnsi;
}

void ConvertGBKToUtf8(std::string& strGBK)
{ 
// 	WCHAR* szUnicode = AnsiToUnicode(strGBK.data());
// 	if (szUnicode == NULL) {
// 		return FALSE;
// 	}
// 	CHAR* szUtf8 = UnicodeToUtf8(szUnicode);
// 	if (szUtf8 )
	int len=MultiByteToWideChar(CP_ACP, 0, (LPCTSTR)strGBK.c_str(), -1, NULL,0); 
	unsigned short * wszUtf8 = new unsigned short[len+1]; 
	memset(wszUtf8, 0, len * 2 + 2); 
	MultiByteToWideChar(CP_ACP, 0, (LPCTSTR)strGBK.c_str(), -1, (LPWSTR)wszUtf8, len);

	len = WideCharToMultiByte(CP_UTF8, 0, (LPWSTR)wszUtf8, -1, NULL, 0, NULL, NULL);
	char *szUtf8=new char[len + 1]; 
	memset(szUtf8, 0, len + 1);
	WideCharToMultiByte (CP_UTF8, 0, (LPWSTR)wszUtf8, -1, szUtf8, len, NULL,NULL); 
	strGBK = szUtf8;
	delete[] szUtf8; 
	delete[] wszUtf8;	
}

void ConvertUtf8ToGBK(std::string& strUtf8)
{
	int len=MultiByteToWideChar(CP_UTF8, 0, strUtf8.c_str(), -1, NULL,0);
	unsigned short * wszGBK = new unsigned short[len+1];
	memset(wszGBK, 0, len * 2 + 2);
	MultiByteToWideChar(CP_UTF8, 0, strUtf8.c_str(), -1, (LPWSTR)wszGBK, len);

	len = WideCharToMultiByte(CP_ACP, 0, (LPWSTR)wszGBK, -1, NULL, 0, NULL, NULL); 
	char *szGBK=new char[len + 1];
	memset(szGBK, 0, len + 1);
	WideCharToMultiByte (CP_ACP, 0, (LPWSTR)wszGBK, -1, szGBK, len, NULL,NULL);

	strUtf8 = szGBK;
	delete[] szGBK;
	delete[] wszGBK;
}


// 枚举系统字体回调函数
int CALLBACK EnumSysFontProc(const LOGFONT *lpelf, const TEXTMETRIC *lpntm, DWORD dwFontType, LPARAM lParam)
{
	if (dwFontType & TRUETYPE_FONTTYPE)
	{
		std::vector<tstring> * arrSysFont = (std::vector<tstring> *)lParam;
		if (arrSysFont != NULL)
		{
			for (int i = 0; i < (int)arrSysFont->size(); i++)
			{
				if ((*arrSysFont)[i] == lpelf->lfFaceName)
					return TRUE;
			}
			arrSysFont->push_back(lpelf->lfFaceName);
		}
	}

	return TRUE;
}

// 枚举系统字体
BOOL EnumSysFont(std::vector<tstring>& arrSysFont)
{
	HDC hDC = ::GetDC(NULL);
	::EnumFontFamiliesEx(hDC, NULL, EnumSysFontProc, (LPARAM)&arrSysFont, 0);
	::ReleaseDC(NULL, hDC);

	return TRUE;
}

// 闪烁窗口标题栏
BOOL FlashWindowEx(HWND hWnd, int nCount)
{
	FLASHWINFO stFlashInfo = {0};
	stFlashInfo.cbSize = sizeof(FLASHWINFO);
	stFlashInfo.hwnd = hWnd;
	stFlashInfo.dwFlags = FLASHW_TRAY | FLASHW_TIMERNOFG;
	stFlashInfo.uCount = nCount;
	stFlashInfo.dwTimeout = 0;
	return ::FlashWindowEx(&stFlashInfo);
}

// 获取系统任务栏区域
BOOL GetTrayWndRect(LPRECT lpRect)
{
	if (NULL == lpRect)
		return FALSE;

	HWND hWnd = ::FindWindow(_T("Shell_TrayWnd"), NULL);
	if (hWnd != NULL)
		return ::GetWindowRect(hWnd, lpRect);
	else
		return FALSE;
}

// _T("%Y-%m-%d %H:%M:%S")
LPCTSTR FormatTime(time_t lTime, LPCTSTR lpFmt)
{
	struct tm * lpTimeInfo;
	static TCHAR cTime[32];

	memset(cTime, 0, sizeof(cTime));

	lpTimeInfo = localtime(&lTime);
	if (lpTimeInfo != NULL)
		_tcsftime(cTime, sizeof(cTime) / sizeof(TCHAR), lpFmt, lpTimeInfo);
	return cTime;
}



BOOL IsDigit(LPCTSTR lpStr)
{
	for (LPCTSTR p = lpStr; *p != _T('\0'); p++)
	{
		if (!isdigit(*p))
			return FALSE;
	}
	return TRUE;
}

void Replace(tstring& strText, LPCTSTR lpOldStr, LPCTSTR lpNewStr)
{
	if (NULL == lpOldStr || NULL == lpNewStr)
		return;

	int nOldStrLen = _tcslen(lpOldStr);
	int nNewStrLen = _tcslen(lpNewStr);

	tstring::size_type nPos = 0;
	while ((nPos = strText.find(lpOldStr, nPos)) != tstring::npos)
	{
		strText.replace(nPos, nOldStrLen, lpNewStr);
		nPos += nNewStrLen;
	}
}

void ReplaceA(std::string& strText, LPCSTR lpOldStr, LPCSTR lpNewStr)
{
	if (NULL == lpOldStr || NULL == lpNewStr)
		return;

	int nOldStrLen = strlen(lpOldStr);
	int nNewStrLen = strlen(lpNewStr);

	std::string::size_type nPos = 0;
	while ((nPos = strText.find(lpOldStr, nPos)) != std::string::npos)
	{
		strText.replace(nPos, nOldStrLen, lpNewStr);
		nPos += nNewStrLen;
	}
}

void ReplaceW(wstring& strText, LPCWSTR lpOldStr, LPCWSTR lpNewStr)
{
	if (NULL == lpOldStr || NULL == lpNewStr)
		return;

	int nOldStrLen = wcslen(lpOldStr);
	int nNewStrLen = wcslen(lpNewStr);

	wstring::size_type nPos = 0;
	while ((nPos = strText.find(lpOldStr, nPos)) != wstring::npos)
	{
		strText.replace(nPos, nOldStrLen, lpNewStr);
		nPos += nNewStrLen;
	}
}

// 编码Html特殊字符
void EncodeHtmlSpecialChars(tstring& strText)
{
	Replace(strText, _T("&"), _T("&amp;"));
	Replace(strText, _T("'"), _T("&#39;"));
	Replace(strText, _T("\""), _T("&quot;"));
	Replace(strText, _T("<"), _T("&lt;"));
	Replace(strText, _T(">"), _T("&gt;"));
	Replace(strText, _T(" "), _T("&nbsp;"));
}

// 解码Html特殊字符
void DecodeHtmlSpecialChars(tstring& strText)
{
	Replace(strText, _T("&#39;"), _T("'"));
	Replace(strText, _T("&quot;"), _T("\""));
	Replace(strText, _T("&lt;"), _T("<"));
	Replace(strText, _T("&gt;"), _T(">"));
	Replace(strText, _T("&nbsp;"), _T(" "));
	Replace(strText, _T("&amp;"), _T("&"));
}

tstring GetBetweenString(LPCTSTR pStr, TCHAR cStart, TCHAR cEnd)
{
	tstring strText;

	if (NULL == pStr)
		return _T("");

	LPCTSTR p1 = _tcschr(pStr, cStart);
	if (NULL == p1)
		return _T("");

	LPCTSTR p2 = _tcschr(p1+1, cEnd);
	if (NULL == p2)
		return _T("");

	int nLen = p2-(p1+1);
	if (nLen <= 0)
		return _T("");
	
	TCHAR * lpText = new TCHAR[nLen+1];
	if (NULL == lpText)
		return _T("");

	memset(lpText, 0, (nLen+1)*sizeof(TCHAR));
	_tcsncpy(lpText, p1+1, nLen);
	strText = lpText;
	delete []lpText;

	return strText;
}

int GetBetweenInt(LPCTSTR pStr, TCHAR cStart, TCHAR cEnd, int nDefValue/* = 0*/)
{
	tstring strText = GetBetweenString(pStr, cStart, cEnd);
	if (!strText.empty() && IsDigit(strText.c_str()))
		return _tcstol(strText.c_str(), NULL, 10);
	else
		return nDefValue;
}

tstring GetBetweenString(LPCTSTR pStr, LPCTSTR pStart, LPCTSTR pEnd)
{
	tstring strText;

	if (NULL == pStr || NULL == pStart || NULL == pEnd)
		return _T("");

	int nStartLen = _tcslen(pStart);

	LPCTSTR p1 = _tcsstr(pStr, pStart);
	if (NULL == p1)
		return _T("");

	LPCTSTR p2 = _tcsstr(p1+nStartLen, pEnd);
	if (NULL == p2)
		return _T("");

	int nLen = p2-(p1+nStartLen);
	if (nLen <= 0)
		return _T("");
	
	TCHAR * lpText = new TCHAR[nLen+1];
	if (NULL == lpText)
		return _T("");
	
	memset(lpText, 0, (nLen+1)*sizeof(TCHAR));
	_tcsncpy(lpText, p1+nStartLen, nLen);
	strText = lpText;
	delete []lpText;

	return strText;
}

int GetBetweenInt(LPCTSTR pStr, LPCTSTR pStart, 
				  LPCTSTR pEnd, int nDefValue/* = 0*/)
{
	tstring strText = GetBetweenString(pStr, pStart, pEnd);
	if (!strText.empty() && IsDigit(strText.c_str()))
		return _tcstol(strText.c_str(), NULL, 10);
	else
		return nDefValue;
}

BOOL DllRegisterServer(LPCTSTR lpszFileName, BOOL bUnregister)
{
	typedef HRESULT (WINAPI * FREG)();

	BOOL bRet = FALSE;

	HMODULE hDLL = ::LoadLibrary(lpszFileName);
	if (NULL == hDLL)
		return FALSE;

	CHAR * lpszFuncName;
	if (!bUnregister)
		lpszFuncName = "DllRegisterServer";
	else
		lpszFuncName = "DllUnregisterServer";
	
	FREG lpfunc = (FREG)::GetProcAddress(hDLL, lpszFuncName);
	if (lpfunc != NULL)
	{
		lpfunc();
		bRet = TRUE;
	}

	::FreeLibrary(hDLL);

	return bRet;
}

BOOL DllRegisterServer(LPCTSTR lpszFileName)
{
	return DllRegisterServer(lpszFileName, FALSE);
}

BOOL DllUnregisterServer(LPCTSTR lpszFileName)
{
	return DllRegisterServer(lpszFileName, TRUE);
}

GUID GetFileTypeGuidByExtension(LPCWSTR lpExtension)
{
	GUID guid = GUID_NULL;

	if (NULL == lpExtension)
		return guid;

	UINT numEncoders = 0, size = 0;
	Gdiplus::Status status = Gdiplus::GetImageEncodersSize(&numEncoders, &size);  
	if (status != Gdiplus::Ok)
		return guid;

	Gdiplus::ImageCodecInfo* lpEncoders = (Gdiplus::ImageCodecInfo*)(malloc(size));
	if (NULL == lpEncoders)
		return guid;

	status = Gdiplus::GetImageEncoders(numEncoders, size, lpEncoders);
	if (Gdiplus::Ok == status)
	{
		for (UINT i = 0; i < numEncoders; i++)
		{
			BOOL bFind = FALSE;
			const WCHAR * pStart = lpEncoders[i].FilenameExtension;
			const WCHAR * pEnd = wcschr(pStart, L';');
			do 
			{
				if (NULL == pEnd)
				{
					LPCWSTR lpExt = ::wcsrchr(pStart, L'.');
					if ((lpExt != NULL) && (wcsicmp(lpExt, lpExtension) == 0))
					{
						guid = lpEncoders[i].FormatID;
						bFind = TRUE;
					}
					break;
				}

				int nLen = pEnd-pStart;
				if (nLen < MAX_PATH)
				{
					WCHAR cBuf[MAX_PATH] = {0};
					wcsncpy(cBuf, pStart, nLen);
					LPCWSTR lpExt = ::wcsrchr(cBuf, L'.');
					if ((lpExt != NULL) && (wcsicmp(lpExt, lpExtension) == 0))
					{
						guid = lpEncoders[i].FormatID;
						bFind = TRUE;
						break;
					}
				}
				pStart = pEnd+1;
				if (L'\0' == *pStart)
					break;
				pEnd = wcschr(pStart, L';');
			} while (1);
			if (bFind)
				break;
		}
	}

	free(lpEncoders);

	return guid;
}

CLSID GetEncoderClsidByExtension(LPCWSTR lpExtension)
{
	CLSID clsid = CLSID_NULL;

	if (NULL == lpExtension)
		return clsid;

	UINT numEncoders = 0, size = 0;
	Gdiplus::Status status = Gdiplus::GetImageEncodersSize(&numEncoders, &size);  
	if (status != Gdiplus::Ok)
		return clsid;

	Gdiplus::ImageCodecInfo* lpEncoders = (Gdiplus::ImageCodecInfo*)(malloc(size));
	if (NULL == lpEncoders)
		return clsid;

	status = Gdiplus::GetImageEncoders(numEncoders, size, lpEncoders);
	if (Gdiplus::Ok == status)
	{
		for (UINT i = 0; i < numEncoders; i++)
		{
			BOOL bFind = FALSE;
			const WCHAR * pStart = lpEncoders[i].FilenameExtension;
			const WCHAR * pEnd = wcschr(pStart, L';');
			do 
			{
				if (NULL == pEnd)
				{
					LPCWSTR lpExt = ::wcsrchr(pStart, L'.');
					if ((lpExt != NULL) && (wcsicmp(lpExt, lpExtension) == 0))
					{
						clsid = lpEncoders[i].Clsid;
						bFind = TRUE;
					}
					break;
				}

				int nLen = pEnd-pStart;
				if (nLen < MAX_PATH)
				{
					WCHAR cBuf[MAX_PATH] = {0};
					wcsncpy(cBuf, pStart, nLen);
					LPCWSTR lpExt = ::wcsrchr(cBuf, L'.');
					if ((lpExt != NULL) && (wcsicmp(lpExt, lpExtension) == 0))
					{
						clsid = lpEncoders[i].Clsid;
						bFind = TRUE;
						break;
					}
				}
				pStart = pEnd+1;
				if (L'\0' == *pStart)
					break;
				pEnd = wcschr(pStart, L';');
			} while (1);
			if (bFind)
				break;
		}
	}

	free(lpEncoders);

	return clsid;
}

// ImageFormatBMP, ImageFormatJPEG, ImageFormatPNG, ImageFormatGIF, ImageFormatTIFF
CLSID GetEncoderClsidByFileType(REFGUID guidFileType)
{
	CLSID clsid = CLSID_NULL;

	UINT numEncoders = 0, size = 0;
	Gdiplus::Status status = Gdiplus::GetImageEncodersSize(&numEncoders, &size);  
	if (status != Gdiplus::Ok)
		return clsid;

	Gdiplus::ImageCodecInfo* lpEncoders = (Gdiplus::ImageCodecInfo*)(malloc(size));
	if (NULL == lpEncoders)
		return clsid;

	status = Gdiplus::GetImageEncoders(numEncoders, size, lpEncoders);
	if (Gdiplus::Ok == status)
	{
		for (UINT i = 0; i < numEncoders; i++)
		{
			if (lpEncoders[i].FormatID == guidFileType)
				clsid = lpEncoders[i].Clsid;
		}
	}

	free(lpEncoders);

	return clsid;
}

// image/bmp, image/jpeg, image/gif, image/tiff, image/png
CLSID GetEncoderClsidByMimeType(LPCWSTR lpMineType)
{
	CLSID clsid = CLSID_NULL;

	if (NULL == lpMineType)
		return clsid;

	UINT numEncoders = 0, size = 0;
	Gdiplus::Status status = Gdiplus::GetImageEncodersSize(&numEncoders, &size);  
	if (status != Gdiplus::Ok)
		return clsid;

	Gdiplus::ImageCodecInfo* lpEncoders = (Gdiplus::ImageCodecInfo*)(malloc(size));
	if (NULL == lpEncoders)
		return clsid;

	status = Gdiplus::GetImageEncoders(numEncoders, size, lpEncoders);
	if (Gdiplus::Ok == status)
	{
		for (UINT i = 0; i < numEncoders; i++)
		{
			if (wcsicmp(lpEncoders[i].MimeType, lpMineType) == 0)
				clsid = lpEncoders[i].Clsid;
		}
	}

	free(lpEncoders);

	return clsid;
}

void odprintf(LPCTSTR format, ...)
{
	int		len;
	LPTSTR	buf;
	va_list	args;	

	va_start(args, format);

#if (_MSC_VER <= 1200)		// VC 6.0及之前版本。
	len = _vtprintf(format, args);
#else
	len = _vsctprintf(format, args);
#endif

	if (len == 0) {
		return;
	}
	len += (1 + 2);

	buf = (LPTSTR) malloc(len * sizeof(TCHAR));
	if (buf) 
	{
#if (_MSC_VER <= 1200)
		len = _vsntprintf(buf, len, format, args);
#else
		len = _vstprintf_s(buf, len, format, args);
#endif
		if (len > 0) 
		{
			while (len && isspace(buf[len-1])) len--;
			buf[len++] = _T('\r');
			buf[len++] = _T('\n');
			buf[len] = 0;
			OutputDebugString(buf);
		}
		free(buf);
	}
	va_end(args);
}

tstring GetFileNameWithoutExtension(LPCTSTR lpszPath)
{
	tstring strFileName = ::PathFindFileName(lpszPath);
	int		nPos = strFileName.rfind(".");
	if (nPos != tstring::npos) {
		return strFileName.substr(0, nPos);
	}
	return strFileName;
}


LPCTSTR GetMacByArp(LPCTSTR lpszIp, LPTSTR lpszMac)
{
	struct in_addr addr;

	DWORD dwMacAddLen = 6;
	BYTE  byMacAddress[6];

#define MAX_TRY_TIMES	3
	addr.S_un.S_addr = inet_addr(lpszIp);
	for (int i = 0; i < MAX_TRY_TIMES; i ++)
	{
		if (SendARP(addr.S_un.S_addr, NULL, (DWORD *)&byMacAddress, &dwMacAddLen) == NO_ERROR)
		{
			// Got Remote MAC Address
			wsprintf(lpszMac, _T("%.2x-%.2x-%.2x-%.2x-%.2x-%.2x"), byMacAddress[0], byMacAddress[1], 
				byMacAddress[2], byMacAddress[3], byMacAddress[4], byMacAddress[5]);
			return lpszMac;
		}
		Sleep(200);
	}
	return NULL;
}

// 显示隐藏在任务栏，来自http://blog.csdn.net/harbinzju/article/details/7047485
BOOL ShowInTaskBar(HWND hWnd, BOOL bShow)
{
	HRESULT			hr; 
	ITaskbarList*	pTaskbarList;

	hr = CoCreateInstance( CLSID_TaskbarList, NULL, CLSCTX_INPROC_SERVER,  
		IID_ITaskbarList, (void**)&pTaskbarList );

	if(SUCCEEDED(hr))
	{
		pTaskbarList->HrInit();
		if(bShow)
			pTaskbarList->AddTab(hWnd);
		else
			pTaskbarList->DeleteTab(hWnd);
		pTaskbarList->Release();
		return TRUE;
	}

	return FALSE;
}

// http://blog.csdn.net/windows_nt/article/details/8470637
HICON BitmapToIcon(HBITMAP hBitmap, int nWidth, int nHeight)
{
	HBITMAP hbmMask = ::CreateCompatibleBitmap(::GetDC(NULL), 
		nWidth, nHeight);

	ICONINFO ii = {0};
	ii.fIcon = TRUE;
	ii.hbmColor = hBitmap;
	ii.hbmMask = hbmMask;

	HICON hIcon = ::CreateIconIndirect(&ii);//一旦不再需要，注意用DestroyIcon函数释放占用的内存及资源
	::DeleteObject(hbmMask);

	return hIcon;
}

BOOL IsValidAddrString(LPCTSTR pszText)
{
	LONG a, b, c, d;

	if (4 == sscanf(pszText, _T("%d.%d.%d.%d"), &a, &b, &c, &d)) 
	{
		if (0<=a && a<=255
			&& 0<=b && b<=255
			&& 0<=c && c<=255
			&& 0<=d && d<=255) 
		{
			return TRUE;
		}
	}
	return FALSE;
}

