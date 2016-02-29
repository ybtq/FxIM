#ifndef _MAINDLG_H_
#define _MAINDLG_H_
#include "Utils/Config.h"
#include "MainFrame/MsgMng.h"

#include "ChatFrame/ChatDlg.h"
#include "ChatFrame/FaceList.h"
#include "ChatFrame/RichEditUtil.h"
#include "ControlEx/UIFriends.hpp"
#include "ControlEx/skin_change_event.hpp"

typedef std::map<Host*, CChatDlg*>	MapChatDlg;
typedef std::vector<MsgBuf*>		VecMsgBuf;

class CMainDlg : public WindowImplBase, public SkinChangedReceiver
{
private:
	CMainDlg();
	~CMainDlg();
	DECLARE_SINGLETON_CLASS(CMainDlg)
public:
	LPCTSTR GetWindowClassName() const;
	virtual void OnFinalMessage(HWND hWnd);
	virtual CDuiString GetSkinFile();
	virtual CDuiString GetSkinFolder();
	virtual CControlUI* CreateControl(LPCTSTR pstrClass);
	virtual UILIB_RESOURCETYPE GetResourceType() const;
	virtual BOOL Receive(SkinChangedParam param);
public:
	BOOL	TaskBar(DWORD nimMode, HICON hIcon = NULL, LPCTSTR lpszTip = NULL);	
	char*	GetNickNameEx();

	BOOL	BrowserFile(HWND hParent, ShareInfo** ppShareInfo);
	BOOL	BrowserFolder(HWND hParent, ShareInfo** ppShareInfo);

	void	UpdateInfo();
	BOOL	RestartApplication();

	void	BroadcastEntry();
	void	BroadcastMsg(ULONG mode, const char* msg = NULL, const char* exMsg = NULL);
protected:
	LRESULT HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT	OnUdpEvent(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT	OnTcpEvent(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnTaskbarNotify(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT	OnMenuClick(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT	OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnRecvObjEvent(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);;
	void	Notify(TNotifyUI& msg);

	void	InitWindow();
	void	Close(UINT nRet = IDOK);
	BOOL	InitCrypt(void);
	BOOL	InitCryptCore(int ctl_flg = 0);
	BOOL	SetupRSAKey(int bitLen, int ctl_flg = 0);
	BOOL	LoadPrivBlob(BYTE *rawBlob, int *rawBlobLen);	//‘ÿ»ÎÀΩ‘ø
	BOOL	StorePrivBlob(BYTE *rawBlob, int rawBlobLen);	//±£¥ÊÀΩ‘ø

	void	BroadcastMsgSub(ULONG addr, USHORT portNo, ULONG mode, 
				const char* msg, const char* exMsg = NULL);

	ULONG	HostStatus();
	

	void	MsgBrEntry(MsgBuf *msg);
	void	MsgAnsEntry(MsgBuf *msg);
	void	MsgBrExit(MsgBuf *msg);
	void	MsgSendMsg(MsgBuf *msg);
	void	MsgRecvMsg(MsgBuf *msg);
	void	MsgGetPubKey(MsgBuf *msg);
	void	MsgAnsPubKey(MsgBuf *msg);
	void	MsgGetAvatar(MsgBuf *msg);
	void	MsgAnsAvatar(MsgBuf *msg);
//	void	MsgSendImage(MsgBuf* msg);

	void	GenerateAvatarIcon(Host* pHost);
	void	AddHost(HostSub *hostSub, ULONG command, char *nickName, char *groupName);
	void	MsgJump();

	CChatDlg*	ShowChatDlg(Host* pHost, BOOL bOpenIfNotExist);
	void		ShowSettingDlg();

private:
	HICON				m_hIcon;
	HICON				m_hIconNull;
	THosts				m_hosts;
	USHORT				m_nPortNo;	
	VecMsgBuf			m_vecMsgBuf;
	BOOL				m_bMsgJumpTimer;
	Host*				m_pMsgJumpHost;

	CFriendsUI*			m_pFriendList;
	CLabelUI*			m_pLabelAvatar;
	CLabelUI*			m_pLabelNickName;
	CLabelUI*			m_pLabelDescription;
public:
	static MapChatDlg	m_mapChatDlg;
};

#endif  //_MAINDLG_H_