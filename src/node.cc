#include "tora.h"
#include "node.h"
#include <vector>

using namespace tora;

static void print_indent(int indent) {
    for (int i=0; i<indent*2; i++) {
        printf(" ");
    }
}

void tora::Node::dump(int indent) {
    print_indent(indent);
    printf("[%s] %s(%zd)\n", this->type_name_str(), node_type2name[this->type], this->list->size());
    for (size_t i=0; i<this->list->size(); i++) {
        this->list->at(i)->dump(indent+1);
    }
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
    printf("[Str]%s(%s)\n", this->type_name_str(), this->str_value.c_str());
}

void tora::RegexpNode::dump(int indent) {
    print_indent(indent);
    printf("[Regexp] %s\n", this->regexp_value.c_str());
}

void tora::Node::dump() {
    this->dump(0);
}
