#pragma once

#include <math.h>

enum TreeElementType
{
    OPERATION_TYPE,
    NUMBER_TYPE,
    NAME_TYPE,
};

enum Operation
{
#define DEF_FUNC(name, ...) \
name,

#include "Functions.hpp"

#undef DEF_FUNC
};

#define DEF_FUNC(name, prior, ...) \
static const int name ## _PRIORITY = prior;

#include "Functions.hpp"

#undef DEF_FUNC

struct TreeElement
{
    TreeElementType type;
    int priority;
    union val
    {
        Operation operation;
        String name;
        double number;
    } value;
};

typedef TreeElement TreeElement_t;

constexpr TreeElement_t TREE_POISON = {};
constexpr size_t MAX_TREE_SIZE      = 1000;
static const char* TREE_WORD_SEPARATOR = " ";
#define TREE_ELEMENT_SPECIFIER "%d"

#define SIZE_VERIFICATION

static const size_t BAD_ID = 0;
