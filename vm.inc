OP_PUSH_TRUE {
    stack.push(new BoolValue(true));
}

OP_PUSH_FALSE {
    stack.push(new BoolValue(false));
}

OP_PUSH_INT {
    stack.push(new IntValue(op->operand.int_value));
}

OP_PUSH_DOUBLE {
    stack.push(new DoubleValue(op->operand.double_value));
}

OP_PUSH_STRING {
    SharedPtr<Value> sv = ((ValueOP*)&(*(op)))->value;
    stack.push(sv);
}

OP_PUSH_VALUE {
    SharedPtr<Value> v = ((ValueOP*)&(*(op)))->value;
    stack.push(v);
}

OP_NEW_RANGE {
    SharedPtr<Value> l = stack.pop();
    SharedPtr<Value> r = stack.pop();
    stack.push(new RangeValue(l->upcast<IntValue>(), r->upcast<IntValue>()));
}

OP_DEFINE_METHOD {
    SharedPtr<Value> code = stack.pop(); // code object
    assert(code->value_type == VALUE_TYPE_CODE);
    std::string &funcname = ((ValueOP*)&(*(op)))->value->upcast<StrValue>()->str_value;
    this->add_function(funcname, code);
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
    if (v1->is_numeric()) {
        SharedPtr<Value> i(v2->to_i());
        SharedPtr<IntValue>v = new IntValue(v2->upcast<IntValue>()->int_value + v1->upcast<IntValue>()->int_value);
        stack.push(v);
    } else if (v1->value_type == VALUE_TYPE_STR) {
        // TODO: support null terminated string
        SharedPtr<StrValue>v = new StrValue();
        SharedPtr<Value> s(v2->to_s());
        v->set_str(std::string(s->upcast<StrValue>()->str_value.c_str()) + std::string(v1->upcast<StrValue>()->str_value.c_str()));
        stack.push(v);
    } else {
        SharedPtr<Value> s(v1->to_s());
        fprintf(stderr, "'%s' is not numeric or string.\n", s->upcast<StrValue>()->str_value.c_str());
        exit(1); // TODO : die
    }
}

