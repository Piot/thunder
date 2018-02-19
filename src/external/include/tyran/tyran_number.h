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
#ifndef tyran_number_h
#define tyran_number_h

#define TYRAN_NUMBER_MAXEXP 2047

#define TYRAN_NUMBER_EXP(a) ((((u16t*) (&a))[3] & 0x7ff0) >> 4)

#define tyran_number_is_normal(a) (TYRAN_NUMBER_EXP(a) != TYRAN_NUMBER_MAXEXP)

#if defined _MSC_VER
#include <float.h>
#define tyran_number_is_nan(x) _isnan(x)
#define tyran_number_is_infinity(x) !_finite(x)
#else
#include <math.h>
#define tyran_number_is_nan(a) isnan(a)
#define tyran_number_is_infinity(a) isinf(a)
#endif

#endif
