/* vim: set filetype=cpp: */

OP_PUSH_TRUE {
    stack.push_back(new BoolValue(true));
}

OP_PUSH_FALSE {
    stack.push_back(new BoolValue(false));
}

OP_PUSH_SELF {
    stack.push_back(this->get_self());
}

OP_PUSH_UNDEF {
    stack.push_back(UndefValue::instance());
}

OP_PUSH_INT {
    stack.push_back(new IntValue(get_int_operand()));
}

OP_PUSH_DOUBLE {
    stack.push_back(new DoubleValue(get_double_operand()));
}

OP_PUSH_STRING {
    const OP * op = ops->at(pc);
    SharedPtr<Value> sv = ((ValueOP*)&(*(op)))->value;
    stack.push_back(sv);
}

OP_GETARG {
    int no = get_int_operand();
    int index = mark_stack.back() - no - 1;
#ifndef NDEBUG
    if (index >= stack.size()) {
        dump_stack();
        abort();
    }
#endif
    stack.push_back(stack.at(index));
}


OP_SETARG {
    int no = get_int_operand();
    SharedPtr<Value> rvalue = stack.back();
    if (rvalue->value_type == VALUE_TYPE_TUPLE) {
        rvalue = rvalue->upcast<TupleValue>()->at(rvalue->upcast<TupleValue>()->size()-1);
    }
    stack[mark_stack.back() - no - 1].reset(rvalue.get());
}

OP_PUSH_VALUE {
    const OP * op = ops->at(pc);
    SharedPtr<Value> v = ((ValueOP*)&(*(op)))->value;
    stack.push_back(v);
}

OP_NEW_RANGE {
    SharedPtr<Value> l = stack.back();
    stack.pop_back();
    SharedPtr<Value> r = stack.back();
    stack.pop_back();

    SharedPtr<IntValue> l2 = l->value_type == VALUE_TYPE_INT ? l->upcast<IntValue>() : new IntValue(l->to_int());
    SharedPtr<IntValue> r2 = r->value_type == VALUE_TYPE_INT ? r->upcast<IntValue>() : new IntValue(r->to_int());
    stack.push_back(new RangeValue(l2, r2));
}

OP_FUNCDEF {
    SharedPtr<Value> code = stack.back(); stack.pop_back(); // code object
    assert(code->value_type == VALUE_TYPE_CODE);
    // printf("FUNCDEF!! %d, %d\n", package_id(), code->upcast<CodeValue>()->func_name_id);
    int pkgid = code->upcast<CodeValue>()->package_id();
    if (!pkgid) {
        pkgid = this->package_id();
        code->upcast<CodeValue>()->package_id(pkgid);
    }
    this->add_function(pkgid, code->upcast<CodeValue>()->func_name_id(), code);
}

OP_CLOSUREDEF {
    SharedPtr<Value> code = stack.back();
    stack.pop_back();

    // take variables from stack. and push it to code->closure_cells.
    assert(code->value_type == VALUE_TYPE_CODE);

    // int closure_vars_cnt = get_int_operand();
    code->upcast<CodeValue>()->pad_list(this->frame_stack->back()->pad_list);

// printf("CLOSUREDEF --\n");
//  for (int i=0; i<closure_vars_cnt; i++) {
//      SharedPtr<Value> v = stack.back(); stack.pop_back();
//      code->upcast<CodeValue>()->closure_vars()->push_back(v);
//  }
    // printf("CLOSUREDEF!! %d, %d\n", package_id(), code->upcast<CodeValue>()->func_name_id);

    int pkgid = code->upcast<CodeValue>()->package_id();
    if (!pkgid) {
        pkgid = this->package_id();
        code->upcast<CodeValue>()->package_id(pkgid);
    }
    this->add_function(pkgid, code->upcast<CodeValue>()->func_name_id(), code);
}

