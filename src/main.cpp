#include "Tree.hpp"

int main()
{
    Tree tree;
    tree.Init();

    TreeNode* cur = tree.root;
    tree.root->value = 5;
    tree.root->right = TreeNode::NewNode(7, TreeNode::NewNode(6, nullptr, nullptr).value, TreeNode::NewNode(8, nullptr, nullptr).value).value;
    tree.root->left = TreeNode::NewNode(3, TreeNode::NewNode(2, nullptr, nullptr).value, TreeNode::NewNode(4, nullptr, nullptr).value).value;

    tree.CountNodes();

    tree.Dump();

    tree.Destructor();

    return 0;
}