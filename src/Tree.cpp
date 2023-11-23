#include <stdlib.h>
#include <string.h>
#include "Tree.hpp"
#include "OneginFunctions.hpp"
#include "MinMax.hpp"


static const size_t BAD_ID = 0;

static const size_t MAX_PATH_LENGTH = 128;
static const size_t MAX_COMMAND_LENGTH = 256;

static FILE* HTML_FILE = NULL;

static TreeNodeResult _recCopy(TreeNode* node);

static ErrorCode _recUpdateParentNodeCount(TreeNode* node, ssize_t change);

static TreeNodeCountResult _recCountNodes(TreeNode* node);

ErrorCode _recRecalcNodes(TreeNode* node);

static ErrorCode _recBuildCellTemplatesGraph(TreeNode* node, FILE* outGraphFile,
                                             size_t curDepth, const size_t maxDepth);

static ErrorCode _recDrawGraph(TreeNode* node, FILE* outGraphFile, size_t curDepth, const size_t maxDepth);

static ErrorCode _recPrint(TreeNode* node, FILE* outFile);

static TreeNodeResult _recRead(Text* input, size_t* tokenNum);

static TreeNodeResult _recReadOpenBracket(Text* input, char* tokenText, const char* openBracket,
                                          size_t* tokenNum);

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

TreeNodeResult TreeNode::New(TreeElement_t value, TreeNode* left, TreeNode* right)
{
    static size_t CURRENT_ID = 1;

    TreeNode* node = (TreeNode*)calloc(1, sizeof(TreeNode));
    if (!node)
        return { NULL, ERROR_NO_MEMORY };

    node->value = value;
    node->nodeCount = 1;

    if (left)
    {
        left->parent = node;
        node->nodeCount += left->nodeCount;
    }
    node->left = left;

    if (right)
    {
        right->parent = node;
        node->nodeCount += right->nodeCount;
    }
    node->right = right;

    node->parent = nullptr;

    node->id = CURRENT_ID++;

    return { node, EVERYTHING_FINE };
}

ErrorCode TreeNode::Delete()
{
    if (this->id == BAD_ID)
        return ERROR_TREE_LOOP;

    this->id = BAD_ID;

    if (this->left)
    {
        if (this->left->parent != this)
            return ERROR_TREE_LOOP;
        RETURN_ERROR(this->left->Delete());
    }
    if (this->right)
    {
        if (this->right->parent != this)
            return ERROR_TREE_LOOP;
        RETURN_ERROR(this->right->Delete());
    }

    if (this->parent)
    {
        if (this->parent->left == this)
        {
            if (this->parent->right == this)
                return ERROR_TREE_LOOP;
            this->parent->left = nullptr;
        }
        else if (this->parent->right == this)
            this->parent->right = nullptr;
        else
            return ERROR_TREE_LOOP;

        _recUpdateParentNodeCount(this->parent, -(ssize_t)this->nodeCount);
    }

    this->value  = TREE_POISON;
    this->left   = nullptr;
    this->right  = nullptr;
    this->parent = nullptr;

    this->nodeCount = 0;

    free(this);

    return EVERYTHING_FINE;
}

TreeNodeResult TreeNode::Copy()
{
    if (this->left && this->left->parent != this)
        return { nullptr, ERROR_TREE_LOOP };
    if (this->right && this->right->parent != this)
        return { nullptr, ERROR_TREE_LOOP };

    return _recCopy(this);
}

ErrorCode TreeNode::AddLeft(TreeNode* left)
{
    MyAssertSoft(left, ERROR_NULLPTR);

    this->left = left;
    this->nodeCount += left->nodeCount;
    left->parent = this;

    if (this->parent)
        return _recUpdateParentNodeCount(this->parent, left->nodeCount);

    return EVERYTHING_FINE;
}
ErrorCode TreeNode::AddRight(TreeNode* right)
{
    MyAssertSoft(right, ERROR_NULLPTR);

    this->right = right;
    this->nodeCount += right->nodeCount;
    right->parent = this;

    if (this->parent)
        return _recUpdateParentNodeCount(this->parent, right->nodeCount);

    return EVERYTHING_FINE;
}