OP_GETCLOSURE {
    // take variable from code->closure_cells[n]

    SharedPtr<LexicalVarsFrame> frame;
    for (auto iter=frame_stack->rbegin(); iter!=frame_stack->rend(); iter++) {
        if ((*iter)->type == FRAME_TYPE_FUNCTION) {
            frame = *iter;
            break;
        }
    }
    assert(frame.get());
    int level = get_int_operand_high();
    int no    = get_int_operand_low();
    assert(frame->code);
    assert(frame->code->func_name_id());
    assert(frame->code->upcast<CodeValue>()->pad_list().get());
    // assert(i < frame->code->upcast<CodeValue>()->closure_vars()->size());
// printf("GETCLOSURE: %d, %d, %s\n", level, no, symbol_table->id2name(frame->code->func_name_id()).c_str());
// dump_value(frame->code);
// frame->code->upcast<CodeValue>()->pad_list()->dump(this);
    if (level == 1) {
        SharedPtr<Value> v = frame->code->upcast<CodeValue>()->pad_list()->get(
            no
        );
        stack.push_back(v);
    } else {
        SharedPtr<Value> v = frame->code->upcast<CodeValue>()->pad_list()->get_dynamic(
            level-1, no
        );
        stack.push_back(v);
    }
}

OP_SETCLOSURE {
    // save variable to code->closure_cells[n]
    SharedPtr<LexicalVarsFrame> frame;
    for (auto iter=frame_stack->begin(); iter!=frame_stack->end(); iter++) {
        if ((*iter)->type == FRAME_TYPE_FUNCTION) {
            frame = *iter;
            break;
        }
    }
    assert(frame.get());
    int level = get_int_operand_high();
    int no    = get_int_operand_low();
    SharedPtr<Value> v = stack.back();
    // SharedPtr<Value> dst = (*(frame->code->upcast<CodeValue>()->closure_vars()))[i];
    if (level == 1) {
        frame->code->upcast<CodeValue>()->pad_list()->set(
            no,
            v
        );
    } else {
        frame->code->upcast<CodeValue>()->pad_list()->set_dynamic(
            level-1,
            no,
            v
        );
    }
    // dump_value(dst);
    // *dst = *v;
}

OP_ADD {
#define BINOP(meth) \
    do { \
    SharedPtr<Value> rhs = stack.back(); \
    stack.pop_back(); \
    const SharedPtr<Value>& lhs = stack.back(); \
    \
    Value * ret = meth(lhs, rhs); \
    stack[stack.size()-1].reset(ret); \
    } while (0)

    BINOP(op_add);
}

OP_SUB {
    BINOP(op_sub);
}

OP_DIV {
    BINOP(op_div);
}

OP_MUL {
    BINOP(op_mul);
}

OP_POW {
    BINOP(op_pow);
}

OP_BITAND {
    BINOP(op_bitand);
}

OP_BITOR {
    BINOP(op_bitor);
}

OP_BITXOR {
    BINOP(op_bitxor);
}

OP_BITLSHIFT {
    BINOP(op_bitlshift);
}

OP_BITRSHIFT {
    BINOP(op_bitrshift);
}

OP_MOD {
    BINOP(op_modulo);
}

OP_BUILTIN_FUNCALL {
#undef BINOP

    SharedPtr<Value> funname(stack.back());
    stack.pop_back();

    int argcnt = get_int_operand();
    if (stack.size() >= 1 && stack.back()->value_type == VALUE_TYPE_TUPLE) {
        SharedPtr<TupleValue> t = stack.back()->upcast<TupleValue>();
        argcnt = t->size();
        stack.pop_back();
        this->extract_tuple(t);
    }
    if (!(stack.size() >= (size_t) argcnt)) {
        dump_stack();
        this->die("[BUG] bad argument: %s requires %d arguments but only %zd items available on stack(OP_BUILTINFUNCALL)\n", this->symbol_table->id2name(funname->upcast<SymbolValue>()->id()).c_str(), argcnt, stack.size());
    }
    ID id = funname->upcast<SymbolValue>()->id();
    assert(funname->value_type == VALUE_TYPE_SYMBOL);
    auto iter = this->builtin_functions.find(id);
    assert(iter != this->builtin_functions.end());
    this->call_native_func(iter->second, argcnt);
}

OP_NEXTSTATE {
     // rewind stack
    stack.resize(frame_stack->back()->top);
}

