#include "Tree.hpp"

int main()
{
    Tree::StartHtmlLogging();

    Tree tree = {};

    TreeNodeResult rootRes = TreeNode::New(10, nullptr, nullptr);
    MyAssertSoft(!rootRes.error, rootRes.error);

    tree.root = rootRes.value;

    TreeNodeResult leftRes = TreeNode::New(5, nullptr, nullptr);
    MyAssertSoft(!leftRes.error, leftRes.error);

    tree.root->AddLeft(leftRes.value);

    TreeNodeResult rightRes = TreeNode::New(15, nullptr, nullptr);
    MyAssertSoft(!rightRes.error, rightRes.error);

    tree.root->AddRight(rightRes.value);

    rightRes.value->AddRight(rightRes.value);

    fprintf(stderr, "Dump: %s\n", ERROR_CODE_NAMES[tree.Dump()]);

    // fprintf(stderr, "Destructor: %s\n", ERROR_CODE_NAMES[tree.Destructor()]);

    fprintf(stderr, "Delete: %s\n", ERROR_CODE_NAMES[tree.root->Delete()]);

    Tree::EndHtmlLogging();

    return 0;
}