OP_FUNCALL {
    SharedPtr<Value> funname(stack.pop());
    std::string &funname_c = funname->upcast<StrValue>()->str_value;
    int argcnt = op->operand.int_value;
    if (stack.size() >= 1 && stack.back()->value_type == VALUE_TYPE_TUPLE) {
        SharedPtr<TupleValue> t = stack.pop()->upcast<TupleValue>();
        argcnt = t->size();
        stack.extract_tuple(t);
    }
    if (!(stack.size() >= (size_t) argcnt)) {
        printf("[BUG] bad argument: %s requires %d arguments but only %zd items available on stack(OP_FUNCALL)\n", funname_c.c_str(), argcnt, stack.size());
        dump_stack();
        abort();
    }
    assert(funname->value_type == VALUE_TYPE_STR);
    if (funname_c == "print") {
        for (int i=0; i<argcnt; i++) {
            SharedPtr<Value> v(stack.pop());
            SharedPtr<Value> s(v->to_s());
            printf("%s", s->upcast<StrValue>()->str_value.c_str());
        }
    } else if (funname_c == "p") {
        SharedPtr<Value> v(stack.pop());
        v->dump();
    } else if (funname_c == "open") {
        SharedPtr<Value> filename(stack.pop());
        std::string mode;
        if (argcnt >= 2) {
            mode = stack.pop()->upcast<StrValue>()->str_value.c_str();
        } else {
            mode = "rb";
        }
        // TODO: check \0
        SharedPtr<FileValue> file = new FileValue();
        if (file->open(
            filename->upcast<StrValue>()->str_value,
            mode
        )) {
            stack.push(file);
        } else {
            abort(); // todo: throw exception
        }
    } else if (funname_c == "say") {
        for (int i=0; i<argcnt; i++) {
            SharedPtr<Value> v(stack.pop());
            SharedPtr<Value> s(v->to_s());
            printf("%s\n", s->upcast<StrValue>()->str_value.c_str());
        }
    } else if (funname_c == "getenv") {
        assert(argcnt==1);
        SharedPtr<Value> v(stack.pop());
        SharedPtr<Value> s(v->to_s());
        char *env = getenv(s->upcast<StrValue>()->str_value.c_str());
        if (env) {
            SharedPtr<StrValue> ret = new StrValue();
            ret->set_str(env);
            stack.push(ret);
        } else {
            stack.push(UndefValue::instance());
        }
    } else if (funname_c == "usleep") {
        // TODO: remove later
        SharedPtr<Value> v(stack.pop());
        assert(v->value_type == VALUE_TYPE_INT);
        SharedPtr<Value> s(v->to_i());
        usleep(s->upcast<IntValue>()->int_value);
    } else if (funname_c == "exit") {
        SharedPtr<Value> v(stack.pop());
        assert(v->value_type == VALUE_TYPE_INT);
        SharedPtr<Value> s(v->to_i());
        exit(s->upcast<IntValue>()->int_value);
    } else {
        auto iter =  this->functions.find(funname->upcast<StrValue>()->str_value.c_str());
        if (iter != this->functions.end()) {
            SharedPtr<CodeValue> code = iter->second->upcast<CodeValue>();
            // printf("calling %s\n", funname->upcast<StrValue>()->str_value.c_str());
            {
                SharedPtr<FunctionFrame> fframe = new FunctionFrame();
                fframe->return_address = pc;
                fframe->orig_ops = ops;
                fframe->top = stack.size() - argcnt;
                function_frames->push_back(fframe);
            }
            pc = -1;
            this->ops = code->code_opcodes;

            SharedPtr<LexicalVarsFrame> frame = new LexicalVarsFrame(lexical_vars_stack->back());
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
                frame->setVar(i, arg);
            }
            lexical_vars_stack->push_back(frame);
        } else {
            fprintf(stderr, "Unknown function: %s\n", funname->upcast<StrValue>()->str_value.c_str());
        }
    }
}

OP_METHOD_CALL {
    SharedPtr<Value> object(stack.pop());
    SharedPtr<Value> funname(stack.pop());
    const char *funname_c = funname->upcast<StrValue>()->str_value.c_str();
    if (!(stack.size() >= (size_t) op->operand.int_value)) {
        printf("[BUG] bad argument: %s requires %d arguments but only %zd items available on stack(OP_FUNCALL)\n", funname_c, op->operand.int_value, stack.size());
        dump_stack();
        abort();
    }
    assert(funname->value_type == VALUE_TYPE_STR);
    switch (object->value_type) {
    case VALUE_TYPE_ARRAY: {
        SharedPtr<ArrayValue>av = object->upcast<ArrayValue>();
        if (strcmp(funname->upcast<StrValue>()->str_value.c_str(), "size") == 0) {
            SharedPtr<IntValue> size = new IntValue(av->size());
            stack.push(size);
        }
        break;
    }
    case VALUE_TYPE_FILE: {
        SharedPtr<FileValue>file = object->upcast<FileValue>();
        if (strcmp(funname->upcast<StrValue>()->str_value.c_str(), "read") == 0) {
            SharedPtr<StrValue> str = file->read();
            stack.push(str);
        }
        break;
    }
    case VALUE_TYPE_STR: {
        SharedPtr<StrValue>sv = object->upcast<StrValue>();
        if (strcmp(funname->upcast<StrValue>()->str_value.c_str(), "match") == 0) {
            if (op->operand.int_value == 1) {
                SharedPtr<AbstractRegexpValue> regex = stack.pop()->upcast<AbstractRegexpValue>();
                SharedPtr<BoolValue> b = new BoolValue(regex->match(std::string(sv->str_value.c_str())));
                stack.push(b);
            } else {
                fprintf(stderr, "Missing args for 'match'\n");
                abort();
            }
        } else if (strcmp(funname->upcast<StrValue>()->str_value.c_str(), "length") == 0) {
            if (op->operand.int_value == 0) {
                stack.push(new IntValue(sv->length()));
            } else {
                fprintf(stderr, "length needs 0 argument\n");
                abort();
            }
        }
        break;
    }
    default:
        fprintf(stderr, "Unknown method %s for %s\n", funname->upcast<StrValue>()->str_value.c_str(), object->type_str());
        // TODO throw exception
        break;
    }
}

