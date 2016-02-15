#ifndef _UICRACK_H_
#define _UICRACK_H_

#ifdef _MSC_VER 
#pragma once
#endif

namespace DuiLib {

/////////////////////////////////////////////////////////////////////////////////////
//
//
// constrol class name and interface name
const TCHAR kWindowClassName[] =		_T("FxIMGuiFoundation");


//////////////////////////////////////////////////////////////////////////
//Control name.
//////////////////////////////////////////////////////////////////////////
const TCHAR kButtonSysCloseName[] =		_T("btn_sys_close");
const TCHAR kButtonSysMinName[] =		_T("btn_sys_min");
const TCHAR kButtonSysMaxName[] =		_T("btn_sys_max");
const TCHAR kButtonSysRestoreName[] =	_T("btn_sys_restore");

const TCHAR kButtonCloseName[] =		_T("btn_close");


//////////////////////////////////////////////////////////////////////////
// 设置控件背景和背景色

BOOL SetControlBackGround(CControlUI* pControl, LPCTSTR pszBgImage, DWORD dwBgColor);

}; // namespace DuiLib

#endif // _UICRACK_H_