//! @file

#pragma once

#include "String.hpp"
#include "Utils.hpp"
#include "TreeSettings.hpp"

struct TreeNodeResult;
/** @struct TreeNode
 * @brief A binary tree node containing value and ptrs to children
 * 
 * @var TreeNode::value - TreeElemen_t value
 * @var TreeNode::left - TreeNode* left
 * @var TreeNode::right - TreeNode* right
 * @var TreeNode::parent - TreeNode* parent
 * @var TreeNode::id - size_t id - unique id of a node, used for dumping
 * @var TreeNode::nodeCount - number of all nodes going from the current one
*/
struct TreeNode
{
    TreeElement_t value;
    TreeNode*     left;
    TreeNode*     right;
    TreeNode*     parent;

    size_t id;

    #ifndef NDEBUG
    size_t nodeCount;
    #endif

    /**
     * @brief Returns a new node result
     * 
     * @param [in] value - value
     * @param [in] left - left child
     * @param [in] right - right child
     * @return TreeNodeResult - new node
     */
    static TreeNodeResult New(TreeElement_t value, TreeNode* left, TreeNode* right);
    static TreeNodeResult New(TreeElement_t value);

    /**
     * @brief Deletes a node
     * 
     * @return Error
     */
    Error Delete();

    /**
     * @brief Copies the node and returns the copy
     * 
     * @return TreeNodeResult the copy
     */
    TreeNodeResult Copy();

    /**
     * @brief Sets the left node.
     * 
     * @param [in] left - the left node.
     * @return Error
     */
    Error SetLeft(TreeNode* left);
    
    /**
     * @brief Sets the right node.
     * 
     * @param [in] right - the right node.
     * @return Error
     */
    Error SetRight(TreeNode* right);
};
struct TreeNodeResult
{
    TreeNode* value;
    Error     error;
};

/** @struct TreeNodeCountResult
 * @brief Used for counting nodes.
 * 
 * @var TreeNodeCountResult::value - how many nodes
 * @var TreeNodeCountResult::error
 */
struct TreeNodeCountResult
{
    size_t value;
    Error  error;
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

    #ifndef NDEBUG
    size_t* size;
    #endif

    /**
     * @brief Initializes a tree with a root node
     * 
     * @param [in] root
     * @return Error
     */
    Error Init(TreeNode* root);

    /**
     * @brief Initializes a tree with an empty root
     * 
     * @return Error 
     */
    Error Init();

    /**
     * @brief Destroys the tree
     * 
     * @return Error
     */
    Error Destructor();

    /**
     * @brief Checks the tree's integrity
     * 
     * @return Error
     */
    Error Verify();
    
    /**
     * @brief Counts nodes in the tree
     * 
     * @return Error
     */
    TreeNodeCountResult CountNodes();

    #ifndef NDEBUG
    /**
     * @brief Recalculates @ref TreeNode::nodeCount for every node in tree
     * 
     * @return Error
     */
    Error RecalculateNodes();
    #endif

    /**
     * @brief Draws a tree using Graphviz
     *
     * @param [in] logFolder - where to put logs
     *  
     * @return Error
     */
    Error Dump();

    static Error StartLogging(const char* logFolder);
    static Error EndLogging();

    /**
     * @brief Saves the tree in pre-order
     * 
     * @param [in] outPath - where to save
     * @return Error
     */
    Error Print(const char* outPath);

    /**
     * @brief Read the tree in pre-order
     * 
     * @attention Make sure to delete the tree before reading into it
     * 
     * @param [in] readPath - what to read
     * @return Error
     */
    Error Read(const char* readPath);
};
