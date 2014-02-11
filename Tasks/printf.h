#ifndef __PRINTF_H__
#define __PRINTF_H__

#include <stdio.h>
#include <stdarg.h>

signed int sprintf(char *pStr, const char *pFormat, ...);
signed int printf(const char *pFormat, ...);

#endif // __PRINTF_H__
