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

static char* text_nodes[] = {
	"p",
	"h1",
	"h2",
	"h3",
	"h4",
	"h5",
	"h6",
	"td",
	"li",
	"div",
	"span",
	"title",
	NULL,
};

/* TODO: Make this a public function in xml.c */
static int
_strcmpnul(char* s1, char* s2) {

	if (s1 == NULL || s2 == NULL) {
		return 1;
	}

	return strcmp(s1, s2);

}

static struct xml_tree_node*
_get_text_parent(struct xml_tree_node* node) {

	struct xml_tree_node* cur = node;

	do {
		for (char** p = text_nodes; *p != NULL; p++) {
			if (_strcmpnul(cur->name, *p) == 0) {
				return cur;
			}
		}
	} while ((cur = cur->parent) != NULL);

	return NULL;

}

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
	struct xml_tree_node *cur_txtp, *prev_txtp;
	char* curline;

	outputf = fopen(output, "a");

	if ((curline = calloc(linelen + 2, sizeof(char))) == NULL) {
		fprintf(stderr, "Could not allocate memory\n");
		return -1;
	}

	_add_indent(curline, indent);

	tree = build_xml_tree(html);

	cur = tree;

	prev_txtp = NULL;

	do {

		char* p;
		size_t wordlen = 0;

		if (_strcmpnul(cur->name, "br") == 0) {
			fprintf(outputf, "%s\n", curline);
			memset(curline, 0, linelen + 2);
			_add_indent(curline, indent);
			continue;
		}

		if ((p = cur->text) == NULL) {
			continue;
		}

		if ((cur_txtp = _get_text_parent(cur)) != prev_txtp) {
			fprintf(outputf, "%s\n\n", curline);
			memset(curline, 0, linelen + 2);
			_add_indent(curline, indent);
			prev_txtp = cur_txtp;
		}

		while (*(p += strspn(p, " ")) != '\0') {

			wordlen = strcspn(p, " ");

			/* Drop to next line */
			if (wordlen + strlen(curline) > linelen) {

				fprintf(outputf, "%s\n", curline);
				memset(curline, 0, linelen + 2);
				_add_indent(curline, indent);

				/* Hyphenate words longer than linelen - indent */
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

	} while ((cur = cur->traverse) != NULL);

	free(curline);
	free_tree(tree);
	fclose(outputf);

	return 0;

}
