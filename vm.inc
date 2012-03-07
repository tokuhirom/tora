/* vim: set filetype=cpp: */

OP_PUSH_TRUE {
    stack.push_back(new BoolValue(true));
}

OP_PUSH_FALSE {
    stack.push_back(new BoolValue(false));
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
    stack.push_back(stack.at(mark_stack.back() - no));
}

OP_SETARG {
    int no = get_int_operand();
    SharedPtr<Value> rvalue = stack.back();
    if (rvalue->value_type == VALUE_TYPE_TUPLE) {
        rvalue = rvalue->upcast<TupleValue>()->at(rvalue->upcast<TupleValue>()->size()-1);
    }
    stack[mark_stack.back() - no].reset(rvalue.get());
}

OP_PUSH_VALUE {
    const OP * op = ops->at(pc);
    SharedPtr<Value> v = ((ValueOP*)&(*(op)))->value;
    stack.push_back(v);
}

OP_NEW_RANGE {
    SharedPtr<Value> l = stack.back();
    stack.pop_back();
    SharedPtr<Value> r = stack.back(); stack.pop_back();
    stack.push_back(new RangeValue(l->upcast<IntValue>(), r->upcast<IntValue>()));
}

OP_FUNCDEF {
    SharedPtr<Value> code = stack.back(); stack.pop_back(); // code object
    assert(code->value_type == VALUE_TYPE_CODE);
    // printf("FUNCDEF!! %d, %d\n", package_id(), code->upcast<CodeValue>()->func_name_id);
    this->add_function(code->upcast<CodeValue>()->func_name_id, code);
}

OP_CLOSUREDEF {
    SharedPtr<Value> code = stack.back(); stack.pop_back(); // code object
    // take variables from stack. and push it to code->closure_cells.
    assert(code->value_type == VALUE_TYPE_CODE);
    for (int i=0; i<get_int_operand(); i++) {
        SharedPtr<Value> v = stack.back(); stack.pop_back();
        code->upcast<CodeValue>()->closure_vars->push_back(v);
    }
    // printf("CLOSUREDEF!! %d, %d\n", package_id(), code->upcast<CodeValue>()->func_name_id);
    this->add_function(code->upcast<CodeValue>()->func_name_id, code);
}

OP_GETCLOSURE {
    // take variable from code->closure_cells[n]

    LexicalVarsFrame* frame = NULL;
    for (auto iter=frame_stack->rbegin(); iter!=frame_stack->rend(); iter++) {
        if ((*iter)->type == FRAME_TYPE_FUNCTION) {
            frame = *iter;
            break;
        }
    }
    assert(frame);
    int i = get_int_operand();
    assert(frame->code);
    assert(frame->code->func_name_id);
    assert(i < frame->code->upcast<CodeValue>()->closure_vars->size());
    SharedPtr<Value> v = frame->code->upcast<CodeValue>()->closure_vars->at(i);
    stack.push_back(v);
}

OP_SETCLOSURE {
    // save variable to code->closure_cells[n]
    LexicalVarsFrame* frame = NULL;
    for (auto iter=frame_stack->begin(); iter!=frame_stack->end(); iter++) {
        if ((*iter)->type == FRAME_TYPE_FUNCTION) {
            frame = *iter;
            break;
        }
    }
    assert(frame);
    int i = get_int_operand();
    SharedPtr<Value> v = stack.back();
    SharedPtr<Value> dst = (*(frame->code->upcast<CodeValue>()->closure_vars))[i];
    *dst = *v;
}

OP_ADD {
    SharedPtr<Value> rhs = stack.back();
    stack.pop_back();
    const SharedPtr<Value>& lhs = stack.back();

    Value * ret = this->op_add(lhs, rhs);
    stack[stack.size()-1].reset(ret);
}