OP_FUNCALL {
    SharedPtr<Value> funname(stack.back());
    stack.pop_back();

    int argcnt = get_int_operand_high();
    if (stack.size() >= 1 && stack.back()->value_type == VALUE_TYPE_TUPLE) {
        SharedPtr<TupleValue> t = stack.back()->upcast<TupleValue>();
        stack.pop_back();
        argcnt = t->size();
        this->extract_tuple(t);
    }
#ifndef NDEBUG
    if (!(stack.size() >= (size_t) argcnt)) {
        dump_stack();
        this->die("[BUG] bad argument: %s requires %d arguments but only %zd items available on stack(OP_FUNCALL)\n", this->symbol_table->id2name(funname->upcast<SymbolValue>()->id()).c_str(), argcnt, stack.size());
    }
#endif
    ID id = funname->upcast<SymbolValue>()->id();
    assert(funname->value_type == VALUE_TYPE_SYMBOL);
    // this->find_package(this->package_id())->dump(this->symbol_table, 1);

// printf("CALLING------ \n");
    // TODO split this part to other op code for performance?
    SharedPtr<Package> pkg = this->package();
    int pkgid = get_int_operand_low();
    if (pkgid != 0) {
        pkg = this->find_package(pkgid);
    }
    auto iter = pkg->find(id);
    if (iter != pkg->end()) {
        CodeValue* code = iter->second->upcast<CodeValue>();

// Disasm::disasm(this->ops);
        SharedPtr<FunctionFrame> fframe(new FunctionFrame(this, argcnt, stack.size(), this->ops));
        fframe->return_address = pc;

        if (code->is_native()) {
            assert(code->callback());
            this->call_native_func(code->callback(), argcnt);
        } else {
            // TODO: vargs support
            // TODO: kwargs support
            int orig_argcnt = argcnt;
            int start_stack_size = stack.size() - argcnt;
            this->ops = code->code_opcodes();

// printf("PARAM count: %zd\n", code->code_params->size());
            for (size_t i=0; i<code->code_params()->size(); i++) {
// printf("PARAMS: %zd\n", i);
                if (argcnt <= i) {
                    int def = code->code_defaults()->at(i);
// printf("default! %d\n", def);
                    if (def >= 0) {
                        pc = def;
// printf("execute! %zd, %d\n", i, pc);
// printf("stack size: %d\n", stack.size());
// Disasm::disasm(this->ops);
                        this->execute();

                        // Note. This is a very bad for performance.
                        // But it works.
                        // We must rewrite argument processing later.
                        SharedPtr<Value> val = stack.back();
                        stack.pop_back();
                        stack.insert(stack.begin() + start_stack_size, val);

// printf("finished stack size: %d\n", stack.size());
                        ++argcnt;
                        fframe->top++;
// printf("execute finished\n", i);
                    } else {
                        throw new ExceptionValue(
                            "ArgumentException: %s::%s needs %d arguments but you passed %d arguments.",
                            this->symbol_table->id2name(pkg->id()).c_str(),
                            this->symbol_table->id2name(funname->upcast<SymbolValue>()->id()).c_str(),
                            code->code_params()->size(),
                            orig_argcnt);
                    }
                }
            }

            fframe->argcnt = argcnt;
            fframe->code = code;

            pc = -1;

            // assert(argcnt == (int)code->code_params->size());
            /**
            *   +------+
            * 0 | arg2 |
            *   | arg1 | <- mark, frame->top
            *   +------+
            */
            mark_stack.push_back(stack.size());
            frame_stack->push_back(fframe);
        }
    } else {
        this->die("Unknown function: %s", this->symbol_table->id2name(funname->upcast<SymbolValue>()->id()).c_str());
        // this->die("Unknown function: %s::%s(%d::%d)", symbol_table->id2name(pkg->id()).c_str(), this->symbol_table->id2name(funname->upcast<SymbolValue>()->id).c_str(), pkg->id(), funname->upcast<SymbolValue>()->id);
    }
}

OP_METHOD_CALL {
    SharedPtr<Value> object(stack.back());
    stack.pop_back();
    SharedPtr<Value> function_id(stack.back());
    stack.pop_back();

    this->call_method(object, function_id);
}

OP_LAST {
    while (1) {
        assert(frame_stack->size() > 0);
        SharedPtr<LexicalVarsFrame> frame(frame_stack->back());
        assert(frame->type != FRAME_TYPE_FUNCTION);
        assert(frame->type != FRAME_TYPE_TRY);
        stack.resize(stack.size());
        frame_stack->pop_back();
        if (frame->type == FRAME_TYPE_WHILE || frame->type == FRAME_TYPE_FOREACH || frame->type == FRAME_TYPE_FOR) {
            pc = get_int_operand();
            break;
        }
    }
}

