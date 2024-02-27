struct xml_prop {
	char* name;
	char* value;
};

struct xml_tree_node {
	/* Name of a node. Text nodes will have this set to NULL. */
	char* name;
	struct xml_tree_node* next;
	struct xml_tree_node* prev;
	struct xml_tree_node* parent;
	struct xml_tree_node* child;
	/* Used if you wish to visit every node in a node tree once in DFS order. */
	struct xml_tree_node* traverse;
	/* Array of tag props. Last prop will have name and value set to NULL. */
	struct xml_prop* props;
	/* The text of a text node. This is NULL for non-text nodes. */
	char* text;
	/* Points to the xml file's content's location in memory. This is only used
	 * by a tree's head node. */
	char* content_ptr;
};

/* strcmp, but if s1 or s2 are NULL, return 1. */
int xml_strcmpnul(char* s1, char* s2);

/* Builds an xml node tree, returns the tree's head. */
/* NOTE: Tree head should be freed using xml_free_tree when no longer in use. */
struct xml_tree_node* xml_build_tree(char* xml);

/* Returns the value of propname in node, or NULL if it doesn't exist. */
char* xml_get_prop(struct xml_tree_node* node, char* propname);

/* Frees all malloc'd data in an xml node tree */
void xml_free_tree(struct xml_tree_node* head);
