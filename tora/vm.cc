#include <vector>

#include "vm.h"
#include "value.h"
#include "tora.h"
#include "frame.h"
#include "inspector.h"
#include "symbols.gen.h"
#include "callback.h"

#include "value/hash.h"
#include "value/code.h"
#include "value/regexp.h"
#include "value/file.h"
#include "value/symbol.h"
#include "value/array.h"
#include "value/object.h"
#include "value/class.h"
#include "value/file_package.h"
#include "value/bool.h"
#include "value/int.h"

#include "object.h"

#include "builtin.h"

#include "lexer.h"
#include "parser.class.h"
#include "compiler.h"
#include "disasm.h"
#include <unistd.h>
#include <algorithm>
#include <functional>
#include <iostream>
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <random>

#ifdef _WIN32
#include <windows.h>
#define dlopen(x, y) (void *) LoadLibrary(x)
#define dlsym(x, y) (void *) GetProcAddress((HMODULE)x, y)
#define dlclose(x) FreeLibrary((HMODULE)x)
const char *dlerror() {
  DWORD err = (int)GetLastError();
  if (err == 0) return NULL;
  static char buf[256];
  FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), buf,
                sizeof buf, NULL);
  return buf;
}
#else
#include <dlfcn.h>
#endif

using namespace tora;

const int INITIAL_STACK_SIZE = 1024;
const int INITIAL_MARK_STACK_SIZE = 128;

VM::VM(SharedPtr<OPArray> &ops_, SharedPtr<SymbolTable> &symbol_table_,
       bool dump_ops)
    : dump_ops_(dump_ops),
      ops(ops_),
      symbol_table(symbol_table_),
      stack(),
      exec_trace(false) {
  sp = 0;
  pc = 0;
  this->stack.reserve(INITIAL_STACK_SIZE);
  this->frame_stack = new std::vector<SharedPtr<LexicalVarsFrame>>();
  this->frame_stack->push_back(new LexicalVarsFrame(this, 0, 0));
  this->global_vars = new std::vector<SharedValue>();
  std::random_device rd;
  this->myrand = new std::mt19937(rd());
  this->mark_stack.reserve(INITIAL_MARK_STACK_SIZE);
  this->file_scope_.reset(new std::map<ID, SharedValue>());

  MortalTrueValue t;
  this->true_value_ = t.get();
  MortalFalseValue f;
  this->false_value_ = f.get();
}

VM::~VM() {
  stack.resize(0);

  delete this->global_vars;
  // assert(this->frame_stack->size() == 1);
  //  while (frame_stack->size()) {
  //      delete this->frame_stack->back();
  //      frame_stack->pop_back();
  //  }
  delete this->frame_stack;
  delete this->myrand;

  /*
  {
      // std::map<ID, CallbackFunction*> builtin_functions_;
      auto iter = builtin_functions_.begin();
      for (;iter != builtin_functions_.end(); iter++) {
          delete iter->second;
      }
  }
  */
}

void VM::init_globals(const std::vector<std::string> &args) {
  // $ARGV
  MortalArrayValue avalue;
  for (auto arg : args) {
    MortalStrValue a(arg);
    array_push_back(avalue.get(), a.get());
  }
  this->global_vars->push_back(avalue.get());

  // $ENV
  MortalObjectValue env(
      this, this->get_builtin_class(SYMBOL_ENV_CLASS).get(), new_undef_value());
  this->global_vars->push_back(env.get());

  // $LIBPATH : Array
  MortalArrayValue libpath;
  MortalStrValue lib("lib");
  array_push_back(libpath.get(), lib.get());
  this->global_vars->push_back(libpath);

  // $REQUIRED : Hash
  MortalHashValue required;
  this->global_vars->push_back(required.get());

  // $STDIN : File
  this->global_vars->push_back(new FileValue(stdin));

  // $STDOUT : File
  this->global_vars->push_back(new FileValue(stdout));

  // $STDERR : File
  this->global_vars->push_back(new FileValue(stderr));
}

