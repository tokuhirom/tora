#include "frame.h"
#include "vm.h"
#include "peek.h"
#include "package.h"
#include "value/code.h"
#include "pad_list.h"

using namespace tora;


SharedPtr<Value> LexicalVarsFrame::get_variable(int id) const {
#ifndef NDEBUG
    if (id >= this->vars.capacity()) {
        fprintf(stderr, "[BUG] id< vars.capacity(). id: %d, capacity: %zd\n", id, this->vars.capacity());
        abort();
    }
#endif
    assert(id < this->vars.capacity());
    return this->pad_list->get(id);
}

LexicalVarsFrame::LexicalVarsFrame(VM *vm, int vars_cnt, size_t top, frame_type_t type_) : refcnt(0), vm_(vm), vars(vars_cnt), top(top), type(type_) {
    this->pad_list = new PadList(
        vars_cnt,
        (vm->frame_stack->size() > 0) ? vm->frame_stack->back()->pad_list.get() : NULL
    );
}

LexicalVarsFrame::~LexicalVarsFrame() {
    if (this->type == FRAME_TYPE_PACKAGE) {
        // printf("LEAVE PACKAGE FROM %s. back to %d\n", vm_->package_name().c_str(), orig_package_id_);
        vm_->package_id(static_cast<PackageFrame*>(this)->orig_package_id_);
    }

    for (auto iter = dynamic_scope_vars.begin(); iter != dynamic_scope_vars.end(); ++iter) {
        const SharedPtr<DynamicScopeData> & dat = *iter;
        dat->package()->set_variable(dat->moniker_id(), dat->value());
    }
}

void LexicalVarsFrame::set_variable_dynamic(int level, int no, const SharedPtr<Value>& v) {
    this->pad_list->set_dynamic(level, no, v);
    // assert(this->pad_list->get_dynamic(level, no).get() == v.get());
}

void LexicalVarsFrame::set_variable(int id, const SharedPtr<Value>& v) {
    assert(id < this->vars.capacity());
    this->pad_list->set(id, v);
}

SharedPtr<Value> LexicalVarsFrame::get_variable_dynamic(int level, int no) const {
    return pad_list->get_dynamic(level, no).get();
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

void LexicalVarsFrame::dump_pad(VM *vm) {
    pad_list->dump(vm);
}

boost::object_pool<FunctionFrame> FunctionFrame::pool_;

