#include <Engine.Base/Log.h>
#include <stdio.h>
#include <stdarg.h>

void trace_print(const char* s)
{
    fprintf(stdout, "[+] ");
    fputs(s, stdout);
}

void trace_print_format(const char* s, ...)
{
    fprintf(stdout, "[+] ");

    va_list va;
    va_start(va, s);
    vfprintf(stdout, s, va);
    va_end(va);
}

void trace_log(const char* s)
{
    fprintf(stdout, "[+] ");
    fputs(s, stdout);

    fputc('\n', stdout);
}

void trace_log_format(const char* s, ...)
{
    fprintf(stdout, "[+] ");

    va_list va;
    va_start(va, s);
    vfprintf(stdout, s, va);
    va_end(va);

    fputc('\n', stdout);
}

void trace_error(const char* s)
{
    fprintf(stderr, "[!] ");
    fputs(s, stderr);

    fputc('\n', stderr);
}

void trace_error_format(const char* s, ...)
{
    fprintf(stderr, "[!] ");

    va_list va;
    va_start(va, s);
    vfprintf(stderr, s, va);
    va_end(va);

    fputc('\n', stderr);
}

void trace_assert(const char* s)
{
    fprintf(stderr, "[+] ");
    fputs(s, stderr);
    fputc('\n', stderr);
    asm("int3");
}

void trace_assert_format(const char* s, ...)
{
    fprintf(stderr, "[!] ");

    va_list va;
    va_start(va, s);
    vfprintf(stderr, s, va);
    va_end(va);

    fputc('\n', stderr);
    asm("int3");
}