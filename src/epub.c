#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "epub.h"
#include "xml.h"

#define PATHMAX 4096

/* The epub standard states that the root file's path must be here. */
static char* container_path = "META-INF/container.xml";

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

static struct xml_tree_node*
_get_text_parent(struct xml_tree_node* node) {

	struct xml_tree_node* cur = node;

	do {
		for (char** p = text_nodes; *p != NULL; p++) {
			if (xml_strcmpnul(cur->name, *p) == 0) {
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
epub_get_rootfile(char* rootfile, char* rootdir) {

	struct xml_tree_node* head;
	struct xml_tree_node* cur;
	char container[PATHMAX];
	char* rf_fullpath;

	sprintf(container, "%s/%s", rootdir, container_path);

	if ((head = xml_build_tree(container)) == NULL) {
		fprintf(stderr, "Could not parse container file\n");
		return -1;
	}

	cur = head;

	while (cur != NULL) {
		if (xml_strcmpnul(cur->name, "rootfiles") == 0) {
			break;
		}
		cur = cur->child;
	}

	if (cur == NULL) {
		xml_free_tree(head);
		return -1;
	}

	cur = cur->child;

	rf_fullpath = strstr(cur->attributes, "full-path");
	if (rf_fullpath == NULL) {
		xml_free_tree(head);
		return -1;
	}
	rf_fullpath = strchr(rf_fullpath, '"') + 1;
	*(strchr(rf_fullpath, '"')) = '\0';

	strcat(rootfile, rootdir);
	strncat(rootfile, rf_fullpath, strcspn(rf_fullpath, "\""));

	xml_free_tree(head);

	return 0;

}

struct spine
epub_get_spine(char* rootfile) {

	struct spine spine;
	struct xml_tree_node* head;
	struct xml_tree_node* cur;
	struct xml_tree_node* spinen = NULL;
	struct xml_tree_node* manifn = NULL;

	spine.hrefnum = 0;

	if ((head = xml_build_tree(rootfile)) == NULL) {
		fprintf(stderr, "Could not parse rootfile\n");
		spine.hrefs = NULL;
		return spine;
	}

	/* Points to package's child node */
	cur = head->child->child;

	/*
	 * Get pointers to manifest and spine nodes, we'll be hopping between them.
	 */
	while (cur != NULL) {

		if (xml_strcmpnul(cur->name, "manifest") == 0) {
			manifn = cur;
		}

		if (xml_strcmpnul(cur->name, "spine") == 0) {
			spinen = cur;
		}

		if (spinen != NULL && manifn != NULL) {
			break;
		}

		cur = cur->next;

	}

	if (spinen == NULL) {
		fprintf(stderr, "EPUB's root file does not contain a spine\n");
		xml_free_tree(head);
		return spine;
	}

	if (manifn == NULL) {
		fprintf(stderr, "EPUB's root file does not contain a manifest\n");
		xml_free_tree(head);
		return spine;
	}

	cur = spinen->child;
	while (cur != NULL) {
		if (xml_strcmpnul(cur->name, "itemref") == 0) {
			spine.hrefnum++;
		}
		cur = cur->next;
	}

	if ((spine.hrefs = malloc(sizeof(char*) * spine.hrefnum)) == NULL) {
		fprintf(stderr, "Could not allocate memory\n");
		xml_free_tree(head);
		return spine;
	}

	cur = spinen->child;

	for (int i = 0; i < spine.hrefnum; i++) {

		struct xml_tree_node* curmanif;
		char *idref, *href, *id;

		while (xml_strcmpnul(cur->name, "itemref") != 0) {
			cur = cur->next;
		}

		idref = strstr(cur->attributes, "idref");
		idref = strchr(idref, '"') + 1;
		*(strchr(idref, '"')) = '\0';

		curmanif = manifn->child;

		while (curmanif != NULL) {

			if (xml_strcmpnul(curmanif->name, "item") != 0) {
				curmanif = curmanif->next;
				continue;
			}

			id = strstr(curmanif->attributes, "id=");
			id = strchr(id, '"') + 1;
			*(strchr(id, '"')) = '\0';

			if (strcmp(idref, id) != 0) {
				*(strchr(id, '\0')) = '"';
				curmanif = curmanif->next;
				continue;
			}

			*(strchr(id, '\0')) = '"';

			href = strstr(curmanif->attributes, "href");
			href = strchr(href, '"') + 1;
			*(strchr(href, '"')) = '\0';

			if ((spine.hrefs[i] = strdup(href)) == NULL) {
				fprintf(stderr, "Could not allocate memory\n");
				for (int j = 0; j < i; j++) {
					free(spine.hrefs[j]);
				}
				free(spine.hrefs);
				xml_free_tree(head);
				spine.hrefs = NULL;
				return spine;
			}

			*(strchr(href, '\0')) = '"';

			curmanif = curmanif->next;

		}

		cur = cur->next;

	}

	xml_free_tree(head);

	return spine;

}

int
epub_html2text(char* html, char* output, int linelen, int indent) {

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

	tree = xml_build_tree(html);

	cur = tree;

	prev_txtp = NULL;

	do {

		char* p;
		size_t wordlen = 0;

		if (xml_strcmpnul(cur->name, "br") == 0) {
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
	xml_free_tree(tree);
	fclose(outputf);

	return 0;

}
