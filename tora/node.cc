#include "tora.h"
#include "node.h"
#include "util.h"
#include <vector>

using namespace tora;

void tora::Node::dump(int indent) {
    print_indent(indent);
    printf("[%s] %s(size: %zd)\n", this->type_name_str(), node_type2name[this->type], this->list->size());
    for (size_t i=0; i<this->list->size(); i++) {
        auto n = this->list->at(i);
        if (n.get()) {
            n->dump(indent+1);
        } else {
            print_indent(indent+1);
            printf("NULL\n");
        }
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
