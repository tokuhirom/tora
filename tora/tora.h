#ifndef TORA_H_
#define TORA_H_

#include "config.h"

// Debug Macro
#ifdef DEBUG
#define DBG(fmt, ...) printf(fmt, __VA_ARGS__)
#define DBG2(fmt) printf(fmt)
#else
#define DBG(...)
#define DBG2(...)
#endif

#ifndef MAXPATHLEN
#ifdef PATH_MAX
#ifdef _POSIX_PATH_MAX
#if PATH_MAX > _POSIX_PATH_MAX
/* POSIX 1990 (and pre) was ambiguous about whether PATH_MAX
 * included the null byte or not.  Later amendments of POSIX,
 * XPG4, the Austin Group, and the Single UNIX Specification
 * all explicitly include the null byte in the PATH_MAX.
 * Ditto for _POSIX_PATH_MAX. */
#define MAXPATHLEN PATH_MAX
#else
#define MAXPATHLEN _POSIX_PATH_MAX
#endif
#else
#define MAXPATHLEN (PATH_MAX + 1)
#endif
#else
#ifdef _POSIX_PATH_MAX
#define MAXPATHLEN _POSIX_PATH_MAX
#else
#define MAXPATHLEN 1024 /* Err on the large side. */
#endif
#endif
#endif

#define TODO()                                                          \
  printf("This function is not implemented yet. %s at %d.\n", __FILE__, \
         __LINE__);                                                     \
  abort();

#ifdef _WIN32
#define TORA_EXPORT __declspec(dllexport)
#else
#define TORA_EXPORT
#endif

namespace tora {

class VM;

typedef void (*dl_callback_t)(VM*);

typedef unsigned int ID;
};

#endif  // TORA_H_
