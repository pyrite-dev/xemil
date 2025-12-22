#include <xmllib.h>

#define INDENT 2

void recursive(xmllib_node_t* node, int indent) {
	int	       i;
	xmllib_node_t* n;
	for(i = 0; i < indent; i++) printf(" ");
	if(node->name != NULL && node->type == XMLLIB_NODE_NODE) {
		printf("<%s>\n", node->name);
		if(node->text != NULL) {
			for(i = 0; i < indent + INDENT; i++) printf(" ");
			printf("%s\n", node->text);
		}
	} else if(node->text != NULL && node->type == XMLLIB_NODE_COMMENT) {
		printf("<!--%s-->\n", node->text);
	}

	n = node->first_child;
	while(n != NULL) {
		recursive(n, indent + INDENT);
		n = n->next;
	}

	if(node->name != NULL && node->type == XMLLIB_NODE_NODE) {
		for(i = 0; i < indent; i++) printf(" ");
		printf("</%s>\n", node->name);
	}
}

int main(int argc, char** argv) {
	int i;

	for(i = 1; i < argc; i++) {
		xmllib_t* h = xmllib_open_file(argv[i]);
		if(h != NULL) {
			printf("%s:\n", argv[i]);
			if(xmllib_parse(h)) {
				if(h->root != NULL) recursive(h->root, INDENT);
			} else {
				int j;
				for(j = 0; j < INDENT; j++) printf(" ");
				printf("Parse error\n");
			}
			xmllib_close(h);
		}
	}
}
