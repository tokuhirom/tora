#ifndef TORA_OPERATOR_H_
#define TORA_OPERATOR_H_

#include "shared_ptr.h"

namespace tora {

class Value;
class SharedValue;

// binary operators
SharedValue op_add(Value* lhs, Value* rhs);
SharedValue op_sub(Value* lhs, Value* rhs);
SharedValue op_div(Value* lhs, Value* rhs);
SharedValue op_mul(Value* lhs, Value* rhs);
SharedValue op_modulo(Value* lhs, Value* rhs);
SharedValue op_pow(Value* lhs, Value* rhs);
SharedValue op_bitand(Value* lhs, Value* rhs);
SharedValue op_bitor(Value* lhs, Value* rhs);
SharedValue op_bitxor(Value* lhs, Value* rhs);
SharedValue op_bitlshift(Value* lhs, Value* rhs);
SharedValue op_bitrshift(Value* lhs, Value* rhs);

// unary operators
SharedValue op_unary_negative(Value* v);

// container operators
SharedPtr<Value> op_get_item(Value* container,
                             Value* index);

template <class operationI, class operationD, class operationS>
bool cmpop(operationI operation_i, operationD operation_d,
           operationS operation_s, Value* lhs,
           Value* rhs);
};

#endif  // TORA_OPERATOR_H_