OP_RETURN {
    assert(mark_stack.size() > 0);
    int top = mark_stack.back();
    SharedPtr<Value> retval;
    // printf("TOP: %zd, %d, %zd\n", mark_stack.size(), top, stack.size());
    // dump_stack();
    if (top < stack.size()) {
        retval.reset(stack.back().get());
        stack.pop_back();
    } else {
        retval.reset(UndefValue::instance());
    }
    mark_stack.pop_back();

    // frame_stack->back()->dump();
    while (1) {
        assert(frame_stack->size() > 0);
        LexicalVarsFrame* frame = frame_stack->back().get();
        if (frame->type == FRAME_TYPE_FUNCTION) {
            SharedPtr<FunctionFrame> fframe = static_cast<FunctionFrame*>(frame);
            pc = fframe->return_address;
            ops = fframe->orig_ops;
            // printf("RETURN :pc: %d\n", pc);

            stack.resize(frame->top - fframe->argcnt);

            frame_stack->pop_back();

            stack.push_back(retval);
            break;
        } else if (frame->type == FRAME_TYPE_TRY) {
            TryFrame* tframe = static_cast<TryFrame*>(frame);
            pc = tframe->return_address;

            stack.resize(frame->top);

            frame_stack->pop_back();

            stack.push_back(retval);
            break;
        } else {
            // printf("THIS IS NOT A FUNCTION FRAME\n");
            frame_stack->pop_back();
        }
    }
}

OP_ENTER {
    SharedPtr<LexicalVarsFrame> frame = new LexicalVarsFrame(this, get_int_operand(), stack.size());
    frame_stack->push_back(frame);
}

OP_ENTER_WHILE {
    SharedPtr<LexicalVarsFrame> frame = new LexicalVarsFrame(this, get_int_operand(), stack.size(), FRAME_TYPE_WHILE);
    frame_stack->push_back(frame);
}

OP_ENTER_FOR {
    SharedPtr<LexicalVarsFrame> frame = new LexicalVarsFrame(this, get_int_operand(), stack.size(), FRAME_TYPE_FOR);
    frame_stack->push_back(frame);
}

OP_LEAVE {
    SharedPtr<LexicalVarsFrame> frame(frame_stack->back());
    stack.resize(frame->top);
    frame_stack->pop_back();
}

OP_PUSH_IDENTIFIER {
    stack.push_back(new SymbolValue(get_int_operand()));
}

OP_DUMP {
    this->dump_stack();
}

OP_JUMP_IF_FALSE {
    const SharedPtr<Value> & v = stack.back();

    if (!v->to_bool()) {
        pc = get_int_operand()-1;
    }
}

OP_JUMP_IF_TRUE {
    const SharedPtr<Value> & v = stack.back();

    if (v->to_bool()) {
        pc = get_int_operand()-1;
    }
}

OP_JUMP {
    pc = get_int_operand()-1;
}

OP_EQ {
    static std::equal_to<int> i;
    static std::equal_to<double> d;
    static std::equal_to<std::string> s;

    const SharedPtr<Value> lhs(stack.back());
    stack.pop_back();
    const SharedPtr<Value> rhs(stack.back());
    stack[stack.size()-1].reset((Value*)BoolValue::instance(cmpop(i, d, s, lhs, rhs)));
}

OP_NE {
    static std::not_equal_to<int> i;
    static std::not_equal_to<double> d;
    static std::not_equal_to<std::string> s;

    const SharedPtr<Value> lhs(stack.back());
    stack.pop_back();
    const SharedPtr<Value> rhs(stack.back());
    stack[stack.size()-1].reset((Value*)BoolValue::instance(cmpop(i, d, s, lhs, rhs)));
}

OP_GT {
    static std::greater<int> i;
    static std::greater<double> d;
    static std::greater<std::string> s;

    const SharedPtr<Value> lhs(stack.back());
    stack.pop_back();
    const SharedPtr<Value> rhs(stack.back());
    stack[stack.size()-1].reset((Value*)BoolValue::instance(cmpop(i, d, s, lhs, rhs)));
}

OP_LT {
    static std::less<int> i;
    static std::less<double> d;
    static std::less<std::string> s;

    const SharedPtr<Value> lhs(stack.back());
    stack.pop_back();
    const SharedPtr<Value> rhs(stack.back());
    stack[stack.size()-1].reset((Value*)BoolValue::instance(cmpop(i, d, s, lhs, rhs)));
}

OP_GE {
    static std::greater_equal<int> i;
    static std::greater_equal<double> d;
    static std::greater_equal<std::string> s;

    const SharedPtr<Value> lhs(stack.back());
    stack.pop_back();
    const SharedPtr<Value> rhs(stack.back());
    stack[stack.size()-1].reset((Value*)BoolValue::instance(cmpop(i, d, s, lhs, rhs)));
}

