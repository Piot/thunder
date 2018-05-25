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
#ifndef tyran_log_h
#define tyran_log_h

#include <tyran/tyran_clib.h>

#include <stdio.h>
#include <stdlib.h>

enum tyran_log_type { TYRAN_LOG_TYPE_VERBOSE, TYRAN_LOG_TYPE_INFO, TYRAN_LOG_TYPE_WARN, TYRAN_LOG_TYPE_ERROR };

typedef struct tyran_log {
	void (*log)(enum tyran_log_type type, const char* string);
} tyran_log;

extern tyran_log g_log;

#define TYRAN_LOG_EX(logtype, ...)                                                                                                                                                                                                                             \
	{                                                                                                                                                                                                                                                          \
		char _temp_str[1024];                                                                                                                                                                                                                                  \
		sprintf(_temp_str, __VA_ARGS__);                                                                                                                                                                                                                       \
		g_log.log(logtype, _temp_str);                                                                                                                                                                                                                         \
	}

#if defined TYRAN_CONFIGURATION_DEBUG

#define TYRAN_LOG_VERBOSE(...) TYRAN_LOG_EX(TYRAN_LOG_TYPE_VERBOSE, __VA_ARGS__)

#define TYRAN_LOG_INFO(...) TYRAN_LOG_EX(TYRAN_LOG_TYPE_INFO, __VA_ARGS__)

#define TYRAN_LOG_WARN(...) TYRAN_LOG_EX(TYRAN_LOG_TYPE_WARN, __VA_ARGS__)

#define TYRAN_LOG_NO_LF(...)                                                                                                                                                                                                                                   \
	{                                                                                                                                                                                                                                                          \
		TYRAN_LOG(0, __VA_ARGS__);                                                                                                                                                                                                                             \
	}
#define TYRAN_SOFT_ERROR(...) TYRAN_LOG_WARN(__VA_ARGS__);
#define TYRAN_BREAK
#define TYRAN_ERROR(...)                                                                                                                                                                                                                                       \
	TYRAN_LOG_EX(TYRAN_LOG_TYPE_ERROR, __VA_ARGS__);                                                                                                                                                                                                           \
	TYRAN_OUTPUT(__VA_ARGS__);                                                                                                                                                                                                                                 \
	TYRAN_BREAK;
#define TYRAN_ASSERT(expression, ...)                                                                                                                                                                                                                          \
	if (!(expression)) {                                                                                                                                                                                                                                       \
		TYRAN_ERROR(__VA_ARGS__);                                                                                                                                                                                                                              \
		TYRAN_BREAK;                                                                                                                                                                                                                                           \
	}

#else

#define TYRAN_LOG_VERBOSE(...)
#define TYRAN_LOG_INFO(...)
#define TYRAN_LOG_WARN(...)
#define TYRAN_LOG_NO_LF(...)
#define TYRAN_SOFT_ERROR(...) TYRAN_LOG_EX(TYRAN_LOG_TYPE_ERROR, __VA_ARGS__);
#define TYRAN_BREAK abort()
#define TYRAN_ERROR(...) TYRAN_LOG_EX(TYRAN_LOG_TYPE_ERROR, __VA_ARGS__);

#define TYRAN_ASSERT(expression, ...)

#endif

#define TYRAN_OUTPUT(...)                                                                                                                                                                                                                                      \
	{                                                                                                                                                                                                                                                          \
		tyran_fprintf(stdout, __VA_ARGS__);                                                                                                                                                                                                                    \
		tyran_fprintf(stdout, "\n");                                                                                                                                                                                                                           \
		tyran_fflush(stdout);                                                                                                                                                                                                                                  \
	}
#define TYRAN_OUTPUT_NO_LF(...)                                                                                                                                                                                                                                \
	{                                                                                                                                                                                                                                                          \
		tyran_fprintf(stdout, __VA_ARGS__);                                                                                                                                                                                                                    \
		tyran_fflush(stdout);                                                                                                                                                                                                                                  \
	}

#endif
