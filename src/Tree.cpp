#include <stdlib.h>
#include "Tree.hpp"

static size_t CURRENT_ID = 0;
static size_t DUMP_ITERATION = 0;

static const size_t MAX_PATH_LENGTH = 256;

TreeNodeCountResult _recCountNodes(TreeNode* node);

ErrorCode _recBuildCellTemplatesGraph(TreeNode* node, FILE* outGraphFile);

ErrorCode _recDrawGraph(TreeNode* node, FILE* outGraphFile);

TreeNodeResult TreeNode::NewNode(TreeElement_t value, TreeNode* left, TreeNode* right)
{
    TreeNode* node = (TreeNode*)calloc(1, sizeof(TreeNode));
    if (!node)
        return { NULL, ERROR_NO_MEMORY };

    node->value = value;
    node->left  = left;
    node->right = right;
    node->id    = CURRENT_ID++;

    return { node, EVERYTHING_FINE };
}

ErrorCode TreeNode::DeleteNode()
{
    if (this->left)
        RETURN_ERROR(this->left->DeleteNode());
    if (this->right)
        RETURN_ERROR(this->right->DeleteNode());

    this->value = TREE_POISON;
    this->left = nullptr;
    this->right = nullptr;

    free(this);

    return EVERYTHING_FINE;
}

TreeNodeResult TreeNode::Copy()
{
    return TreeNode::NewNode(this->value, this->left, this->right);
}

ErrorCode Tree::Init()
{
    TreeNodeResult rootRes = TreeNode::NewNode(TREE_POISON, nullptr, nullptr);
    RETURN_ERROR(rootRes.error);

    TreeNode* root = rootRes.value;

    this->root = root;
    this->size = 1;

    return EVERYTHING_FINE;
}

ErrorCode Tree::Destructor()
{
    return this->root->DeleteNode();
}

ErrorCode Tree::Verify()
{
    if (!this->root)
        return ERROR_NO_ROOT;

    size_t oldSize = this->size;
    TreeNodeCountResult countResult = this->CountNodes();
    RETURN_ERROR(countResult.error);

    if (countResult.value != oldSize)
        return ERROR_BAD_TREE;

    return EVERYTHING_FINE;
}

TreeNodeCountResult Tree::CountNodes()
{
    TreeNodeCountResult countResult = _recCountNodes(this->root);
    RETURN_ERROR_RESULT(countResult, SIZET_POISON);

    this->size = countResult.value;
    return countResult;
}

TreeNodeCountResult _recCountNodes(TreeNode* node)
{
    if (!node)
        return { 0, EVERYTHING_FINE };

    size_t count = 1;

    TreeNodeCountResult result = _recCountNodes(node->left);
    RETURN_ERROR_RESULT(result, SIZET_POISON);

    count += result.value;

    result = _recCountNodes(node->right);
    RETURN_ERROR_RESULT(result, SIZET_POISON);

    count += result.value;

    return { count, EVERYTHING_FINE };
}

#define FONT_SIZE "10"
#define FONT_NAME "\"Fira Code Bold\""
#define BACK_GROUND_COLOR "\"#de97d4\""
#define NODE_COLOR "\"#fae1f6\""
#define NODE_FRAME_COLOR "\"#000000\""
#define ROOT_COLOR "\"#c95b90\""
#define FREE_HEAD_COLOR "\"#b9e793\""

ErrorCode Tree::Dump()
{
    char outGraphPath[] = "";
    sprintf(outGraphPath, "%s/Iteration %zu", DOT_FOLDER, DUMP_ITERATION);

    FILE* outGraphFile = fopen(outGraphPath, "w");
    MyAssertSoft(outGraphFile, ERROR_BAD_FILE);

    fprintf(outGraphFile, 
    "digraph\n"
    "{\n"
    "rankdir = TB;\n"
    "node[shape = record, color = " NODE_FRAME_COLOR ", fontname = " FONT_NAME ", fontsize = " FONT_SIZE "];\n"
    "bgcolor = " BACK_GROUND_COLOR ";\n"
    );

    fprintf(outGraphFile, "NODE_%zu[rank = \"min\", style = \"filled\", fillcolor = " NODE_COLOR ", ",
                           this->root->id);
    if (this->root->value == TREE_POISON)
        fprintf(outGraphFile, "label = \"{Value:\\nPOISON|{Left|Right}}\"];\n");
    else
        fprintf(outGraphFile, "label = \"{Value:\\n" TREE_ELEMENT_SPECIFIER "|{Left|Right}}\"];\n", this->root->value);

    RETURN_ERROR(_recBuildCellTemplatesGraph(this->root->left, outGraphFile));
    RETURN_ERROR(_recBuildCellTemplatesGraph(this->root->right, outGraphFile));

    RETURN_ERROR(_recDrawGraph(this->root, outGraphFile));

    fprintf(outGraphFile, "\n}\n");

    fclose(outGraphFile);

    return EVERYTHING_FINE;
}

ErrorCode _recBuildCellTemplatesGraph(TreeNode* node, FILE* outGraphFile)
{
    if (!node)
        return EVERYTHING_FINE;

    fprintf(outGraphFile, "NODE_%zu[style = \"filled\", fillcolor = " NODE_COLOR ", ", node->id);
    if (node->value == TREE_POISON)
        fprintf(outGraphFile, "label = \"{Value:\\nPOISON|{Left|Right}}\"];\n");
    else
        fprintf(outGraphFile, "label = \"{Value:\\n" TREE_ELEMENT_SPECIFIER "|{Left|Right}}\"];\n", node->value);
    
    RETURN_ERROR(_recBuildCellTemplatesGraph(node->left, outGraphFile));
    return _recBuildCellTemplatesGraph(node->right, outGraphFile);
}

ErrorCode _recDrawGraph(TreeNode* node, FILE* outGraphFile)
{
    if (!node)
        return EVERYTHING_FINE;

    if (node->left)
        fprintf(outGraphFile, "NODE_%zu->NODE_%zu;\n", node->id, node->left->id);
    if (node->right)
        fprintf(outGraphFile, "NODE_%zu->NODE_%zu;\n", node->id, node->right->id);

    RETURN_ERROR(_recDrawGraph(node->left, outGraphFile));
    return _recDrawGraph(node->right, outGraphFile);
}

#undef FONT_SIZE
#undef FONT_NAME
#undef BACK_GROUND_COLOR
#undef NODE_COLOR
#undef NODE_FRAME_COLOR
#undef ROOT_COLOR
#undef FREE_HEAD_COLOR
