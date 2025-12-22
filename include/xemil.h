#ifndef __XEMIL_H__
#define __XEMIL_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>

#define XLDECL extern

typedef struct xl_driver    xl_driver_t;
typedef struct xemil	    xemil_t;
typedef struct xl_node	    xl_node_t;
typedef struct xl_attribute xl_attribute_t;

#define XL_SKIPPABLE(x) (((x) == ' ') || ((x) == '\t') || ((x) == '\n') || ((x) == '\r'))

struct xl_driver {
	int (*open)(xemil_t* handle);
	int (*read)(xemil_t* handle, void* data, int size);
	void (*close)(xemil_t* handle);
};

enum XL_NODE_TYPE {
	XL_NODE_COMMENT = 0,
	XL_NODE_NODE,
	XL_NODE_PROCESS
};

struct xl_attribute {
	char* key;
	char* value;

	xl_attribute_t* prev;
	xl_attribute_t* next;
};

struct xl_node {
	int		type;
	char*		name;
	char*		text;
	xl_attribute_t* first_attribute;

	xl_node_t* root;
	xl_node_t* parent;

	xl_node_t* first_child;

	xl_node_t* prev;
	xl_node_t* next;
};

struct xemil {
	xl_driver_t* driver;
	void*	     drv_opaque;
	void*	     drv_arg;
	xl_node_t*   root;
};

#ifdef __cplusplus
extern "C" {
#endif

/* core.c */
XLDECL xemil_t* xl_open(xl_driver_t* driver, void* arg);

XLDECL int xl_parse(xemil_t* handle);

XLDECL void xl_close(xemil_t* handle);

/* file.c */
XLDECL xl_driver_t* xl_driver_file;

XLDECL xemil_t* xl_open_file(const char* filename);

/* util.c */
XLDECL char* xl_util_trim(const char* str);

/* unicode.c */
XLDECL int xl_unicode_count(unsigned char c);

XLDECL int xl_unicode_8_to_32(const char* input, int* output);

XLDECL int xl_unicode_32_to_8(const int input, char* output);

/* array.c */
XLDECL void xl_array_push(int** array, int value);

XLDECL int xl_array_length(int** array);

XLDECL void xl_array_pop(int** array);

XLDECL void xl_array_free(int** array);

#ifdef __cplusplus
}
#endif

#endif