void VM::die(const char *format, ...) {
  va_list ap;
  char p[4096 + 1];
  va_start(ap, format);
  vsnprintf(p, 4096, format, ap);
  va_end(ap);
  std::string s = p;
  throw new_str_value(s);
}

void VM::die(const SharedPtr<tora::Value> &exception) {
  throw SharedPtr<tora::Value>(exception);
}

SharedPtr<tora::Value> VM::eval(std::istream *is, const std::string &fname) {
  VM *vm = this;

  Scanner scanner(is, fname);

  Node *yylval = NULL;
  int token_number;
  tora::Parser parser(fname);
  do {
    token_number = scanner.scan(&yylval);
    parser.set_lineno(scanner.lineno());
    parser.parse(token_number, yylval);
  } while (token_number != 0);
  if (scanner.in_heredoc()) {
    throw new ExceptionValue("Unexpected EOF in heredoc.");
  }
  if (parser.is_failure()) {
    throw new ExceptionValue("Parsing failed.");
  }

  // compile
  // SharedPtr<SymbolTable> symbol_table = new SymbolTable();
  Compiler compiler(vm->symbol_table, fname);
  compiler.init_globals();
  compiler.compile(parser.root_node());
  if (vm->dump_ops()) {
    printf("Dumping %s\n", fname.c_str());
    Disasm::disasm(compiler.ops);
  }
  if (compiler.error) {
    throw new ExceptionValue("Compilation failed.");
  }

  // run it
  // tora::VM vm(compiler.ops, symbol_table);
  // vm->init_globals(argc-optind, argv+optind);

  // save orig ops.
  SharedPtr<OPArray> orig_ops = vm->ops;
  int orig_pc = vm->pc;
  size_t orig_stack_size = vm->stack.size();
  size_t orig_frame_size = vm->frame_stack->size();

  if (0) {
    Disasm::disasm(compiler.ops);
  }

  vm->ops = compiler.ops;
  vm->pc = 0;
  vm->execute();

  // restore
  vm->ops = orig_ops;
  vm->pc = orig_pc;

  // remove frames
  while (orig_frame_size < vm->frame_stack->size()) {
    //      delete vm->frame_stack->back();
    vm->frame_stack->pop_back();
  }

  if (orig_stack_size < vm->stack.size()) {
    SharedPtr<tora::Value> ret = vm->stack.back();
    vm->stack.pop_back();
    while (orig_stack_size < vm->stack.size()) {
      vm->stack.pop_back();
    }
    return ret;
  } else {
    return new_undef_value();
  }
}

static SharedPtr<tora::Value> builtin_eval(VM *vm, tora::Value *v) {
  assert(v->value_type == VALUE_TYPE_STR);

  std::stringstream ss(*get_str_value(v) + ";");
  return vm->eval(&ss, "<eval>");
}

/**
 * do $file; => eval(open($file).read())
 */
static SharedPtr<tora::Value> builtin_do(VM *vm, tora::Value *v) {
  assert(v->value_type == VALUE_TYPE_STR);
  std::string fname = v->to_s();
  std::ifstream ifs(fname.c_str(), std::ios::in);
  if (ifs.is_open()) {
    return vm->eval(&ifs, fname);
  } else {
    throw new ExceptionValue(*get_str_value(v) + " : " +
                             get_strerror(get_errno()));
  }
}

void VM::use(tora::Value *package_v, bool need_copy) {
  ID package_id = symbol_id(package_v);
  std::string package = symbol_table->id2name(package_id);
  this->require_package(package);
  if (need_copy) {
    this->copy_all_public_symbols(symbol_id(package_v));
  } else {
    MortalFilePackageValue fpv(package_id, file_scope_map_[package_id]);
    this->file_scope_->insert(
        file_scope_body_t::value_type(package_id, fpv.get()));
  }
}

