#include "Tree.hpp"

int main()
{
    Tree::StartHtmlLogging();

    Tree tree = {};

    TreeNodeResult rootRes = TreeNode::New(10, nullptr, nullptr);
    MyAssertSoft(!rootRes.error, rootRes.error);

    tree.Init(rootRes.value);

    TreeNodeResult leftRes = TreeNode::New(5, nullptr, nullptr);
    MyAssertSoft(!leftRes.error, leftRes.error);

    tree.root->AddLeft(leftRes.value);

    TreeNodeResult rightRes = TreeNode::New(15, nullptr, nullptr);
    MyAssertSoft(!rightRes.error, rightRes.error);

    tree.root->AddRight(rightRes.value);

    ErrorCode error = tree.Dump();
    MyAssertSoft(!error, error);

    TreeNodeResult looperRes = TreeNode::New(8, leftRes.value, rightRes.value);
    MyAssertSoft(!looperRes.error, looperRes.error);

    rightRes.value->AddLeft(looperRes.value);
    leftRes.value->AddRight(looperRes.value);

    error = tree.Dump();
    MyAssertSoft(!error, error);

    error = tree.Print("tree.txt");
    MyAssertSoft(!error, error);

    error = tree.Destructor();
    MyAssertSoft(!error, error);

    Tree newTree = {};

    error = newTree.Read("tree.txt");
    MyAssertSoft(!error, error);

    error = newTree.Dump();
    MyAssertSoft(!error, error);

    error = newTree.Verify();
    MyAssertSoft(!error, error);

    error = newTree.Print("newTree.txt");
    MyAssertSoft(!error, error);

    error = newTree.Destructor();
    MyAssertSoft(!error, error);

    Tree::EndHtmlLogging();

    return 0;
}