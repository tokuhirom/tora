#ifndef TORA_OPERATOR_H_
#define TORA_OPERATOR_H_

#include "shared_ptr.h"

namespace tora {

class Value;

// binary operators
Value* op_add(const SharedPtr<Value>& lhs, const SharedPtr<Value>& rhs);
Value* op_sub(const SharedPtr<Value>& lhs, const SharedPtr<Value>& rhs);
Value* op_div(const SharedPtr<Value>& lhs, const SharedPtr<Value>& rhs);
Value* op_mul(const SharedPtr<Value>& lhs, const SharedPtr<Value>& rhs);
Value* op_modulo(const SharedPtr<Value>& lhs, const SharedPtr<Value>& rhs);
Value* op_pow(const SharedPtr<Value>& lhs, const SharedPtr<Value>& rhs);
Value* op_bitand(const SharedPtr<Value>& lhs, const SharedPtr<Value>& rhs);
Value* op_bitor(const SharedPtr<Value>& lhs, const SharedPtr<Value>& rhs);
Value* op_bitxor(const SharedPtr<Value>& lhs, const SharedPtr<Value>& rhs);
Value* op_bitlshift(const SharedPtr<Value>& lhs, const SharedPtr<Value>& rhs);
Value* op_bitrshift(const SharedPtr<Value>& lhs, const SharedPtr<Value>& rhs);

// unary operators
Value* op_unary_negative(const SharedPtr<Value>& v);

// container operators
SharedPtr<Value> op_get_item(const SharedPtr<Value>& container,
                             const SharedPtr<Value>& index);

template <class operationI, class operationD, class operationS>
bool cmpop(operationI operation_i, operationD operation_d,
           operationS operation_s, const SharedPtr<Value>& lhs,
           const SharedPtr<Value>& rhs);
};

#endif  // TORA_OPERATOR_H_