void VM::require_package(const std::string &package) {
  VM *vm = this;
  SharedValue libpath = vm->global_vars->at(2);
  SharedValue required = vm->global_vars->at(3);
  std::string s = package;
  {
    auto iter = s.find("::");
    while (iter != std::string::npos) {
      s.replace(iter, 2, "/");
      iter = s.find("::");
    }
    s += ".tra";
  }

  // inc check
  if (hash_has_key(required.get(), s)) {
    if (hash_get_item(required.get(), s)->value_type == VALUE_TYPE_UNDEF) {
      throw new_str_value("Compilation failed in require");
    } else {
      return;
    }
  }

  // load
  for (int i = 0; i < array_size(libpath.get()); i++) {
    std::string realfilename;
    realfilename = array_get_item(libpath.get(), i)->to_s();
    realfilename += "/";
    realfilename += s;
    struct stat stt;
    if (stat(realfilename.c_str(), &stt) == 0) {
      std::ifstream ifs(realfilename.c_str());
      if (ifs.is_open()) {
        std::shared_ptr<std::map<ID, SharedValue>> orig_file_scope(
            this->file_scope_);
        std::shared_ptr<std::map<ID, SharedValue>> file_scope_tmp(
            new std::map<ID, SharedValue>);
        VM *vm = this;

        this->file_scope_map_.insert(
            std::map<ID, std::shared_ptr<std::map<ID, SharedValue>>>::
                value_type(symbol_table->get_id(package), file_scope_tmp));
        this->file_scope_ = file_scope_tmp;
        (void)vm->eval(&ifs, realfilename);

        MortalStrValue realfilename_sv(realfilename);
        hash_set_item(required.get(), s, realfilename_sv.get());

        vm->file_scope_ = orig_file_scope;
        return;
      } else {
        MortalUndefValue undef;
        hash_set_item(required.get(), s, undef.get());
        throw new ExceptionValue(realfilename + " : " +
                                 get_strerror(get_errno()));
      }
    }
  }

  // not found...
  std::string message = std::string("Cannot find ") + s + " in $LIBPATH\n";
  for (int i = 0; i < array_size(libpath.get()); i++) {
    message += "  " + array_get_item(libpath.get(), i)->to_s();
  }
  throw new ExceptionValue(message);
}