OP_LE {
    static std::less_equal<int> i;
    static std::less_equal<double> d;
    static std::less_equal<std::string> s;

    const SharedPtr<Value> lhs(stack.back());
    stack.pop_back();
    const SharedPtr<Value> rhs(stack.back());
    stack[stack.size()-1].reset((Value*)BoolValue::instance(cmpop(i, d, s, lhs, rhs)));
}

OP_SETLOCAL {
    const SharedPtr<Value> &rvalue = stack.back();
    if (rvalue->value_type == VALUE_TYPE_TUPLE) {
        frame_stack->back()->set_variable(
            get_int_operand(),
            rvalue->upcast<TupleValue>()->at(rvalue->upcast<TupleValue>()->size()-1)
        );
    } else {
        frame_stack->back()->set_variable(
            get_int_operand(),
            rvalue
        );
    }
}

OP_SETDYNAMIC {
    // frame_stack->back()->dump_vars();
    int level = (get_int_operand() >> 16) & 0x0000FFFF;
    int no    = get_int_operand() & 0x0000ffff;
    DBG("SETDYNAMIC %d, %d\n", level, no);
    // const SharedPtr<LexicalVarsFrame> & frame = frame_stack->at(frame_stack->size()-level-1);
    const SharedPtr<Value> & rvalue = stack.back();
    if (rvalue->value_type == VALUE_TYPE_TUPLE) {
        frame_stack->back()->set_variable_dynamic(
            level,
            no,
            rvalue->upcast<TupleValue>()->at(rvalue->upcast<TupleValue>()->size()-1)
        );
    } else {
        frame_stack->back()->set_variable_dynamic(
            level,
            no,
            rvalue
        );
    }
}

OP_GETDYNAMIC {
    // lexical vars
    int level = (get_int_operand() >> 16) & 0x0000FFFF;
    int no    = get_int_operand() & 0x0000ffff;
    /*
    const SharedPtr<LexicalVarsFrame> & frame = frame_stack->at(frame_stack->size()-level-1);
    SharedPtr<Value>v = frame->get_variable(no);
    */
    SharedPtr<Value> v = frame_stack->back()->get_variable_dynamic(level, no);

    // dump_frame();
    //// printf("DUMP: %d, %d, %s, %zd\n", level, no, frame->type_str(), frame->vars.size());
    stack.push_back(v);
}

OP_GETLOCAL {
    // lexical vars
    SharedPtr<Value>v = frame_stack->back()->get_variable(get_int_operand());
    stack.push_back(v);
}

OP_GET_ITEM {
    SharedPtr<Value> index(stack.back());
    stack.pop_back();
    const SharedPtr<Value> & container = stack.back();

    SharedPtr<Value> ret(op_get_item(container, index));
    stack.back().reset(ret.get());
}

OP_SET_ITEM {
    SharedPtr<Value> index(stack.back());
    stack.pop_back();
    SharedPtr<Value> container(stack.back());
    stack.pop_back();
    SharedPtr<Value> rvalue(stack.back());
    stack.pop_back();

    SharedPtr<Value> ret = this->set_item(container, index, rvalue);
    if (ret->is_exception()) {
        this->die(ret);
    } else {
        stack.push_back(rvalue);
    }
}

OP_UNARY_NEGATIVE {
    const SharedPtr<Value>& v = stack.back();
    stack[stack.size()-1].reset(op_unary_negative(v));
}

OP_MAKE_HASH {
    SharedPtr<HashValue> h = new HashValue();
    int size = get_int_operand();
    for (int i=0; i<size; i+=2) {
        SharedPtr<StrValue> k = stack.back()->to_s();
        stack.pop_back();
        SharedPtr<Value> v = stack.back(); stack.pop_back();
        h->set(k->str_value(), v);
    }
    stack.push_back(h);
}

OP_MAKE_ARRAY {
    SharedPtr<ArrayValue> a = new ArrayValue();
    int array_size = get_int_operand();
    for (int i=0; i<array_size; i++) {
        SharedPtr<Value> v = stack.back(); stack.pop_back();
        a->push(v);
    }
    stack.push_back(a);
}

OP_MAKE_TUPLE {
    SharedPtr<TupleValue> t = new TupleValue();
    int tuple_size = get_int_operand();
    for (int i=0; i<tuple_size; i++) {
        SharedPtr<Value> v = stack.back(); stack.pop_back();
        t->push(v);
    }
    stack.push_back(t);
}

