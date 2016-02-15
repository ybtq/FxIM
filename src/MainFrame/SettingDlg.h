#ifndef _SETTINGDLG_H_
#define _SETTINGDLG_H_

#include "Utils/Config.h"
#include "ControlEx/skin_change_event.hpp"

class CSettingDlg : public WindowImplBase, public SkinChangedReceiver
{
public:
	CSettingDlg();
	~CSettingDlg();

	LPCTSTR GetWindowClassName() const;
	virtual void OnFinalMessage(HWND hWnd);
	virtual CDuiString GetSkinFile();
	virtual CDuiString GetSkinFolder();
	virtual UILIB_RESOURCETYPE GetResourceType() const;
	virtual void InitWindow();

	virtual BOOL Receive(SkinChangedParam param);
protected:
	void	InitTabLayoutNetWork();

	void	Notify(TNotifyUI& msg);

	void	OnBtnNetSegmentAdd();
	void	OnBtnNetSegmentDel();

	void	OnBrowserAvatarFile();
	
	void	OnOK();
	
private:
	BOOL		m_bPersonalChange;
	BOOL		m_bGeneralChange;
	BOOL		m_bNetworkChange;

private:
	
	CTabLayoutUI*		m_pTabLayoutSetting;

	// 个人设置页
	CVerticalLayoutUI*	m_pVerticalLayoutPersonal;
	CLabelUI*			m_pLabelAvatar;
	CEditUI*			m_pEditNickName;
	CEditUI*			m_pEditGroupName;

	// 常规设置页
	CVerticalLayoutUI*	m_pVerticalLayoutGeneral;

	// 网络设置页
	CVerticalLayoutUI*	m_pVerticalLayoutNetwork;
	CComboBoxUI*		m_pComboAdapter;
};

#endif //_SETTINGDLG_H_