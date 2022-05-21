#ifndef INUT_ASSERT_H
#define INUT_ASSERT_H

#include "inut/test_report.h"
#include "inut/defs.h"
#include <stdio.h>

// TODO: dynamic string size
// TODO: handle name conflicts
#define INUT_MAX_STR 4096
extern char _inut_buffer[INUT_MAX_STR];

#define inut_typecheck(type,var) { typedef void (*type_t)(type); type_t tmp = (type_t)0; if(0) tmp(var);}

#define _inut_wrap(block) do {\
	block\
} while(0)

#define assert_zero(a) assert_int_equal(a, 0)

#define _inut_assert_sprintf(LAB, FMTA, FMTB, VALA, VALB) sprintf(_inut_buffer, INUT_TAB INUT_RED "• Error: Expected %s " FMTA " to equal " FMTB INUT_RESET "\n\n" INUT_TAB INUT_GREEN "+ expected" INUT_RESET " " INUT_RED "- actual" INUT_RESET "\n\n" INUT_TAB INUT_GREEN "+ " FMTA INUT_RESET "\n" INUT_TAB INUT_RED "- " FMTB INUT_RESET "\n\n" INUT_TAB "at %s, line %d", LAB, VALA, VALB, VALA, VALB, __FILE__, __LINE__)

#define assert_int_equal(expected, actual) _inut_wrap(\
int _inut_expected = (expected);\
int _inut_actual = (actual);\
if(_inut_expected != _inut_actual) {\
    _inut_assert_sprintf("integer", "%d", "%d", _inut_expected, _inut_actual);\
    return fail(_inut_buffer);\
})


#define assert_int_zero(actual) assert_int_equal(0, actual)

#define assert_uint_equal(expected, actual) _inut_wrap(\
unsigned int _inut_expected = (expected);\
unsigned int _inut_actual = (actual);\
if(_inut_expected != _inut_actual) {\
    _inut_assert_sprintf("unsigned integer", "%u", "%u", _inut_expected, _inut_actual);\
    return fail(_inut_buffer);\
})


#define assert_ptr_equal(expected, actual) _inut_wrap(\
void * _inut_expected = (expected);\
void * _inut_actual = (actual);\
if(_inut_expected != _inut_actual) {\
    if (!_inut_expected) {\
    	_inut_assert_sprintf("pointer", "%s", "%p", "NULL", _inut_actual);\
    }\
    else if (!_inut_actual) {\
    	_inut_assert_sprintf("pointer", "%p", "%s", _inut_expected, "NULL");\
    }\
    else {\
    	_inut_assert_sprintf("pointer", "%p", "%p", _inut_expected, _inut_actual);\
    }\
    return fail(_inut_buffer);\
})

#define assert(actual) assert_boolean(actual)

#define assert_boolean(actual) _inut_wrap(\
int _inut_actual = (actual);\
if(!_inut_actual) {\
    _inut_assert_sprintf("boolean", "%s", "%s", "true", "false");\
    return fail(_inut_buffer);\
})

#endif

