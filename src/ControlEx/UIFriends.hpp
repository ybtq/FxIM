#ifndef UIFRIENDS_HPP
#define UIFRIENDS_HPP

//#include "UIListCommonDefine.hpp"

namespace DuiLib
{

class CFriendNodeUI;
class CFriendGroupUI;
class CFriendsUI : public CTreeViewUI, public IDialogBuilderCallback
{
public:
	enum {SCROLL_TIMERID = 10, FRIENDNODEJUMP_TIMERID = 11};

	CFriendsUI();
	~CFriendsUI();

	double			CalculateDelay(double state);
	void			DoEvent(TEventUI& event);

	CControlUI*		CreateControl(LPCTSTR pstrClass);	
	bool			AddFriendNode(Host* pHost, int nIndex = -1);
	bool			UpdateFriendNode(Host* pHost);
	bool			DeleteFriendNode(Host* pHost);
	void			SetFriendNodeJump(Host* pHost, bool bJump);

	bool			AddFriendGroup(LPCTSTR pstrGroup, int nIndex = -1);
private:
	LONG				m_lDelayDeltaY;
	LONG				m_lDelayNumber;
	LONG				m_lDelayLeft;
	CStdPtrArray		m_arrayJump;
	bool				m_bJumpTimer;
//	CPaintManagerUI&	m_paintManager;	
};

class CFriendGroupUI : public CTreeNodeUI
{
public:
	CFriendGroupUI();

	LPCTSTR		GetClass() const;
	LPVOID		GetInterface(LPCTSTR pstrName);

	void		DoEvent(TEventUI& event);
};

class CFriendNodeUI : public CTreeNodeUI
{
public:
	CFriendNodeUI();
	~CFriendNodeUI();

	LPCTSTR		GetClass() const;
	LPVOID		GetInterface(LPCTSTR pstrName);

	bool		Activate();
	void		DoInit();
	void		DoEvent(TEventUI& event);
	bool		Select(bool bSelect = true);

	Host*		GetHost();
	void		SetJump(bool bJump);
	void		DoJump();

private:
	bool			m_bJump;
	bool			m_bFlag;
	SIZE			m_cXYAvatar;
	SIZE			m_cXYAvatar2;
	CLabelUI*		m_pLabelAvatar;
	CLabelUI*		m_pLabelAvatar2;
	CTabLayoutUI	*m_pFriendTab;
};

} // DuiLib

#endif // UIFRIENDS_HPP