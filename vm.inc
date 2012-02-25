/* vim: set filetype=cpp: */

OP_PUSH_TRUE {
    stack.push(new BoolValue(true));
}

OP_PUSH_FALSE {
    stack.push(new BoolValue(false));
}

OP_PUSH_UNDEF {
    stack.push(UndefValue::instance());
}

OP_PUSH_INT {
    stack.push(new IntValue(get_int_operand()));
}

OP_PUSH_DOUBLE {
    stack.push(new DoubleValue(get_double_operand()));
}

OP_PUSH_STRING {
    OP * op = ops->at(pc);
    SharedPtr<Value> sv = ((ValueOP*)&(*(op)))->value;
    stack.push(sv);
}

OP_PUSH_VALUE {
    OP * op = ops->at(pc);
    SharedPtr<Value> v = ((ValueOP*)&(*(op)))->value;
    stack.push(v);
}

OP_NEW_RANGE {
    SharedPtr<Value> l = stack.pop();
    SharedPtr<Value> r = stack.pop();
    stack.push(new RangeValue(l->upcast<IntValue>(), r->upcast<IntValue>()));
}

OP_FUNCDEF {
    SharedPtr<Value> code = stack.pop(); // code object
    assert(code->value_type == VALUE_TYPE_CODE);
    this->add_function(code->upcast<CodeValue>()->func_name_id, code);
}

OP_CLOSUREDEF {
    SharedPtr<Value> code = stack.pop(); // code object
    // take variables from stack. and push it to code->closure_cells.
    assert(code->value_type == VALUE_TYPE_CODE);
    for (int i=0; i<get_int_operand(); i++) {
        SharedPtr<Value> v = stack.pop();
        code->upcast<CodeValue>()->closure_vars->push_back(v);
    }
    this->add_function(code->upcast<CodeValue>()->func_name_id, code);
}

OP_GETCLOSURE {
    // take variable from code->closure_cells[n]
    SharedPtr<LexicalVarsFrame> frame;
    for (auto iter=frame_stack->begin(); iter!=frame_stack->end(); iter++) {
        if ((*iter)->type == FRAME_TYPE_FUNCTION) {
            frame = *iter;
            break;
        }
    }
    assert(frame);
    int i = get_int_operand();
    SharedPtr<Value> v = frame->code->upcast<CodeValue>()->closure_vars->at(i);
    stack.push(v);
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
    assert(frame);
    int i = get_int_operand();
    SharedPtr<Value> v = stack.back();
    (*(frame->code->upcast<CodeValue>()->closure_vars))[i]->assign(v);
}

OP_SUB {
    std::minus<int>    i;
    std::minus<double> d;
    this->binop(i, d);
}

OP_DIV {
    std::divides<int>    i;
    std::divides<double> d;
    this->binop(i, d);
}

OP_MUL {
    std::multiplies<int>    i;
    std::multiplies<double> d;
    this->binop(i, d);
}

OP_ADD {
    SharedPtr<Value> v1(stack.pop());
    SharedPtr<Value> v2(stack.pop());
    if (v2->is_numeric()) {
        SharedPtr<Value> i(v2->to_i());
        SharedPtr<IntValue>v = new IntValue(v2->upcast<IntValue>()->int_value + v1->upcast<IntValue>()->int_value);
        stack.push(v);
    } else if (v2->value_type == VALUE_TYPE_STR) {
        // TODO: support null terminated string
        SharedPtr<StrValue>v = new StrValue();
        SharedPtr<Value> s(v1->to_s());
        v->set_str(v2->upcast<StrValue>()->str_value + s->upcast<StrValue>()->str_value);
        stack.push(v);
    } else {
        SharedPtr<Value> s(v1->to_s());
        fprintf(stderr, "'%s' is not numeric or string.\n", s->upcast<StrValue>()->str_value.c_str());
        exit(1); // TODO : die
    }
}