OP_EXTRACT_TUPLE {
    SharedPtr<TupleValue> t = stack.back()->upcast<TupleValue>();
    stack.pop_back();
    this->extract_tuple(t);
}

OP_PRE_INCREMENT {
    // ++$i
    SharedPtr<Value> i = stack.back();
    stack.pop_back();
    // printf("stack %zd\n", stack.size());
    if (i->value_type == VALUE_TYPE_INT) {
        i->upcast<IntValue>()->tora__incr__();
        stack.push_back(i);
    } else {
        this->die("%s does not supports increment operator.\n", i->type_str());
    }
}

OP_POST_INCREMENT {
    // $i++
    SharedPtr<Value> i = stack.back(); stack.pop_back();
    if (i->value_type == VALUE_TYPE_INT) {
        SharedPtr<IntValue>ii = i->upcast<IntValue>();
        SharedPtr<IntValue>tmp = ii->clone();
        ii->tora__incr__();
        stack.push_back(tmp);
    } else {
        this->die("%s does not supports post increment operator.\n", i->type_str());
    }
}

OP_PRE_DECREMENT {
    // --$i
    SharedPtr<Value> i = stack.back(); stack.pop_back();
    if (i->value_type == VALUE_TYPE_INT) {
        i->upcast<IntValue>()->tora__decr__();
        stack.push_back(i);
    } else {
        this->die("%s does not supports decrement operator.\n", i->type_str());
    }
}

OP_POST_DECREMENT {
    // $i--
    SharedPtr<Value> i = stack.back(); stack.pop_back();
    if (i->value_type == VALUE_TYPE_INT) {
        SharedPtr<IntValue>ii = i->upcast<IntValue>();
        SharedPtr<IntValue>tmp = ii->clone();
        ii->tora__decr__();
        stack.push_back(tmp);
    } else {
        this->die("%s does not supports post decrement operator.\n", i->type_str());
    }
}

OP_FILE_TEST {
    // -f $file
    SharedPtr<StrValue> fname = stack.back()->to_s();
    stack.pop_back();
    struct stat buf;
    switch (get_int_operand()) {
    case 'f':
        stack.push_back(new BoolValue(stat(fname->c_str(), &buf)==0 && buf.st_mode & S_IFREG));
        break;
    case 'd':
        stack.push_back(new BoolValue(stat(fname->c_str(), &buf)==0 && buf.st_mode & S_IFDIR));
        break;
    case 'x':
        stack.push_back(new BoolValue(stat(fname->c_str(), &buf)==0 && buf.st_mode & S_IXUSR));
        break;
    case 'e':
        stack.push_back(new BoolValue(stat(fname->c_str(), &buf)==0));
        break;
    case 's':
        if (stat(fname->c_str(), &buf) == 0) {
            stack.push_back(new IntValue(buf.st_size));
        } else {
            stack.push_back(UndefValue::instance());
        }
        break;
    default:
        printf("[BUG] Unknown operand: %c\n", get_int_operand());
        abort();
    }
}

OP_GETGLOBAL {
    int globalvarno = get_int_operand();
    stack.push_back(this->global_vars->at(globalvarno));
}

OP_POP_TOP {
    stack.pop_back();
}

OP_ENTER_FOREACH {
    SharedPtr<Value>a = stack.back();
    stack.pop_back();
    SharedPtr<Value> iiter;
    if (a->value_type == VALUE_TYPE_ARRAY) {
        SharedPtr<ArrayValue::iterator> iter = new ArrayValue::iterator();
        iter->parent = a->upcast<ArrayValue>();
        iter->counter = 0;
        iiter = iter;
    } else if (a->value_type == VALUE_TYPE_RANGE) {
        SharedPtr<RangeValue::Iterator> iter = new RangeValue::Iterator(a->upcast<RangeValue>());
        iiter = iter;
    } else if (a->value_type == VALUE_TYPE_HASH) {
        SharedPtr<HashValue::iterator> iter = new HashValue::iterator(a->upcast<HashValue>());
        iiter = iter;
    } else if (a->value_type == VALUE_TYPE_OBJECT) {
        ObjectValue * o = a->upcast<ObjectValue>();
        Package *pkg = this->find_package(o->package_id());
        auto iter = pkg->find(this->symbol_table->get_id("__iter__"));
        if (iter != pkg->end()) {
            CodeValue * code = iter->second->upcast<CodeValue>();
            if (code->is_native()) {
                stack.push_back(a);
               this->call_native_func(code->callback(), 0);
               iiter = stack.back();
               stack.pop_back();
               if (iiter->value_type == VALUE_TYPE_EXCEPTION) {
                    this->die(iiter);
                    TODO();
               }
            } else {
                TODO();
            }
        } else {
            this->die("This object doesn't support iteration\n");
            abort();
        }
    } else {
        this->die("This object doesn't support iteration\n");
        abort();
    }

    int vars_cnt = get_int_operand();
    ForeachFrame* fframe = new ForeachFrame(this, vars_cnt, stack.size());
    fframe->iter = iiter;
    frame_stack->push_back(fframe);
}