static TreeNodeResult _recCopy(TreeNode* node)
{
    MyAssertSoftResult(node, nullptr, ERROR_NULLPTR);
    if (node->id == BAD_ID)
        return { nullptr, ERROR_TREE_LOOP };
    
    size_t oldId = node->id;
    node->id = BAD_ID;

    TreeNodeResult leftChild = { nullptr, EVERYTHING_FINE };
    if (node->left)
    {
        if (node->left->parent != node)
            return { nullptr, ERROR_TREE_LOOP };
        leftChild = _recCopy(node->left);
    }
    RETURN_ERROR_RESULT(leftChild, nullptr);

    TreeNodeResult rightChild = { nullptr, EVERYTHING_FINE };

    if (node->right)
    {
        if (node->right->parent != node)
        {
            leftChild.value->Delete();
            return { nullptr, ERROR_TREE_LOOP };
        }
        rightChild = _recCopy(node->right);
    }
    if (rightChild.error)
    {
        leftChild.value->Delete();
        return { nullptr, rightChild.error };
    }

    TreeNodeResult copy = TreeNode::New(node->value, leftChild.value, rightChild.value);

    if (copy.error)
    {
        leftChild.value->Delete();
        rightChild.value->Delete();

        return { nullptr, copy.error };
    }

    node->id = oldId;

    return copy;
}

static ErrorCode _recUpdateParentNodeCount(TreeNode* node, ssize_t change)
{
    MyAssertSoft(node, ERROR_NULLPTR);
    if (node->id == BAD_ID)
        return ERROR_TREE_LOOP;

    size_t oldId = node->id;
    node->id = BAD_ID;

    node->nodeCount += change;

    if (node->parent)
    {
        if (node->parent->left != node && node->parent->right != node)
            return ERROR_TREE_LOOP;
        RETURN_ERROR(_recUpdateParentNodeCount(node->parent, change));
    }

    node->id = oldId;

    return EVERYTHING_FINE;
}

ErrorCode Tree::Init(TreeNode* root)
{
    MyAssertSoft(root, ERROR_NULLPTR);

    this->root = root;
    this->size = &root->nodeCount;

    return EVERYTHING_FINE;
}

ErrorCode Tree::Destructor()
{
    ERR_DUMP_RET(this);

    return this->root->Delete();
}

ErrorCode Tree::Verify()
{
    if (!this->root)
        return ERROR_NO_ROOT;

    if (this->root->parent)
        return ERROR_TREE_LOOP;

    if (*this->size > MAX_TREE_SIZE)
        return ERROR_BAD_SIZE;
    
    #ifdef SIZE_VERIFICATION

    TreeNodeCountResult sizeRes = _recCountNodes(this->root);
    RETURN_ERROR(sizeRes.error);

    if (sizeRes.value != *this->size)
        return ERROR_BAD_TREE;

    #endif

    return EVERYTHING_FINE;
}

TreeNodeCountResult Tree::CountNodes()
{
    ERR_DUMP_RET_RESULT(this, SIZET_POISON);

    TreeNodeCountResult countResult = _recCountNodes(this->root);

    return countResult;
}

static TreeNodeCountResult _recCountNodes(TreeNode* node)
{
    MyAssertSoftResult(node, SIZET_POISON, ERROR_NULLPTR);
    if (node->id == BAD_ID)
        return { SIZET_POISON, ERROR_TREE_LOOP };

    size_t oldId = node->id;
    node->id = BAD_ID;

    size_t count = 1;

    TreeNodeCountResult countResult = {};

    if (node->left)
    {
        if (node->left->parent != node)
            return { SIZET_POISON, ERROR_TREE_LOOP };
        countResult = _recCountNodes(node->left);
        RETURN_ERROR_RESULT(countResult, SIZET_POISON);
    }

    count += countResult.value;

    if (node->right)
    {
        if (node->right->parent != node)
            return { SIZET_POISON, ERROR_TREE_LOOP };
        countResult = _recCountNodes(node->right);
        RETURN_ERROR_RESULT(countResult, SIZET_POISON);
    }

    count += countResult.value;

    node->id = oldId;

    return { count, EVERYTHING_FINE };
}

ErrorCode Tree::RecalculateNodes()
{
    ERR_DUMP_RET(this);

    return _recRecalcNodes(this->root);
}

