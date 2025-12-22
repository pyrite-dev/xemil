#include <xmllib.h>

#include "../external/stb_ds.h"

xmllib_t* xmllib_open(xmllib_driver_t* driver, void* arg) {
	xmllib_t* handle = malloc(sizeof(*handle));
	memset(handle, 0, sizeof(*handle));

	handle->driver	= driver;
	handle->drv_arg = arg;

	if(!handle->driver->open(handle)) {
		free(handle);
		return NULL;
	}

	return handle;
}

enum STATES {
	STATE_INITIAL = 0,
	STATE_TAG,
	STATE_STRING,
	STATE_SPECIAL,
	STATE_COMMENT,
	STATE_DOCTYPE,
};

#define TAKE_AS_NODE(x) \
	{ \
		char* old = node; \
		char  buf[4]; \
		int new = xmllib_unicode_32_to_8((x), buf); \
\
		node = malloc(strlen(old) + new + 1); \
		strcpy(node, old); \
		memcpy(node + strlen(old), buf, new); \
		node[strlen(old) + new] = 0; \
		node_count++; \
	}

#define CLEANUP \
	{ \
		if(node != NULL) free(node); \
		arrfree(state); \
	}

#define ERROR \
	{ \
		printf("%d\n", __LINE__); \
		CLEANUP; \
		longjmp(err, 1); \
	}

#define STATE state[arrlen(state) - 1]

int xmllib_parse(xmllib_t* handle) {
	jmp_buf	       err;
	int*	       state	  = NULL;
	char*	       node	  = NULL;
	int	       node_count = 0;
	int	       nest_level = 0;
	xmllib_node_t* current	  = NULL;

	if(setjmp(err)) {
		return 0;
	}

	arrput(state, STATE_INITIAL);

	while(1) {
		char in[4];
		int  c;
		int  cp;

		if(handle->driver->read(handle, &in[0], 1) < 1) ERROR;

		c = xmllib_unicode_count(in[0]);
		if((c > 1) && handle->driver->read(handle, &in[1], c - 1) < (c - 1)) longjmp(err, 1);

		xmllib_unicode_8_to_32(in, &cp);

		if(cp == '<' && STATE != STATE_COMMENT && STATE != STATE_DOCTYPE) {
			if(STATE == STATE_INITIAL) {
				arrput(state, STATE_TAG);

				node	   = malloc(1);
				node[0]	   = 0;
				node_count = 0;
			} else if(STATE == STATE_STRING) {
				TAKE_AS_NODE(cp);
			} else {
				ERROR;
			}
		} else if(cp == '>') {
			if(STATE == STATE_TAG || STATE == STATE_DOCTYPE || STATE == STATE_COMMENT) {
				if((STATE != STATE_COMMENT || node[strlen(node) - 1] == '-') && (STATE != STATE_DOCTYPE || node[strlen(node) - 1] == ']')) {
					arrpop(state);

					if(STATE == STATE_INITIAL) {
						/* parsed tag */
						xmllib_node_t* n    = NULL;
						xmllib_node_t* targ = NULL;

						if(node[0] == '!') {
							/* special tag */
							if(strlen(node) > 1 && node[1] == '-') {
								/* comment */

								if(strlen(node) > (3 + 2)) {
									n = malloc(sizeof(*n));

									n->type = XMLLIB_NODE_COMMENT;
									n->name = NULL;
									n->text = malloc(strlen(node) + 1);

									strcpy(n->text, node + 3);
									n->text[strlen(n->text) - 2] = 0;
								}
							} else {
								/* doctype or something - we ignore this for now */
							}
						} else if(node[0] == '?') {
							/* process */
						} else {
							/* normal tag */
							if(node[0] == '/') {
								/* close tag */

								if(nest_level > 0) {
									if(strcmp(current->name, node + 1) != 0) ERROR;

									current = current->parent;
								}

								nest_level--;
							} else {
								int i;

								n = malloc(sizeof(*n));

								if(handle->root == NULL) handle->root = n;

								n->type = XMLLIB_NODE_NODE;
								n->name = malloc(strlen(node) + 1);
								n->text = NULL;

								strcpy(n->name, node);
								i = 0;
								while(1) {
									int cp;
									int nb = xmllib_unicode_8_to_32(n->name + i, &cp);

									if(cp == 0) break;

									if(cp == ' ' || cp == '\t' || cp == '\n' || cp == '\r') {
										n->name[i] = 0;
										break;
									}

									i += nb;
								}

								n->root	  = handle->root;
								n->parent = current;

								if(node[strlen(node) - 1] != '/') {
									/* non self-closing tag */
									targ = n;
									nest_level++;
								}
							}

							if(nest_level < 0) ERROR;
							if(nest_level == 0) break;
						}

						if(n != NULL) {
							xmllib_node_t* last = NULL;

							n->first_child = NULL;

							if(current != NULL && current->first_child != NULL) {
								last = current->first_child;
								while(last->next != NULL) last = last->next;
							}

							n->prev = last;
							n->next = NULL;

							if(n->prev != NULL) {
								n->prev->next = n;
							}

							if(current != NULL && current->first_child == NULL) {
								current->first_child = n;
							}
						}

						if(targ != NULL) current = targ;

						free(node);
						node = NULL;
					}
				} else {
					TAKE_AS_NODE(cp);
				}
			} else if(STATE == STATE_STRING) {
				TAKE_AS_NODE(cp);
			} else {
				ERROR;
			}
		} else if(cp == '"' && STATE != STATE_COMMENT && STATE != STATE_DOCTYPE) {
			if(STATE == STATE_TAG) {
				arrput(state, STATE_STRING);
			} else if(STATE == STATE_STRING) {
				arrpop(state);
			}
			TAKE_AS_NODE(cp);
		} else if(STATE == STATE_TAG || STATE == STATE_STRING || STATE == STATE_SPECIAL || STATE == STATE_COMMENT || STATE == STATE_DOCTYPE) {
			if(node_count == 0 && cp == '!') {
				arrpop(state);
				arrput(state, STATE_SPECIAL);
			} else if(node_count == 1 && STATE == STATE_SPECIAL && cp == '-') {
				arrpop(state);
				arrput(state, STATE_COMMENT);
			} else if(node_count == 1 && STATE == STATE_SPECIAL) {
				arrpop(state);
				arrput(state, STATE_DOCTYPE);
			}
			TAKE_AS_NODE(cp);
		} else if(STATE == STATE_INITIAL) {
		} else {
			ERROR;
		}
	}

	CLEANUP;

	return 1;
}

void xmllib_close(xmllib_t* handle) {
	handle->driver->close(handle);
	free(handle);
}
