#include "frame.h"
#include "vm.h"
#include "value/code.h"

using namespace tora;

LexicalVarsFrame::LexicalVarsFrame(int vars_cnt, size_t top, frame_type_t type_) : vars(vars_cnt), top(top), type(type_) {
}

boost::object_pool<FunctionFrame> FunctionFrame::pool_;

PackageFrame::~PackageFrame() {
    // printf("LEAVE PACKAGE FROM %s. back to %d\n", vm_->package_name().c_str(), orig_package_id_);
    vm_->package_id(orig_package_id_);
}