void VM::call_native_func(const CallbackFunction *callback, int argcnt) {
  switch (callback->argc) {
    case CallbackFunction::type_vmv: {
      std::vector<SharedPtr<Value>> vec;
      for (int i = 0; i < argcnt; i++) {
        SharedPtr<Value> arg = stack.back();
        stack.pop_back();
        vec.push_back(arg);
      }
      SharedPtr<Value> ret = callback->func_vmv(this, vec);
      stack.push_back(ret);
      break;
    }
    case CallbackFunction::type_vm0: {
      SharedPtr<Value> ret = callback->func_vm0(this);
      assert(ret->value_type != VALUE_TYPE_EXCEPTION);
      stack.push_back(ret);
      break;
    }
    case CallbackFunction::type_vm1: {
      if (argcnt != 1) {
        throw new ExceptionValue(
            "ArgumentException: The method requires %d arguments but you "
            "passed %d.",
            1, argcnt);
      }
      SharedPtr<Value> v = stack.back();
      stack.pop_back();
      SharedPtr<Value> ret = callback->func_vm1(this, v.get());
      assert(!(ret->value_type == VALUE_TYPE_EXCEPTION &&
               ret->upcast<ExceptionValue>()->exception_type() !=
                   EXCEPTION_TYPE_STOP_ITERATION));
      stack.push_back(ret);
      break;
    }
    case CallbackFunction::type_vm2: {
      if (argcnt != 2) {
        throw new ExceptionValue(
            "ArgumentException: The method requires %d arguments but you "
            "passed %d.",
            2, argcnt);
      }
      SharedPtr<Value> v = stack.back();
      stack.pop_back();
      SharedPtr<Value> v2 = stack.back();
      stack.pop_back();

      SharedPtr<Value> ret = callback->func_vm2(this, v.get(), v2.get());
      assert(ret->value_type != VALUE_TYPE_EXCEPTION);
      stack.push_back(ret);
      break;
    }
    case CallbackFunction::type_vm3: {
      if (argcnt != 3) {
        throw new ExceptionValue(
            "ArgumentException: The method requires %d arguments but you "
            "passed %d.",
            3, argcnt);
      }
      SharedPtr<Value> v = stack.back();
      stack.pop_back();
      SharedPtr<Value> v2 = stack.back();
      stack.pop_back();
      SharedPtr<Value> v3 = stack.back();
      stack.pop_back();

      SharedPtr<Value> ret =
          callback->func_vm3(this, v.get(), v2.get(), v3.get());
      assert(ret->value_type != VALUE_TYPE_EXCEPTION);
      stack.push_back(ret);
      break;
    }
    case CallbackFunction::type_vm4: {
      if (argcnt != 4) {
        throw new ExceptionValue(
            "ArgumentException: The method requires %d arguments but you "
            "passed %d.",
            4, argcnt);
      }
      SharedPtr<Value> v = stack.back();
      stack.pop_back();
      SharedPtr<Value> v2 = stack.back();
      stack.pop_back();
      SharedPtr<Value> v3 = stack.back();
      stack.pop_back();
      SharedPtr<Value> v4 = stack.back();
      stack.pop_back();

      SharedPtr<Value> ret =
          callback->func_vm4(this, v.get(), v2.get(), v3.get(), v4.get());
      assert(ret->value_type != VALUE_TYPE_EXCEPTION);
      stack.push_back(ret);
      break;
    }
    case CallbackFunction::type_const_int: {
      for (int i = 0; i < argcnt; i++) {
        stack.pop_back();  // Foo::Bar.baz();
      }
      MortalIntValue r(callback->const_int);
      stack.push_back(r.get());
      break;
    }
    case CallbackFunction::type_vm_self_v: {
      if (argcnt < 1) {
        throw new ExceptionValue(
            "ArgumentException: The method requires %d arguments but you "
            "passed %d.",
            4, argcnt);
      }

      SharedPtr<Value> self = stack.back();
      stack.pop_back();

      std::vector<SharedPtr<Value>> vec;
      for (int i = 1; i < argcnt; i++) {
        SharedPtr<Value> arg = stack.back();
        stack.pop_back();
        vec.push_back(arg);
      }
      SharedPtr<Value> ret = callback->func_vm_self_v(this, self.get(), vec);
      stack.push_back(ret);
      break;
    }
    default: {
      throw new ExceptionValue("Unknown callback type: %d", callback->argc);
    }
  }
}

void VM::register_standard_methods() {
  load_builtin_objects(this);
  Init_builtins(this);

  // TODO: move to Init_builtins.
  this->add_builtin_function("eval", new CallbackFunction(builtin_eval));
  this->add_builtin_function("do", new CallbackFunction(builtin_do));
}

void VM::copy_all_public_symbols(ID srcid) {
  std::shared_ptr<std::map<ID, SharedValue>> src(this->file_scope_map_[srcid]);

  for (auto iter = src->begin(); iter != src->end(); iter++) {
    this->file_scope_->insert(
        file_scope_body_t::value_type(iter->first, iter->second));
  }
}

// TODO use Value*
SharedPtr<tora::Value> VM::set_item(const SharedPtr<tora::Value> &container,
                                    const SharedPtr<tora::Value> &index,
                                    const SharedPtr<tora::Value> &rvalue)
    const {
  switch (container->value_type) {
    case VALUE_TYPE_OBJECT:
      return object_set_item(container.get(), index.get(), rvalue.get()).get();
    case VALUE_TYPE_HASH: {
      std::string key = index->to_s();
      hash_set_item(container.get(), key, rvalue.get());
      return new_undef_value();
    }
    case VALUE_TYPE_ARRAY: {
      array_set_item(container.get(), index->to_int(), rvalue.get());
      return new_undef_value();
    }
    default:
      throw new ExceptionValue(
          "%s is not a container. You cannot store item.\n",
          container->type_str());
  }
}

