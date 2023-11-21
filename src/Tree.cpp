#include <stdlib.h>
#include "Tree.hpp"
#include "OneginFunctions.hpp"

static size_t CURRENT_ID = 0;
static size_t DUMP_ITERATION = 0;

static const size_t MAX_PATH_LENGTH = 128;
static const size_t MAX_COMMAND_LENGTH = 256;

TreeNodeCountResult _recCountNodes(TreeNode* node);

ErrorCode _recBuildCellTemplatesGraph(TreeNode* node, FILE* outGraphFile);

ErrorCode _recDrawGraph(TreeNode* node, FILE* outGraphFile);

ErrorCode _recPrint(TreeNode* node, FILE* outFile);

#define ERR_DUMP_RET(tree)                              \
do                                                      \
{                                                       \
    ErrorCode _verifyError = tree->Verify();            \
    if (_verifyError)                                   \
    {                                                   \
        tree->Dump();                                   \
        return _verifyError;                            \
    }                                                   \
} while (0);

#define ERR_DUMP_RET_RESULT(tree, poison)               \
do                                                      \
{                                                       \
    ErrorCode _verifyError = tree->Verify();            \
    if (_verifyError)                                   \
    {                                                   \
        tree->Dump();                                   \
        return { poison, _verifyError };                \
    }                                                   \
} while (0);

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
    ERR_DUMP_RET(this);
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
#define TREE_COLOR "\"#ff7be9\""
#define NODE_COLOR "\"#fae1f6\""
#define NODE_FRAME_COLOR "\"#000000\""
#define ROOT_COLOR "\"#c95b90\""
#define FREE_HEAD_COLOR "\"#b9e793\""

ErrorCode Tree::Dump()
{
    RETURN_ERROR(this->Verify());

    char outGraphPath[MAX_PATH_LENGTH] = "";
    sprintf(outGraphPath, "%s/Iteration%zu.dot", DOT_FOLDER, DUMP_ITERATION);

    FILE* outGraphFile = fopen(outGraphPath, "w");
    MyAssertSoft(outGraphFile, ERROR_BAD_FILE);

    fprintf(outGraphFile, 
    "digraph\n"
    "{\n"
    "rankdir = TB;\n"
    "node[shape = record, color = " NODE_FRAME_COLOR ", fontname = " FONT_NAME ", fontsize = " FONT_SIZE "];\n"
    "bgcolor = " BACK_GROUND_COLOR ";\n"
    );

    fprintf(outGraphFile, "TREE[rank = \"min\", style = \"filled\", fillcolor = " TREE_COLOR ", "
                          "label = \"{Tree|Size: %zu|<root>Root}\"];", this->size);

    fprintf(outGraphFile, "NODE_%zu[style = \"filled\", fillcolor = " NODE_COLOR ", ",
                           this->root->id);
    if (this->root->value == TREE_POISON)
        fprintf(outGraphFile, "label = \"{Value:\\nPOISON|{<left>Left|<right>Right}}\"];\n");
    else
        fprintf(outGraphFile, "label = \"{Value:\\n" TREE_ELEMENT_SPECIFIER "|{<left>Left|<right>Right}}\"];\n", this->root->value);

    RETURN_ERROR(_recBuildCellTemplatesGraph(this->root->left, outGraphFile));
    RETURN_ERROR(_recBuildCellTemplatesGraph(this->root->right, outGraphFile));

    RETURN_ERROR(_recDrawGraph(this->root, outGraphFile));
    fprintf(outGraphFile, "\n");
    fprintf(outGraphFile, "TREE:root->NODE_%zu\n", this->root->id);

    fprintf(outGraphFile, "}\n");
    fclose(outGraphFile);

    char command[MAX_COMMAND_LENGTH] = "";
    sprintf(command, "dot %s -T png -o %s/Iteration%zu.png", outGraphPath, IMG_FOLDER, DUMP_ITERATION);
    system(command);

    return EVERYTHING_FINE;
}

ErrorCode _recBuildCellTemplatesGraph(TreeNode* node, FILE* outGraphFile)
{
    if (!node)
        return EVERYTHING_FINE;

    fprintf(outGraphFile, "NODE_%zu[style = \"filled\", fillcolor = " NODE_COLOR ", ", node->id);
    if (node->value == TREE_POISON)
        fprintf(outGraphFile, "label = \"{Value:\\nPOISON|{<left>Left|<right>Right}}\"];\n");
    else
        fprintf(outGraphFile, "label = \"{Value:\\n" TREE_ELEMENT_SPECIFIER "|{<left>Left|<right>Right}}\"];\n", node->value);
    
    RETURN_ERROR(_recBuildCellTemplatesGraph(node->left, outGraphFile));
    return _recBuildCellTemplatesGraph(node->right, outGraphFile);
}

ErrorCode _recDrawGraph(TreeNode* node, FILE* outGraphFile)
{
    if (!node)
        return EVERYTHING_FINE;

    if (node->left)
        fprintf(outGraphFile, "NODE_%zu:left->NODE_%zu;\n", node->id, node->left->id);
    if (node->right)
        fprintf(outGraphFile, "NODE_%zu:right->NODE_%zu;\n", node->id, node->right->id);

    RETURN_ERROR(_recDrawGraph(node->left, outGraphFile));
    return _recDrawGraph(node->right, outGraphFile);
}

ErrorCode Tree::Print(const char* outPath)
{
    MyAssertSoft(outPath, ERROR_NULLPTR);
    ERR_DUMP_RET(this);

    FILE* outFile = fopen(outPath, "w");
    MyAssertSoft(outFile, ERROR_BAD_FILE);

    return _recPrint(this->root, outFile);
}

ErrorCode _recPrint(TreeNode* node, FILE* outFile)
{
    if (!node)
    {
        fprintf(outFile, "nil ");
        return EVERYTHING_FINE;
    }

    fprintf(outFile, "( " TREE_ELEMENT_SPECIFIER " ", node->value);
    RETURN_ERROR(_recPrint(node->left, outFile));
    RETURN_ERROR(_recPrint(node->right, outFile));
    fprintf(outFile, ") ");

    return EVERYTHING_FINE;
}

ErrorCode Tree::Read(const char* inPath)
{
    MyAssertSoft(inPath, ERROR_NULLPTR);
    ERR_DUMP_RET(this);

    Text input = CreateText(inPath, ' ');
}

#undef FONT_SIZE
#undef FONT_NAME
#undef BACK_GROUND_COLOR
#undef NODE_COLOR
#undef NODE_FRAME_COLOR
#undef ROOT_COLOR
#undef FREE_HEAD_COLOR
