#pragma once
#include "Types.h"

#define EPRINT(s) trace_print(s)
#define EPRINTF(s, ...) trace_print_format(s, __VA_ARGS__)

#define ELOG(s) trace_log(s)
#define ELOGF(s, ...) trace_log_format(s, __VA_ARGS__)

#define EERROR(s) trace_error(s)
#define EERRORF(s, ...) trace_error_format(s, __VA_ARGS__)

#define EASSERT(c, s) do { if(!(c)) trace_assert(s); } while(0)
#define EASSERTF(c, s, ...)  do { if(!(c)) trace_assert_format(s, __VA_ARGS__); } while(0)

#if defined(ENGINE_DEBUG_BUILD)
#define EDEBUG(s) trace_log("(debug) " s)
#define EDEBUGF(s, ...) trace_log_format("(debug) " s, __VA_ARGS__)
#else
#define EDEBUG(s)
#define EDEBUGF(s, ...)
#endif

#ifndef __builtin_assert
#define __builtin_assert(c) EASSERT(c, #c)
#endif

LIB_LINKAGE EXTERN_C void trace_print(const char*);
LIB_LINKAGE EXTERN_C void trace_print_format(const char*, ...);
LIB_LINKAGE EXTERN_C void trace_log(const char*);
LIB_LINKAGE EXTERN_C void trace_log_format(const char*, ...);
LIB_LINKAGE EXTERN_C void trace_error(const char*);
LIB_LINKAGE EXTERN_C void trace_error_format(const char*, ...);
LIB_LINKAGE EXTERN_C void trace_assert(const char*);
LIB_LINKAGE EXTERN_C void trace_assert_format(const char*, ...);