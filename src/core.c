#include <xemil.h>

xemil_t* xl_open(xl_driver_t* driver, void* arg) {
	xemil_t* handle = malloc(sizeof(*handle));
	memset(handle, 0, sizeof(*handle));

	handle->driver	= driver;
	handle->drv_arg = arg;

	handle->pre  = NULL;
	handle->root = NULL;

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
	STATE_MISC,
	STATE_IGNORE_TAG
};

#define TAKE_AS_NODE(x) \
	if(node == NULL) { \
		ERROR; \
	} else { \
		char* old = node; \
		char  buf[4]; \
		int new = xl_unicode_32_to_8((x), &buf[0]); \
\
		node = malloc(strlen(old) + new + 1); \
		strcpy(node, old); \
		memcpy(node + strlen(old), &buf[0], new); \
		node[strlen(old) + new] = 0; \
		node_count++; \
\
		free(old); \
	}

#define CLEANUP \
	{ \
		if(node != NULL) free(node); \
		xl_array_free(&state); \
	}

#define ERROR \
	{ \
		printf("%d\n", __LINE__); \
		CLEANUP; \
		longjmp(err, 1); \
	}

#define STATE state[xl_array_length(&state) - 1]

static xl_attribute_t* xl_parse_attribute(const char* str) {
	xl_attribute_t* first	= NULL;
	xl_attribute_t* current = NULL;
	char*		s	= malloc(strlen(str) + 1);
	int		i;
	int		st    = 0;
	int		start = -1;
	int		dq    = 0;

	strcpy(s, str);

	i = 0;
	while(1) {
		int cp;
		int new = xl_unicode_8_to_32(s + i, &cp);

		if(st == 0 && XL_SKIPPABLE(cp)) {
		} else if(st == 0 && cp != 0) {
			xl_attribute_t* old = current;

			st    = 1;
			start = i;

			current = malloc(sizeof(*current));

			current->key   = NULL;
			current->value = NULL;

			current->prev = NULL;
			current->next = NULL;

			if(old != NULL) {
				current->prev = old;
				old->next     = current;
			}

			if(first == NULL) first = current;
		}

		if(st == 1 && (cp == '=' || cp == 0)) {
			char* b = malloc(i - start + 1);
			memcpy(b, s + start, i - start);
			b[i - start] = 0;

			current->key = b;

			st = 2;

			if(cp == '=') {
				current->value	  = malloc(1);
				current->value[0] = 0;
			}
		} else if(st == 2 && ((dq ? 0 : XL_SKIPPABLE(cp)) || cp == 0)) {
			st = 0;
		} else if(st == 2 && cp == '"') {
			dq = dq ? 0 : 1;
		} else if(st == 2) {
			char* old = current->value;

			current->value = malloc(strlen(old) + new + 1);
			strcpy(current->value, old);
			memcpy(current->value + strlen(old), s + i, new);
			current->value[strlen(old) + new] = 0;

			free(old);
		}

		i += new;

		if(cp == 0) break;
	}

	free(s);

	return first;
}

