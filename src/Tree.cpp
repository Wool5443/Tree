#include <stdlib.h>
#include "string.h"
#include "Tree.hpp"
#include "OneginFunctions.hpp"

static size_t CURRENT_ID = 0;
static size_t DUMP_ITERATION = 0;

static const size_t MAX_PATH_LENGTH = 128;
static const size_t MAX_COMMAND_LENGTH = 256;

static FILE* HTML_FILE = NULL;

struct _TreeNodeCountResult
{
    size_t value;
    ErrorCode error;
};

_TreeNodeCountResult _recCountNodes(TreeNode* node);

ErrorCode _recBuildCellTemplatesGraph(TreeNode* node, FILE* outGraphFile);

ErrorCode _recDrawGraph(TreeNode* node, FILE* outGraphFile);

ErrorCode _recPrint(TreeNode* node, FILE* outFile);

TreeNodeResult _recRead(Text* input, size_t* tokenNum);

TreeNodeResult _recReadOpenBracket(Text* input, char* tokenText, const char* openBracket, size_t* tokenNum);

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
    return TreeNode::New(this->value, this->left, this->right);
}

ErrorCode Tree::Init(TreeNode* root)
{
    MyAssertSoft(root, ERROR_NULLPTR);

    this->root = root;
    return this->UpdateTree();
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
    RETURN_ERROR(this->UpdateTree());

    if (this->size != oldSize)
        return ERROR_BAD_TREE;

    return EVERYTHING_FINE;
}

ErrorCode Tree::UpdateTree()
{
    _TreeNodeCountResult countResult = _recCountNodes(this->root);
    RETURN_ERROR(countResult.error);

    this->size = countResult.value;
    return EVERYTHING_FINE;
}

_TreeNodeCountResult _recCountNodes(TreeNode* node)
{
    if (!node)
        return { 0, EVERYTHING_FINE };

    size_t count = 1;

    _TreeNodeCountResult result = _recCountNodes(node->left);
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
                          "label = \"{Tree|Error: %s|Size: %zu|<root>Root}\"];", ERROR_CODE_NAMES[this->Verify()],
                                                                                 this->size);

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

    if (HTML_FILE)
        fprintf(HTML_FILE, "<img src = \"%s/Iteration%zu.png\"/>\n", IMG_FOLDER, DUMP_ITERATION);

    DUMP_ITERATION++;

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

    RETURN_ERROR(_recPrint(this->root, outFile));

    fclose(outFile);

    return EVERYTHING_FINE;
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

    Text input = CreateText(inPath, ' ');

    size_t tokenNum = 0;

    TreeNodeResult rootRes = _recRead(&input, &tokenNum);
    RETURN_ERROR(rootRes.error);

    return this->Init(rootRes.value);
}

TreeNodeResult _recRead(Text* input, size_t* tokenNum)
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

TreeNodeResult _recReadOpenBracket(Text* input, char* tokenText, const char* openBracket, size_t* tokenNum)
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

#undef FONT_SIZE
#undef FONT_NAME
#undef BACK_GROUND_COLOR
#undef NODE_COLOR
#undef NODE_FRAME_COLOR
#undef ROOT_COLOR
#undef FREE_HEAD_COLOR