OP_BUILTIN_FUNCALL {
    SharedPtr<Value> funname(stack.pop());
    int argcnt = get_int_operand();
    if (stack.size() >= 1 && stack.back()->value_type == VALUE_TYPE_TUPLE) {
        SharedPtr<TupleValue> t = stack.pop()->upcast<TupleValue>();
        argcnt = t->size();
        stack.extract_tuple(t);
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
    if (iter->second->argc==0) {
        SharedPtr<Value> ret = iter->second->func0();
        stack.push(ret);
    } else if (iter->second->argc==1) {
        SharedPtr<Value> v = stack.pop();
        SharedPtr<Value> ret = iter->second->func1(v);
        stack.push(ret);
    } else if (iter->second->argc==-1) {
        std::vector<SharedPtr<Value>> vec;
        for (int i=0; i<argcnt; i++) {
            SharedPtr<Value> arg = stack.pop();
            vec.push_back(arg);
        }
        SharedPtr<Value> ret = iter->second->funcv(vec);
        stack.push(ret);
    } else if (iter->second->argc==-2) {
        SharedPtr<Value> ret = iter->second->func_vm0(this);
        if (ret->value_type == VALUE_TYPE_EXCEPTION) {
            this->die(ret);
        } else {
            stack.push(ret);
        }
    } else if (iter->second->argc==-3) {
        SharedPtr<Value> v = stack.pop();
        SharedPtr<Value> ret = iter->second->func_vm1(this, v);
        if (ret->value_type == VALUE_TYPE_EXCEPTION) {
            this->die(ret);
        } else {
            stack.push(ret);
        }
    }
}

OP_FUNCALL {
    SharedPtr<Value> funname(stack.pop());
    int argcnt = get_int_operand();
    if (stack.size() >= 1 && stack.back()->value_type == VALUE_TYPE_TUPLE) {
        SharedPtr<TupleValue> t = stack.pop()->upcast<TupleValue>();
        argcnt = t->size();
        stack.extract_tuple(t);
    }
    if (!(stack.size() >= (size_t) argcnt)) {
        printf("[BUG] bad argument: %s requires %d arguments but only %zd items available on stack(OP_FUNCALL)\n", this->symbol_table->id2name(funname->upcast<SymbolValue>()->id).c_str(), argcnt, stack.size());
        dump_stack();
        abort();
    }
    ID id = funname->upcast<SymbolValue>()->id;
    assert(funname->value_type == VALUE_TYPE_SYMBOL);
    // this->find_package(this->package_id())->dump(this->symbol_table, 1);

    SharedPtr<Package> pkg = this->find_package(this->package_id());
    auto iter = pkg->find(id);
    if (iter != pkg->end()) {
        SharedPtr<CodeValue> code = iter->second->upcast<CodeValue>();
        // printf("calling %s\n", funname->upcast<StrValue>()->str_value.c_str());

        SharedPtr<FunctionFrame> fframe = new FunctionFrame(argcnt, frame_stack->back());
        fframe->return_address = pc;
        fframe->orig_ops = ops;
        fframe->top = stack.size() - argcnt;
        fframe->code = code;

        pc = -1;
        this->ops = code->code_opcodes;

        // TODO: vargs support
        // TODO: kwargs support
        assert(argcnt == (int)code->code_params->size());
        for (int i=0; i<argcnt; i++) {
            SharedPtr<Value> arg = stack.pop();
            /*
            std::string *argname = code->code_params->at(i);
            // printf("set lexical var: %d for %s\n", i, argname->c_str());
            frame->setVar(argname, arg);
            */
            fframe->setVar(i, arg);
        }
        frame_stack->push_back(fframe);
    } else {
        this->die("Unknown function: %s", this->symbol_table->id2name(funname->upcast<SymbolValue>()->id).c_str());
    }
}

OP_METHOD_CALL {
    SharedPtr<Value> object(stack.pop());
    SharedPtr<Value> funid(stack.pop());
    if (!(stack.size() >= (size_t) get_int_operand())) {
        // printf("[BUG] bad argument: %s requires %d arguments but only %zd items available on stack(OP_FUNCALL)\n", funname_c, get_int_operand(), stack.size());
        dump_stack();
        abort();
    }
    assert(funid->value_type == VALUE_TYPE_SYMBOL);

    switch (object->value_type) {
    case VALUE_TYPE_STR:
    case VALUE_TYPE_ARRAY: {
        auto iter = this->standard[object->value_type]->find(funid->upcast<SymbolValue>()->id);
        if (iter == this->standard[object->value_type]->end()) {
            this->die("Unknown method %s for %s\n", this->symbol_table->id2name(funid->upcast<SymbolValue>()->id).c_str(), object->type_str());
            break;
        } else {
            if (iter->second->argc==0) {
                if (get_int_operand() == 0) {
                    SharedPtr<Value> v = iter->second->func0(object);
                    stack.push(v);
                } else {
                    this->die("Argument error %s for %s\n", funid->upcast<StrValue>()->str_value.c_str(), object->type_str());
                    break;
                }
            } else if (iter->second->argc==1) {
                if (get_int_operand() == 1) {
                    SharedPtr<Value> arg1 = stack.pop();
                    SharedPtr<Value> v = iter->second->func1(object, arg1);
                    stack.push(v);
                } else {
                    this->die("Argument error %s for %s\n", funid->upcast<StrValue>()->str_value.c_str(), object->type_str());
                    break;
                }
            }
        }
        break;
    }
    case VALUE_TYPE_SYMBOL: {
        // Foo.new();
        SharedPtr<Package> pkg = this->find_package(object->upcast<SymbolValue>()->id);
        auto iter = pkg->find(funid->upcast<SymbolValue>()->id);
        if (iter != pkg->end()) {
            SharedPtr<Value>code_v = iter->second;
            assert(code_v->value_type == VALUE_TYPE_CODE);
            SharedPtr<CodeValue> code = code_v->upcast<CodeValue>();
            int argcnt = get_int_operand();

            {
                SharedPtr<FunctionFrame> fframe = new FunctionFrame(argcnt, frame_stack->back());
                fframe->return_address = pc;
                fframe->orig_ops = ops;
                fframe->top = stack.size() - argcnt;
                fframe->code = code;

                pc = -1;
                this->ops = code->code_opcodes;

                // TODO: vargs support
                // TODO: kwargs support
                assert(argcnt == (int)code->code_params->size());
                for (int i=0; i<argcnt; i++) {
                    SharedPtr<Value> arg = stack.pop();
                    fframe->setVar(i, arg);
                }
                frame_stack->push_back(fframe);
            }
        } else {
            if (funid->upcast<SymbolValue>()->id == this->symbol_table->get_id("new")) {
                // process parameters
                stack.push(new ObjectValue(object->upcast<SymbolValue>()->id));
            } else {
                this->die("Unknown method %s for %s\n", this->symbol_table->id2name(funid->upcast<SymbolValue>()->id).c_str(), this->symbol_table->id2name(object->upcast<SymbolValue>()->id).c_str());
            }
        }
        break;
    }
    /*
    case VALUE_TYPE_FILE: {
        SharedPtr<FileValue>file = object->upcast<FileValue>();
        if (strcmp(funname->upcast<StrValue>()->str_value.c_str(), "read") == 0) {
            SharedPtr<StrValue> str = file->read();
            str->dump(0);
            stack.push(str);
        }
        break;
    }
    */
    case VALUE_TYPE_OBJECT:{
        SharedPtr<Package> pkg = this->find_package(object->upcast<ObjectValue>()->package_id());
        auto iter = pkg->find(funid->upcast<SymbolValue>()->id);
        if (iter != pkg->end()) {
            SharedPtr<Value>code_v = iter->second;
            assert(code_v->value_type == VALUE_TYPE_CODE);
            SharedPtr<CodeValue> code = code_v->upcast<CodeValue>();
            int argcnt = get_int_operand();

            {
                SharedPtr<FunctionFrame> fframe = new FunctionFrame(argcnt, frame_stack->back());
                fframe->return_address = pc;
                fframe->orig_ops = ops;
                fframe->top = stack.size() - argcnt;
                fframe->code = code;
                fframe->self = object;

                pc = -1;
                this->ops = code->code_opcodes;

                // TODO: vargs support
                // TODO: kwargs support
                assert(argcnt == (int)code->code_params->size());
                for (int i=0; i<argcnt; i++) {
                    SharedPtr<Value> arg = stack.pop();
                    fframe->setVar(i, arg);
                }
                frame_stack->push_back(fframe);
            }
        } else {
            this->die("Unknown method %s for %s\n", this->symbol_table->id2name(funid->upcast<SymbolValue>()->id).c_str(), this->symbol_table->id2name(object->upcast<SymbolValue>()->id).c_str());
        }
        break;
    }
    default:
        this->die("Unknown method %s for %s\n", this->symbol_table->id2name(funid->upcast<SymbolValue>()->id).c_str(), object->type_str());
        break;
    }
}

OP_RETURN {
    SharedPtr<Value> retval = stack.pop();

    // frame_stack->back()->dump();
    while (1) {
        assert(frame_stack->size() > 0);
        SharedPtr<LexicalVarsFrame> frame = frame_stack->back();
        if (frame->type == FRAME_TYPE_FUNCTION) {
            SharedPtr<FunctionFrame> fframe = frame->upcast<FunctionFrame>();
            pc = fframe->return_address;
            ops = fframe->orig_ops;
            // printf("RETURN :orig: %d, current: %d\n", fframe->top, stack.size());

            while (stack.size() > (size_t)frame->top) {
                stack.pop();
            }

            frame_stack->pop_back();

            stack.push(retval);
            break;
        } else if (frame->type == FRAME_TYPE_TRY) {
            SharedPtr<TryFrame> tframe = frame->upcast<TryFrame>();
            pc = tframe->return_address;
            // printf("RETURN :orig: %d, current: %d\n", tframe->top, stack.size());

            while (stack.size() > frame->top) {
                stack.pop();
            }

            frame_stack->pop_back();

            stack.push(retval);
            break;
        } else {
            // printf("THIS IS NOT A FUNCTION FRAME\n");
            while (stack.size() > frame->top) {
                stack.pop();
            }
            frame_stack->pop_back();
        }
    }
}

OP_ENTER {
    SharedPtr<LexicalVarsFrame> frame = new LexicalVarsFrame(get_int_operand(), frame_stack->back());
    frame->top = stack.size();
    frame_stack->push_back(frame);
}

OP_LEAVE {
    SharedPtr<LexicalVarsFrame> frame = frame_stack->back();
    while (stack.size() > (size_t)frame->top) {
        stack.pop();
    }
    frame_stack->pop_back();
}

OP_PUSH_IDENTIFIER {
    stack.push(new SymbolValue(get_int_operand()));
}

OP_DUMP {
    this->dump_stack();
}

OP_JUMP_IF_FALSE {
    SharedPtr<Value> v(stack.pop());

    SharedPtr<Value> b(v->to_b());
    if (!b->upcast<BoolValue>()->bool_value) {
        pc = get_int_operand()-1;
    }
}

OP_JUMP {
    pc = get_int_operand()-1;
}

OP_EQ {
    std::equal_to<int> i;
    std::equal_to<double> d;
    this->cmpop(i, d);
}

OP_GT {
    std::greater<int> i;
    std::greater<double> d;
    this->cmpop(i, d);
}

OP_LT {
    std::less<int> i;
    std::less<double> d;
    this->cmpop(i, d);
}

OP_GE {
    std::greater_equal<int> i;
    std::greater_equal<double> d;
    this->cmpop(i, d);
}

OP_LE {
    std::less_equal<int> i;
    std::less_equal<double> d;
    this->cmpop(i, d);
}

OP_SETLOCAL {
    SharedPtr<Value> rvalue = stack.pop();
    frame_stack->back()->setVar(
        get_int_operand(),
        rvalue
    );
    stack.push(rvalue);
}

OP_SETDYNAMIC {
    // frame_stack->back()->dump_vars();
    SharedPtr<LexicalVarsFrame> frame = frame_stack->back();
    int level = (get_int_operand() >> 16) & 0x0000FFFF;
    int no    = get_int_operand() & 0x0000ffff;
    DBG("SETDYNAMIC %d, %d\n", level, no);
    for (int i=0; i<level; i++) {
        frame = frame->up;
    }
    SharedPtr<Value> rvalue = stack.pop();
    frame->setVar(
        no,
        rvalue
    );
    stack.push(rvalue);
}

OP_GETDYNAMIC {
    // lexical vars
    SharedPtr<LexicalVarsFrame> frame = frame_stack->back();
    int level = (get_int_operand() >> 16) & 0x0000FFFF;
    int no    = get_int_operand() & 0x0000ffff;
    for (int i=0; i<level; i++) {
        frame = frame->up;
    }
    SharedPtr<Value>v = frame->find(no);
    if (v) {
        DBG2("found lexical var\n");
        stack.push(v);
    } else { // TODO: remove this and use 'my' keyword?
        DBG2("There is no variable...\n");
        v = UndefValue::instance();
        frame_stack->back()->setVar(get_int_operand(), v);
        stack.push(v);
    }
}

OP_GETLOCAL {
    // lexical vars
    SharedPtr<Value>v = frame_stack->back()->find(get_int_operand());
    if (v) {
        // printf("found lexical var\n");
        stack.push(v);
    } else { // TODO: remove this and use 'my' keyword?
        v = UndefValue::instance();
        frame_stack->back()->setVar(get_int_operand(), v);
        stack.push(v);
    }
}

OP_GET_ITEM {
    SharedPtr<Value> index(stack.pop());
    SharedPtr<Value> container(stack.pop());

    SharedPtr<Value> ret = container->get_item(&(*index));
    stack.push(ret);
}

OP_SET_ITEM {
    SharedPtr<Value> index(stack.pop());
    SharedPtr<Value> container(stack.pop());
    SharedPtr<Value> rvalue(stack.pop());

    container->set_item(index, rvalue);
    stack.push(rvalue);
}

OP_UNARY_NEGATIVE {
    SharedPtr<Value> v(stack.pop());
    SharedPtr<Value> result = v->tora__neg__();
    stack.push(result);
}

OP_MAKE_HASH {
    SharedPtr<HashValue> h = new HashValue();
    int size = get_int_operand();
    for (int i=0; i<size; i+=2) {
        SharedPtr<StrValue> k = stack.pop()->to_s();
        SharedPtr<Value> v = stack.pop();
        h->set(k->str_value, v);
    }
    stack.push(h);
}

OP_MAKE_ARRAY {
    SharedPtr<ArrayValue> a = new ArrayValue();
    int array_size = get_int_operand();
    for (int i=0; i<array_size; i++) {
        SharedPtr<Value> v = stack.pop();
        a->push(v);
    }
    stack.push(a);
}

OP_MAKE_TUPLE {
    SharedPtr<TupleValue> t = new TupleValue();
    int tuple_size = get_int_operand();
    for (int i=0; i<tuple_size; i++) {
        SharedPtr<Value> v = stack.pop();
        t->push(v);
    }
    stack.push(t);
}

OP_EXTRACT_TUPLE {
    SharedPtr<TupleValue> t = stack.pop()->upcast<TupleValue>();
    stack.extract_tuple(t);
}

OP_PRE_INCREMENT {
    // ++$i
    SharedPtr<Value> i = stack.pop();
    if (i->value_type == VALUE_TYPE_INT) {
        i->upcast<IntValue>()->tora__incr__();
        stack.push(i);
    } else {
        this->die("%s does not supports increment operator.\n", i->type_str());
    }
}

OP_POST_INCREMENT {
    // $i++
    SharedPtr<Value> i = stack.pop();
    if (i->value_type == VALUE_TYPE_INT) {
        SharedPtr<IntValue>ii = i->upcast<IntValue>();
        SharedPtr<IntValue>tmp = ii->clone();
        ii->tora__incr__();
        stack.push(tmp);
    } else {
        this->die("%s does not supports post increment operator.\n", i->type_str());
    }
}

OP_PRE_DECREMENT {
    // --$i
    SharedPtr<Value> i = stack.pop();
    if (i->value_type == VALUE_TYPE_INT) {
        i->upcast<IntValue>()->tora__decr__();
        stack.push(i);
    } else {
        this->die("%s does not supports decrement operator.\n", i->type_str());
    }
}

OP_POST_DECREMENT {
    // $i--
    SharedPtr<Value> i = stack.pop();
    if (i->value_type == VALUE_TYPE_INT) {
        SharedPtr<IntValue>ii = i->upcast<IntValue>();
        SharedPtr<IntValue>tmp = ii->clone();
        ii->tora__decr__();
        stack.push(tmp);
    } else {
        this->die("%s does not supports post decrement operator.\n", i->type_str());
    }
}

OP_FILE_TEST {
    // -f $file
    SharedPtr<StrValue> fname = stack.pop()->to_s();
    struct stat buf;
    switch (get_int_operand()) {
    case 'f':
        stack.push(new BoolValue(stat(fname->c_str(), &buf)==0 && buf.st_mode & S_IFREG));
        break;
    case 'd':
        stack.push(new BoolValue(stat(fname->c_str(), &buf)==0 && buf.st_mode & S_IFDIR));
        break;
    case 'e':
        stack.push(new BoolValue(stat(fname->c_str(), &buf)==0));
        break;
    default:
        printf("[BUG] Unknown operand: %c\n", get_int_operand());
        abort();
    }
}

OP_GETGLOBAL {
    int globalvarno = get_int_operand();
    stack.push(this->global_vars->at(globalvarno));
}

OP_POP_TOP {
    stack.pop();
}

OP_ENTER_FOREACH {
    SharedPtr<Value>a = stack.pop();
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
    } else {
        fprintf(stderr, "This object doesn't support iteration\n");
        a->dump();
        abort();
    }

    int vars_cnt = get_int_operand();
    SharedPtr<ForeachFrame> fframe = new ForeachFrame(vars_cnt, frame_stack->back());
    fframe->top = stack.size();
    fframe->iter = iiter;
    frame_stack->push_back(fframe);
}

