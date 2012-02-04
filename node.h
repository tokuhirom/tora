#ifndef TORA_NODE_H_
#define TORA_NODE_H_

#include <vector>
#include "nodes.gen.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

namespace tora {

class BinaryNode;
class MethodCallNode;
class NodeNode;
class IfNode;
class ForNode;
class FuncdefNode;
class FuncallNode;
class IntNode;
class StrNode;
class ArgsNode;

class Node {
public:
    node_type_t type;
    virtual ~Node() {
        // printf("%s llll \n", this->type_name_str());
    }
    const char *type_name_str() {
        return node_type2name[this->type];
    }
    virtual void dump(int indent) = 0;
    void dump();

    BinaryNode* binary() {
        return (BinaryNode*)this;
    }
    MethodCallNode* to_method_call_node() {
        return (MethodCallNode*)this;
    }
    NodeNode* to_node_node() {
        return (NodeNode*)this;
    }
    IfNode* to_if_node() {
        return (IfNode*)this;
    }
    ForNode *to_for_node() {
        return (ForNode*)this;
    }
    FuncdefNode *to_funcdef_node() {
        return (FuncdefNode*)this;
    }
    FuncallNode *to_funcall_node() {
        return (FuncallNode*)this;
    }
    IntNode *to_int_node() {
        return (IntNode*)this;
    }
    StrNode *to_str_node() {
        return (StrNode*)this;
    }
    ArgsNode *to_args_node() {
        return (ArgsNode*)this;
    }
};

class ArgsNode: public Node {
public:
    std::vector<Node*> *args;
    ArgsNode(node_type_t type_, std::vector<Node*> *args_) {
        this->type = type_;
        this->args = args_;
    }
    ~ArgsNode() {
        delete args;
    }
    void dump(int indent);
};

class StrNode : public Node {
public:
    const char*str_value;
    StrNode(node_type_t type_, const char *str) {
        this->type = type_;
        this->str_value = strdup(str);
    }
    ~StrNode() {
        free((void*)this->str_value);
    }
    void dump(int indent);
};

class VoidNode: public Node {
public:
    VoidNode(node_type_t t) {
        this->type = t;
    }
    ~VoidNode() { }
    void dump(int indent);
};

class IntNode: public Node {
public:
    int int_value;
    IntNode(node_type_t type_, int i): Node() {
        type = type_;
        int_value = i;
    }
    ~IntNode() {
    }
    void dump(int indent);
};

class FuncallNode: public Node {
public:
    Node *name;
    std::vector<Node*> *args;
    FuncallNode(Node *name_, std::vector<Node*>*args_) {
        this->type = NODE_FUNCALL;
        this->name = name_;
        this->args = args_;
    }
    ~FuncallNode() {
        delete name;
        delete args;
    }
    void dump(int indent);
};

class FuncdefNode: public Node {
public:
    Node *name;
    std::vector<Node*> *params;
    Node *block;
    FuncdefNode(Node *n, std::vector<Node*>*p, Node*b):Node() {
        this->type = NODE_FUNCDEF;
        name = n;
        params = p;
        block = b;
    }
    ~FuncdefNode() {
        delete name;
        delete params;
        delete block;
    }
    void dump(int indent);
};

class ForNode: public Node {
public:
    Node *initialize;
    Node *cond;
    Node *postfix;
    Node *block;
    ForNode(Node *i, Node*c, Node*p, Node*b): Node() {
        this->type = NODE_FOR;
        initialize = i;
        cond = c;
        postfix = p;
        block = b;
    }
    ~ForNode() {
        delete initialize;
        delete cond;
        delete postfix;
        delete block;
    }
    void dump(int indent);
};

class IfNode: public Node {
public:
    Node *cond;
    Node *if_body;
    Node *else_body;
    IfNode(node_type_t t, Node *c, Node*i, Node*e) : Node() {
        type = t;
        cond = c;
        if_body = i;
        else_body = e;
    }
    ~IfNode() {
        delete cond;
        delete if_body;
        delete else_body;
    }
    void dump(int indent);
};

class NodeNode: public Node {
public:
    Node *node;
    NodeNode(node_type_t t,Node *n) : Node() {
        type = t;
        node = n;
    }
    ~NodeNode() {
        delete node;
    }
    void dump(int indent);
};

class MethodCallNode: public Node {
public:
    Node *object;
    Node *method;
    std::vector<Node*> *args;

    MethodCallNode(Node*o, Node*m, std::vector<Node*>*a):Node() {
        type = NODE_METHOD_CALL;
        object = o;
        method = m;
        args = a;
    }
    ~MethodCallNode() {
        delete object;
        delete method;
        delete args;
    }
    void dump(int indent);
};

class BinaryNode: public Node {
public:
    Node *left;
    Node *right;
    BinaryNode(node_type_t t, Node* l, Node *r) {
        type = t;
        left = l;
        right = r;
        assert(l);
        assert(r);
    }
    ~BinaryNode() {
        delete left;
        delete right;
    }
    void dump(int indent);
};

};

#endif // TORA_NODE_H_

