#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "Tree.hpp"
#include "MinMax.hpp"

static const size_t MAX_PATH_LENGTH = 128;
static const size_t MAX_COMMAND_LENGTH = 256;

static FILE*        HTML_FILE  = NULL;
static const char*  LOG_FOLDER = nullptr;

static TreeNodeResult _recCopy(TreeNode* node);

#ifndef NDEBUG
static Error _recUpdateParentNodeCount(TreeNode* node, ssize_t change);
#endif

static TreeNodeCountResult _recCountNodes(TreeNode* node);

#ifndef NDEBUG
Error _recRecalcNodes(TreeNode* node);
#endif

static Error _recBuildCellTemplatesGraph(TreeNode* node, FILE* outGraphFile,
                                             size_t curDepth, const size_t maxDepth);

static Error _recDrawGraph(TreeNode* node, FILE* outGraphFile, size_t curDepth, const size_t maxDepth);

static Error _recPrint(TreeNode* node, FILE* outFile);

static TreeNodeResult _recRead(SplitString* split, size_t* wordNum);

static TreeNodeResult _recReadOpenBracket(SplitString* split, size_t* wordNum);

#define ERR_DUMP_RET(tree)                              \
do                                                      \
{                                                       \
    Error _verifyError = tree->Verify();                \
    if (_verifyError)                                   \
    {                                                   \
        tree->Dump();                                   \
        return _verifyError;                            \
    }                                                   \
} while (0);

#define ERR_DUMP_RET_RESULT(tree, poison)               \
do                                                      \
{                                                       \
    Error _verifyError = tree->Verify();                \
    if (_verifyError)                                   \
    {                                                   \
        tree->Dump();                                   \
        return { poison, _verifyError };                \
    }                                                   \
} while (0);

TreeNodeResult TreeNode::New(TreeElement_t value)
{
    return TreeNode::New(value, nullptr, nullptr);
}

TreeNodeResult TreeNode::New(TreeElement_t value, TreeNode* left, TreeNode* right)
{
    static size_t CURRENT_ID = 1;

    TreeNode* node = (TreeNode*)calloc(1, sizeof(TreeNode));
    if (!node)
        return { NULL, CREATE_ERROR(ERROR_NO_MEMORY) };

    node->value = value;

    #ifndef NDEBUG
    node->nodeCount = 1;
    #endif

    if (left)
    {
        left->parent = node;
        #ifndef NDEBUG
        node->nodeCount += left->nodeCount;
        #endif
    }
    node->left = left;

    if (right)
    {
        right->parent = node;
        #ifndef NDEBUG
        node->nodeCount += right->nodeCount;
        #endif
    }
    node->right  = right;
    node->parent = nullptr;
    node->id     = CURRENT_ID++;

    return { node, Error() };
}

Error TreeNode::Delete()
{
    if (this->id == BAD_ID)
        return CREATE_ERROR(ERROR_TREE_LOOP);

    this->id = BAD_ID;

    if (this->parent)
    {
        if (this->parent->left == this)
        {
            if (this->parent->right == this)
                return CREATE_ERROR(ERROR_TREE_LOOP);
            this->parent->left = nullptr;
        }
        else if (this->parent->right == this)
            this->parent->right = nullptr;
        else
            return CREATE_ERROR(ERROR_TREE_LOOP);

        #ifndef NDEBUG
        _recUpdateParentNodeCount(this->parent, -(ssize_t)this->nodeCount);
        #endif
    }

    if (this->left)
    {
        if (this->left->parent != this)
            return CREATE_ERROR(ERROR_TREE_LOOP);
        RETURN_ERROR(this->left->Delete());
    }
    if (this->right)
    {
        if (this->right->parent != this)
            return CREATE_ERROR(ERROR_TREE_LOOP);
        RETURN_ERROR(this->right->Delete());
    }

    this->value  = TREE_POISON;
    this->left   = nullptr;
    this->right  = nullptr;
    this->parent = nullptr;

    #ifndef NDEBUG
    this->nodeCount = SIZET_POISON;
    #endif

    free(this);

    return Error();
}

