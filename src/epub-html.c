#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "xml.h"

typedef int flag_t;

struct html_rules {

};

struct html_content {
	char* tag;
	struct html_rules* rules;
};

static void
_add_indent(char* line, int indent) {

	for (int i = 0; i < indent; i++) {
		strcat(line, " ");
	}

}

int
html_write_to_file(char* html, char* output, int linelen, int indent) {

	FILE* outputf;
	struct xml_tree_node* tree;
	struct xml_tree_node* cur;
	char* curline;

	outputf = fopen(output, "a");

	curline = calloc(linelen + 2, sizeof(char));

	_add_indent(curline, indent);

	tree = build_xml_tree(html);

	cur = tree;

	do {

		char* p = cur->text;

		if (p == NULL) {
			continue;
		}

		while (*(p += strspn(p, " ")) != '\0') {

			size_t wordlen = strcspn(p, " ");

			if (wordlen + strlen(curline) > linelen) {

				fprintf(outputf, "%s\n", curline);
				memset(curline, 0, linelen + 2);
				_add_indent(curline, indent);

				while (wordlen > linelen - indent) {

					strncat(curline, p, linelen - indent - 1);
					strcat(curline, "-");

					fprintf(outputf, "%s\n", curline);

					memset(curline, 0, linelen + 2);

					p += linelen - indent - 1;
					wordlen -= linelen  - indent - 1;

					_add_indent(curline, indent);
				}

			}

			strncat(curline, p, wordlen);
			strcat(curline, " ");

			p += strcspn(p, " ");

		}

		fprintf(outputf, "\n\n");

	} while ((cur = cur->traverse) != NULL);

	free(curline);
	free_tree(tree);
	fclose(outputf);

	return 0;

}
