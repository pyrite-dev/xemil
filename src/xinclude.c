#include <xemil.h>

void xl_xinclude_scan(xemil_t* handle, xl_node_t* node) {
	xl_node_t* child;

	if(node->type == XL_NODE_NODE && node->name != NULL && strcmp(node->name, "xi:include") == 0) {
		char* href     = xl_get_attribute(node, "href");
		char* parse    = xl_get_attribute(node, "parse");
		char* xpointer = xl_get_attribute(node, "xpointer");

		if(href != NULL) {
			xemil_t* new;

			if(handle->path == NULL) {
				href = xl_util_strdup(href);
			} else {
				href = xl_util_strvacat(handle->path, href, NULL);
			}

			if(parse == NULL) parse = "xml";
			if(xpointer == NULL) xpointer = "element(/)";

			if((new = xl_open_file(href)) != NULL) {
				new->do_xinclude = 1;
				new->new_text	 = handle->new_text;
				if(xl_parse(new) && new->root != NULL) {
					xl_node_t** nodes;
					if((nodes = xl_xpointer(new->root, xpointer)) != NULL) {
						xl_replace(node, nodes[0]);

						if(nodes[0] == new->root) new->root = NULL;

						free(nodes);
					}
				}
			}

			if(new != NULL) xl_close(new);

			free(href);
		}
	}

	child = node->first_child;

	while(child != NULL) {
		xl_xinclude_scan(handle, child);

		child = child->next;
	}
}