ErrorCode _recRecalcNodes(TreeNode* node)
{
    MyAssertSoft(node, ERROR_NULLPTR);

    if (node->id == BAD_ID)
        return ERROR_TREE_LOOP;

    size_t oldId = node->id;
    node->id = BAD_ID;

    node->nodeCount = 1;

    if (!node->left && !node->right)
        return EVERYTHING_FINE;
    
    if (node->left)
    {
        RETURN_ERROR(_recRecalcNodes(node->left));
        node->nodeCount += node->left->nodeCount;
    }
    if (node->right)
    {
        RETURN_ERROR(_recRecalcNodes(node->right));
        node->nodeCount += node->right->nodeCount;
    }

    node->id = BAD_ID;

    return EVERYTHING_FINE;
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
    static size_t DUMP_ITERATION = 0;

    MyAssertSoft(this->root, ERROR_NO_ROOT);

    if (HTML_FILE)
        fprintf(HTML_FILE, 
        "<h1>Iteration %zu</h1>\n"
        "<style>\n"
        ".content {\n"
        "max-width: 500px;\n"
        "margin: auto;\n"
        "}\n"
        "</style>,\n",
        DUMP_ITERATION);

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
                          "label = \"{Tree|Error: %s|Size: %zu|<root>Root}\"];",
                          ERROR_CODE_NAMES[this->Verify()], *this->size);

    fprintf(outGraphFile, "NODE_%zu[style = \"filled\", fillcolor = " NODE_COLOR ", ",
                           this->root->id);
    if (this->root->value == TREE_POISON)
        fprintf(outGraphFile, "label = \"{Value:\\nPOISON|{<left>Left|<right>Right}}\"];\n");
    else
        fprintf(outGraphFile,
        "label = \"{Value:\\n" TREE_ELEMENT_SPECIFIER "|"
        "{<left>Left|<right>Right}}\"];\n", this->root->value);

    const size_t MAX_DEPTH = min(*this->size, MAX_TREE_SIZE);

    RETURN_ERROR(_recBuildCellTemplatesGraph(this->root->left,  outGraphFile, 0, MAX_DEPTH));
    RETURN_ERROR(_recBuildCellTemplatesGraph(this->root->right, outGraphFile, 0, MAX_DEPTH));

    RETURN_ERROR(_recDrawGraph(this->root, outGraphFile, 0, *this->size));
    fprintf(outGraphFile, "\n");
    fprintf(outGraphFile, "TREE:root->NODE_%zu\n", this->root->id);

    fprintf(outGraphFile, "}\n");
    fclose(outGraphFile);

    char command[MAX_COMMAND_LENGTH] = "";
    sprintf(command, "dot %s -T png -o %s/Iteration%zu.png", outGraphPath, IMG_FOLDER, DUMP_ITERATION);
    system(command);

    if (HTML_FILE)
        fprintf(HTML_FILE, "<img src = \"%s/Iteration%zu.png\"/>\n", IMG_FOLDER, DUMP_ITERATION);

    DUMP_ITERATION++;

    return EVERYTHING_FINE;
}

static ErrorCode _recBuildCellTemplatesGraph(TreeNode* node, FILE* outGraphFile,
                                             size_t curDepth, const size_t maxDepth)
{
    MyAssertSoft(node, ERROR_NULLPTR);

    if (curDepth > maxDepth)
        return EVERYTHING_FINE;

    fprintf(outGraphFile, "NODE_%zu[style = \"filled\", fillcolor = " NODE_COLOR ", ", node->id);
    if (node->value == TREE_POISON)
        fprintf(outGraphFile, "label = \"{Value:\\nPOISON|{<left>Left|<right>Right}}\"];\n");
    else
        fprintf(outGraphFile, "label = \"{Value:\\n" TREE_ELEMENT_SPECIFIER "|{<left>Left|<right>Right}}\"];\n", node->value);
    
    if (node->left)
        RETURN_ERROR(_recBuildCellTemplatesGraph(node->left,  outGraphFile, curDepth + 1, maxDepth));
    if (node->right)
        RETURN_ERROR(_recBuildCellTemplatesGraph(node->right, outGraphFile, curDepth + 1, maxDepth));

    return EVERYTHING_FINE;
}

#undef FONT_SIZE
#undef FONT_NAME
#undef BACK_GROUND_COLOR
#undef NODE_COLOR
#undef NODE_FRAME_COLOR
#undef ROOT_COLOR
#undef FREE_HEAD_COLOR

static ErrorCode _recDrawGraph(TreeNode* node, FILE* outGraphFile, size_t curDepth, const size_t maxDepth)
{
    MyAssertSoft(node, ERROR_NULLPTR);
    if (curDepth > maxDepth)
        return EVERYTHING_FINE;

    if (node->left)
    {
        fprintf(outGraphFile, "NODE_%zu:left->NODE_%zu;\n", node->id, node->left->id);
        RETURN_ERROR(_recDrawGraph(node->left, outGraphFile, curDepth + 1, maxDepth));
    }
    if (node->right)
    {
        fprintf(outGraphFile, "NODE_%zu:right->NODE_%zu;\n", node->id, node->right->id);
        RETURN_ERROR(_recDrawGraph(node->right, outGraphFile, curDepth + 1, maxDepth));
    }
    return EVERYTHING_FINE;
}

