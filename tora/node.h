#ifndef TORA_NODE_H_
#define TORA_NODE_H_

#include <vector>
#include "nodes.gen.h"
#include "shared_ptr.h"
#include "prim.h"

#include <string>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

namespace tora {

class BinaryNode;
class MethodCallNode;
class IfNode;
class ForNode;
class ForEachNode;
class FuncdefNode;
class FuncallNode;
class IntNode;
class DoubleNode;
class StrNode;
class TryNode;

class Node {
  PRIM_DECL();

 public:
  node_type_t type;
  std::vector<SharedPtr<Node>> children_;
  int lineno;

  Node(node_type_t n = NODE_UNKNOWN) : refcnt(0) { type = n; }
  Node(node_type_t type_, Node* node) : refcnt(0), type(type_) {
    children_.push_back(node);
  }
  const std::vector<SharedPtr<Node>>& children() const { return children_; }
  std::vector<SharedPtr<Node>>& children() { return children_; }
  /**
   * ~Node is not a virtual for performance reason.
   * Do not override it.
   */
  virtual ~Node() {}
  const char* type_name_str() const { return node_type2name[this->type]; }
  virtual void dump(int indent);
  void dump();

  Node* at(int i) const { return children_.at(i).get(); }

  size_t size() const { return children_.size(); }

  template <class Y>
  Y* upcast() {
    return static_cast<Y*>(this);
  }

  void push_back(Node* n) { children_.push_back(n); }
  void push_back(const SharedPtr<Node>& n) { children_.push_back(n); }
  void pop_back() { children_.pop_back(); }
  const SharedPtr<Node>& back() { return children_.back(); }
};

class StrNode : public Node {
 public:
  std::string str_value;
  StrNode(node_type_t type_, const char* str) : Node(type_), str_value(str) {}
  StrNode(node_type_t type_, const std::string& str)
      : Node(type_), str_value(str) {}
  void dump(int indent);
};

class RegexpNode : public Node {
 public:
  std::string regexp_value;
  int flags;
  RegexpNode(node_type_t type_, const std::string& str, int flags_)
      : Node(type_), regexp_value(str), flags(flags_) {}
  void dump(int indent);
};

class IntNode : public Node {
 public:
  int int_value;
  IntNode(node_type_t type_, int i) : Node() {
    type = type_;
    int_value = i;
  }
  void dump(int indent);
};

class DoubleNode : public Node {
 public:
  double double_value;
  DoubleNode(node_type_t type_, double d) : Node() {
    type = type_;
    double_value = d;
  }
  void dump(int indent);
};

class FuncallNode : public Node {
 public:
  bool is_bare;
  SharedPtr<Node> name() { return this->children_.at(0); }
  SharedPtr<Node> args() { return this->children_.at(1)->upcast<Node>(); }
  FuncallNode(SharedPtr<Node> name_, SharedPtr<Node> args_, bool bare = false)
      : Node(NODE_FUNCALL) {
    this->children_.push_back(name_);
    this->children_.push_back(args_);
    is_bare = bare;
  }
};

class FuncdefNode : public Node {
 public:
  SharedPtr<Node> name() { return this->children_.at(0); }
  bool have_params() { return this->children_.at(1).get() != NULL; }
  SharedPtr<Node> params() { return this->children_.at(1)->upcast<Node>(); }
  SharedPtr<Node> block() { return this->children_.at(2); }
  FuncdefNode(SharedPtr<Node> n, SharedPtr<Node> p, SharedPtr<Node> b)
      : Node(NODE_FUNCDEF) {
    this->children_.push_back(n);
    this->children_.push_back(p);
    this->children_.push_back(b);
  }
};

class ForNode : public Node {
 public:
  ForNode(SharedPtr<Node> i, SharedPtr<Node> c, SharedPtr<Node> p,
          SharedPtr<Node> b)
      : Node(NODE_FOR) {
    this->children_.push_back(i);
    this->children_.push_back(c);
    this->children_.push_back(p);
    this->children_.push_back(b);
  }
  SharedPtr<Node> initialize() { return this->children_.at(0); }
  SharedPtr<Node> cond() { return this->children_.at(1); }
  SharedPtr<Node> postfix() { return this->children_.at(2); }
  SharedPtr<Node> block() { return this->children_.at(3); }
};

class ForEachNode : public Node {
 public:
  SharedPtr<Node> source() { return this->children_.at(0); }
  SharedPtr<Node> vars() { return this->children_.at(1); }
  SharedPtr<Node> block() { return this->children_.at(2); }
  ForEachNode(SharedPtr<Node> src_, SharedPtr<Node> vars_, SharedPtr<Node> b)
      : Node(NODE_FOREACH) {
    this->children_.push_back(src_);
    this->children_.push_back(vars_);
    this->children_.push_back(b);
  }
};

class IfNode : public Node {
 public:
  SharedPtr<Node> cond() { return this->children_.at(0); }
  SharedPtr<Node> if_body() { return this->children_.at(1); }
  SharedPtr<Node> else_body() { return this->children_.at(2); }
  IfNode(node_type_t t, SharedPtr<Node> c, SharedPtr<Node> i, SharedPtr<Node> e)
      : Node(t) {
    this->children_.push_back(c);
    this->children_.push_back(i);
    this->children_.push_back(e);
  }
};

class MethodCallNode : public Node {
 public:
  SharedPtr<Node> object() { return this->children_.at(0); }
  SharedPtr<Node> method() { return this->children_.at(1); }
  SharedPtr<Node> args() { return this->children_.at(2)->upcast<Node>(); }

  MethodCallNode(SharedPtr<Node> o, SharedPtr<Node> m, SharedPtr<Node> a)
      : Node(NODE_METHOD_CALL) {
    this->children_.push_back(o);
    this->children_.push_back(m);
    this->children_.push_back(a);
  }
};

class ClassNode : public Node {
 public:
  SharedPtr<Node> klass() { return this->children_.at(0); }
  SharedPtr<Node> parent() { return this->children_.at(1); }
  SharedPtr<Node> roles() { return this->children_.at(2)->upcast<Node>(); }
  SharedPtr<Node> block() { return this->children_.at(3); }

  ClassNode(SharedPtr<Node> k, SharedPtr<Node> p, SharedPtr<Node> r,
            SharedPtr<Node> b)
      : Node(NODE_CLASS) {
    this->children_.push_back(k);
    this->children_.push_back(p);
    this->children_.push_back(r);
    this->children_.push_back(b);
  }
};

class BinaryNode : public Node {
 public:
  SharedPtr<Node> left() const { return this->children_.at(0); }
  SharedPtr<Node> right() const { return this->children_.at(1); }
  BinaryNode(node_type_t t, SharedPtr<Node> l, SharedPtr<Node> r) : Node(t) {
    this->children_.push_back(l);
    this->children_.push_back(r);
  }
};
};

#endif  // TORA_NODE_H_
