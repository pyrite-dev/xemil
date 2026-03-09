#include <xemil.h>

static xl_node_t** xl_xpointer_element(xl_node_t* node, const char* path) {
	XL_LIST_BEGIN('/');

	if(strlen(token) == 0) {
		nodes	 = malloc(sizeof(*nodes) * 2);
		nodes[0] = scanned_node;
		nodes[1] = NULL;

		while(nodes[0]->parent != NULL) nodes[0] = nodes[0]->parent;
	} else {
		nodes = xl_get_nodes(scanned_node, token);
	}

	XL_LIST_END;
}

xl_node_t** xl_xpointer(xl_node_t* node, const char* path) {
	xl_node_t** r = malloc(sizeof(*r) * 2);
	char*	    p = xl_util_strdup(path);
	int	    i;
	int	    s  = 0;
	int	    br = 0;

	r[0] = node;
	r[1] = NULL;

	for(i = 0;; i++) {
		int trig = 0;

		if(p[i] == '(') {
			br++;
		} else if(p[i] == ')') {
			br--;
			if(br == 0) trig = 1;
		}
		if(p[i] == 0) break;
		if(trig) {
			int j;
			xl_node_t** new = malloc(sizeof(*new));
			char* m		= malloc(i - s + 2);

			m[i - s + 1] = 0;
			memcpy(m, p + s, i - s + 1);

			new[0] = NULL;

			for(j = 0; r[j] != NULL; j++) {
				xl_node_t** nodes = NULL;
				char*	    name  = strchr(m, '(');

				if(name != NULL) {
					char* n = strrchr(name, ')');
					if(n != NULL) n[0] = 0;

					nodes = xl_xpointer_element(r[j], name + 1);
				}

				if(nodes != NULL) {
					XL_MERGE(new, nodes);

					free(nodes);
				}
			}
			free(m);
			free(r);
			r = new;

			s = i + 1;
		}
	}

	if(r[0] == NULL) {
		free(r);
		r = NULL;
	}

	return r;
}
