#include "operator.h"
#include "value.h"

using namespace tora;

/**
 * binary addition operator
 */
Value * tora::op_add(const SharedPtr<Value>& lhs, const SharedPtr<Value>& rhs) {
    if (lhs->value_type == VALUE_TYPE_INT) {
        if (rhs->value_type == VALUE_TYPE_DOUBLE) {
            // upgrade to double type
            return new DoubleValue(lhs->to_double() + rhs->to_double());
        } else {
            int i = rhs->to_int();
            return new IntValue(lhs->upcast<IntValue>()->int_value + i);
        }
    } else if (lhs->value_type == VALUE_TYPE_STR) {
        // TODO: support null terminated string
        SharedPtr<Value> s(rhs->to_s());
        return new StrValue(lhs->upcast<StrValue>()->str_value + s->upcast<StrValue>()->str_value);
    } else if (lhs->value_type == VALUE_TYPE_DOUBLE) {
        return new DoubleValue(lhs->to_double() + rhs->to_double());
    } else {
        SharedPtr<Value> s(lhs->to_s());
        throw new ExceptionValue("'%s' is not numeric or string.\n", s->upcast<StrValue>()->str_value.c_str());
    }
}

/**
 * subtract lhs and rhs.
 * return value must be return by caller.
 */
Value * tora::op_sub(const SharedPtr<Value>& lhs, const SharedPtr<Value> & rhs) {
    if (lhs->value_type == VALUE_TYPE_DOUBLE) {
        return new DoubleValue(lhs->to_double() - rhs->to_double());
    } else if (lhs->value_type == VALUE_TYPE_INT) {
        if (rhs->value_type == VALUE_TYPE_DOUBLE) { // upgrade
            return new DoubleValue(lhs->to_double() - rhs->to_double());
        } else {
            return new IntValue(lhs->upcast<IntValue>()->int_value - rhs->to_int());
        }
    } else { 
        SharedPtr<Value> s(lhs->to_s());
        throw new ExceptionValue("'%s' is not numeric. You cannot subtract.", s->upcast<StrValue>()->str_value.c_str());
    }
    abort();
}

Value * tora::op_div(const SharedPtr<Value> &lhs, const SharedPtr<Value> &rhs) {
    if (lhs->value_type == VALUE_TYPE_DOUBLE) {
        return new DoubleValue(lhs->to_double() / rhs->to_double());
    } else if (lhs->value_type == VALUE_TYPE_INT) {
        if (rhs->value_type == VALUE_TYPE_DOUBLE) { // upgrade
            return new DoubleValue(lhs->to_double() / rhs->to_double());
        } else {
            return new IntValue(lhs->upcast<IntValue>()->int_value / rhs->upcast<IntValue>()->int_value);
        }
    } else { 
        SharedPtr<Value> s(lhs->to_s());
        throw new ExceptionValue("'%s' is not numeric. You cannot divide.", s->upcast<StrValue>()->str_value.c_str());
    }
    abort();
}

Value * tora::op_mul(const SharedPtr<Value> &lhs, const SharedPtr<Value> &rhs) {
    if (lhs->value_type == VALUE_TYPE_DOUBLE) {
        return new DoubleValue(lhs->to_double() * rhs->to_double());
    } else if (lhs->value_type == VALUE_TYPE_INT) {
        if (rhs->value_type == VALUE_TYPE_DOUBLE) { // upgrade
            return new DoubleValue(lhs->to_double() * rhs->to_double());
        } else {
            return new IntValue(lhs->upcast<IntValue>()->int_value * rhs->upcast<IntValue>()->int_value);
        }
    } else { 
        SharedPtr<Value> s(lhs->to_s());
        throw new ExceptionValue("'%s' is not numeric. You cannot multiply.", s->upcast<StrValue>()->str_value.c_str());
    }
    abort();
}