void VM::dump_frame() {
  printf("-- dump frame             --\n");
  int i = 0;
  for (auto f = frame_stack->begin(); f != frame_stack->end(); f++) {
    printf("type: %s [%d]\n", (*f)->type_str(), i++);
    /*
    for (size_t n=0; n<(*f)->vars.size(); n++) {
        printf("  %ld:", (long int) n);
        SharedPtr<Value> val = (*f)->vars.at(n);
        if (val.get()) {
            dump_value(val);
        } else {
            printf(" (null)\n");
        }
    }
    */
  }
  printf("---------------\n");
}

void VM::dump_stack() {
  printf("-- STACK DUMP --\nSP: %d\n", sp);
  for (size_t i = 0; i < stack.size(); i++) {
    printf("[%ld] ", (long int)i);
    dump_value(stack.at(i));
  }
  printf("----------------\n");
}

void VM::extract_tuple(const Value *t) {
  assert(type(t) == VALUE_TYPE_TUPLE);
  tra_int ts = tuple_size(t);
  for (int i = 0; i < ts; i++) {
    this->stack.push_back(tuple_get_item(t, i));
  }
}

SharedPtr<tora::Value> VM::get_self() {
  auto iter = this->frame_stack->rbegin();
  for (; iter != this->frame_stack->rend(); ++iter) {
    if ((*iter)->type == FRAME_TYPE_FUNCTION) {
      if ((*iter)->upcast<FunctionFrame>()->self) {
        return (*iter)->upcast<FunctionFrame>()->self;
      } else {
        return new_undef_value();
      }
    }
  }
  throw new ExceptionValue("Cannot call 'self' method out of method.");
}

void VM::handle_exception(const SharedPtr<Value> &exception) {
  assert(frame_stack->size() > 0);

  int lineno = ops->get_lineno(pc);

  while (1) {
    if (frame_stack->size() == 1) {
      if (exception->value_type == VALUE_TYPE_STR) {
        fprintf(stderr, "%s line %d.\n", get_str_value(exception)->c_str(),
                lineno);
      } else if (exception->value_type == VALUE_TYPE_EXCEPTION) {
        if (exception->upcast<ExceptionValue>()->exception_type() ==
            EXCEPTION_TYPE_GENERAL) {
          fprintf(stderr, "%s line %d\n",
                  exception->upcast<ExceptionValue>()->message().c_str(),
                  lineno);
        } else if (exception->upcast<ExceptionValue>()->exception_type() ==
                   EXCEPTION_TYPE_ERRNO) {
          fprintf(stderr, "%s line %d\n",
                  get_strerror(
                      exception->upcast<ErrnoExceptionValue>()->get_errno())
                      .c_str(),
                  lineno);
        } else {
          fprintf(stderr, "%s line %d\n",
                  exception->upcast<ExceptionValue>()->message().c_str(),
                  lineno);
        }
      } else {
        fprintf(stderr, "died\n");
        dump_value(exception);
      }
      exit(1);
    }

    SharedPtr<LexicalVarsFrame> frame(frame_stack->back());
    if (frame->type == FRAME_TYPE_FUNCTION) {
      FunctionFrame *fframe = static_cast<FunctionFrame *>(frame.get());
      pc = fframe->return_address;
      ops = fframe->orig_ops;

      stack.resize(frame->top);

      frame_stack->pop_back();
    } else if (frame->type == FRAME_TYPE_TRY) {
      TryFrame *tframe = static_cast<TryFrame *>(frame.get());
      pc = tframe->return_address + 1;

      stack.resize(frame->top);

      MortalTupleValue t;
      tuple_push(t.get(), new_undef_value());
      tuple_push(t.get(), exception.get());

      frame_stack->pop_back();

      stack.push_back(t.get());

      break;
    } else {
      // printf("THIS IS NOT A FUNCTION FRAME\n");
      stack.resize(frame->top);
      frame_stack->pop_back();
    }
  }
}

