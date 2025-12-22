#include <xmllib.h>

#define INDENT 2

void recursive(xl_node_t* node, int indent) {
	int	   i;
	xl_node_t* n;
	for(i = 0; i < indent; i++) printf(" ");
	if(node->name != NULL && (node->type == XL_NODE_NODE || node->type == XL_NODE_PROCESS)) {
		xl_attribute_t* a;

		printf("<%s%s", node->type == XL_NODE_PROCESS ? "?" : "", node->name);

		a = node->first_attribute;
		while(a != NULL) {
			if(a->value == NULL) {
				printf(" %s", a->key);
			} else {
				printf(" %s=\"%s\"", a->key, a->value);
			}
			a = a->next;
		}

		printf("%s>\n", node->type == XL_NODE_PROCESS ? "?" : "");
		if(node->text != NULL) {
			for(i = 0; i < indent + INDENT; i++) printf(" ");
			printf("%s\n", node->text);
		}
	} else if(node->text != NULL && node->type == XL_NODE_COMMENT) {
		printf("<!--%s-->\n", node->text);
	}

	n = node->first_child;
	while(n != NULL) {
		recursive(n, indent + INDENT);
		n = n->next;
	}

	if(node->name != NULL && node->type == XL_NODE_NODE) {
		for(i = 0; i < indent; i++) printf(" ");
		printf("</%s>\n", node->name);
	}
}

int main(int argc, char** argv) {
	int i;

	for(i = 1; i < argc; i++) {
		xmllib_t* h = xl_open_file(argv[i]);
		if(h != NULL) {
			printf("%s:\n", argv[i]);
			if(xl_parse(h)) {
				if(h->root != NULL) recursive(h->root, INDENT);
			} else {
				int j;
				for(j = 0; j < INDENT; j++) printf(" ");
				printf("Parse error\n");
			}
			xl_close(h);
		}
	}
}
