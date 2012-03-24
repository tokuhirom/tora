#include <stddef.h>
#include "picohttpparser.h"

#include "tora.h"
#include <vm.h>
#include <value/array.h>
#include <value/hash.h>
#include <value/tuple.h>
#include <package.h>
#include <shared_ptr.h>

#define MAX_HEADER_NAME_LEN 1024
#define MAX_HEADERS         128

using namespace tora;

enum {
    HEADERS_NONE = 0,       // don't parse headers. It's fastest. if you want only special headers, also fastest.
    HEADERS_AS_HASHREF = 1,    // HTTP::Headers compatible HashRef, { header_name => "header_value" or ["val1", "val2"] }
    HEADERS_AS_ARRAYREF = 2,    // Ordered ArrayRef : [ name, value, name2, value2 ... ]
};

//////////////////////////////////////////////////////////////////////////////////
// utils.

static inline
char tou(char ch)
{
  if ('a' <= ch && ch <= 'z')
    ch -= 'a' - 'A';
  return ch;
}

static inline char tol(char const ch)
{
  return ('A' <= ch && ch <= 'Z')
    ? ch - ('A' - 'a')
    : ch;
}

/* copy src to dest with normalization.
   dest must have enough size for src */
static inline void normalize_response_header_name(
        char* const dest,
        const char* const src, size_t const len) {
    size_t i;
    for(i = 0; i < len; i++) {
        dest[i] = tol(src[i]);
    }
}

static inline void concat_multiline_header(BytesValue * val, const char * const cont, size_t const cont_len) {
    val->str_value() += "\n"; // XXX: is it collect?
    val->str_value() += std::string(cont, cont_len);
}

static
int header_is(const struct phr_header* header, const char* name,
		     size_t len)
{
  const char* x, * y;
  if (header->name_len != len)
    return 0;
  for (x = header->name, y = name; len != 0; --len, ++x, ++y)
    if (tou(*x) != *y)
      return 0;
  return 1;
}

static inline
size_t find_ch(const char* s, size_t len, char ch)
{
  size_t i;
  for (i = 0; i != len; ++i, ++s)
    if (*s == ch)
      break;
  return i;
}


static inline
int hex_decode(const char ch)
{
  int r;
  if ('0' <= ch && ch <= '9')
    r = ch - '0';
  else if ('A' <= ch && ch <= 'F')
    r = ch - 'A' + 0xa;
  else if ('a' <= ch && ch <= 'f')
    r = ch - 'a' + 0xa;
  else
    r = -1;
  return r;
}

static
char* url_decode(const char* s, size_t len)
{
  char* dbuf, * d;
  size_t i;
  
  for (i = 0; i < len; ++i)
    if (s[i] == '%')
      goto NEEDS_DECODE;
  return (char*)s;
  
 NEEDS_DECODE:
  dbuf = (char*)malloc(len - 1);
  assert(dbuf != NULL);
  memcpy(dbuf, s, i);
  d = dbuf + i;
  while (i < len) {
    if (s[i] == '%') {
      int hi, lo;
      if ((hi = hex_decode(s[i + 1])) == -1
	  || (lo = hex_decode(s[i + 2])) == -1) {
        free(dbuf);
    	return NULL;
      }
      *d++ = hi * 16 + lo;
      i += 3;
    } else
      *d++ = s[i++];
  }
  *d = '\0';
  return dbuf;
}

static inline
int store_url_decoded(const SharedPtr<HashValue> &env, const char* name, size_t name_len,
			       const char* value, size_t value_len)
{
    char* decoded = url_decode(value, value_len);
    if (decoded == NULL) {
        return -1;
    }
  
    if (decoded == value) {
        env->set(std::string(name, name_len), new StrValue(std::string(value, value_len)));
    } else {
        env->set(std::string(name, name_len), new StrValue(std::string(decoded)));
        free(decoded);
    }
    return 0;
}

//////////////////////////////////////////////////////////////////////////////////
// bindings.

