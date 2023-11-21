#ifndef TREE_HPP
#define TREE_HPP

#include "Utils.hpp"
#include "TreeSettings.ini"

static const char* DOT_FOLDER = "log/dot";
static const char* IMG_FOLDER = "log/img";

struct TreeNodeResult;
struct TreeNode
{
    TreeElement_t value;
    TreeNode* left;
    TreeNode* right;
    size_t id;

    static TreeNodeResult New(TreeElement_t value, TreeNode* left, TreeNode* right);
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

    ErrorCode Init(TreeNode* root);
    ErrorCode Destructor();
    ErrorCode Verify();
    
    TreeNodeCountResult CountNodes();

    ErrorCode Dump();

    ErrorCode Print(const char* outPath);
    ErrorCode Read(const char* inPath);
};

#endif
