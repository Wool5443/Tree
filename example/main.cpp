#include "Tree.hpp"

int main()
{
    const char* logFolder = "../log";

    Tree::StartLogging(logFolder);

    Tree tree = {};

    TreeNodeResult rootRes = TreeNode::New(10, nullptr, nullptr);
    SoftAssert(!rootRes.error, rootRes.error);

    tree.Init(rootRes.value);

    TreeNodeResult leftRes = TreeNode::New(5, nullptr, nullptr);
    SoftAssert(!leftRes.error, leftRes.error);

    tree.root->SetLeft(leftRes.value);

    TreeNodeResult rightRes = TreeNode::New(15, nullptr, nullptr);
    SoftAssert(!rightRes.error, rightRes.error);

    tree.root->SetRight(rightRes.value);

    Error error = tree.Dump();
    SoftAssert(!error, error);

    error = tree.Print("tree.txt");
    SoftAssert(!error, error);

    error = tree.Destructor();
    SoftAssert(!error, error);

    Tree newTree = {};

    error = newTree.Read("tree.txt");
    SoftAssert(!error, error);

    error = newTree.Dump();
    SoftAssert(!error, error);

    error = newTree.Verify();
    SoftAssert(!error, error);

    error = newTree.Print("newTree.txt");
    SoftAssert(!error, error);

    error = newTree.Destructor();
    SoftAssert(!error, error);

    Tree::EndLogging();

    return 0;
}