void VM::execute() {
  bool next = false;
  do {
    try {
      if (exec_trace) {
        this->execute_trace();
      } else {
        this->execute_normal();
      }
      next = false;
    }
    catch (Value *exception) {
      SharedPtr<Value> v(exception);
      handle_exception(v);
      next = true;
    }
    catch (const SharedPtr<Value> &exception) {
      handle_exception(exception);
      next = true;
    };
  } while (next);
}

void VM::dump_value(const SharedPtr<Value> &v) {
  Inspector ins(this);
  printf("%s\n", ins.inspect(v).c_str());
}

void VM::dump_pad() {
  assert(this->frame_stack->size() != 0);
  this->frame_stack->back()->dump_pad(this);
}

void VM::function_call(int argcnt, const SharedPtr<CodeValue> &code,
                       const SharedPtr<Value> &self) {
  assert(code.get());

  SharedPtr<FunctionFrame> fframe =
      new FunctionFrame(this, argcnt, stack.size(), this->ops);
  fframe->return_address = this->pc;
  fframe->argcnt = argcnt;
  fframe->code = code;
  fframe->self = self;

  pc = -1;
  this->ops = code->code_opcodes();

// TODO: vargs support
// TODO: kwargs support
#ifndef NDEBUG
  if (code->code_params() && argcnt != (int)code->code_params()->size()) {
    fprintf(stderr,
            "[BUG] argument count mismatch. name: %s, argcnt: %d, "
            "code_params.size(): %ld\n",
            symbol_table->id2name(code->package_id()).c_str(), argcnt,
            (long int)code->code_params()->size());
    abort();
  }
#endif
  mark_stack.push_back(stack.size());
  frame_stack->push_back(fframe);
}

void VM::function_call_ex(int argcnt, const SharedPtr<CodeValue> &code,
                          const SharedPtr<Value> &self) {
  if (code->is_native()) {
    this->call_native_func(code->callback(), argcnt);
  } else {
    // inject codevalue to VM, and execute it.
    size_t pc = this->pc;
    // TODO: catch exceptions in destroy

    SharedPtr<OPArray> end_ops = new OPArray();
    end_ops->push_back(new OP(OP_END), -1);

    this->function_call(argcnt, code, self);
    this->frame_stack->back()->upcast<FunctionFrame>()->return_address = -1;
    SharedPtr<OPArray> orig_ops =
        this->frame_stack->back()->upcast<FunctionFrame>()->orig_ops;
    this->frame_stack->back()->upcast<FunctionFrame>()->orig_ops = end_ops;
    this->pc = 0;
    this->execute();

    // restore
    // TODO: restore variables by RAII
    this->pc = pc;
    this->ops = orig_ops;
  }
}

void VM::load_dynamic_library(const std::string &filename,
                              const std::string &endpoint) {
  void *handle = dlopen(filename.c_str(), RTLD_LAZY | RTLD_GLOBAL);
  if (!handle) {
    std::string errmsg(dlerror());
    throw new ExceptionValue(errmsg);
  }

  dlerror();  // clear last error

  dl_callback_t sym = (dl_callback_t)dlsym(handle, endpoint.c_str());
  const char *err = dlerror();
  if (err != NULL) {
    std::string errmsg(err);
    throw new ExceptionValue(errmsg);
  }
  sym(this);

  //  if (dlclose(handle) != 0) {
  //      std::string errmsg(dlerror());
  //      throw new ExceptionValue(errmsg);
  //  }
}

void VM::add_library_path(const std::string &dir) {
  SharedValue libpath = this->global_vars->at(GLOBAL_VAR_LIBPATH);
  MortalStrValue dir_s(dir);
  array_push_back(libpath.get(), dir_s.get());
}

/**
 * push arguments to stack. and call this method.
 *
 * call method. if method is not available, call parent class' method.
 */
