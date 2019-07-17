#ifndef _USER_METHOD_H
#define _USER_METHOD_H

#define ARRAY_SIZE(a) (sizeof(a)/sizeof(a[0]))
typedef int (*method_cb_t)(int , char*,int);
struct method
{
	char* str;
	method_cb_t cb;	
};

enum header_flag
{
	OK =0,
	NO_METHOD = 1,
	PAGE_NOT_FOUND = 2,
};

#endif