OP_SUB {
    SharedPtr<Value> rhs = stack.back();
    stack.pop_back();
    const SharedPtr<Value>& lhs = stack.back();

    Value * ret = this->op_sub(lhs, rhs);
    stack[stack.size()-1].reset(ret);
}

OP_DIV {
    SharedPtr<Value> rhs(stack.back()); /* rvalue */
    stack.pop_back();
    const SharedPtr<Value>& lhs = stack.back();

    Value * ret = this->op_div(lhs, rhs);
    stack[stack.size()-1].reset(ret);
}

OP_MUL {
    SharedPtr<Value> rhs(stack.back()); /* rvalue */
    stack.pop_back();
    const SharedPtr<Value>& lhs = stack.back();

    Value * ret = this->op_mul(lhs, rhs);
    stack[stack.size()-1].reset(ret);
}

OP_POW {
    SharedPtr<Value> rhs = stack.back();
    stack.pop_back();
    const SharedPtr<Value>& lhs = stack.back();

    Value * ret = this->op_pow(lhs, rhs);
    stack[stack.size()-1].reset(ret);
}

OP_BUILTIN_FUNCALL {
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
        printf("[BUG] bad argument: %s requires %d arguments but only %zd items available on stack(OP_FUNCALL)\n", this->symbol_table->id2name(funname->upcast<SymbolValue>()->id).c_str(), argcnt, stack.size());
        dump_stack();
        abort();
    }
    ID id = funname->upcast<SymbolValue>()->id;
    assert(funname->value_type == VALUE_TYPE_SYMBOL);
    auto iter = this->builtin_functions.find(id);
    assert(iter != this->builtin_functions.end());
    this->call_native_func(iter->second, argcnt);
    if (stack.back()->value_type == VALUE_TYPE_EXCEPTION) {
        SharedPtr<Value> v = stack.back();
        stack.pop_back();
        this->die(v);
    }
}

OP_NEXTSTATE {
     // rewind stack
    stack.resize(frame_stack->back()->top);
}

OP_FUNCALL {
    SharedPtr<Value> funname(stack.back());
    stack.pop_back();

    int argcnt = get_int_operand();
    if (stack.size() >= 1 && stack.back()->value_type == VALUE_TYPE_TUPLE) {
        SharedPtr<TupleValue> t = stack.back()->upcast<TupleValue>();
        stack.pop_back();
        argcnt = t->size();
        this->extract_tuple(t);
    }
#ifndef NDEBUG
    if (!(stack.size() >= (size_t) argcnt)) {
        printf("[BUG] bad argument: %s requires %d arguments but only %zd items available on stack(OP_FUNCALL)\n", this->symbol_table->id2name(funname->upcast<SymbolValue>()->id).c_str(), argcnt, stack.size());
        dump_stack();
        abort();
    }
#endif
    ID id = funname->upcast<SymbolValue>()->id;
    assert(funname->value_type == VALUE_TYPE_SYMBOL);
    // this->find_package(this->package_id())->dump(this->symbol_table, 1);

    const SharedPtr<Package>& pkg = this->package();
    auto iter = pkg->find(id);
    if (iter != pkg->end()) {
        CodeValue* code = iter->second->upcast<CodeValue>();
        // printf("calling %s\n", funname->upcast<StrValue>()->str_value.c_str());

        FunctionFrame* fframe = new FunctionFrame(argcnt, stack.size(), this->ops);
        fframe->return_address = pc;
        fframe->argcnt = argcnt;
        fframe->code = code;

        pc = -1;
        this->ops = code->code_opcodes;

        // TODO: vargs support
        // TODO: kwargs support
        assert(argcnt == (int)code->code_params->size());
        /**
         *   +------+
         * 0 | arg2 |
         *   | arg1 | <- mark, frame->top
         *   +------+
         */
        mark_stack.push_back(stack.size()-1);
        frame_stack->push_back(fframe);
    } else {
        this->die("Unknown function: %s", this->symbol_table->id2name(funname->upcast<SymbolValue>()->id).c_str());
        // this->die("Unknown function: %s::%s(%d::%d)", symbol_table->id2name(pkg->id()).c_str(), this->symbol_table->id2name(funname->upcast<SymbolValue>()->id).c_str(), pkg->id(), funname->upcast<SymbolValue>()->id);
    }
}

