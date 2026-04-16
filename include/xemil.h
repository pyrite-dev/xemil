#ifndef __XEMIL_H__
#define __XEMIL_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include <stdarg.h>
#include <errno.h>

#define XLDECL extern

typedef struct xl_driver    xl_driver_t;
typedef struct xemil	    xemil_t;
typedef struct xl_node	    xl_node_t;
typedef struct xl_attribute xl_attribute_t;
typedef struct xl_param	    xl_param_t;

#define XL_SKIPPABLE(x) (((x) == ' ') || ((x) == '\t') || ((x) == '\n') || ((x) == '\r'))

struct xl_driver {
	int (*open)(xemil_t* handle);
	int (*read)(xemil_t* handle, void* data, int size);
	void (*close)(xemil_t* handle);
};

enum XL_NODE_TYPE {
	XL_NODE_COMMENT = 0,
	XL_NODE_NODE,
	XL_NODE_PROCESS,
	XL_NODE_TEXT
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
	char*		text_raw;
	xl_attribute_t* first_attribute;

	xl_node_t* root;
	xl_node_t* parent;

	xl_node_t* first_child;

	xl_node_t* prev;
	xl_node_t* next;
};

struct xl_param {
	int new_text;
	int do_xinclude;
};

struct xemil {
	xl_driver_t* driver;
	void*	     drv_opaque;
	void*	     drv_arg;
	char*	     path;
	xl_node_t*   pre;
	xl_node_t*   root;

	xl_param_t param;
};

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#define XL_MERGE(target, source) \
	{ \
		xl_node_t** target##_old = target; \
		int	    target##_k, target##_l; \
		int	    target##_len = 0; \
\
		for(target##_k = 0; target##_old[target##_k] != NULL; target##_k++) target##_len++; \
		for(target##_k = 0; source[target##_k] != NULL; target##_k++) target##_len++; \
\
		target		     = malloc(sizeof(*target) * (target##_len + 1)); \
		target[target##_len] = NULL; \
\
		for(target##_k = 0; target##_old[target##_k] != NULL; target##_k++) target[target##_k] = target##_old[target##_k]; \
		for(target##_l = 0; source[target##_l] != NULL; target##_l++) target[target##_k + target##_l] = source[target##_l]; \
\
		free(target##_old); \
	}

#define XL_LIST_BEGIN(x) \
	xl_node_t** r = malloc(sizeof(*r) * 2); \
	char*	    p = xl_util_strdup(path); \
	int	    i; \
	int	    s = 0; \
\
	r[0] = node; \
	r[1] = NULL; \
\
	for(i = 0;; i++) { \
		if(p[i] == x || p[i] == 0) { \
			char old = p[i]; \
			int  j; \
			xl_node_t** new = malloc(sizeof(*new)); \
			char* token	= p + s; \
\
			new[0] = NULL; \
\
			p[i] = 0; \
\
			for(j = 0; r[j] != NULL; j++) { \
				xl_node_t** nodes	 = NULL; \
				xl_node_t*  scanned_node = r[j];

#define XL_LIST_END \
				if(nodes != NULL) { \
					XL_MERGE(new, nodes); \
\
					free(nodes); \
				} \
			} \
			free(r); \
			r = new; \
\
			s = i + 1; \
\
			if(old == 0) break; \
		} \
	} \
\
	if(r[0] == NULL) { \
		free(r); \
		r = NULL; \
	} \
\
	return r;
/* clang-format on */

/* core.c */
XLDECL xemil_t* xl_open(xl_driver_t* driver, void* arg);
XLDECL int	xl_parse(xemil_t* handle);
XLDECL void	xl_close(xemil_t* handle);
XLDECL char*	xl_get_attribute(xl_node_t* node, const char* key);
XLDECL void	xl_free(xl_node_t* node);
XLDECL void	xl_replace(xl_node_t* node, xl_node_t* new_node);

XLDECL xl_node_t** xl_get_nodes(xl_node_t* node, const char* name); /* NULL-terminated */
XLDECL xl_node_t** xl_get_path(xl_node_t* node, const char* path);  /* NULL-terminated */

/* file.c */
XLDECL xl_driver_t* xl_driver_file;

XLDECL xemil_t* xl_open_file(const char* filename);

/* util.c */
XLDECL char* xl_util_trim(const char* str);
XLDECL char* xl_util_strdup(const char* str);
XLDECL char* xl_util_strvacat(const char* str, ...);
XLDECL int   xl_util_is_integer(const char* str);

/* unicode.c */
XLDECL int xl_unicode_count(unsigned char c);
XLDECL int xl_unicode_8_to_32(const char* input, int* output);
XLDECL int xl_unicode_32_to_8(const int input, char* output);

/* array.c */
XLDECL void xl_array_push(int** array, int value);
XLDECL int  xl_array_length(int** array);
XLDECL void xl_array_pop(int** array);
XLDECL void xl_array_free(int** array);

/* xpointer.c */
xl_node_t** xl_xpointer(xl_node_t* node, const char* path);

/* xinclude.c */
void xl_xinclude_scan(xemil_t* handle, xl_node_t* node);

#ifdef __cplusplus
}
#endif

#endif
