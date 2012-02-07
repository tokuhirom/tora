#include "tora.h"
#include "node.h"
#include <vector>

using namespace tora;

static void print_indent(int indent) {
    for (int i=0; i<indent*2; i++) {
        printf(" ");
    }
}

static void dump_args(std::vector<SharedPtr<tora::Node>> *args, int indent) {
    for (size_t i=0; i<args->size(); i++) {
        args->at(i)->dump(indent+1);
    }
}

void tora::ArgsNode::dump(int indent) {
    print_indent(indent);
    printf("[Args]%s(%zd)\n", this->type_name_str(), this->args->size());
    dump_args(this->args, indent+1);
}

void tora::VoidNode::dump(int indent) {
    print_indent(indent);
    printf("[Void]%s\n", this->type_name_str());
}

void tora::FuncdefNode::dump(int indent) {
    print_indent(indent);
    printf("[Funcdef]%s\n", this->type_name_str());
    this->name->dump(indent+1);
    dump_args(params, indent+1);
    this->block->dump(indent+1);
}

void tora::ForNode::dump(int indent) {
    print_indent(indent);
    printf("[Funcdef]%s\n", this->type_name_str());
    this->initialize->dump(indent+1);
    this->cond->dump(indent+1);
    this->postfix->dump(indent+1);
    this->block->dump(indent+1);
}

void tora::FuncallNode::dump(int indent) {
    print_indent(indent);
    printf("[Binary]%s\n", this->type_name_str());
    dump_args(this->args, indent+1);
}

void tora::BinaryNode::dump(int indent) {
    print_indent(indent);
    printf("[Binary]%s\n", this->type_name_str());
    this->left->dump(indent+1);
    this->right->dump(indent+1);
}

void tora::MethodCallNode::dump(int indent) {
    print_indent(indent);
    printf("[MethodCall]%s\n", this->type_name_str());
    this->object->dump(indent+1);
    this->method->dump(indent+1);
    dump_args(this->args, indent);
}

void tora::IntNode::dump(int indent) {
    print_indent(indent);
    printf("[Int]%s(%d)\n", this->type_name_str(), this->int_value);
}

void tora::DoubleNode::dump(int indent) {
    print_indent(indent);
    printf("[Double]%s(%lf)\n", this->type_name_str(), this->double_value);
}

void tora::StrNode::dump(int indent) {
    print_indent(indent);
    printf("[Str]%s(%s)\n", this->type_name_str(), this->str_value);
}

void tora::NodeNode::dump(int indent) {
    print_indent(indent);
    printf("[Node]%s\n", this->type_name_str());
    this->node->dump(indent+1);
}

void tora::IfNode::dump(int indent) {
    print_indent(indent);
    printf("[If]%s\n", this->type_name_str());
    this->cond->dump(indent+1);
    this->if_body->dump(indent+1);
    this->else_body->dump(indent+1);
}

void tora::Node::dump() {
    this->dump(0);
}