OP_METHOD_CALL {
    SharedPtr<Value> object(stack.back());
    stack.pop_back();
    SharedPtr<Value> funid(stack.back());
    stack.pop_back();
    if (!(stack.size() >= (size_t) get_int_operand())) {
        // printf("[BUG] bad argument: %s requires %d arguments but only %zd items available on stack(OP_FUNCALL)\n", funname_c, get_int_operand(), stack.size());
        dump_stack();
        abort();
    }
    assert(funid->value_type == VALUE_TYPE_SYMBOL);

    switch (object->value_type) {
    case VALUE_TYPE_SYMBOL: {
        // Foo.new();
        SharedPtr<Package> pkg = this->find_package(object->upcast<SymbolValue>()->id);
        auto iter = pkg->find(funid->upcast<SymbolValue>()->id);
        if (iter != pkg->end()) {
            SharedPtr<Value>code_v = iter->second;
            assert(code_v->value_type == VALUE_TYPE_CODE);
            SharedPtr<CodeValue> code = code_v->upcast<CodeValue>();
            int argcnt = get_int_operand();
            if (code->is_native()) {
                // FunctionFrame* fframe = new FunctionFrame(argcnt, stack.size()-argcnt);
                // fframe->return_address = pc;
                // fframe->top = stack.size() - argcnt;
                // fframe->code = code;

                // frame_stack->push_back(fframe);
                stack.push_back(object);
                this->call_native_func(code->callback(), argcnt+1);
                // delete fframe;
                // frame_stack->pop_back();
            } else {
                {
                    FunctionFrame* fframe = new FunctionFrame(argcnt, stack.size(), ops);
                    fframe->return_address = pc;
                    fframe->code = code;
                    fframe->argcnt = argcnt;

                    pc = -1;
                    this->ops = code->code_opcodes;

                    // TODO: vargs support
                    // TODO: kwargs support
                    assert(argcnt == (int)code->code_params->size());
                    mark_stack.push_back(stack.size()-1);
                    frame_stack->push_back(fframe);
                }
            }
        } else {
            if (funid->upcast<SymbolValue>()->id == this->symbol_table->get_id("new")) {
                // process parameters
                stack.push_back(new ObjectValue(object->upcast<SymbolValue>()->id, this));
            } else {
                this->die("Unknown method %s for %s\n", this->symbol_table->id2name(funid->upcast<SymbolValue>()->id).c_str(), this->symbol_table->id2name(object->upcast<SymbolValue>()->id).c_str());
            }
        }
        break;
    }
    case VALUE_TYPE_STR:
    case VALUE_TYPE_CODE:
    case VALUE_TYPE_ARRAY:
    case VALUE_TYPE_FILE:
    case VALUE_TYPE_OBJECT:{
        ID pkgid;
        if (object->value_type == VALUE_TYPE_STR) {
            pkgid = this->symbol_table->get_id("String");
        } else if (object->value_type == VALUE_TYPE_ARRAY) {
            pkgid = this->symbol_table->get_id("Array");
        } else if (object->value_type == VALUE_TYPE_FILE) {
            pkgid = this->symbol_table->get_id("File");
        } else if (object->value_type == VALUE_TYPE_CODE) {
            pkgid = this->symbol_table->get_id("Code");
        } else if (object->value_type == VALUE_TYPE_OBJECT) {
            pkgid = object->upcast<ObjectValue>()->package_id();
        } else {
            abort();
        }
        SharedPtr<Package> pkg = this->find_package(pkgid);
        auto iter = pkg->find(funid->upcast<SymbolValue>()->id);
        if (iter != pkg->end()) {
            SharedPtr<Value>code_v = iter->second;
            assert(code_v->value_type == VALUE_TYPE_CODE);
            SharedPtr<CodeValue> code = code_v->upcast<CodeValue>();
            int argcnt = get_int_operand();

            if (code->is_native()) {
                // FunctionFrame* fframe = new FunctionFrame(argcnt, stack.size()-argcnt);
                // fframe->return_address = pc;
                // fframe->code = code;

                // frame_stack->push_back(fframe);
                stack.push_back(object);
                this->call_native_func(code->callback(), argcnt+1);
                // delete fframe;
                // frame_stack->pop_back();
            } else {
                FunctionFrame* fframe = new FunctionFrame(argcnt, stack.size(), ops);
                fframe->return_address = pc;
                fframe->argcnt = argcnt;
                fframe->code = code;
                fframe->self = object;

                pc = -1;
                this->ops = code->code_opcodes;

                // TODO: vargs support
                // TODO: kwargs support
                assert(argcnt == (int)code->code_params->size());
                mark_stack.push_back(stack.size()-1);
                frame_stack->push_back(fframe);
            }
        } else {
            funid->dump(1);
            object->dump(1);
            this->die("Unknown method %s for %s\n", this->symbol_table->id2name(funid->upcast<SymbolValue>()->id).c_str(), this->symbol_table->id2name(object->upcast<ObjectValue>()->package_id()).c_str());
        }
        break;
    }
    default:
        this->die("Unknown method %s for %s\n", this->symbol_table->id2name(funid->upcast<SymbolValue>()->id).c_str(), object->type_str());
        break;
    }
}

