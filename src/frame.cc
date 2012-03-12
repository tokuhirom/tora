#include "frame.h"
#include "vm.h"
#include "peek.h"
#include "package.h"
#include "value/code.h"

using namespace tora;


LexicalVarsFrame::LexicalVarsFrame(VM *vm, int vars_cnt, size_t top, frame_type_t type_) : vm_(vm), vars(vars_cnt), top(top), type(type_) {
}

LexicalVarsFrame::~LexicalVarsFrame() {
    for (auto iter = dynamic_scope_vars.begin(); iter != dynamic_scope_vars.end(); ++iter) {
        const SharedPtr<DynamicScopeData> & dat = *iter;
        dat->package()->set_variable(dat->moniker_id(), dat->value());
    }
}

void LexicalVarsFrame::push_dynamic_scope_var(Package* pkgid, ID monikerid, const SharedPtr<Value> &target) {
    SharedPtr<DynamicScopeData> v(new DynamicScopeData(pkgid, monikerid, target));
    this->dynamic_scope_vars.push_back(v);
}

const char *LexicalVarsFrame::type_str() const {
    switch (type) {
    case FRAME_TYPE_FUNCTION:
        return "function";
    case FRAME_TYPE_LEXICAL:
        return "lexical";
    case FRAME_TYPE_PACKAGE:
        return "package";
    case FRAME_TYPE_TRY:
        return "try";
    case FRAME_TYPE_FOREACH:
        return "foreach";
    case FRAME_TYPE_WHILE:
        return "while";
    case FRAME_TYPE_FOR:
        return "for";
    }
    abort();
}

boost::object_pool<FunctionFrame> FunctionFrame::pool_;

PackageFrame::~PackageFrame() {
    // printf("LEAVE PACKAGE FROM %s. back to %d\n", vm_->package_name().c_str(), orig_package_id_);
    vm_->package_id(orig_package_id_);
}