Value * tora::op_pow(const SharedPtr<Value> &lhs, const SharedPtr<Value> &rhs) {
    if (lhs->value_type == VALUE_TYPE_DOUBLE) {
        return new DoubleValue(pow(lhs->to_double(), rhs->to_double()));
    } else if (lhs->value_type == VALUE_TYPE_INT) {
        if (rhs->value_type == VALUE_TYPE_DOUBLE) { // upgrade
            return new DoubleValue(pow(lhs->to_double(), rhs->to_double()));
        } else {
            return new IntValue((int)pow(lhs->to_double(), rhs->to_double()));
        }
    } else { 
        SharedPtr<Value> s(lhs->to_s());
        throw new ExceptionValue("'%s' is not numeric. You cannot pow.", s->upcast<StrValue>()->str_value.c_str());
    }
    abort();
}

Value * tora::op_bitand(const SharedPtr<Value> &lhs, const SharedPtr<Value> &rhs) {
    if (lhs->value_type == VALUE_TYPE_INT) {
        return new IntValue(lhs->to_int() & rhs->to_int());
    } else { 
        SharedPtr<Value> s(lhs->to_s());
        throw new ExceptionValue("'%s' is not integer. You cannot and.", s->upcast<StrValue>()->str_value.c_str());
    }
    abort();
}

Value * tora::op_bitor(const SharedPtr<Value> &lhs, const SharedPtr<Value> &rhs) {
    if (lhs->value_type == VALUE_TYPE_INT) {
        return new IntValue(lhs->to_int() | rhs->to_int());
    } else { 
        SharedPtr<Value> s(lhs->to_s());
        throw new ExceptionValue("'%s' is not integer. You cannot bit or.", s->upcast<StrValue>()->str_value.c_str());
    }
    abort();
}

Value * tora::op_bitxor(const SharedPtr<Value> &lhs, const SharedPtr<Value> &rhs) {
    if (lhs->value_type == VALUE_TYPE_INT) {
        return new IntValue(lhs->to_int() ^ rhs->to_int());
    } else { 
        SharedPtr<Value> s(lhs->to_s());
        throw new ExceptionValue("'%s' is not integer. You cannot xor.", s->upcast<StrValue>()->str_value.c_str());
    }
    abort();
}

Value * tora::op_bitlshift(const SharedPtr<Value> &lhs, const SharedPtr<Value> &rhs) {
    if (lhs->value_type == VALUE_TYPE_INT) {
        return new IntValue(lhs->to_int() << rhs->to_int());
    } else { 
        SharedPtr<Value> s(lhs->to_s());
        throw new ExceptionValue("'%s' is not integer. You cannot <<.", s->upcast<StrValue>()->str_value.c_str());
    }
    abort();
}

Value * tora::op_bitrshift(const SharedPtr<Value> &lhs, const SharedPtr<Value> &rhs) {
    if (lhs->value_type == VALUE_TYPE_INT) {
        return new IntValue(lhs->to_int() >> rhs->to_int());
    } else { 
        SharedPtr<Value> s(lhs->to_s());
        throw new ExceptionValue("'%s' is not integer. You cannot >>.", s->upcast<StrValue>()->str_value.c_str());
    }
    abort();
}

Value * tora::op_modulo(const SharedPtr<Value> &lhs, const SharedPtr<Value> &rhs) {
    if (lhs->value_type == VALUE_TYPE_INT) {
        double x = lhs->to_int();
        double y = rhs->to_int();
        return new IntValue(x-y*floor(x/y));
    } else { 
        SharedPtr<Value> s(lhs->to_s());
        throw new ExceptionValue("'%s' is not integer. You cannot >>.", s->upcast<StrValue>()->str_value.c_str());
    }
    abort();
}

Value* tora::op_unary_negative(const SharedPtr<Value> & v) {
    switch (v->value_type) {
    case VALUE_TYPE_INT:
        return new IntValue(-(v->upcast<IntValue>()->int_value));
    case VALUE_TYPE_DOUBLE:
        return new DoubleValue(-(v->upcast<DoubleValue>()->double_value));
    case VALUE_TYPE_OBJECT:
        TODO();
    default:
        throw new ExceptionValue("%s is not a numeric. You cannot apply unary negative operator.\n", v->type_str());
    }
}

