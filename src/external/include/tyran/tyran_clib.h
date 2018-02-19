/*

MIT License

Copyright (c) 2012 Peter Bjorklund

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/
#ifndef TYRAN_CLIB_H
#define TYRAN_CLIB_H

#if !defined TORNADO_OS_NACL
#include <memory.h>
#else
#include <stdio.h>
#endif

#include <arpa/inet.h>
#include <stdlib.h>

#if defined TORNADO_OS_IOS || defined TORNADO_OS_NACL
#include <string.h>
#endif

char* tyran_str_dup(const char* str);

void tyran_alloc_init(void);
void tyran_alloc_reset(void);

void* tyran_malloc_debug(size_t size, const char* source_file, int line_number);
void tyran_free_debug(void* p, const char* source_file, int line_number);

#define tyran_malloc(size) tyran_malloc_debug(size, __FILE__, __LINE__)
#define tyran_malloc_type(T) (T*) tyran_malloc_debug(sizeof(T), __FILE__, __LINE__)
#define tyran_malloc_type_count(T, N) (T*) tyran_malloc_debug((N) * sizeof(T), __FILE__, __LINE__)

#define tyran_free(p) tyran_free_debug(p, __FILE__, __LINE__)

#define tyran_memcpy_type(T, dest, source, N) memcpy(dest, source, (N) * sizeof(T))
#define tyran_memcpy_type_n(dest, source, N) memcpy(dest, source, (N) * sizeof(*dest))
#define tyran_memmove_octets memmove
#define tyran_memmove_type(T, dest, source, N) memmove(dest, source, (N) * sizeof(T))

#define tyran_memcpy_octets(dest, source, N) memcpy(dest, source, (N))

#define tyran_memcmp memcmp
#define tyran_memset_type(T, V) memset(T, V, sizeof(*T))
#define tyran_memset_type_n(T, V, N) memset(T, V, sizeof(*T) * (N))
#define tyran_mem_clear_type_n(T, N) tyran_memset_type_n(T, 0, N);
#define tyran_mem_clear_type(T) tyran_memset_type(T, 0);
#define tyran_mem_clear(D, N) memset(D, 0, N)

#define tyran_htonl htonl
#define tyran_htons htons
#define tyran_ntohl ntohl
#define tyran_ntohs ntohs

#if defined WIN32
// #pragma warning( disable : 4100 )

#define tyran_sscanf sscanf_s
#define tyran_snprintf sprintf_s
#define tyran_strncpy(dest, dest_size, source, source_size) strncpy_s(dest, dest_size, source, source_size)
#define tyran_strcpy(dest, dest_size, source) strcpy_s(dest, dest_size, source)
#define tyran_fopen fopen_s
#define tyran_fread fread
#define tyran_fclose fclose
#define tyran_strncat strncat_s
#define tyran_strlen strlen

#else

#define tyran_sscanf sscanf
#define tyran_snprintf snprintf
#define tyran_strncpy(dest, dest_size, source, source_size) strncpy(dest, source, source_size)
#define tyran_strcpy(dest, dest_size, source) strcpy(dest, source)
#define tyran_strpbrk strpbrk
#define tyran_strncat strncat
#define tyran_strlen strlen
#define tyran_fopen(F, N, M) *F = fopen(N, M)
#define tyran_fread fread
#define tyran_fclose fclose
#define tyran_strncat strncat

#endif

#define tyran_strcmp strcmp
#define tyran_strncmp strncmp
#define tyran_strstr strstr
#define tyran_fprintf fprintf
#define tyran_fputs fputs
#define tyran_fflush fflush
#define tyran_strdup(str) tyran_str_dup(str)
#define tyran_str_equal(a, b) (tyran_strcmp(a, b) == 0)
#define tyran_str_chr(a, b) strchr(a, b)

#define tyran_fmod fmod
#define tyran_pow pow

#endif