OP_RETURN {
    assert(function_frames->size() > 0);
    SharedPtr<FunctionFrame> fframe = function_frames->back();
    function_frames->pop_back();
    pc = fframe->return_address;
    ops = fframe->orig_ops;
    // printf("RETURN :orig: %d, current: %d\n", fframe->top, stack.size());
    if (fframe->top == (int)stack.size()) {
        stack.push(UndefValue::instance());
    }

    lexical_vars_stack->pop_back();
}

OP_ENTER {
    SharedPtr<LexicalVarsFrame> frame = new LexicalVarsFrame(lexical_vars_stack->back());
    lexical_vars_stack->push_back(frame);
}

OP_LEAVE {
    lexical_vars_stack->pop_back();
}

OP_PUSH_IDENTIFIER {
    SharedPtr<Value> sv = ((ValueOP*)&(*(op)))->value;
    stack.push(sv);
}

OP_DUMP {
    this->dump_stack();
}

OP_JUMP_IF_FALSE {
    SharedPtr<Value> v(stack.pop());

    SharedPtr<Value> b(v->to_b());
    if (!b->upcast<BoolValue>()->bool_value) {
        pc = op->operand.int_value-1;
    }
}

OP_JUMP {
    pc = op->operand.int_value-1;
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
    lexical_vars_stack->back()->setVar(
        op->operand.int_value,
        rvalue
    );
    stack.push(rvalue);
}

OP_SETDYNAMIC {
    // lexical_vars_stack->back()->dump_vars();
    SharedPtr<LexicalVarsFrame> frame = lexical_vars_stack->back();
    int level = (op->operand.int_value >> 16) & 0x0000FFFF;
    int no    = op->operand.int_value & 0x0000ffff;
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
    SharedPtr<LexicalVarsFrame> frame = lexical_vars_stack->back();
    int level = (op->operand.int_value >> 16) & 0x0000FFFF;
    int no    = op->operand.int_value & 0x0000ffff;
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
        lexical_vars_stack->back()->setVar(op->operand.int_value, v);
        stack.push(v);
    }
}

OP_GETLOCAL {
    // lexical vars
    SharedPtr<Value>v = lexical_vars_stack->back()->find(op->operand.int_value);
    if (v) {
        // printf("found lexical var\n");
        stack.push(v);
    } else { // TODO: remove this and use 'my' keyword?
        v = UndefValue::instance();
        lexical_vars_stack->back()->setVar(op->operand.int_value, v);
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
    int size = op->operand.int_value;
    for (int i=0; i<size; i+=2) {
        SharedPtr<StrValue> k = stack.pop()->to_s();
        SharedPtr<Value> v = stack.pop();
        h->set(k->str_value, v);
    }
    stack.push(h);
}

OP_MAKE_ARRAY {
    SharedPtr<ArrayValue> a = new ArrayValue();
    int array_size = op->operand.int_value;
    for (int i=0; i<array_size; i++) {
        SharedPtr<Value> v = stack.pop();
        a->push(v);
    }
    stack.push(a);
}

OP_MAKE_TUPLE {
    SharedPtr<TupleValue> t = new TupleValue();
    int tuple_size = op->operand.int_value;
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

OP_UNARY_INCREMENT {
    SharedPtr<IntValue> i = stack.pop()->upcast<IntValue>();
    if (i->value_type == VALUE_TYPE_INT) {
        i->tora__incr__();
        stack.push(i);
    } else {
        abort(); // TODO: throw exception
    }
}

OP_GETGLOBAL {
    int globalvarno = op->operand.int_value;
    stack.push(this->global_vars->at(globalvarno));
}

OP_POP_TOP {
    stack.pop();
}

