
#ifndef __NUO_UTILS_TREE__
#define __NUO_UTILS_TREE__


#include "NuoCollection.h"

class CollectablebinaryTreeNode : public NuoObject<CollectablebinaryTreeNode>
{

public:

	NuoMemberPtr<CollectablebinaryTreeNode, CollectablebinaryTreeNode> _l;
	NuoMemberPtr<CollectablebinaryTreeNode, CollectablebinaryTreeNode> _r;

private:

	std::string _name;

public:

	CollectablebinaryTreeNode(const std::string name);
	virtual ~CollectablebinaryTreeNode();

	const std::string& Name();
	std::string ToString();

private:

	std::string ComposeString(int indent);

};


#endif