void VM::call_method(const SharedPtr<Value> &object,
                     const SharedPtr<Value> &function_id_v) {
  if (!(stack.size() >= (size_t)get_int_operand())) {
    // printf("[BUG] bad argument: %s requires %d arguments but only %ld items
    // available on stack(OP_FUNCALL)\n", funname_c, get_int_operand(), (long
    // int) stack.size());
    dump_stack();
    abort();
  }

  if (object->value_type == VALUE_TYPE_UNDEF) {
    throw new ExceptionValue(
        "NullPointerException: Can't call method %s on an undefined value.",
        symbol_name(this, function_id_v.get()).c_str());
  }

  assert(function_id_v->value_type == VALUE_TYPE_SYMBOL);
  ID function_id = symbol_id(function_id_v.get());

  std::set<ID> seen;
  if (object->value_type == VALUE_TYPE_OBJECT) {
    this->call_method(object, object_class(object.get()),
                      function_id, seen);
  } else if (object->value_type == VALUE_TYPE_SYMBOL) {
    printf("[OBSOLETE] MAY NOT REACHE HERE\n");
    abort();
    // this->call_method(object,
    // this->get_class(symbol_id(object)), function_id, seen);
  } else if (object->value_type == VALUE_TYPE_CLASS) {
    // class method
    this->call_method(object, object.get(), function_id, seen);
  } else if (object->value_type == VALUE_TYPE_FILE_PACKAGE) {
    Value *v = file_package_find(object.get(), function_id);
    if (v) {
      int argcnt = get_int_operand();
      switch (v->value_type) {
        case VALUE_TYPE_CODE:
          this->function_call_ex(argcnt, v->upcast<CodeValue>(), object);
          break;
        case VALUE_TYPE_CLASS:
          stack.push_back(v);
          break;
        default:
          this->die("Unknown stuff in FilePackage: %s\n", v->type_str());
      }
    } else {
      this->call_method(
          object, get_builtin_class(symbol_table->get_id(object->type_str())),
          function_id, seen);
    }
  } else {
    this->call_method(
        object, get_builtin_class(symbol_table->get_id(object->type_str())),
        function_id, seen);
  }
}

void VM::call_method(const SharedPtr<Value> &object,
                     const SharedPtr<Value> &klass, ID function_id,
                     std::set<ID> &seen) {
  seen.insert(class_name_id(klass.get()));
  // std::cout << klass->name() << " " << id2name(function_id) << std::endl;

  Value* code_v = class_get_method(klass.get(), function_id);
  if (code_v) {
    assert(code_v->value_type == VALUE_TYPE_CODE);
    SharedPtr<CodeValue> code = code_v->upcast<CodeValue>();
    int argcnt = get_int_operand();

    if (code->is_native()) {
      // FunctionFrame* fframe = new FunctionFrame(this, argcnt,
      // stack.size()-argcnt);
      // fframe->return_address = pc;
      // fframe->code = code;

      // frame_stack->push_back(fframe);
      stack.push_back(object);
      this->call_native_func(code->callback(), argcnt + 1);
      // delete fframe;
      // frame_stack->pop_back();
    } else {
      SharedPtr<FunctionFrame> fframe =
          new FunctionFrame(this, argcnt, stack.size(), ops);
      fframe->return_address = pc;
      fframe->argcnt = argcnt;
      fframe->code = code;
      fframe->self = object;

      pc = -1;
      this->ops = code->code_opcodes();

      // TODO: vargs support
      // TODO: kwargs support
      // TODO: support code_defaults
      if (code->code_params() && argcnt != code->code_params()->size()) {
        throw new ArgumentExceptionValue(
            "%s::%s needs %d arguments but you passed %d arguments",
            symbol_table->id2name(class_name_id(klass.get())).c_str(),
            symbol_table->id2name(function_id).c_str(),
            code->code_params()->size());
      }
      // assert(argcnt == (int)code->code_params()->size());
      mark_stack.push_back(stack.size());
      frame_stack->push_back(fframe);
    }
  } else {
    // find in super class.
    SharedPtr<Value> super = class_superclass(klass.get()).get();
    if (super.get()) {
      this->call_method(object, super, function_id, seen);
      // symbol class
    } else if (object->value_type == VALUE_TYPE_CLASS &&
               seen.find(SYMBOL_CLASS_CLASS) == seen.end()) {
      this->call_method(object, this->get_builtin_class(SYMBOL_CLASS_CLASS),
                        function_id, seen);
      return;
      // object class(UNIVERSAL)
    } else if (class_name_id(klass.get()) != SYMBOL_OBJECT_CLASS &&
               seen.find(SYMBOL_OBJECT_CLASS) == seen.end()) {
      this->call_method(object, this->get_builtin_class(SYMBOL_OBJECT_CLASS),
                        function_id, seen);
      return;
    } else {
      // dump_value(function_id);
      // dump_value(object);
      this->die(
          "Unknown method %s for %s",
          this->symbol_table->id2name(function_id).c_str(),
          this->symbol_table->id2name(object->object_package_id()).c_str());
    }
  }
}

