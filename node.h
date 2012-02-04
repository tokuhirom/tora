#ifndef TORA_NODE_H_
#define TORA_NODE_H_

#include <vector>
#include "nodes.gen.h"

namespace tora {

struct BinaryNode;

struct Node {
    node_type_t type;
    union {
        struct Node *node;
        int int_value;
        const char*str_value;
        std::vector<struct Node*> *args;

        struct {
            struct Node *name;
            std::vector<struct Node*> *args;
        } funcall;
        struct {
            struct Node *name;
            std::vector<struct Node*> *params;
            struct Node *block;
        } funcdef;
        struct {
            struct Node *cond;
            struct Node *if_body;
            struct Node *else_body;
        } if_stmt;
        struct {
            struct Node *initialize;
            struct Node *cond;
            struct Node *postfix;
            struct Node *block;
        } for_stmt;
        struct {
            struct Node *object;
            struct Node *method;
            std::vector<struct Node*> *args;
        } method_call;
    };
    Node() { }
    Node(node_type_t t) {
        this->type = t;
    }
    const char *type_name_str() {
        return node_type2name[this->type];
    }
    virtual void dump(int indent);
    void dump();
    BinaryNode* binary() {
        return (BinaryNode*)this;
    }
};

struct BinaryNode: public Node {
    struct Node *left;
    struct Node *right;
    BinaryNode(node_type_t t, Node* l, Node *r) {
        type = t;
        left = l;
        right = r;
    }
    ~BinaryNode() {
        delete left;
        delete right;
    }
    void dump(int indent);
};

};

#endif // TORA_NODE_H_

