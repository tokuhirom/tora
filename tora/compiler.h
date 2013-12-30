#ifndef TORA_COMPILER_H_
#define TORA_COMPILER_H_

#include "shared_ptr.h"
#include "prim.h"
#include "op.h"
#include <stdarg.h>
#include <vector>
#include <string>
#include <memory>
#include "value/bytes.h"

namespace tora {

class Node;
class OPArray;
class OP;
class ValueOP;
class SymbolTable;

enum block_type_t {
  BLOCK_TYPE_BLOCK,
  BLOCK_TYPE_SUB,
  BLOCK_TYPE_FILE,
  BLOCK_TYPE_TRY,
  BLOCK_TYPE_FUNCDEF,
  BLOCK_TYPE_CLASS,
};

class Block {
 public:
  block_type_t type;
  std::vector<std::string> vars;
  Block(block_type_t t) : type(t) {}
  ~Block() {}
};

class Compiler {
  std::string package_;
  std::string filename_;

 public:
  std::shared_ptr<OPArray> ops;
  std::vector<Block> *blocks;
  std::vector<std::string> *global_vars;
  std::shared_ptr<std::vector<std::string>> closure_vars;
  SharedPtr<SymbolTable> symbol_table;
  int error;
  bool in_try_block;
  bool dump_ops;
  bool in_class_context;

  bool in_loop_context;
  std::vector<int *> last_labels;

  SharedPtr<Node> current_node;

  void package(const std::string &p) { package_ = p; }
  std::string &package() { return package_; }

  Compiler(const SharedPtr<SymbolTable> &symbol_table_,
           const std::string &filename);
  ~Compiler();

  void define_my(SharedPtr<Node> node);
  void define_global_var(const char *name) {
    auto iter = global_vars->begin();
    for (; iter != global_vars->end(); iter++) {
      if (*iter == name) {
        printf("[BUG] duplicated global variable name: %s\n", name);
        abort();
      }
    }
    global_vars->push_back(name);
  }
  int find_global_var(std::string &name) {
    auto iter = global_vars->begin();
    for (; iter != global_vars->end(); iter++) {
      if (*iter == name) {
        return iter - global_vars->begin();
      }
    }
    return -1;
  }
  void init_globals();
  void compile(const SharedPtr<Node> &node);
  void push_block(block_type_t t) { this->blocks->push_back(Block(t)); }
  void pop_block() { this->blocks->pop_back(); }
  void set_lvalue(SharedPtr<Node> node);
  void set_variable(std::string &varname);
  int find_localvar(std::string name, int &level, bool &need_closure,
                    bool &is_arg, int &funcdef_level);
  void define_localvar(const char *name) {
    this->define_localvar(std::string(name));
  }
  void define_localvar(const std::string name) {
    Block &block = this->blocks->back();
    for (size_t i = 0; i < block.vars.size(); i++) {
      if (block.vars.at(i) == name) {
        this->fail("Duplicated variable: %s\n", name.c_str());
        return;
      }
    }
    block.vars.push_back(name);
  }
  void dump_localvars();
  void fail(const char *format, ...);

  void push_op(OP *op);
  void push_op(const SharedPtr<OP> &op) { this->push_op(op.get()); }
  void push_op(const SharedPtr<ValueOP> &op) { this->push_op(op.get()); }

  bool is_builtin(const std::string &s);

  /**
   * This is RAII guard object.
   */
  class TryGuard {
   private:
    Compiler *c;
    bool orig;

   public:
    TryGuard(Compiler *c_, bool b) : c(c_) {
      orig = c_->in_try_block;
      c->in_try_block = b;
    }
    ~TryGuard() {
      c->in_try_block = orig;  // restore
    }
  };
};
};

#endif  // TORA_COMPILER_H_
