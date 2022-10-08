

#include "Tree.h"


CollectablebinaryTreeNode::CollectablebinaryTreeNode(const std::string name)
	: _l(this), _r(this),
	  _name(name)
{
}


CollectablebinaryTreeNode::~CollectablebinaryTreeNode()
{
}


const std::string& CollectablebinaryTreeNode::Name()
{
	return _name;
}


std::string CollectablebinaryTreeNode::ToString()
{
	return ComposeString(0);
}


std::string CollectablebinaryTreeNode::ComposeString(int indent)
{
	std::string indentStr;
	for (int i = 0; i < indent; ++i)
		indentStr = indentStr + "  ";

	std::string content = indentStr + Name() + "\n";
	
	if (_l)
		content = content + _l->ComposeString(indent + 1);
	if (_r)
		content = content + _r->ComposeString(indent + 1);

	return content;
}