int xl_parse(xemil_t* handle) {
	jmp_buf	   err;
	int*	   state      = NULL;
	char*	   node	      = NULL;
	int	   node_count = 0;
	int	   nest_level = 0;
	xl_node_t* current    = NULL;

	if(setjmp(err)) {
		return 0;
	}

	xl_array_push(&state, STATE_INITIAL);

	while(1) {
		char in[4];
		int  c;
		int  cp;

		if(handle->driver->read(handle, &in[0], 1) < 1) ERROR;

		c = xl_unicode_count(in[0]);
		if((c > 1) && handle->driver->read(handle, &in[1], c - 1) < (c - 1)) longjmp(err, 1);

		xl_unicode_8_to_32(in, &cp);

		if(cp == '<' && STATE != STATE_COMMENT) {
			if(STATE == STATE_INITIAL) {
				xl_array_push(&state, STATE_TAG);

				node	   = malloc(1);
				node[0]	   = 0;
				node_count = 0;
			} else if(STATE == STATE_STRING) {
				TAKE_AS_NODE(cp);
			} else if(STATE == STATE_MISC) {
				xl_array_push(&state, STATE_IGNORE_TAG);
			} else {
				ERROR;
			}
		} else if(cp == '>') {
			if(STATE == STATE_TAG || STATE == STATE_COMMENT || STATE == STATE_MISC || STATE == STATE_IGNORE_TAG) {
				if((STATE != STATE_COMMENT || node[strlen(node) - 1] == '-')) {
					xl_array_pop(&state);

					if(STATE == STATE_INITIAL) {
						/* parsed tag */
						xl_node_t* n	= NULL;
						xl_node_t* targ = NULL;

						if(node[0] == '!') {
							/* special tag */
							if(strlen(node) > 2 && node[1] == '-') {
								/* comment */

								if(strlen(node) > (1 + 3 + 2)) {
									n = malloc(sizeof(*n));

									n->type		   = XL_NODE_COMMENT;
									n->name		   = NULL;
									n->text		   = malloc(strlen(node) + 1);
									n->first_attribute = NULL;

									strcpy(n->text, node + 3);
									n->text[strlen(n->text) - 2] = 0;
								}
							} else if(strlen(node) > 8 && memcmp(&node[1], "[CDATA[", 7) == 0 && node[strlen(node) - 1] == ']' && node[strlen(node) - 2] == ']') {
								char* old = current->text;

								if(old == NULL) {
									current->text = malloc(strlen(node));
									strcpy(current->text, node + 8);
								} else {
									current->text = malloc(strlen(old) + strlen(node));
									strcpy(current->text, old);
									strcpy(current->text + strlen(old), node + 8);
									free(old);
								}
								if(strlen(current->text) > 2) {
									current->text[strlen(current->text) - 2] = 0;
								}
							} else {
								/* DOCTYPE or something - ignored for now */
							}
						} else {
							/* normal tag */
							int proc = 0;

							if(node[0] == '/') {
								/* close tag */
								char* str;

								if(nest_level > 0) {
									if(strcmp(current->name, node + 1) != 0) ERROR;

									targ = current->parent;
								}

								nest_level--;

								if(current->text != NULL) {
									str = xl_util_trim(current->text);
									free(current->text);
									current->text = str;

									if(strlen(current->text) == 0) {
										free(current->text);
										current->text = NULL;
									}
								}

								proc = 1;
							} else {
								int   i;
								int   set = 0;
								char* s	  = malloc(strlen(node) + 1);
								int   sl;

								n = malloc(sizeof(*n));

								if(node[0] == '?' && node[strlen(node) - 1] == '?') {
									n->type = XL_NODE_PROCESS;

									strcpy(s, node + 1);
									s[strlen(s) - 1] = 0;
								} else {
									n->type = XL_NODE_NODE;

									strcpy(s, node);
								}

								sl = node[strlen(s) - 1] == '/' ? 1 : 0;

								if(sl) {
									s[strlen(s) - 1] = 0;
								}

								n->name		   = malloc(strlen(s) + 1);
								n->text		   = NULL;
								n->first_attribute = NULL;

								strcpy(n->name, s);
								i = 0;
								while(1) {
									int icp;
									int nb = xl_unicode_8_to_32(n->name + i, &icp);

									if(icp == 0) break;

									if(XL_SKIPPABLE(icp)) {
										n->name[i] = 0;
										set	   = 1;
									} else if(set) {
										n->first_attribute = xl_parse_attribute(s + i);

										break;
									}

									i += nb;
								}

								if(n->type == XL_NODE_NODE && !sl) {
									/* non self-closing tag */
									targ = n;
									nest_level++;

									proc = 1;
								}
								free(s);
							}

							if(proc) {
								if(nest_level < 0) ERROR;
								if(nest_level == 0) break;
							}
						}

						if(n != NULL) {
							if(nest_level == 0 && (n->type == XL_NODE_COMMENT || n->type == XL_NODE_PROCESS)) {
								if(handle->pre == NULL) {
									handle->pre = n;

									n->next = NULL;
								} else {
									xl_node_t* last = handle->pre;
									while(last->next != NULL) last = last->next;

									n->prev	   = last;
									n->next	   = NULL;
									last->next = n;
								}
							} else {
								xl_node_t* last = NULL;
								if(handle->root == NULL) handle->root = n;

								n->root	  = handle->root;
								n->parent = current;

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
		} else if(cp == '"' && STATE != STATE_COMMENT && STATE != STATE_MISC) {
			if(STATE == STATE_TAG) {
				xl_array_push(&state, STATE_STRING);
			} else if(STATE == STATE_STRING) {
				xl_array_pop(&state);
			}

			TAKE_AS_NODE(cp);
		} else if(STATE == STATE_TAG || STATE == STATE_STRING || STATE == STATE_SPECIAL || STATE == STATE_COMMENT || STATE == STATE_MISC || STATE == STATE_IGNORE_TAG) {
			if(node_count == 0 && cp == '!') {
				xl_array_pop(&state);
				xl_array_push(&state, STATE_SPECIAL);
			} else if(node_count == 1 && STATE == STATE_SPECIAL && cp == '-') {
				xl_array_pop(&state);
				xl_array_push(&state, STATE_COMMENT);
			} else if(node_count == 1 && STATE == STATE_SPECIAL) {
				xl_array_pop(&state);
				xl_array_push(&state, STATE_MISC);
			}
			TAKE_AS_NODE(cp);
		} else if(STATE == STATE_INITIAL) {
			if(current != NULL) {
				char* old = current->text;
				if(old != NULL) {
					char buf[4];
					int new = xl_unicode_32_to_8(cp, &buf[0]);

					current->text = malloc(strlen(old) + new + 1);
					strcpy(current->text, old);
					memcpy(current->text + strlen(old), &buf[0], new);
					current->text[strlen(old) + new] = 0;

					free(old);
				} else {
					char buf[4];
					int new = xl_unicode_32_to_8(cp, &buf[0]);

					current->text = malloc(new + 1);
					memcpy(current->text, &buf[0], new);
					current->text[new] = 0;
				}
			}
		} else {
			ERROR;
		}
	}

	CLEANUP;

	return 1;
}

static void recursive_free(xl_node_t* node) {
	xl_node_t*	n = node->first_child;
	xl_attribute_t* a = node->first_attribute;
	while(n != NULL) {
		recursive_free(n);

		n = n->next;
	}

	while(a != NULL) {
		free(a->key);
		if(a->value != NULL) free(a->value);

		a = a->next;
	}

	if(node->name != NULL) free(node->name);
	if(node->text != NULL) free(node->text);
	free(node);
}

void xl_close(xemil_t* handle) {
	if(handle->root != NULL) recursive_free(handle->root);

	handle->driver->close(handle);
	free(handle);
}
