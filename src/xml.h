struct xml_tree_node {
	char* name;
	struct xml_tree_node* next;
	struct xml_tree_node* prev;
	struct xml_tree_node* parent;
	struct xml_tree_node* child;
	char* attributes;
	char* text;
	char* content_ptr;
};

struct xml_tree_node* build_xml_tree(char* xml);

void free_tree(struct xml_tree_node* head);
