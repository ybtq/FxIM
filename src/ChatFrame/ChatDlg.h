#ifndef _CHATDLG_H_
#define _CHATDLG_H_
#include "ChatFrame/IImageOle.h"
#include "ControlEx/UIFileTrans.h"
#include "ControlEx/skin_change_event.hpp"

typedef std::map<tstring, IImageOle*>	MapImageOle;
typedef std::map<CControlUI*, CControlUI*>	MapControl;

class CChatDlg : public WindowImplBase, public SkinChangedReceiver
{
public:
	CChatDlg(Host* pHost);
	~CChatDlg();

public:
	LPCTSTR GetWindowClassName() const;
	virtual void OnFinalMessage(HWND hWnd);
	virtual CDuiString GetSkinFile();
	virtual CDuiString GetSkinFolder();
	virtual CControlUI* CreateControl(LPCTSTR pstrClass);
	virtual UILIB_RESOURCETYPE GetResourceType() const;
	
	virtual BOOL Receive(SkinChangedParam param);
protected:
	void	Notify(TNotifyUI& msg);
	void	InitWindow();
	void	UpdatePersonalInfo();

	LRESULT HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT	OnSysCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnNotify(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnRButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnFaceCtrlSel(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT	OnMenuClick(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnTcpEvent(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT	OnSendObjEvent(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnRecvObjEvent(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	void	_RichEdit_ReplaceSel(CRichEditUI * pRichEdit, LPCTSTR lpszNewText, CharFormatLite& cfLite, int nStartIndent);
	BOOL	_RichEdit_InsertFace(CRichEditUI * pRichEdit, LPCTSTR lpszFileName, int nFaceId, int nFaceIndex, IImageOle** ppImageOle = NULL);
	
	BOOL	HandleSysFaceId(CRichEditUI * pRichEdit, LPCTSTR& p, tstring& strText, CharFormatLite& cfLite, int nStartIndent);
	BOOL	HandleSysFaceIndex(CRichEditUI * pRichEdit, LPCTSTR& p, tstring& strText, CharFormatLite& cfLite, int nStartIndent);
	BOOL	HandleCustomPic(CRichEditUI * pRichEdit, LPCTSTR& p, tstring& strText, CharFormatLite& cfLite, int nStartIndent);
	tstring	EncodeFontString(CharFormatLite& cfLite);
	BOOL	HandleCharFormatLite(LPCTSTR& p, CharFormatLite& cfLite);
	void	AddMsg(CRichEditUI * pRichEdit, LPCTSTR lpText, int nStartIndent);
	void	AddMsgToRichEditInput(LPCTSTR lpText);
	void	AddMsgToRichEditView(LPCTSTR lpText, LPCTSTR lpName, time_t msgTime, COLORREF crName);
	void	AddTipMsgToRichEditView(LPCTSTR lpText);
	void	OnBtnEmotion(TNotifyUI& msg);
	void	OnBtnImage(TNotifyUI& msg);
	void	OnBtnBold(TNotifyUI& msg);
	void	OnBtnItalic(TNotifyUI& msg);
	void	OnBtnUnderLine(TNotifyUI& msg);
	void	OnBtnColor(TNotifyUI& msg);
	void	OnBtnFile(TNotifyUI& msg);
	void	OnBtnFileRight(TNotifyUI& msg);
	void	OnBtnScreenShots(TNotifyUI& msg);

	void	OnItemSelectComboFontName(TNotifyUI& msg);
	void	OnItemSelectComboFontSize(TNotifyUI& msg);

	void	OnBrowserFile();
	void	OnBrowserFolder();
	void	SetButtonFileShareText(bool bVisible);
	BOOL	MakeShareStr(char *dest, ShareInfo *shareInfo, int maxLen);

	BOOL	AddSendFileTrans(ULONG nPacketNo);
	BOOL	AddRecvFileTrans(ULONG nPacketNo);
	void	AdjustWindowSize(int nRightPartWidth);
	
	void	OnBtnSendMsg();
	void	OnBtnSendMsgRight(TNotifyUI& msg);
	void	SendMsgSub();	
	void	SendGetPubKeyMsg();

	void	DisposeLocalMsg(tstring& strText);
	void	AddImageMsg(MsgBuf *msgBuf);
	
	BOOL	EncryptMsg(char *dest, Host *host, const char *src);
	BOOL	DecryptMsg(MsgBuf *msgBuf);		//解析消息

	bool	AddPanel(LPCTSTR pszText, CControlUI* pControlPanel, 
				bool bHasCloseBtn = false, LPCTSTR pszCloseBtnName = NULL);
	void	AdjustLayoutOption();
	void	CreateFileTrans();
	void	CloseFileTrans(TNotifyUI& msg);	

//	void	CreateMsgLog();
//	void	CloseMsgLog(TNotifyUI& msg);
public:
	void	SendPubKeyNotify(HostSub *hostSub, BYTE *pubkey, int len, int e, int capa);
	void	AddMsgBuf(MsgBuf *msg);
	void	_RichEdit_SetFont(CRichEditUI* pRichEdit, CharFormatLite& cfLite);
	void	ShowFlashWindow();
	void	SetShareInfo(ShareInfo* pShareInfo);
private:
	Host*		m_pHost;
	SendEntry	m_sendEntry;
	
	ShareInfo*	m_pShareInfo;
	ShareInfo*	m_pShareInfoImage;			// 消息中的图片
	char*		m_szShare;
	char		m_szMsgBuf[MAX_UDPBUF];		// 存储的是处理过的消息(消息中自定义图片不是文件路径，还是文件名)

	MapImageOle	m_mapImageOle;

	CDuiString	m_strWarningIcon;
	CDuiString	m_strTipFontString;
	CDuiString	m_strRichEditViewZoom;

private:
	CButtonUI*				m_pBtnSysRestore;
	CButtonUI*				m_pBtnSysMax;
	COptionUI*				m_pOptionFont;
	COptionUI*				m_pOptionEmotion;
	CButtonUI*				m_pBtnImage;
	CHorizontalLayoutUI*	m_pLayoutFontbar;
	CRichEditUI*			m_pRichEditInput;
	CRichEditUI*			m_pRichEditView;

	CComboBoxUI*			m_pComboFontName;
	CComboBoxUI*			m_pComboFontSize;
	COptionUI*				m_pOptionBold;
	COptionUI*				m_pOptionItalic;
	COptionUI*				m_pOptionUnderline;

	CLabelUI*				m_pLabelAvatar;
	CLabelUI*				m_pLabelNickName;
	CLabelUI*				m_pLabelDescription;

	CButtonUI*				m_pBtnFileShare;

	// 右侧面板
	CVerticalLayoutUI*		m_pVLayoutRightPanel;
	CHorizontalLayoutUI*	m_pHLayoutOption;			// 右侧面板标签页按钮组
	CTabLayoutUI*			m_pTLayoutPanel;
	CVerticalLayoutUI*		m_pVLayoutPersonal;
	CFileTransUI*			m_pFileTrans;
	MapControl				m_mapButtonClose;
};


#endif // _CHATDLG_H_