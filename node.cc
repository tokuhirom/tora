#include "tora.h"

// TODO: move to Node::dump()

static void print_indent(int indent) {
    for (int i=0; i<indent*2; i++) {
        printf(" ");
    }
}

void tora::Node::dump(int indent) {
    print_indent(indent);
    switch (this->type) {
    case NODE_STRING:
        printf("NODE_STRING('%s')\n", this->str_value);
        break;
    case NODE_INT:
        printf("NODE_INT(%d)\n", this->int_value);
        break;
    case NODE_TRUE:
        printf("NODE_TRUE\n");
        break;
    case NODE_FALSE:
        printf("NODE_FALSE\n");
        break;
    case NODE_IDENTIFIER:
        printf("NODE_IDENTIFIER(%s)\n", this->str_value);
        break;
    case NODE_NEWLINE:
        printf("NODE_NEWLINE\n");
        break;

    case NODE_FUNCALL: {
        printf("NODE_FUNCALL\n");
        this->funcall.name->dump(indent+1);
        std::vector<Node*>*args = this->funcall.args;
        print_indent(indent+1);
        printf("arguments:\n");
        for (size_t i=0; i<args->size(); i++) {
            args->at(i)->dump(indent+2);
        }
        break;
    }
    case NODE_GT:
        printf("NODE_GT\n");
        this->binary.left->dump(indent+1);
        this->binary.right->dump(indent+1);
        break;
    case NODE_ADD:
        printf("NODE_ADD\n");
        this->binary.left->dump(indent+1);
        this->binary.right->dump(indent+1);
        break;
    case NODE_SUB:
        printf("NODE_SUB\n");
        this->binary.left->dump(indent+1);
        this->binary.right->dump(indent+1);
        break;
    case NODE_MUL:
        printf("NODE_MUL\n");
        this->binary.left->dump(indent+1);
        this->binary.right->dump(indent+1);
        break;
    case NODE_DIV:
        printf("NODE_DIV\n");
        this->binary.left->dump(indent+1);
        this->binary.right->dump(indent+1);
        break;
    case NODE_STMTS:
        printf("NODE_STMTS\n");
        this->binary.left->dump(indent+1);
        this->binary.right->dump(indent+1);
        break;
    case NODE_IF:
        printf("NODE_IF\n");
        this->binary.left->dump(indent+1);
        this->binary.right->dump(indent+1);
        break;
    case NODE_ROOT:
        printf("NODE_ROOT\n");
        this->node->dump(indent+1);
        break;
    case NODE_BLOCK:
        printf("NODE_BLOCK\n");
        this->node->dump(indent+1);
        break;
    case NODE_GET_ITEM:
        printf("NODE_GET_ITEM\n");
        this->binary.left->dump(indent+1);
        this->binary.right->dump(indent+1);
        break;
    case NODE_GETVARIABLE:
        printf("NODE_GETVARIABLE[%s]\n", this->str_value);
        break;
    case NODE_SETVARIABLE:
        printf("NODE_SETVARIABLE\n");
        this->set_value.lvalue->dump(indent+1);
        this->set_value.rvalue->dump(indent+1);
        break;
    case NODE_MY:
        printf("NODE_MY\n");
        this->node->dump(indent+1);
        break;
    case NODE_MAKE_ARRAY: {
        printf("NODE_MAKE_ARRAY\n");
        print_indent(indent+1);
        auto args = this->args;
        printf("arguments:\n");
        for (size_t i=0; i<args->size(); i++) {
            args->at(i)->dump(indent+2);
        }
        break;
    }
    case NODE_WHILE:
        printf("NODE_WHILE\n");
        print_indent(indent+1);
        printf("condition:\n");
        this->binary.left->dump(indent+2);
        print_indent(indent+1);
        printf("body:\n");
        this->binary.right->dump(indent+2);
        break;

    default:
        printf("Unknown this: %s\n", this->type_name_str());
        break;
    }
}
void tora::Node::dump() {
    this->dump(0);
}
