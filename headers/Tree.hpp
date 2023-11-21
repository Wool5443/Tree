//! @file

#ifndef TREE_HPP
#define TREE_HPP

#include "Utils.hpp"
#include "TreeSettings.ini"

/**
 * @brief Log folders
 */
static const char* DOT_FOLDER = "log/dot";
static const char* IMG_FOLDER = "log/img";

struct TreeNodeResult;
/** @struct TreeNode
 * @brief A binary tree node containing value and ptrs to children
 * 
 * @var TreeNode::value - TreeElemen_t value
 * @var TreeNode::left - TreeNode* left
 * @var TreeNode::right - TreeNode* right
 * @var TreeNode::id - size_t id - unique id of a node, used for dumping
*/
struct TreeNode
{
    TreeElement_t value;
    TreeNode* left;
    TreeNode* right;
    size_t id;

    /**
     * @brief Returns a new node result
     * 
     * @param value - value
     * @param left - left child
     * @param right - right child
     * @return TreeNodeResult - new node
     */
    static TreeNodeResult New(TreeElement_t value, TreeNode* left, TreeNode* right);

    /**
     * @brief Deletes a node
     * 
     * @return Error
     */
    ErrorCode DeleteNode();

    /**
     * @brief Copies the node and returns the copy
     * 
     * @return TreeNodeResult the copy
     */
    TreeNodeResult Copy();
};
struct TreeNodeResult
{
    TreeNode* value;
    ErrorCode error;
};

/** @struct Tree
 * @brief Represents a binary tree
 * 
 * @var Tree::root - root of the tree
 * @var Tree::size - number of nodes in the tree
 */
struct Tree
{
    TreeNode* root;
    size_t size;

    /**
     * @brief Initializes a tree with a root node
     * 
     * @param root
     * @return Error
     */
    ErrorCode Init(TreeNode* root);

    /**
     * @brief Destroys the tree
     * 
     * @return Error
     */
    ErrorCode Destructor();

    /**
     * @brief Checks the tree's integrity
     * 
     * @return Error
     */
    ErrorCode Verify();
    
    /**
     * @brief Updates the tree by counting its nodes
     * 
     * @return Error
     */
    ErrorCode UpdateTree();

    /**
     * @brief Draws a tree into @ref IMG_FOLDER using Graphviz
     * 
     * @return Error
     */
    ErrorCode Dump();

    /**
     * @brief Saves the tree in pre-order
     * 
     * @param outPath - where to save
     * @return Error
     */
    ErrorCode Print(const char* outPath);

    /**
     * @brief Read the tree in pre-order
     * 
     * @param inPath - what to read
     * @return Error
     */
    ErrorCode Read(const char* inPath);
};

#endif
