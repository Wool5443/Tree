#ifndef TREE_HPP
#define TREE_HPP

#include "Utils.hpp"
#include "TreeSettings.ini"

const char* DOT_FOLDER = "log/dot";
const char* IMG_FOLDER = "log/img";

struct TreeNodeResult;
struct TreeNode
{
    TreeElement_t value;
    TreeNode* left;
    TreeNode* right;
    size_t id;

    static TreeNodeResult NewNode(TreeElement_t value, TreeNode* left, TreeNode* right);
    ErrorCode DeleteNode();
    TreeNodeResult Copy();
};
struct TreeNodeResult
{
    TreeNode* value;
    ErrorCode error;
};

struct TreeNodeCountResult
{
    size_t value;
    ErrorCode error;
};

struct Tree
{
    TreeNode* root;
    size_t size;

    ErrorCode Init();
    ErrorCode Destructor();
    ErrorCode Verify();
    
    TreeNodeCountResult CountNodes();

    ErrorCode Dump();
};

#endif
