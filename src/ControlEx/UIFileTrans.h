#ifndef UIFILETRANS_HPP
#define UIFILETRANS_HPP



namespace DuiLib
{
class CSendFileElementUI;
class CRecvFileElementUI;
class CFileTransUI : public CListUI, public IDialogBuilderCallback
{
public:
	enum {SCROLL_TIMERID = 10};

	CFileTransUI();
	~CFileTransUI();

	double			CalculateDelay(double state);
	CControlUI*		CreateControl(LPCTSTR pstrClass);	
	void			DoEvent(TEventUI& event);
	void			DoInit();

	bool			AddSendFileObj(SendFileObj* obj);
	bool			AddRecvFileObj(RecvFileObj* obj);

	bool			RemoveShareElement(CControlUI* pElement);
	void			RemoveAllShareElement();
	LONG			GetShareCount();
protected:

private:
	LONG				m_lDelayDeltaY;
	LONG				m_lDelayNumber;
	LONG				m_lDelayLeft;

	LONG				m_nShareCount;			// 发送 和 接收项的总和
};

class CSendFileElementUI : public CListContainerElementUI
{
public:
	CSendFileElementUI();

	LPCTSTR		GetClass() const;
	LPVOID		GetInterface(LPCTSTR pstrName);
	void		DoInit();

	bool		OnBtnCancel(void* param);

	void		StartSendFile();
	void		SetTransState();
	void		EndSendFile();
private:
	int				m_nPercent;			// For IPMSG
	
	CProgressUI*	m_pProgress;
	CLabelUI*		m_pLabelSpeed;
	CButtonUI*		m_pBtnCancel;
};

class CRecvFileElementUI : public CListContainerElementUI
{
public:
	CRecvFileElementUI();

	LPCTSTR		GetClass() const;
	LPVOID		GetInterface(LPCTSTR pstrName);
	void		DoInit();

	bool		OnBtnRecv(void* param);
	bool		OnBtnSaveAs(void* param);
	bool		OnBtnCancel(void* param);

	void		StartRecvFile();
	void		SetTransState();
	void		EndRecvFile();
private:
	int				m_nPercent;			// For IPMSG

	CLabelUI*		m_pLableFname;
	CProgressUI*	m_pProgress;
	CLabelUI*		m_pLabelSpeed;
	CButtonUI*		m_pBtnRecv;
	CButtonUI*		m_pBtnSaveAs;
	CButtonUI*		m_pBtnCancel;	
};
}

#endif //UIFILETRANS_HPP