TreeNodeResult TreeNode::Copy()
{
    if (this->left && this->left->parent != this)
        return { nullptr, CREATE_ERROR(ERROR_TREE_LOOP) };
    if (this->right && this->right->parent != this)
        return { nullptr, CREATE_ERROR(ERROR_TREE_LOOP) };

    return _recCopy(this);
}

Error TreeNode::SetLeft(TreeNode* left)
{
    SoftAssert(left, ERROR_NULLPTR);

    #ifndef NDEBUG
    this->nodeCount += left->nodeCount;
    if (this->left)
        this->nodeCount -= this->left->nodeCount;
    #endif
    this->left = left;
    left->parent = this;

    #ifndef NDEBUG
    if (this->parent)
        return _recUpdateParentNodeCount(this->parent, left->nodeCount);
    #endif

    return Error();
}

Error TreeNode::SetRight(TreeNode* right)
{
    SoftAssert(right, ERROR_NULLPTR);

    #ifndef NDEBUG
    this->nodeCount += right->nodeCount;
    if (this->right)
        this->nodeCount -= this->right->nodeCount;
    #endif
    this->right = right;
    right->parent = this;

    #ifndef NDEBUG
    if (this->parent)
        return _recUpdateParentNodeCount(this->parent, right->nodeCount);
    #endif

    return Error();
}

