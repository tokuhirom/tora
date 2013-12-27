#ifndef TORA_OP_ARRAY_H_
#define TORA_OP_ARRAY_H_

#include <vector>

#include "op.h"

namespace tora {

class OPArray {
  PRIM_DECL()
 private:
  std::vector<op_type_t> op_types;
  std::vector<OP*> ops;
  std::vector<size_t> lineno;

 public:
  OPArray() : refcnt(0) {}
  OPArray(const OPArray& opa) : refcnt(0) {
    op_types = opa.op_types;
    ops = opa.ops;
    lineno = opa.lineno;
  }
  ~OPArray() {
    auto iter = ops.begin();
    for (; iter != ops.end(); iter++) {
      delete *iter;
    }
  }
  const OP* at(size_t i) const {
#ifdef NDEBUG
    // operator[] is faster.
    return this->ops[i];
#else
    return this->ops.at(i);
#endif
  }
  op_type_t op_type_at(size_t i) const { return this->op_types[i]; }
  size_t size() const { return this->ops.size(); }
  void push_back(OP* o, size_t lineno) {
    o->retain();
    this->ops.push_back(o);
    this->op_types.push_back(o->op_type);
    this->lineno.push_back(lineno);
  }
  void push_back(SharedPtr<ValueOP>& o, size_t lineno) {
    o->retain();
    this->ops.push_back(&(*(o)));
    this->op_types.push_back(o->op_type);
    this->lineno.push_back(lineno);
  }
  void push_back(const SharedPtr<OP>& o, size_t lineno) {
    o->retain();
    this->ops.push_back(&(*(o)));
    this->op_types.push_back(o->op_type);
    this->lineno.push_back(lineno);
  }

  size_t get_lineno(size_t pc) { return lineno[pc]; }
};
};

#endif  // TORA_OP_ARRAY_H_
