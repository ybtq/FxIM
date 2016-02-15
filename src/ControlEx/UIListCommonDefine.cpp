#include "stdafx.h"
#include "ControlEx/UIListCommonDefine.hpp"

namespace DuiLib
{

double CalculateDelay(double state)
{
    return pow(state, 2);
}

void Node::SetParent(Node* parent)
{
	m_pParent = parent;
}

Node::Node()
	: m_pParent(NULL)
{}

Node::Node(NodeData t)
	: m_data(t)
	, m_pParent(NULL)
{}

Node::Node(NodeData t, Node* parent)
: m_data (t)
, m_pParent (parent)
{}

Node::~Node() 
{
	for (int i = 0; i < ChildrenSize(); ++i)
		delete m_children[i]; 
}

NodeData& Node::Data()
{
	return m_data;
}

int Node::ChildrenSize() const
{
	return static_cast<int>(m_children.size());
}

Node* Node::GetChild(int i)
{
	return m_children[i];
}

Node* Node::GetParent()
{
	return ( m_pParent);
}

bool Node::IsHasChildren() const
{
	return ChildrenSize() > 0;
}

bool Node::IsFolder() const
{
	return m_data.bFolder;
}

void Node::AddChild(Node* child)
{
	child->SetParent(this); 
	m_children.push_back(child); 
}


void Node::RemoveChild(Node* child)
{
	Children::iterator iter = m_children.begin();
	for (; iter < m_children.end(); ++iter)
	{
		if (*iter == child) 
		{
			m_children.erase(iter);
			return;
		}
	}
}

Node* Node::GetLastChild()
{
	if (IsHasChildren())
	{
		return GetChild(ChildrenSize() - 1)->GetLastChild();
	}
	return this;
}

}