OP_LAST {
    while (1) {
        assert(frame_stack->size() > 0);
        boost::scoped_ptr<LexicalVarsFrame> frame(frame_stack->back());
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
    SharedPtr<Value> retval = stack.back();
    stack.pop_back();

    mark_stack.pop_back();

    // frame_stack->back()->dump();
    while (1) {
        assert(frame_stack->size() > 0);
        boost::scoped_ptr<LexicalVarsFrame> frame(frame_stack->back());
        if (frame->type == FRAME_TYPE_FUNCTION) {
            FunctionFrame* fframe = static_cast<FunctionFrame*>(frame.get());
            pc = fframe->return_address;
            ops = fframe->orig_ops;
            // printf("RETURN :orig: %d, current: %d\n", fframe->top, stack.size());

            stack.resize(frame->top - fframe->argcnt);

            frame_stack->pop_back();

            stack.push_back(retval);
            break;
        } else if (frame->type == FRAME_TYPE_TRY) {
            TryFrame* tframe = static_cast<TryFrame*>(frame.get());
            pc = tframe->return_address;
            // printf("RETURN :orig: %d, current: %d\n", tframe->top, stack.size());

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
    LexicalVarsFrame* frame = new LexicalVarsFrame(get_int_operand(), stack.size());
    frame_stack->push_back(frame);
}

OP_ENTER_WHILE {
    LexicalVarsFrame* frame = new LexicalVarsFrame(get_int_operand(), stack.size(), FRAME_TYPE_WHILE);
    frame_stack->push_back(frame);
}

OP_ENTER_FOR {
    LexicalVarsFrame* frame = new LexicalVarsFrame(get_int_operand(), stack.size(), FRAME_TYPE_FOR);
    frame_stack->push_back(frame);
}

OP_LEAVE {
    boost::scoped_ptr<LexicalVarsFrame> frame(frame_stack->back());
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
    stack[stack.size()-1].reset((Value*)BoolValue::instance(this->cmpop(i, d, s, lhs, rhs)));
}

OP_NE {
    static std::not_equal_to<int> i;
    static std::not_equal_to<double> d;
    static std::not_equal_to<std::string> s;

    const SharedPtr<Value> lhs(stack.back());
    stack.pop_back();
    const SharedPtr<Value> rhs(stack.back());
    stack[stack.size()-1].reset((Value*)BoolValue::instance(this->cmpop(i, d, s, lhs, rhs)));
}

OP_GT {
    static std::greater<int> i;
    static std::greater<double> d;
    static std::greater<std::string> s;

    const SharedPtr<Value> lhs(stack.back());
    stack.pop_back();
    const SharedPtr<Value> rhs(stack.back());
    stack[stack.size()-1].reset((Value*)BoolValue::instance(this->cmpop(i, d, s, lhs, rhs)));
}

OP_LT {
    static std::less<int> i;
    static std::less<double> d;
    static std::less<std::string> s;

    const SharedPtr<Value> lhs(stack.back());
    stack.pop_back();
    const SharedPtr<Value> rhs(stack.back());
    stack[stack.size()-1].reset((Value*)BoolValue::instance(this->cmpop(i, d, s, lhs, rhs)));
}

OP_GE {
    static std::greater_equal<int> i;
    static std::greater_equal<double> d;
    static std::greater_equal<std::string> s;

    const SharedPtr<Value> lhs(stack.back());
    stack.pop_back();
    const SharedPtr<Value> rhs(stack.back());
    stack[stack.size()-1].reset((Value*)BoolValue::instance(this->cmpop(i, d, s, lhs, rhs)));
}

OP_LE {
    static std::less_equal<int> i;
    static std::less_equal<double> d;
    static std::less_equal<std::string> s;

    const SharedPtr<Value> lhs(stack.back());
    stack.pop_back();
    const SharedPtr<Value> rhs(stack.back());
    stack[stack.size()-1].reset((Value*)BoolValue::instance(this->cmpop(i, d, s, lhs, rhs)));
}

OP_SETLOCAL {
    SharedPtr<Value> rvalue = stack.back();
    if (rvalue->value_type == VALUE_TYPE_TUPLE) {
        frame_stack->back()->setVar(
            get_int_operand(),
            rvalue->upcast<TupleValue>()->at(rvalue->upcast<TupleValue>()->size()-1)
        );
    } else {
        frame_stack->back()->setVar(
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
    LexicalVarsFrame* frame = frame_stack->at(frame_stack->size()-level-1);
    SharedPtr<Value> rvalue = stack.back();
    if (rvalue->value_type == VALUE_TYPE_TUPLE) {
        frame->setVar(
            no,
            rvalue->upcast<TupleValue>()->at(rvalue->upcast<TupleValue>()->size()-1)
        );
    } else {
        frame->setVar(
            no,
            rvalue
        );
    }
}

OP_GETDYNAMIC {
    // lexical vars
    int level = (get_int_operand() >> 16) & 0x0000FFFF;
    int no    = get_int_operand() & 0x0000ffff;
    LexicalVarsFrame* frame = frame_stack->at(frame_stack->size()-level-1);
    SharedPtr<Value>v = frame->find(no);
    if (v) {
        DBG2("found lexical var\n");
        stack.push_back(v);
    } else { // TODO: remove this and use 'my' keyword?
        DBG2("There is no variable...\n");
        v = UndefValue::instance();
        frame_stack->back()->setVar(get_int_operand(), v);
        stack.push_back(v);
    }
}

OP_GETLOCAL {
    // lexical vars
    SharedPtr<Value>v = frame_stack->back()->find(get_int_operand());
    if (v) {
        // printf("found lexical var\n");
        stack.push_back(v);
    } else { // TODO: remove this and use 'my' keyword?
        v = UndefValue::instance();
        frame_stack->back()->setVar(get_int_operand(), v);
        stack.push_back(v);
    }
}

OP_GET_ITEM {
    SharedPtr<Value> index(stack.back());
    stack.pop_back();
    SharedPtr<Value> container(stack.back());
    stack.pop_back();

    SharedPtr<Value> ret = container->get_item(&(*index));
    if (ret->is_exception()) { this->die(ret); }
    else { stack.push_back(ret); }
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
    const SharedPtr<Value>& v = TOP();
    SharedPtr<Value> r = this->unary_negative(v);
    if (r->is_exception()) { this->die(r); }
    stack.pop_back();
    stack.push_back(r);
}

OP_MAKE_HASH {
    SharedPtr<HashValue> h = new HashValue();
    int size = get_int_operand();
    for (int i=0; i<size; i+=2) {
        SharedPtr<StrValue> k = stack.back()->to_s();
        stack.pop_back();
        SharedPtr<Value> v = stack.back(); stack.pop_back();
        h->set(k->str_value, v);
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
        SharedPtr<RangeValue::iterator> iter = new RangeValue::iterator(a->upcast<RangeValue>());
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
    ForeachFrame* fframe = new ForeachFrame(vars_cnt, stack.size());
    fframe->iter = iiter;
    frame_stack->push_back(fframe);
}

OP_FOR_ITER {
    // TOS is an iterator. Call its next() method. If this yields a new value, push it on the stack (leaving the iterator below it). If the iterator indicates it is exhausted TOS is popped, and the byte code counter is incremented by delta.

    auto fframe = frame_stack->back()->upcast<ForeachFrame>();
    assert(fframe->type == FRAME_TYPE_FOREACH);
    stack.resize(fframe->top);
    SharedPtr<Value> iter = fframe->iter;
    if (iter->value_type == VALUE_TYPE_ARRAY_ITERATOR) {
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
    } else if (iter->value_type == VALUE_TYPE_RANGE_ITERATOR) {
        SharedPtr<RangeValue::iterator> riter = iter->upcast<RangeValue::iterator>();
        if (riter->counter <= riter->parent->right->int_value) {
            SharedPtr<IntValue> val = new IntValue(riter->counter);
            riter->counter++;
            stack.push_back(val);
        } else {
            stack.push_back(new StopIterationExceptionValue());
        }
    } else if (iter->value_type == VALUE_TYPE_HASH_ITERATOR) {
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
    } else if (iter->value_type == VALUE_TYPE_OBJECT) {
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
                    if (val->upcast<ExceptionValue>()->exception_type == EXCEPTION_TYPE_STOP_ITERATION) {
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
    } else {
        this->die("[BUG] TOS is not a iterator object: %s.", iter->type_str());
    }
}

OP_JUMP_IF_STOP_EXCEPTION {
    SharedPtr<Value> top = stack.back();
    if (top->value_type == VALUE_TYPE_EXCEPTION && top->upcast<ExceptionValue>()->exception_type == EXCEPTION_TYPE_STOP_ITERATION) {
        pc = get_int_operand()-1;
    }
}

OP_DOTDOTDOT {
    fprintf(stderr, "This is not implemented yet.\n");
    exit(1);
}

OP_TRY {
    TryFrame* fframe = new TryFrame(stack.size());
    fframe->return_address = get_int_operand()-1;
    frame_stack->push_back(fframe);
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
        if (include->value_type == VALUE_TYPE_INT && include->upcast<IntValue>()->int_value == 1) {
            SharedPtr<Value> v = this->copy_all_public_symbols(mod_name->upcast<SymbolValue>()->id, this->package_id());
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
    PackageFrame* pframe = new PackageFrame(stack.size(), this->package_id(), this);
    pframe->top = stack.size();
    this->frame_stack->push_back(pframe);
    this->package_id_ = package_id;
}

OP_PACKAGE_LEAVE {
    assert(this->frame_stack->back()->type == FRAME_TYPE_PACKAGE);
    // TODO: this may not works since FrameStack object's destructor is not virtual.
    PackageFrame * pframe = static_cast<PackageFrame*>(this->frame_stack->back());
    delete pframe;
    this->frame_stack->pop_back();
}

