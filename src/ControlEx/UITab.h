#ifndef _UITAB_H_
#define _UITAB_H_

#include <map>

typedef std::map<CControlUI*, CControlUI*>	MapControl;

namespace DuiLib
{
class CTabUI : public CHorizontalLayoutUI
{
public:
	CTabUI();
	~CTabUI();
	
	void		SetTabLayout(CTabLayoutUI* pTabLayout);
	bool		AddPage(LPCTSTR pszText, CControlUI* pControlPage, bool bHasCloseBtn);

protected:
	void		DoInit();
	void		AdjustControl();

	bool		OnCloseTab(void* param);
	bool		OnSelect(void* param);

private:
//	MapControl	m_mapOption;
	MapControl	m_mapButtonClose;
	CTabLayoutUI*	m_pTabLayout;
};
} // namespace DuiLib
#endif //_UITAB_H_