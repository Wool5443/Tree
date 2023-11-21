#include "Tree.hpp"

int main()
{
    Tree tree;
    tree.Init();

    TreeNode* cur = tree.root;
    for (int i = 0; i < 10; i++)
    {
        TreeNodeResult nodeRes = TreeNode::NewNode((double)i, nullptr, nullptr);
        cur->right = nodeRes.value;
        cur->left  = nodeRes.value->Copy().value;
        cur = cur->right;
    }

    tree.CountNodes();

    tree.Dump();

    tree.Destructor();

    return 0;
}