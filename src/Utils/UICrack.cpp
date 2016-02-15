#include "StdAfx.h"
#include "Utils/UICrack.h"

namespace DuiLib {

BOOL SetControlBackGround(CControlUI* pControl, LPCTSTR pszBgImage, DWORD dwBgColor)
{
	if (pControl == NULL) {
		return FALSE;
	}

	pControl->SetBkColor(dwBgColor);

	if (*pszBgImage != 0)
	{
		int			nx, ny;
		TCHAR		szBuf[MAX_PATH] = {0};
		STRINGorID	sBitmap(pszBgImage);
		TImageInfo*	data = CRenderEngine::LoadImage(sBitmap);
		if (data != NULL) 
		{
			nx = data->nX > 2 ? data->nX - 2 : 0;
			ny = data->nY > 2 ? data->nY - 2 : 0;
#if defined(UNDER_WINCE)
			_stprintf(szBuf, _T("file='%s' corner='%d,%d,1,1'"), pszBgImage, nx, ny);
#else
			_stprintf_s(szBuf, MAX_PATH - 1, _T("file='%s' corner='%d,%d,1,1'"), pszBgImage, nx, ny);
#endif
			pControl->SetBkImage(szBuf);
			delete data;
		}
	}
	else {
		pControl->SetBkImage(_T(""));
	}	

	return TRUE;
}

}; // namespace DuiLib