#pragma once

#include <vector>
#include <string>
#include <time.h>

#if defined(UNICODE) || defined(_UNICODE)
	typedef std::wstring tstring;
#else
	typedef std::string tstring;
#endif

CHAR *	UnicodeToUtf8(const WCHAR * lpszStr);
WCHAR * AnsiToUnicode(const CHAR * lpszStr);
WCHAR * Utf8ToUnicode(const CHAR * lpszStr);
CHAR *	UnicodeToAnsi(const WCHAR * lpszStr);

void	ConvertGBKToUtf8(std::string& strGBK);
void	ConvertUtf8ToGBK(std::string& strUtf8);


BOOL	EnumSysFont(std::vector<tstring>& arrSysFont);		// 枚举系统字体
BOOL	FlashWindowEx(HWND hWnd, int nCount);				// 闪烁窗口标题栏
BOOL	GetTrayWndRect(LPRECT lpRect);						// 获取系统任务栏区域
LPCTSTR	FormatTime(time_t lTime, LPCTSTR lpFmt);

BOOL	IsDigit(LPCTSTR lpStr);

void	Replace(tstring& strText, LPCTSTR lpOldStr, LPCTSTR lpNewStr);
void	ReplaceA(std::string& strText, LPCSTR lpOldStr, LPCSTR lpNewStr);
void	ReplaceW(std::wstring& strText, LPCWSTR lpOldStr, LPCWSTR lpNewStr);

void	EncodeHtmlSpecialChars(tstring& strText);			// 编码Html特殊字符
void	DecodeHtmlSpecialChars(tstring& strText);			// 解码Html特殊字符

tstring GetBetweenString(LPCTSTR pStr, TCHAR cStart, TCHAR cEnd);
int		GetBetweenInt(LPCTSTR pStr, TCHAR cStart, TCHAR cEnd, int nDefValue = 0);

tstring GetBetweenString(LPCTSTR pStr, LPCTSTR pStart, LPCTSTR pEnd);
int		GetBetweenInt(LPCTSTR pStr, LPCTSTR pStart, LPCTSTR pEnd, int nDefValue = 0);

BOOL	DllRegisterServer(LPCTSTR lpszFileName);
BOOL	DllUnregisterServer(LPCTSTR lpszFileName);

GUID	GetFileTypeGuidByExtension(LPCWSTR lpExtension);
CLSID	GetEncoderClsidByExtension(LPCWSTR lpExtension);
CLSID	GetEncoderClsidByFileType(REFGUID guidFileType);
CLSID	GetEncoderClsidByMimeType(LPCWSTR lpMineType);

tstring GetFileNameWithoutExtension(LPCTSTR lpszPath);

LPCTSTR	GetMacByArp(LPCTSTR lpszIp, LPTSTR lpszMac);

BOOL	ShowInTaskBar(HWND hWnd, BOOL bShow);

void	odprintf(LPCTSTR format, ...);

HICON	BitmapToIcon(HBITMAP hBitmap, int nWidth, int nHeight);

BOOL	IsValidAddrString(LPCTSTR pszText);