OP_FOR_ITER {
    auto fframe = frame_stack->back()->upcast<ForeachFrame>();
    assert(fframe->type == FRAME_TYPE_FOREACH);
    stack.resize(fframe->top);
    const SharedPtr<Value> & iter = fframe->iter;
    switch (iter->value_type) {
    case VALUE_TYPE_ARRAY_ITERATOR: {
        SharedPtr<ArrayValue::iterator> aiter = iter->upcast<ArrayValue::iterator>();
        if (aiter->counter < aiter->parent->size()) {
            SharedPtr<Value> val = aiter->parent->at(aiter->counter);
            aiter->counter++;
            SharedPtr<TupleValue> t = new TupleValue();
            t->push(val);
            stack.push_back(t);
        } else {
            stack.push_back(new StopIterationExceptionValue());
        }
        break;
    }
    case VALUE_TYPE_RANGE_ITERATOR: {
        RangeValue::Iterator* riter = static_cast<RangeValue::Iterator*>(iter.get());
        if (riter->counter <= riter->parent->right()->int_value()) {
            const SharedPtr<Value> & cur = fframe->current_value;
            if (cur.get() && cur->refcnt == 2) {
                // value is not copied.
                assert(cur->value_type == VALUE_TYPE_INT);
                cur->upcast<IntValue>()->int_value(riter->counter);
                stack.push_back(cur);
            } else {
                IntValue * iv = new IntValue(riter->counter);
                stack.push_back(iv);
                fframe->current_value.reset(iv);
            }
            // fframe->current_value.reset(iv);
            riter->counter++;
        } else {
            stack.push_back(new StopIterationExceptionValue());
        }
        break;
    }
    case VALUE_TYPE_HASH_ITERATOR: {
        SharedPtr<HashValue::iterator> riter = iter->upcast<HashValue::iterator>();
        if (!riter->finished()) {
            SharedPtr<Value> key = riter->getkey();
            SharedPtr<Value> val = riter->getval();
            riter->increment();
            SharedPtr<TupleValue> t = new TupleValue();
            t->push(val);
            t->push(key);
            stack.push_back(t);
        } else {
            stack.push_back(new StopIterationExceptionValue());
        }
        break;
    }
    case VALUE_TYPE_OBJECT: {
        ObjectValue * o = iter->upcast<ObjectValue>();
        Package *pkg = this->find_package(o->package_id());
        auto iter = pkg->find(this->symbol_table->get_id("__next__"));
        if (iter != pkg->end()) {
            CodeValue * code = iter->second->upcast<CodeValue>();
            if (code->is_native()) {
                stack.push_back(o);
               this->call_native_func(code->callback(), 0);
               SharedPtr<Value> val = stack.back();
               stack.pop_back();
               if (val->value_type == VALUE_TYPE_EXCEPTION) {
                    if (val->upcast<ExceptionValue>()->exception_type() == EXCEPTION_TYPE_STOP_ITERATION) {
                        stack.push_back(val);
                    } else {
                        this->die(val);
                    }
               } else {
                    stack.push_back(val);
               }
            } else {
                TODO();
            }
        } else {
            this->die("'%s' does not have a '__next__' method.", symbol_table->id2name(o->package_id()).c_str());
        }
        break;
    }
    default:
        this->die("[BUG] TOS is not a iterator object: %s.", iter->type_str());
    }
}

OP_JUMP_IF_STOP_EXCEPTION {
    const SharedPtr<Value> &top = stack.back();
    if (top->value_type == VALUE_TYPE_EXCEPTION && top->upcast<ExceptionValue>()->exception_type() == EXCEPTION_TYPE_STOP_ITERATION) {
        pc = get_int_operand()-1;
    }
}

