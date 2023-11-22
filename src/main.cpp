#include "Tree.hpp"

int main()
{
    Tree::StartHtmlLogging();

    Tree tree = {};
    tree.Init(TreeNode::New(0, nullptr, nullptr).value);

    TreeNode* cur = tree.root;
    // tree.root->value = 5;
    // tree.root->right = TreeNode::New(7, TreeNode::New(6, nullptr, nullptr).value, TreeNode::New(8, nullptr, nullptr).value).value;
    // tree.root->left = TreeNode::New(3, TreeNode::New(2, nullptr, nullptr).value, TreeNode::New(4, nullptr, nullptr).value).value;

    // tree.root->value = 10;
    // tree.root->left = TreeNode::New(5, nullptr, nullptr).value;
    // tree.root->right = TreeNode::New(15, TreeNode::New(12, nullptr, nullptr).value, TreeNode::New(20, nullptr, nullptr).value).value;

    // tree.root->value = 10;
    // tree.root->right = TreeNode::New(15, nullptr, nullptr).value;
    // tree.root->left = tree.root->right->Copy().value;
    // tree.root->right->right = tree.root->right->Copy().value;
    // tree.root->right->right->right = tree.root->right;

    tree.root->value = 10;
    // TreeNode* bottom = nullptr;
    TreeNode* bottom = TreeNode::New(7, nullptr, nullptr).value;
    TreeNode* left = TreeNode::New(5, nullptr, bottom).value;
    TreeNode* right = TreeNode::New(15, bottom, nullptr).value;
    tree.root->left = left;
    tree.root->right = right;

    fprintf(stderr, "%s\n", ERROR_CODE_NAMES[tree.UpdateTree()]);
    fprintf(stderr, "%zu\n", tree.size);

    tree.Dump();

    tree.Print("tree.txt");

    tree.Destructor();

    // Tree newTree = {};

    // newTree.Read("tree.txt");

    // newTree.size = 1;

    // newTree.Dump();

    // newTree.Print("newTree.txt");

    // newTree.Destructor();

    Tree::EndHtmlLogging();

    return 0;
}