static TreeNodeResult _recCopy(TreeNode* node)
{
    SoftAssertResult(node, nullptr, ERROR_NULLPTR);
    if (node->id == BAD_ID)
        return { nullptr, CREATE_ERROR(ERROR_TREE_LOOP) };
    
    size_t oldId = node->id;
    node->id = BAD_ID;

    TreeNodeResult leftChild = { nullptr, Error() };
    if (node->left)
    {
        if (node->left->parent != node)
            return { nullptr, CREATE_ERROR(ERROR_TREE_LOOP) };
        leftChild = _recCopy(node->left);
    }
    RETURN_RESULT(leftChild);

    TreeNodeResult rightChild = { nullptr, Error() };

    if (node->right)
    {
        if (node->right->parent != node)
        {
            leftChild.value->Delete();
            return { nullptr, CREATE_ERROR(ERROR_TREE_LOOP) };
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

#ifndef NDEBUG
static Error _recUpdateParentNodeCount(TreeNode* node, ssize_t change)
{
    SoftAssert(node, ERROR_NULLPTR);
    if (node->id == BAD_ID)
        return CREATE_ERROR(ERROR_TREE_LOOP);

    size_t oldId = node->id;
    node->id = BAD_ID;

    node->nodeCount += change;

    if (node->parent)
    {
        if (node->parent->left != node && node->parent->right != node)
            return CREATE_ERROR(ERROR_TREE_LOOP);
        RETURN_ERROR(_recUpdateParentNodeCount(node->parent, change));
    }

    node->id = oldId;

    return Error();
}
#endif

Error Tree::Init(TreeNode* root)
{
    SoftAssert(root, ERROR_NULLPTR);

    this->root = root;
    #ifndef NDEBUG
    this->size = &root->nodeCount;
    #endif

    return Error();
}

Error Tree::Init()
{
    TreeNodeResult rootRes = TreeNode::New(TREE_POISON, nullptr, nullptr);
    RETURN_ERROR(rootRes.error);

    this->root = rootRes.value;
    #ifndef NDEBUG
    this->size = &rootRes.value->nodeCount;
    #endif

    return Error();
}

Error Tree::Destructor()
{
    ERR_DUMP_RET(this);

    RETURN_ERROR(this->root->Delete());

    this->root = nullptr;
    #ifndef NDEBUG
    this->size = nullptr;
    #endif
    
    return Error();
}

Error Tree::Verify()
{
    if (!this->root)
        return CREATE_ERROR(ERROR_NO_ROOT);

    if (this->root->parent)
        return CREATE_ERROR(ERROR_TREE_LOOP);

    #ifndef NDEBUG
    if (*this->size > MAX_TREE_SIZE)
        return CREATE_ERROR(ERROR_BAD_SIZE);
    
    TreeNodeCountResult sizeRes = _recCountNodes(this->root);
    RETURN_ERROR(sizeRes.error);

    if (sizeRes.value != *this->size)
        return CREATE_ERROR(ERROR_BAD_TREE);
    #endif

    return Error();
}

TreeNodeCountResult Tree::CountNodes()
{
    ERR_DUMP_RET_RESULT(this, SIZET_POISON);

    return _recCountNodes(this->root);
}

static TreeNodeCountResult _recCountNodes(TreeNode* node)
{
    SoftAssertResult(node, SIZET_POISON, ERROR_NULLPTR);
    if (node->id == BAD_ID)
        return { SIZET_POISON, CREATE_ERROR(ERROR_TREE_LOOP) };

    size_t oldId = node->id;
    node->id = BAD_ID;

    size_t count = 1;

    TreeNodeCountResult countResult = {};

    if (node->left)
    {
        if (node->left->parent != node)
            return { SIZET_POISON, CREATE_ERROR(ERROR_TREE_LOOP) };
        countResult = _recCountNodes(node->left);
        RETURN_RESULT(countResult);
    }

    count += countResult.value;

    if (node->right)
    {
        if (node->right->parent != node)
            return { SIZET_POISON, CREATE_ERROR(ERROR_TREE_LOOP) };
        countResult = _recCountNodes(node->right);
        RETURN_RESULT(countResult);
    }

    count += countResult.value;

    node->id = oldId;

    return { count, Error() };
}

#ifndef NDEBUG
Error Tree::RecalculateNodes()
{
    ERR_DUMP_RET(this);

    return _recRecalcNodes(this->root);
}

Error _recRecalcNodes(TreeNode* node)
{
    SoftAssert(node, ERROR_NULLPTR);

    if (node->id == BAD_ID)
        return CREATE_ERROR(ERROR_TREE_LOOP);

    size_t oldId = node->id;
    node->id = BAD_ID;

    node->nodeCount = 1;

    if (!node->left && !node->right)
        return Error();
    
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

    return Error();
}
#endif

#define FONT_SIZE "10"
#define FONT_NAME "\"Fira Code Bold\""
#define BACK_GROUND_COLOR "\"#de97d4\""
#define TREE_COLOR "\"#ff7be9\""
#define NODE_COLOR "\"#fae1f6\""
#define NODE_FRAME_COLOR "\"#000000\""
#define ROOT_COLOR "\"#c95b90\""
#define FREE_HEAD_COLOR "\"#b9e793\""

Error Tree::Dump()
{
    static size_t DUMP_ITERATION = 0;

    SoftAssert(this->root, ERROR_NO_ROOT);

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
    sprintf(outGraphPath, "%s/dot/Iteration%zu.dot", LOG_FOLDER, DUMP_ITERATION);

    FILE* outGraphFile = fopen(outGraphPath, "w");
    SoftAssert(outGraphFile, ERROR_BAD_FILE);

    fprintf(outGraphFile, 
    "digraph\n"
    "{\n"
    "rankdir = TB;\n"
    "node[shape = record, color = " NODE_FRAME_COLOR ", fontname = " FONT_NAME ", fontsize = " FONT_SIZE "];\n"
    "bgcolor = " BACK_GROUND_COLOR ";\n"
    );

    fprintf(outGraphFile, "TREE[rank = \"min\", style = \"filled\", fillcolor = " TREE_COLOR ", "
                          "label = \"{Tree|Error: %s|"
                          #ifndef NDEBUG
                          "Size: %zu|"
                          #endif
                          "<root>Root}\"];",
                          this->Verify().GetErrorName()
                          #ifndef NDEBUG
                          , *this->size
                          #endif
                          );

    fprintf(outGraphFile, "\nNODE_%p[style = \"filled\", fillcolor = " NODE_COLOR ", ",
                           this->root);
    if (this->root->value == TREE_POISON)
        fprintf(outGraphFile, "label = \"{Value:\\nPOISON|{<left>Left|<right>Right}}\"];\n");
    else
        fprintf(outGraphFile,
        "label = \"{Value:\\n" TREE_ELEMENT_SPECIFIER "|"
        "{<left>Left|<right>Right}}\"];\n", this->root->value);

    size_t MAX_DEPTH = MAX_TREE_SIZE;
    #ifndef NDEBUG
    MAX_DEPTH = min(*this->size, MAX_TREE_SIZE);
    #endif

    if (this->root->left)
        RETURN_ERROR(_recBuildCellTemplatesGraph(this->root->left,  outGraphFile, 0, MAX_DEPTH));
    if (this->root->right)
        RETURN_ERROR(_recBuildCellTemplatesGraph(this->root->right, outGraphFile, 0, MAX_DEPTH));

    RETURN_ERROR(_recDrawGraph(this->root, outGraphFile, 0, MAX_DEPTH));
    fprintf(outGraphFile, "\n");
    fprintf(outGraphFile, "TREE:root->NODE_%p\n", this->root);

    fprintf(outGraphFile, "}\n");
    fclose(outGraphFile);

    char command[MAX_COMMAND_LENGTH] = "";
    sprintf(command, "dot %s -T png -o %s/img/Iteration%zu.png", outGraphPath, LOG_FOLDER, DUMP_ITERATION);
    system(command);

    if (HTML_FILE)
        fprintf(HTML_FILE, "<img src = \"%s/img/Iteration%zu.png\"/>\n", LOG_FOLDER, DUMP_ITERATION);

    DUMP_ITERATION++;

    return Error();
}

static Error _recBuildCellTemplatesGraph(TreeNode* node, FILE* outGraphFile,
                                             size_t curDepth, const size_t maxDepth)
{
    SoftAssert(node, ERROR_NULLPTR);

    size_t nodeId = node->id;
    
    if (curDepth > maxDepth)
        return Error();

    fprintf(outGraphFile, "NODE_%p[style = \"filled\", fillcolor = " NODE_COLOR ", ", node);
    fprintf(outGraphFile, "label = \"{Value:\\n");
    if (node->value == TREE_POISON)
        fprintf(outGraphFile, "POISON");
    else
        fprintf(outGraphFile, TREE_ELEMENT_SPECIFIER, node->value);
    fprintf(outGraphFile, "|id:\\n");

    if (node->id == BAD_ID)
        fprintf(outGraphFile, "BAD_ID");
    else
        fprintf(outGraphFile, "%zu", node->id);

    #ifndef NDEBUG
    fprintf(outGraphFile, "|node count:\\n%zu", node->nodeCount);
    #endif
    fprintf(outGraphFile, "|{<left>left|<right>right}}\"];\n");
    
    if (node->left)
        RETURN_ERROR(_recBuildCellTemplatesGraph(node->left,  outGraphFile, curDepth + 1, maxDepth));
    if (node->right)
        RETURN_ERROR(_recBuildCellTemplatesGraph(node->right, outGraphFile, curDepth + 1, maxDepth));

    return Error();
}

#undef FONT_SIZE
#undef FONT_NAME
#undef BACK_GROUND_COLOR
#undef NODE_COLOR
#undef NODE_FRAME_COLOR
#undef ROOT_COLOR
#undef FREE_HEAD_COLOR

static Error _recDrawGraph(TreeNode* node, FILE* outGraphFile, size_t curDepth, const size_t maxDepth)
{
    SoftAssert(node, ERROR_NULLPTR);
    if (curDepth > maxDepth)
        return Error();

    if (node->left)
    {
        fprintf(outGraphFile, "NODE_%p:left->NODE_%p;\n", node, node->left);
        RETURN_ERROR(_recDrawGraph(node->left, outGraphFile, curDepth + 1, maxDepth));
    }
    if (node->right)
    {
        fprintf(outGraphFile, "NODE_%p:right->NODE_%p;\n", node, node->right);
        RETURN_ERROR(_recDrawGraph(node->right, outGraphFile, curDepth + 1, maxDepth));
    }
    return Error();
}

Error Tree::Print(const char* outPath)
{
    SoftAssert(outPath, ERROR_NULLPTR);
    ERR_DUMP_RET(this);

    FILE* outFile = fopen(outPath, "wb");
    SoftAssert(outFile, ERROR_BAD_FILE);

    RETURN_ERROR(_recPrint(this->root, outFile));

    fclose(outFile);

    return Error();
}

static Error _recPrint(TreeNode* node, FILE* outFile)
{
    if (!node)
    {
        fprintf(outFile, "nil%s", TREE_WORD_SEPARATOR);
        return Error();
    }

    fprintf(outFile, "(%s" TREE_ELEMENT_SPECIFIER "%s", TREE_WORD_SEPARATOR, node->value, TREE_WORD_SEPARATOR);
    RETURN_ERROR(_recPrint(node->left, outFile));
    RETURN_ERROR(_recPrint(node->right, outFile));
    fprintf(outFile, ")%s", TREE_WORD_SEPARATOR);

    return Error();
}

Error Tree::Read(const char* readPath)
{
    SoftAssert(readPath, ERROR_NULLPTR);

    FILE* readFile = fopen(readPath, "rb");
    if (!readFile) return CREATE_ERROR(ERROR_BAD_FILE);

    size_t fileSize = GetFileSize(readPath);
    char*  buffer   = (char*)calloc(fileSize + 1, 1);
    if (!buffer)
    {
        fclose(readFile);
        return CREATE_ERROR(ERROR_NO_MEMORY);
    }

    if (fread(buffer, 1, fileSize, readFile) != fileSize)
    {
        fclose(readFile);
        free(buffer);
        return CREATE_ERROR(ERROR_BAD_FILE);
    }

    String string = {};
    string.Create(buffer, fileSize);

    SplitStringResult splitRes = string.Split(TREE_WORD_SEPARATOR);
    RETURN_ERROR(splitRes.error);

    size_t wordNum = 0;

    TreeNodeResult rootRes = _recRead(&splitRes.value, &wordNum);

    RETURN_ERROR(rootRes.error);

    splitRes.value.Destructor();

    return this->Init(rootRes.value);
}

static TreeNodeResult _recRead(SplitString* split, size_t* wordsNum)
{
    SoftAssertResult(split, nullptr, ERROR_NULLPTR);

    String* currentWord = &split->words[(*wordsNum)++];

    char* openBracket = strchr(currentWord->buf, '(');
    if (openBracket)
        return _recReadOpenBracket(split, wordsNum);

    const char* nil = strstr(currentWord->buf, "nil");
    if (nil)
        return { nullptr, Error() };
    return { nullptr, CREATE_ERROR(ERROR_SYNTAX) };
}

static TreeNodeResult _recReadOpenBracket(SplitString* split, size_t* wordNum)
{
    String openBracketString = {};

    String* word = &split->words[(*wordNum)++];

    TreeElement_t value = TREE_POISON;
    int readChars = 0;

    if (sscanf(word->buf, TREE_ELEMENT_SPECIFIER "%n", &value, &readChars) != 1)
        return { nullptr, CREATE_ERROR(ERROR_SYNTAX) };
    
    TreeNodeResult leftRes = _recRead(split, wordNum);
    RETURN_RESULT(leftRes);

    TreeNodeResult rightRes = _recRead(split, wordNum);
    RETURN_RESULT(rightRes);

    TreeNodeResult nodeRes = TreeNode::New(value, leftRes.value, rightRes.value);
    RETURN_RESULT(nodeRes);

    word = &split->words[(*wordNum)++];

    const char* closeBracket = strchr(word->buf, ')');
    if (!closeBracket)
        return { nullptr, CREATE_ERROR(ERROR_SYNTAX) };
    
    return nodeRes;
}

Error Tree::StartLogging(const char* logFolder)
{
    SoftAssert(logFolder, ERROR_NULLPTR);

    LOG_FOLDER = logFolder;

    char HTML_FILE_PATH[MAX_PATH_LENGTH];
    sprintf(HTML_FILE_PATH, "%s/log.html", logFolder);

    HTML_FILE = fopen(HTML_FILE_PATH, "w");
    SoftAssert(HTML_FILE, ERROR_BAD_FILE);

    fprintf(HTML_FILE, 
        "<style>\n"
        ".content {\n"
        "max-width: 500px;\n"
        "margin: auto;\n"
        "}\n"
        "</style>,\n"
        "<body>\n"
        "<div class=\"content\">");

    return Error();
}

Error Tree::EndLogging()
{
    if (HTML_FILE)
    {
        fprintf(HTML_FILE, "</div>\n</body>\n");
        fclose(HTML_FILE);
    }
    HTML_FILE = NULL;

    return Error();
}
