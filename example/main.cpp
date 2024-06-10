#include "Tree.hpp"

int main()
{
    const char* logFolder = "../log";

    Tree::StartLogging(logFolder);

    Tree tree = {};

    TreeNodeResult rootRes = TreeNode::New(10, nullptr, nullptr);
    MyAssertSoft(!rootRes.error, rootRes.error);

    tree.Init(rootRes.value);

    TreeNodeResult leftRes = TreeNode::New(5, nullptr, nullptr);
    MyAssertSoft(!leftRes.error, leftRes.error);

    tree.root->SetLeft(leftRes.value);

    TreeNodeResult rightRes = TreeNode::New(15, nullptr, nullptr);
    MyAssertSoft(!rightRes.error, rightRes.error);

    tree.root->SetRight(rightRes.value);

    ErrorCode error = tree.Dump();
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

    Tree::EndLogging();

    return 0;
}