#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>

#include <libxml/parser.h>
#include <libxml/tree.h>

/* The epub standard states that the root file's path must be here. */
#define CONTAINER_PATH "META-INF/container.xml"

#define ZIP_PATH_MAX 260
#define PATHMAX 4096
#define BUFSIZE 1024

struct spine {
	char** hrefs;
	int hrefnum;
};

/* TODO: Check for...
 * - Cannot find rootfile
 */

int
xml_get_rootfile(char* rootfile, char* rootdir) {

	xmlDocPtr doc;
	xmlNodePtr cur;
	xmlChar* parsed_rootfile;
	char container[PATHMAX];

	sprintf(container, "%s/%s", rootdir, CONTAINER_PATH);

	doc = xmlParseFile(container);

	cur = xmlDocGetRootElement(doc);

	cur = cur->children;

	while (cur != NULL) {
		if (xmlStrcmp(cur->name, (xmlChar*) "rootfiles") == 0) {
			break;
		}
		cur = cur->next;
	}

	if (cur == NULL) {
		xmlFreeDoc(doc);
		return -1;
	}

	cur = cur->children->next;

	parsed_rootfile = xmlGetProp(cur, (xmlChar*) "full-path");

	sprintf(rootfile, "%s/%s", rootdir, parsed_rootfile);

	xmlFree(parsed_rootfile);
	xmlFreeDoc(doc);

	return 0;

}

struct spine
xml_get_spine(char* rootfile) {

	struct spine spine;
	xmlDocPtr doc;
	xmlNodePtr cur;
	xmlNodePtr root_node, spine_node, manifest_node;
	xmlChar *cur_spine_idref, *cur_manifest_idref;

	spine.hrefnum = 0;

	doc = xmlParseFile(rootfile);

	root_node = xmlDocGetRootElement(doc);

	cur = root_node->children;

	while (xmlStrcmp(cur->name, (xmlChar*) "manifest") != 0) {
		if ((cur = cur->next) == NULL) {
			fprintf(stderr, "Could not find manifest in rootfile\n");
			xmlFreeDoc(doc);
			return spine;
		}
	}
	manifest_node = cur;

	cur = root_node->children;

	while (xmlStrcmp(cur->name, (xmlChar*) "spine") != 0) {
		if ((cur = cur->next) == NULL) {
			fprintf(stderr, "Could not find spine in rootfile\n");
			xmlFreeDoc(doc);
			return spine;
		}
	}
	spine_node = cur;

	cur = spine_node->children;

	while (cur != NULL) {
		if (xmlStrcmp(cur->name, (xmlChar*) "itemref") == 0) {
			spine.hrefnum++;
		}
		cur = cur->next;
	}

	spine.hrefs = malloc(sizeof(char*) * spine.hrefnum);

	if (spine.hrefs == NULL) {
		fprintf(stderr, "Could not allocate memory\n");
		xmlFreeDoc(doc);
		return spine;
	}

	/*
	 * Get the item's idref, then jump to the manifest to find it's respective
	 * href.
	 */
	for (int i = 0; i < spine.hrefnum; i++) {

		cur = spine_node->children;

		for (int j = 0; j <= i; j++) {
			do {
				cur = cur->next;
			} while (xmlStrcmp(cur->name, (xmlChar*) "itemref") != 0);
		}

		cur_spine_idref = xmlGetProp(cur, (xmlChar*) "idref");

		cur = manifest_node->children;

		while (cur != NULL) {
			if (xmlStrcmp(cur->name, (xmlChar*) "item") == 0) {

				cur_manifest_idref = xmlGetProp(cur, (xmlChar*) "id");

				if (xmlStrcmp(cur_spine_idref, cur_manifest_idref) == 0) {
					spine.hrefs[i] = (char*) xmlGetProp(cur, (xmlChar*) "href");
					xmlFree(cur_manifest_idref);
					break;
				}

				xmlFree(cur_manifest_idref);
			}
			cur = cur->next;
		}

		xmlFree(cur_spine_idref);

	}

	xmlFreeDoc(doc);

	return spine;

}
