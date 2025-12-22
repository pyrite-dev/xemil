#include <xmllib.h>

void recursive(xmllib_node_t* node, int indent){
	int i;
	xmllib_node_t* n;
	for(i = 0; i < indent; i++) printf(" ");
	printf("%s\n", node->name);
	
	n = node->first_child;
	while(n != NULL){
		recursive(n, indent + 2);
		n = n->next;
	}
}

int main(int argc, char** argv){
	int i;

	for(i = 1; i < argc; i++){
		xmllib_t* h = xmllib_open_file(argv[i]);
		if(h != NULL){
			printf("%s:\n", argv[i]);
			if(xmllib_parse(h)){
				if(h->root != NULL) recursive(h->root, 2);
			}else{
				printf("  Parse error\n");
			}
			xmllib_close(h);
		}
	}
}
