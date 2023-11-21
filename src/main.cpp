#include "Tree.hpp"

int main()
{
    Tree tree = {};
    tree.Init(TreeNode::New(0, nullptr, nullptr).value);

    TreeNode* cur = tree.root;
    // tree.root->value = 5;
    // tree.root->right = TreeNode::New(7, TreeNode::New(6, nullptr, nullptr).value, TreeNode::New(8, nullptr, nullptr).value).value;
    // tree.root->left = TreeNode::New(3, TreeNode::New(2, nullptr, nullptr).value, TreeNode::New(4, nullptr, nullptr).value).value;

    tree.root->value = 10;
    tree.root->left = TreeNode::New(5, nullptr, nullptr).value;
    tree.root->right = TreeNode::New(15, TreeNode::New(12, nullptr, nullptr).value, TreeNode::New(20, nullptr, nullptr).value).value;

    tree.CountNodes();

    tree.Dump();

    tree.Print("tree.txt");

    tree.Destructor();

    Tree newTree = {};

    newTree.Read("tree.txt");

    newTree.Dump();

    newTree.Print("newTree.txt");

    newTree.Destructor();

    return 0;
}