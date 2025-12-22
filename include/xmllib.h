#ifndef __XMLLIB_H__
#define __XMLLIB_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>

#define XLDECL extern

typedef struct xl_driver xl_driver_t;
typedef struct xmllib	 xmllib_t;
typedef struct xl_node	 xl_node_t;

#define XL_SKIPPABLE(x) (((x) == ' ') || ((x) == '\t') || ((x) == '\n') || ((x) == '\r'))

struct xl_driver {
	int (*open)(xmllib_t* handle);
	int (*read)(xmllib_t* handle, void* data, int size);
	void (*close)(xmllib_t* handle);
};

enum XL_NODE_TYPE {
	XL_NODE_COMMENT = 0,
	XL_NODE_NODE,
	XL_NODE_PROCESS,
	XL_NODE_DOCTYPE
};

struct xl_node {
	int   type;
	char* name;
	char* text;

	xl_node_t* root;
	xl_node_t* parent;

	xl_node_t* first_child;

	xl_node_t* prev;
	xl_node_t* next;
};

struct xmllib {
	xl_driver_t* driver;
	void*	     drv_opaque;
	void*	     drv_arg;
	xl_node_t*   root;
};

#ifdef __cplusplus
extern "C" {
#endif

/* core.c */
XLDECL xmllib_t* xl_open(xl_driver_t* driver, void* arg);

XLDECL int xl_parse(xmllib_t* handle);

XLDECL void xl_close(xmllib_t* handle);

/* file.c */
XLDECL xl_driver_t* xl_driver_file;

XLDECL xmllib_t* xl_open_file(const char* filename);

/* util.c */
XLDECL char* xl_util_trim(const char* str);

/* unicode.c */
XLDECL int xl_unicode_count(unsigned char c);

XLDECL int xl_unicode_8_to_32(const char* input, int* output);

XLDECL int xl_unicode_32_to_8(const int input, char* output);

#ifdef __cplusplus
}
#endif

#endif
