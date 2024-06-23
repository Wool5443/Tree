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

constexpr int OPERATION_PRIORITIES[] = {
#define DEF_FUNC(name, priority, ...) \
priority,

#include "Functions.hpp"

#undef DEF_FUNC
};

struct TreeElement
{
    TreeElementType type;
    int priority;
    union
    {
        Operation operation;
        String name;
        double number;
    };

constexpr TreeElement()
    : type(NUMBER_TYPE), priority(0), number(0) {}
constexpr TreeElement(Operation operation)
    : type(OPERATION_TYPE), priority(OPERATION_PRIORITIES[operation]),
      operation(operation) {}
constexpr TreeElement(String name)
    : type(NAME_TYPE), priority(0),
      name(name) {}
constexpr TreeElement(double number)
    : type(NUMBER_TYPE), priority(0),
      number(number) {}
};

typedef TreeElement TreeElement_t;

constexpr TreeElement_t TREE_POISON = {};
constexpr size_t MAX_TREE_SIZE      = 1000;
static const char* TREE_WORD_SEPARATOR = " ";
#define TREE_ELEMENT_SPECIFIER "%d"

#define SIZE_VERIFICATION

static const size_t BAD_ID = 0;