OP_FOR_ITER {
    // TOS is an iterator. Call its next() method. If this yields a new value, push it on the stack (leaving the iterator below it). If the iterator indicates it is exhausted TOS is popped, and the byte code counter is incremented by delta.

    auto fframe = frame_stack->back()->upcast<ForeachFrame>();
    assert(fframe->type == FRAME_TYPE_FOREACH);
    SharedPtr<Value> iter = fframe->iter;
    if (iter->value_type == VALUE_TYPE_ARRAY_ITERATOR) {
        SharedPtr<ArrayValue::iterator> aiter = iter->upcast<ArrayValue::iterator>();
        if (aiter->counter < aiter->parent->size()) {
            SharedPtr<Value> val = aiter->parent->at(aiter->counter);
            aiter->counter++;
            SharedPtr<TupleValue> t = new TupleValue();
            t->push(val);
            stack.push(t);
        } else {
            stack.push(new ExceptionValue(EXCEPTION_TYPE_STOP_ITERATION));
        }
    } else if (iter->value_type == VALUE_TYPE_RANGE_ITERATOR) {
        SharedPtr<RangeValue::iterator> riter = iter->upcast<RangeValue::iterator>();
        if (riter->counter <= riter->parent->right->int_value) {
            SharedPtr<IntValue> val = new IntValue(riter->counter);
            riter->counter++;
            SharedPtr<TupleValue> t = new TupleValue();
            t->push(val);
            stack.push(t);
        } else {
            stack.push(new ExceptionValue(EXCEPTION_TYPE_STOP_ITERATION));
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
            stack.push(t);
        } else {
            stack.push(new ExceptionValue(EXCEPTION_TYPE_STOP_ITERATION));
        }
    } else {
        fprintf(stderr, "[BUG] TOS is not a iterator object.\n");
        iter->dump();
        abort();
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
    SharedPtr<TryFrame> fframe = new TryFrame(frame_stack->back());
    fframe->return_address = get_int_operand()-1;
    fframe->top = stack.size();
    frame_stack->push_back(fframe);
}

OP_DIE {
    SharedPtr<Value> exception = stack.pop();
    this->die(exception);
}

OP_USE {
    SharedPtr<Value> mod_name = stack.pop();
    SharedPtr<Value> include  = stack.pop();
    SharedPtr<Value> ret = this->require(mod_name);
    if (ret->is_exception()) {
        this->die(ret);
    } else {
        if (include->value_type == VALUE_TYPE_INT && include->upcast<IntValue>()->int_value == 1) {
            SharedPtr<Value> v = this->copy_all_public_symbols(mod_name->upcast<SymbolValue>()->id, this->package);
            if (v->is_exception()) {
                this->die(ret);
            }
        }
        stack.push(ret);
    }
}

OP_NOT {
    SharedPtr<BoolValue> v = stack.pop()->to_b();
    stack.push(v->tora__not__());
}

OP_PACKAGE_ENTER {
    ID package_id = get_int_operand();
    SharedPtr<PackageFrame> pframe = new PackageFrame(this->package, this, frame_stack->back());
    this->frame_stack->push_back(pframe);
    this->package = this->symbol_table->id2name(package_id);
}

OP_PACKAGE_LEAVE {
    assert(this->frame_stack->back()->type == FRAME_TYPE_PACKAGE);
    this->frame_stack->pop_back();
}