ErrorCode Tree::Print(const char* outPath)
{
    MyAssertSoft(outPath, ERROR_NULLPTR);
    ERR_DUMP_RET(this);

    FILE* outFile = fopen(outPath, "w");
    MyAssertSoft(outFile, ERROR_BAD_FILE);

    RETURN_ERROR(_recPrint(this->root, outFile));

    fclose(outFile);

    return EVERYTHING_FINE;
}

static ErrorCode _recPrint(TreeNode* node, FILE* outFile)
{
    if (!node)
    {
        fprintf(outFile, "nil%c", TOKEN_SEPARATOR);
        return EVERYTHING_FINE;
    }

    fprintf(outFile, "(%c" TREE_ELEMENT_SPECIFIER "%c", TOKEN_SEPARATOR, node->value, TOKEN_SEPARATOR);
    RETURN_ERROR(_recPrint(node->left, outFile));
    RETURN_ERROR(_recPrint(node->right, outFile));
    fprintf(outFile, ")%c", TOKEN_SEPARATOR);

    return EVERYTHING_FINE;
}

ErrorCode Tree::Read(const char* readPath)
{
    MyAssertSoft(readPath, ERROR_NULLPTR);

    Text input = CreateText(readPath, TOKEN_SEPARATOR);
    PrintTextTokens(&input, stdout, TOKEN_SEPARATOR);

    size_t tokenNum = 0;

    TreeNodeResult rootRes = _recRead(&input, &tokenNum);
    DestroyText(&input);

    RETURN_ERROR(rootRes.error);

    return this->Init(rootRes.value);
}

static TreeNodeResult _recRead(Text* input, size_t* tokenNum)
{
    MyAssertSoftResult(*tokenNum < input->numberOfTokens, NULL, ERROR_INDEX_OUT_OF_BOUNDS);

    const String* token = &input->tokens[(*tokenNum)++];
    char* tokenText = (char*)token->text;
    tokenText[token->length] = '\0';

    const char* openBracket = strchr(tokenText, '(');
    if (openBracket)
        return _recReadOpenBracket(input, tokenText, openBracket, tokenNum);

    const char* nil = strstr(tokenText, "nil");
    if (nil)
        return { nullptr, EVERYTHING_FINE };
    return { nullptr, ERROR_SYNTAX };
}

static TreeNodeResult _recReadOpenBracket(Text* input, char* tokenText, const char* openBracket, size_t* tokenNum)
{
    if (!StringIsEmptyChars(tokenText, '(') || !StringIsEmptyChars(openBracket + 1, '\0'))
        return { nullptr, ERROR_SYNTAX };

    const String* token = &input->tokens[(*tokenNum)++];
    tokenText = (char*)token->text;
    tokenText[token->length] = '\0';

    TreeElement_t value = TREE_POISON;
    int readChars = 0;

    if (sscanf(tokenText, TREE_ELEMENT_SPECIFIER "%n", &value, &readChars) != 1 ||
        !StringIsEmptyChars(tokenText + readChars, '\0'))
        return { nullptr, ERROR_SYNTAX };
    
    TreeNodeResult leftRes = _recRead(input, tokenNum);
    RETURN_ERROR_RESULT(leftRes, nullptr);

    TreeNodeResult rightRes = _recRead(input, tokenNum);
    RETURN_ERROR_RESULT(rightRes, nullptr);

    TreeNodeResult nodeRes = TreeNode::New(value, leftRes.value, rightRes.value);
    RETURN_ERROR_RESULT(nodeRes, nullptr);

    token = &input->tokens[(*tokenNum)++];
    tokenText = (char*)token->text;
    tokenText[token->length] = '\0';

    const char* closeBracket = strchr(tokenText, ')');
    if (!closeBracket)
        return { nullptr, ERROR_SYNTAX };
    
    return nodeRes;
}

ErrorCode Tree::StartHtmlLogging()
{
    HTML_FILE = fopen(HTML_FILE_PATH, "w");
    MyAssertSoft(HTML_FILE, ERROR_BAD_FILE);

    fprintf(HTML_FILE, 
        "<style>\n"
        ".content {\n"
        "max-width: 500px;\n"
        "margin: auto;\n"
        "}\n"
        "</style>,\n"
        "<body>\n"
        "<div class=\"content\">");

    return EVERYTHING_FINE;
}

ErrorCode Tree::EndHtmlLogging()
{
    if (HTML_FILE)
    {
        fprintf(HTML_FILE, "</div>\n</body>\n");
        fclose(HTML_FILE);
    }
    HTML_FILE = NULL;


    return EVERYTHING_FINE;
}
