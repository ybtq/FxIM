#ifndef UILISTCOMMONDEFINE_HPP
#define UILISTCOMMONDEFINE_HPP

#include <math.h>

namespace DuiLib
{

struct NodeData
{
	int			nLevel;
	bool		bFolder;
	bool		bChildVisible;
//	bool		bHasChild;
	CDuiString	sAvatar;
	CDuiString	sName;
	CDuiString	sId;
	CDuiString	sDescription;
	CListContainerElementUI* pListElement;
};

double CalculateDelay(double state);

class Node
{
public:
	Node();
	explicit Node(NodeData t);
	Node(NodeData t, Node* parent);
	~Node();
	NodeData&	Data();
	int			ChildrenSize() const;
	Node*		GetChild(int i);
	Node*		GetParent();
	bool		IsFolder() const;
	bool		IsHasChildren() const;
	void		AddChild(Node* child);
	void		RemoveChild(Node* child);
	Node*		GetLastChild();

private:
	void		SetParent(Node* parent);

private:
	typedef std::vector <Node*>	Children;

	Children	m_children;
	Node*		m_pParent;

	NodeData    m_data;
};

} // DuiLib

#endif // UILISTCOMMONDEFINE_HPP