static SharedPtr<Value> HTTP_Parser_parse_http_request(VM *vm, Value *str, Value *hash_v) {
    if (str->value_type != VALUE_TYPE_BYTES) {
        throw new ExceptionValue("You should not pass the non-bytes value: %s", str->type_str());
    }
    if (hash_v->value_type != VALUE_TYPE_HASH) {
        throw new ExceptionValue("second argument should be Hash.");
    }
    SharedPtr<HashValue> hash(static_cast<HashValue*>(hash_v));
    hash->clear();

    SharedPtr<StrValue> last_value;

    const char* method;
    size_t method_len;
    const char* path;
    size_t path_len;
    int minor_version;
    struct phr_header headers[MAX_HEADERS];
    size_t num_headers = MAX_HEADERS;
    char tmp[MAX_HEADER_NAME_LEN + sizeof("HTTP_") - 1];
    size_t question_at;

    int ret = phr_parse_request(
        static_cast<BytesValue*>(str)->c_str(), static_cast<BytesValue*>(str)->size(),
        &method, &method_len,
        &path, &path_len,
        &minor_version,
        headers,
        &num_headers,
        0
    );
    if (ret < 0) {
        goto done;
    }
    hash->set("REQUEST_METHOD", new StrValue(std::string(method, method_len)));
    hash->set("REQUEST_URI", new StrValue(std::string(path, path_len)));
    hash->set("SCRIPT_NAME", new StrValue(std::string("")));
    path_len = find_ch(path, path_len, '#'); /* strip off all text after # after storing request_uri */
    question_at = find_ch(path, path_len, '?');

    if (store_url_decoded(hash, "PATH_INFO", sizeof("PATH_INFO") - 1, path,
                question_at)
        != 0) {
        hash->clear();
        ret = -1;
        goto done;
    }
    if (question_at != path_len) {
        ++question_at;
    }

    hash->set("QUERY_STRING", new StrValue(std::string(path+question_at, path_len - question_at)));
    sprintf(tmp, "HTTP/1.%d", minor_version);
    hash->set("SERVER_PROTOCOL", new StrValue(tmp));

    for (size_t i = 0; i < num_headers; ++i) {
        if (headers[i].name != NULL) {
            const char* name;
            size_t name_len;
            if (header_is(headers + i, "CONTENT-TYPE", sizeof("CONTENT-TYPE") - 1)) {
                name = "CONTENT_TYPE";
                name_len = sizeof("CONTENT_TYPE") - 1;
            } else if (header_is(headers + i, "CONTENT-LENGTH",
                    sizeof("CONTENT-LENGTH") - 1)) {
                name = "CONTENT_LENGTH";
                name_len = sizeof("CONTENT_LENGTH") - 1;
            } else {
                const char* s;
                char* d;
                size_t n;
                if (sizeof(tmp) - 5 < headers[i].name_len) {
                    hash->clear();
                    ret = -1;
                    goto done;
                }
                strcpy(tmp, "HTTP_");
                for (s = headers[i].name, n = headers[i].name_len, d = tmp + 5;
                    n != 0;
                    s++, --n, d++) {
                    *d = *s == '-' ? '_' : tou(*s);
                }
                name = tmp;
                name_len = headers[i].name_len + 5;
            }

            if (hash->has_key(std::string(name, name_len))) {
                SharedPtr<StrValue> v = hash->get(std::string(name, name_len))->upcast<StrValue>();
                v->str_value() += std::string(", ") + std::string(headers[i].value, headers[i].value_len);
                last_value = v.get();
            } else {
                SharedPtr<StrValue> v = new StrValue(std::string(headers[i].value, headers[i].value_len));
                hash->set(std::string(name, name_len), v);
                last_value = v.get();
            }
        } else {
            /* continuing lines of a mulitiline header */
            last_value->str_value() += std::string(headers[i].value, headers[i].value_len);
        }
    }

done:
    return new IntValue(ret);
}

static SharedPtr<Value> HTTP_Parser_parse_http_response(VM *vm, Value *bytes_v, Value *opt, Value *special_headers) {
    if (bytes_v->value_type != VALUE_TYPE_BYTES) {
        throw new ExceptionValue("You must pass bytes value for this function.");
    }

    int minor_version, status;
    const char* msg;
    size_t msg_len;
    struct phr_header headers[MAX_HEADERS];
    size_t num_headers = MAX_HEADERS;
    size_t last_len = 0;
    int const ret             = phr_parse_response(
        static_cast<BytesValue*>(bytes_v)->c_str(), static_cast<BytesValue*>(bytes_v)->size(),
        &minor_version, &status, &msg, &msg_len, headers, &num_headers, last_len);

    SharedPtr<Value> res_headers;
    switch (opt->to_int()) {
    case HEADERS_AS_ARRAYREF:
        res_headers.reset(new ArrayValue());
        break;
        // av_extend((AV*)res_headers, (num_headers * 2) - 1);
    default:
        throw new ExceptionValue("Currently supported only HEADERS_AS_ARRAYREF: %d", opt->to_int());
    }

    char name[MAX_HEADER_NAME_LEN]; /* temp buffer for normalized names */
    BytesValue *last_element_value_sv = NULL;
    for (size_t i = 0; i < num_headers; i++) {
        struct phr_header const h = headers[i];
        if (h.name != NULL) {
            if(h.name_len > sizeof(name)) {
                /* skip if name_len is too long */
                continue;
            }
            normalize_response_header_name(
                name, h.name, h.name_len);
            SharedPtr<BytesValue> namesv  = new BytesValue(name, h.name_len);
            SharedPtr<BytesValue> valuesv = new BytesValue(
                h.value, h.value_len);
            res_headers->upcast<ArrayValue>()->push(namesv);
            res_headers->upcast<ArrayValue>()->push(valuesv);
            last_element_value_sv = valuesv.get();
        } else {
            if (last_element_value_sv) {
                concat_multiline_header(last_element_value_sv, h.value, h.value_len);
            }
        }
    }
    // my ($ret, $minor_version, $status, $message, $headers) = parse_http_response(b"HTTP/1.1 200 OK\r\nHost: example.com\r\n\r\nhogehoge", HEADERS_AS_ARRAYREF, undef);
    SharedPtr<TupleValue> tuple = new TupleValue();
    tuple->push_front(new IntValue(ret));
    tuple->push_front(new IntValue(minor_version));
    tuple->push_front(new IntValue(status));
    tuple->push_front(new BytesValue(msg, msg_len));
    tuple->push_front(res_headers);
    return tuple;
}

extern "C" {

void Init_HTTP_Parser(VM* vm) {
    SharedPtr<Package> pkg = vm->find_package("HTTP::Parser");
    pkg->add_method("parse_http_request", new CallbackFunction(HTTP_Parser_parse_http_request));
    pkg->add_method("parse_http_response", new CallbackFunction(HTTP_Parser_parse_http_response));
    pkg->add_constant("HEADERS_NONE", HEADERS_NONE);
    pkg->add_constant("HEADERS_AS_HASHREF", HEADERS_AS_HASHREF);
    pkg->add_constant("HEADERS_AS_ARRAYREF", HEADERS_AS_ARRAYREF);
}

}
