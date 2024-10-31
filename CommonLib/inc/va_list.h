#pragma once

typedef PBYTE               va_list;

#define STACKITEM_SIZE      sizeof(PVOID)

// Initializes the va_list
#define va_start(List,LastArg)     \
            ((List)=((va_list)&(LastArg) + STACKITEM_SIZE))

// Retrieves the value of the next argument
// And increases the List pointer
#define va_arg(List, Type)	\
	((List) += STACKITEM_SIZE, *((Type *)((List) - STACKITEM_SIZE)))