void VM::add_constant(const char *name, int n) {
  this->add_function(get_id(name), new CallbackFunction(n));
}

void VM::add_function(const char *name, const CallbackFunction *cb) {
  this->add_function(get_id(name), cb);
}

void VM::add_function(ID id, const CallbackFunction *cb) {
  this->file_scope_->insert(
      file_scope_body_t::value_type(id, new CodeValue(0, id, cb)));
}

void VM::add_function(const SharedPtr<CodeValue> &code) {
  this->file_scope_->insert(
      file_scope_body_t::value_type(code->func_name_id(), code.get()));
}

void VM::add_class(const SharedPtr<Value> &klass) {
  assert(klass->value_type == VALUE_TYPE_CLASS);
  this->file_scope_->insert(file_scope_body_t::value_type(
      class_name_id(klass.get()), klass.get()));
}

const SharedPtr<Value> &VM::get_builtin_class(ID name_id) const {
  auto iter = builtin_classes_.find(name_id);
  if (iter != builtin_classes_.end()) {
    return iter->second;
  } else {
    fprintf(stderr, "%s is not a builtin class.",
            symbol_table->id2name(name_id).c_str());
    abort();
  }
}

void VM::klass(const SharedPtr<Value> &k) {
  assert(k->value_type == VALUE_TYPE_CLASS);
  klass_.reset(k.get());
}

void VM::add_builtin_class(const SharedPtr<Value> &klass) {
  this->builtin_classes_.insert(std::make_pair(class_name_id(klass.get()), klass));
}

SharedPtr<Value> VM::get_class(ID name_id) const {
  {
    auto iter = this->file_scope_->find(name_id);
    if (iter != this->file_scope_->end()) {
      if (iter->second->value_type != VALUE_TYPE_CLASS) {
        throw new ExceptionValue("%s is not a class", id2name(name_id).c_str());
      }
      return iter->second.get();
    }
  }
  {
    auto iter = builtin_classes_.find(name_id);
    if (iter != builtin_classes_.end()) {
      if (iter->second->value_type != VALUE_TYPE_CLASS) {
        throw new ExceptionValue("%s is not a class", id2name(name_id).c_str());
      }
      return iter->second;
    }
  }
  throw new ExceptionValue("There is no class named %s",
                           id2name(name_id).c_str());
}

std::string VM::id2name(ID id) const { return symbol_table->id2name(id); }

ID VM::get_id(const std::string &name) const {
  return symbol_table->get_id(name);
}

Value *VM::to_bool(bool b) const {
  return b ? this->true_value_.get() : this->false_value_.get();
}

void VM::add_builtin_function(const char *name, CallbackFunction *func) {
  ID id = this->symbol_table->get_id(std::string(name));
  this->builtin_functions_.insert(
      std::make_pair(id, new CodeValue(0, id, func)));
}
