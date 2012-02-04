#include "tora.h"
#include "dump_tree.h"

// TODO: move to TNode::dump()

static void print_indent(int indent) {
    for (int i=0; i<indent*2; i++) {
        printf(" ");
    }
}

void tora::dump_tree(TNode *node, int indent) {
    print_indent(indent);
    switch (node->type) {
    case NODE_STRING:
        printf("NODE_STRING('%s')\n", node->str_value);
        break;
    case NODE_INT:
        printf("NODE_INT(%d)\n", node->int_value);
        break;
    case NODE_TRUE:
        printf("NODE_TRUE\n");
        break;
    case NODE_FALSE:
        printf("NODE_FALSE\n");
        break;
    case NODE_IDENTIFIER:
        printf("NODE_IDENTIFIER(%s)\n", node->str_value);
        break;
    case NODE_NEWLINE:
        printf("NODE_NEWLINE\n");
        break;

    case NODE_FUNCALL: {
        printf("NODE_FUNCALL\n");
        tora::dump_tree(node->funcall.name, indent+1);
        std::vector<TNode*>*args = node->funcall.args;
        print_indent(indent+1);
        printf("arguments:\n");
        for (size_t i=0; i<args->size(); i++) {
            tora::dump_tree(args->at(i), indent+2);
        }
        break;
    }
    case NODE_GT:
        printf("NODE_GT\n");
        tora::dump_tree(node->binary.left, indent+1);
        tora::dump_tree(node->binary.right, indent+1);
        break;
    case NODE_ADD:
        printf("NODE_ADD\n");
        tora::dump_tree(node->binary.left, indent+1);
        tora::dump_tree(node->binary.right, indent+1);
        break;
    case NODE_SUB:
        printf("NODE_SUB\n");
        tora::dump_tree(node->binary.left, 1);
        tora::dump_tree(node->binary.right, 1);
        break;
    case NODE_MUL:
        printf("NODE_MUL\n");
        tora::dump_tree(node->binary.left, 1);
        tora::dump_tree(node->binary.right, 1);
        break;
    case NODE_DIV:
        printf("NODE_DIV\n");
        tora::dump_tree(node->binary.left, 1);
        tora::dump_tree(node->binary.right, 1);
        break;
    case NODE_STMTS:
        printf("NODE_STMTS\n");
        tora::dump_tree(node->binary.left, indent+1);
        tora::dump_tree(node->binary.right, indent+1);
        break;
    case NODE_IF:
        printf("NODE_IF\n");
        tora::dump_tree(node->binary.left, 1);
        tora::dump_tree(node->binary.right, 1);
        break;
    case NODE_ROOT:
        printf("NODE_ROOT\n");
        tora::dump_tree(node->node, indent+1);
        break;
    case NODE_BLOCK:
        printf("NODE_BLOCK\n");
        tora::dump_tree(node->node, indent+1);
        break;
    case NODE_GET_ITEM:
        printf("NODE_GET_ITEM\n");
        tora::dump_tree(node->binary.left, indent+1);
        tora::dump_tree(node->binary.right, indent+1);
        break;
    case NODE_GETVARIABLE:
        printf("NODE_GETVARIABLE[%s]\n", node->str_value);
        break;
    case NODE_SETVARIABLE:
        printf("NODE_SETVARIABLE\n");
        tora::dump_tree(node->set_value.lvalue, indent+1);
        tora::dump_tree(node->set_value.rvalue, indent+1);
        break;
    case NODE_MY:
        printf("NODE_MY\n");
        tora::dump_tree(node->node, indent+1);
        break;
    case NODE_MAKE_ARRAY: {
        printf("NODE_MAKE_ARRAY\n");
        print_indent(indent+1);
        auto args = node->args;
        printf("arguments:\n");
        for (size_t i=0; i<args->size(); i++) {
            tora::dump_tree(args->at(i), indent+2);
        }
        break;
    }
    case NODE_WHILE:
        printf("NODE_WHILE\n");
        print_indent(indent+1);
        printf("condition:\n");
        tora::dump_tree(node->binary.left, indent+2);
        print_indent(indent+1);
        printf("body:\n");
        tora::dump_tree(node->binary.right, indent+2);
        break;

    default:
        printf("Unknown node: %s\n", node->type_name_str());
        break;
    }
}
void tora::dump_tree(TNode *node) {
    tora::dump_tree(node, 0);
}
