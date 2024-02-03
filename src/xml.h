struct xml_tree_node {
	char* name;
	struct xml_tree_node* next;
	struct xml_tree_node* prev;
	struct xml_tree_node* parent;
	struct xml_tree_node* child;
	struct xml_tree_node* traverse;
	char* attributes;
	char* text;
	char* content_ptr;
};

int xml_strcmpnul(char* s1, char* s2);

struct xml_tree_node* xml_build_tree(char* xml);

void xml_free_tree(struct xml_tree_node* head);