OP_DOTDOTDOT {
    fprintf(stderr, "This is not implemented yet.\n");
    exit(1);
}

OP_TRY {
    TryFrame* fframe = new TryFrame(this, stack.size());
    fframe->return_address = get_int_operand()-1;
    frame_stack->push_back(fframe);
    mark_stack.push_back(stack.size());
}

OP_DIE {
    SharedPtr<Value> exception = stack.back();
    stack.pop_back();
    throw exception;
}

OP_USE {
    SharedPtr<Value> mod_name = stack.back();
    stack.pop_back();
    SharedPtr<Value> include  = stack.back();
    stack.pop_back();
    SharedPtr<Value> ret = this->require(mod_name.get());
    if (ret->is_exception()) {
        this->die(ret);
    } else {
        if (include->value_type == VALUE_TYPE_INT && include->upcast<IntValue>()->int_value() == 1) {
            SharedPtr<Value> v = this->copy_all_public_symbols(mod_name->upcast<SymbolValue>()->id(), this->package_id());
            if (v->is_exception()) {
                this->die(ret);
            }
        }
        stack.push_back(ret);
    }
}

OP_NOT {
    SharedPtr<Value> v = stack.back();
    stack.pop_back();
    stack.push_back(new BoolValue(!v->to_bool()));
}

OP_PACKAGE_ENTER {
    ID package_id = get_int_operand();
    // printf("entering %s(%d)\n", symbol_table->id2name(package_id).c_str(), package_id);
    PackageFrame* pframe = new PackageFrame(this, stack.size(), this->package_id());
    pframe->top = stack.size();
    this->frame_stack->push_back(pframe);
    this->package_id(package_id);
}

OP_PACKAGE_LEAVE {
    assert(this->frame_stack->back()->type == FRAME_TYPE_PACKAGE);
    this->frame_stack->pop_back();
}

OP_DEREF {
    // ${self}
    // dereference object
    const SharedPtr<Value> & obj = stack.back();
    if (obj->value_type != VALUE_TYPE_OBJECT) {
        throw new ExceptionValue("You cannot dereference no object value.");
    }
    stack[stack.size()-1].reset(obj->upcast<ObjectValue>()->data().get());
}

OP_SET_PACKAGE_VARIABLE {
    // $Foo::Bar = 3;
    SharedPtr<Value> pkgid = stack.back();
    stack.pop_back();

    SharedPtr<Value> rval = stack.back();
    stack.pop_back();

    int vid = get_int_operand();
    Package *pkg = find_package(pkgid->upcast<SymbolValue>()->id());
    pkg->set_variable(vid, rval);

    stack.push_back(rval);
}

OP_GET_PACKAGE_VARIABLE {
    // $Foo::Bar = 3;
    SharedPtr<Value> pkgid = stack.back();
    stack.pop_back();

    int vid = get_int_operand();
    Package *pkg = find_package(pkgid->upcast<SymbolValue>()->id());
    auto iter = pkg->find(vid);
    SharedPtr<Value> ret(iter!=pkg->end() ? iter->second : UndefValue::instance());
    stack.push_back(ret);
}

OP_LOCAL {
    /**
     * stacks: varname, 
     */

    // I know it's very slow. but dynamic scope is not important part in tora interpreter.
    // If you want to improvents performance, patches welcome.
    // TODO: optimize
    const SharedPtr<Value> & snode = stack.back();
    assert(snode->value_type == VALUE_TYPE_STR);

    std::string package;
    std::string moniker;
    split_package_varname(snode->upcast<StrValue>()->str_value(), package, moniker);

    ID pkgid = symbol_table->get_id(package);
    ID vid = symbol_table->get_id(std::string("$") + moniker);

    Package *pkg = find_package(pkgid);
    auto iter = pkg->find(vid);

    SharedPtr<Value> ret(iter!=pkg->end() ? iter->second : UndefValue::instance());
    frame_stack->back()->push_dynamic_scope_var(pkg, vid, ret);
    pkg->set_variable(vid, UndefValue::instance());

    stack.pop_back();
}

OP_SET_PARENT {
    const SharedPtr<Package> & pkg = this->package();
    Package * super = this->find_package(get_int_operand());
    if (super == pkg.get()) {
        throw new ExceptionValue("You cannot make itself as superclass.");
    }
    assert(super);
    pkg->superclass(super);
    assert(pkg->superclass());
}

