#include <stdio.h>

#include "xml.h"

#define PATHMAX 4096

/* The epub standard states that the root file's path must be here. */
char* container_path = "META-INF/container.xml";

struct spine {
	char** hrefs;
	int hrefnum;
};

/* Just so I don't have to constantly check if node->name is NULL */
int
_strcmpnul(char* s1, char* s2) {

	if (s1 == NULL || s2 == NULL) {
		return 1;
	}

	if (strcmp(s1, s2) == 0) {
		return 0;
	}

	return 1;

}

int
xml_get_rootfile(char* rootfile, char* rootdir) {

	struct xml_tree_node* head;
	struct xml_tree_node* cur;
	char container[PATHMAX];
	char* rf_fullpath;

	sprintf(container, "%s/%s", rootdir, container_path);

	head = build_xml_tree(container);

	while (cur != NULL) {
		if (_strcmpnul(cur->name, "rootfiles") == 0) {
			break;
		}
		cur = cur->child;
	}

	if (cur == NULL) {
		free_tree(head);
		return -1;
	}

	rf_fullpath = strstr(cur->attributes, "full-path");
	if (rf_fullpath == NULL) {
		free_tree(head);
		return -1;
	}
	rf_fullpath = strchr(rf_fullpath, '"');

	strcat(rootfile, rootdir);
	strncat(rootfile, rf_fullpath, strcspn(rf_fullpath, "\"");

	free_tree(head);

	return 0;

}

struct spine
xml_get_spine(char* rootfile) {

	struct spine spine;
	struct xml_tree_node* head;
	struct xml_tree_node* cur;
	struct xml_tree_node* spinen = NULL;
	struct xml_tree_node* manifn = NULL;

	spine.hrefnum;

	head = build_xml_tree(rootfile);

	/* Points to package's child node */
	cur = head->child->child;

	/*
	 * Get pointers to manifest and spine nodes, we'll be hopping between them.
	 */
	while (cur != NULL) {

		if (_strcmpnul(cur->name, "manifest") == 0) {
			manifn = cur;
		}

		if (_strcmpnul(cur->name, "spine") == 0) {
			spinen = cur;
		}

		if (spinen != NULL && manifn != NULL) {
			break;
		}

		cur = cur->next;

	}

	if (cur == NULL) {
		fprintf(stderr, "Rootfile does not contain all necessary data\n");
		free_tree(head);
		return spine;
	}

	cur = spinen->child;
	while (cur != NULL) {
		if (_strcmpnul(cur->name, "itemref") == 0) {
			spine.hrefnum++;
		}
	}

	spine.hrefs = malloc(sizeof(char*) * spine.hrefnum);

	if (spine.hrefs == NULL) {
		fprintf(stderr, "Could not allocate memory\n");
		free_tree(head);
		return spine;
	}


	cur = spinen->child;

	for (int i = 0; i < spine.hrefnum; i++) {

		struct xml_tree_node* curmanif;
		char *idref, *href, *id;

		while (_strcmpnul(cur->name, "itemref") != 0) {
			cur = cur->next;
		}

		idref = strstr(cur->attributes, "idref");
		idref = strchr(idref, '"') + 1;
		*(strchr(idref, '"')) = '\0';

		curmanif = manifn->child;

		while (curmanif != NULL) {

			if (_strcmpnul(curmanif->name, "item") != 0) {
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

			/* TODO: Check for failed malloc */
			spine.hrefs[i] = strdup(href);

			*(strchr(href, '\0')) = '"';

		}

	}

	return spine;

}
