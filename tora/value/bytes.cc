#include "bytes.h"
#include "../value.h"

using namespace tora;

int BytesValue::length() {
    return this->bytes_value.size();
}

BytesValue::~BytesValue() { }

