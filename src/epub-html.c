#include <stdio.h>

#include "xml.h"

typedef int flag_t;

struct html_rules {

};

struct html_content {
	char* tag;
	struct html_rules* rules;
};

int
html_write_to_file(char* html, char* output) {

	struct xml_tree_node* tree;
	struct xml_tree_node* cur;

	tree = build_xml_tree(html);

	cur = tree;

	while (cur != NULL) {
		if (cur->text != NULL) {
			printf("%s\n", cur->text);
		}
		cur = cur->traverse;
	}

	free_tree(tree);

	return 0;

}
