#ifndef TORA_H_
#define TORA_H_
#define TORA_VERSION_STR "0.0.1"

#include <vector>
#include "value.h"

// Debug Macro
#ifdef DEBUG
#define DBG(fmt, ...) printf(fmt, __VA_ARGS__)
#define DBG2(fmt) printf(fmt)
#else
#define DBG(...)
#define DBG2(...)
#endif

#include "nodes.gen.h"

namespace tora {

struct TNode {
    node_type_t type;
    union {
        struct TNode *node;
        int int_value;
        const char*str_value;
        struct {
            struct TNode *left;
            struct TNode *right;
        } binary;
        struct {
            struct TNode *name;
            std::vector<struct TNode*> *args;
        } funcall;
        struct {
            struct TNode *name;
            std::vector<struct TNode*> *params;
            struct TNode *block;
        } funcdef;
        struct {
            struct TNode *cond;
            struct TNode *if_body;
            struct TNode *else_body;
        } if_stmt;
        std::vector<struct TNode*> *args;
        struct {
            struct TNode *lvalue;
            struct TNode *rvalue;
        } set_value;
        struct {
            struct TNode *container;
            struct TNode *index;
            struct TNode *rvalue;
        } set_item;
        struct {
            struct TNode *initialize;
            struct TNode *cond;
            struct TNode *postfix;
            struct TNode *block;
        } for_stmt;
    };
    const char *type_name_str() {
        return node_type2name[this->type];
    }
};

};

#include "op.h"

#endif // TORA_H_

