#ifndef __XMLLIB_H__
#define __XMLLIB_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>

#define XLDECL extern

typedef struct xmllib_driver xmllib_driver_t;
typedef struct xmllib	     xmllib_t;
typedef struct xmllib_node   xmllib_node_t;

struct xmllib_driver {
	int (*open)(xmllib_t* handle);
	int (*read)(xmllib_t* handle, void* data, int size);
	void (*close)(xmllib_t* handle);
};

enum XMLLIB_NODE_TYPE {
	XMLLIB_NODE_COMMENT = 0,
	XMLLIB_NODE_NODE,
	XMLLIB_NODE_PROCESS,
	XMLLIB_NODE_DOCTYPE
};

struct xmllib_node {
	int   type;
	char* name;
	char* text;

	xmllib_node_t* root;
	xmllib_node_t* parent;

	xmllib_node_t* first_child;

	xmllib_node_t* prev;
	xmllib_node_t* next;
};

struct xmllib {
	xmllib_driver_t* driver;
	void*		 drv_opaque;
	void*		 drv_arg;
	xmllib_node_t*	 root;
};

#ifdef __cplusplus
extern "C" {
#endif

/* core.c */
XLDECL xmllib_t* xmllib_open(xmllib_driver_t* driver, void* arg);

XLDECL int xmllib_parse(xmllib_t* handle);

XLDECL void xmllib_close(xmllib_t* handle);

/* file.c */
XLDECL xmllib_driver_t* xmllib_driver_file;

XLDECL xmllib_t* xmllib_open_file(const char* filename);

/* unicode.c */
XLDECL int xmllib_unicode_count(unsigned char c);

XLDECL int xmllib_unicode_8_to_32(const char* input, int* output);

XLDECL int xmllib_unicode_32_to_8(const int input, char* output);

#ifdef __cplusplus
}
#endif

#endif
