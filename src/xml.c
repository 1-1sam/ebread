#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "xml.h"

/* Used to initialize newly created nodes */
static struct xml_tree_node null_node = {
	.name = NULL,
	.next = NULL,
	.prev = NULL,
	.parent = NULL,
	.child = NULL,
	.traverse = NULL,
	.props = NULL,
	.text = NULL,
	.content_ptr = NULL,
};

/* Reads the entire file into a string. */
static char*
_read_xml_file(char* xml) {

	FILE* xmlf = fopen(xml, "r");
	long size;
	char* read;

	if (xmlf == NULL) {
		fprintf(stderr, "%s: Could not open\n", xml);
		return NULL;
	}

	/* Get the file size */
	fseek(xmlf, 0, SEEK_END);
	size = ftell(xmlf);
	rewind(xmlf);

	/* Read the entire file into read and add null terminator */
	if ((read = malloc(sizeof(char) * size + 1)) == NULL) {
		fprintf(stderr, "Could not allocate memory\n");
		fclose(xmlf);
		return NULL;
	}
	fread(read, sizeof(char), size, xmlf);
	read[size] = '\0';

	/* Replace tabs, newlines, etc. with spaces */
	for (char* p = read; *p != '\0'; p++) {
		if (isspace(*p)) {
			*p = ' ';
		}
	}

	fclose(xmlf);
	return read;

}

static int
_is_end_tag(char* tag, struct xml_tree_node* node) {

	if (*tag != '/' || node->name == NULL) {
		return 0;
	}

	if (strcmp(tag + 1, node->name) == 0) {
		return 1;
	}

	return 0;

}

static struct xml_prop*
_parse_props(char* propstr) {

	struct xml_prop* props;
	int propnum = 0;
	int cur = 0;
	char* p = propstr;

	/* Get number of props and check for formatting errors */
	/* Basically checking to see if each prop looks like 'name = "value"' */
	while (*(p += strspn(p, " ")) != '\0') {

		/* Find '=' sign */
		p += strcspn(p, "= ");
		p += strspn(p, " ");

		if (*p != '=') {
			return NULL;
		}

		/* Find quotation marks */
		p++;
		p += strspn(p, " ");

		if (*p != '"') {
			return NULL;
		}

		p++;

		p += strcspn(p, "\"");

		if (*p == '\0') {
			return NULL;
		}

		p++;

		propnum++;

	}

	if ((props = malloc(sizeof(struct xml_prop) * (propnum + 1))) == NULL) {
		fprintf(stderr, "Could not allocate memory\n");
		return NULL;
	}

	p = propstr;

	/* Now begin splitting up the props */
	while (*(p += strspn(p, " ")) != '\0') {

		props[cur].name = p;
		p = strchr(p, '=');
		p = strchr(p, '"') + 1;
		*(props[cur].name + strcspn(props[cur].name, " =")) = '\0';

		props[cur].value = p;
		p = strchr(p, '"') + 1;
		*(p - 1) = '\0';

		cur++;

	}

	/* Terminating array member */
	props[propnum].name = NULL;
	props[propnum].value = NULL;

	return props;

}

static int
_parse_tag(char* tag, struct xml_tree_node* node) {

	char* name;
	char* attributes;

	/* Get rid of trailing slash (for single-tag nodes) */
	if (*(strchr(tag, '\0') - 1) == '/') {
		*(strrchr(tag, '/')) = '\0';
	}

	name = tag + strspn(tag, " ");

	attributes = name + strcspn(tag, " ");
	attributes += strspn(attributes, " ");

	*(name + strcspn(name, " ")) = '\0';

	node->name = name;

	if (*attributes == '\0') {
		node->props = NULL;
	} else {
		if ((node->props = _parse_props(attributes)) == NULL) {
			return -1;
		}
	}

	return 0;

}

static struct xml_tree_node*
_make_child_node(struct xml_tree_node* parent) {

	struct xml_tree_node* rtrn = parent;

	/* Create initial child node if it does not exist */
	if (rtrn->child == NULL) {
		if ((rtrn->child = malloc(sizeof(struct xml_tree_node))) == NULL) {
			return NULL;
		}
		*(rtrn->child) = null_node;
		rtrn->child->parent = rtrn;
		rtrn = rtrn->child;
	/* Add new child node at the end of child node line */
	} else {
		rtrn = rtrn->child;
		while (rtrn->next != NULL) {
			rtrn = rtrn->next;
		}
		if ((rtrn->next = malloc(sizeof(struct xml_tree_node))) == NULL) {
			return NULL;
		}
		*(rtrn->next) = null_node;
		rtrn->next->prev = rtrn;
		rtrn->next->parent = rtrn->parent;
		rtrn = rtrn->next;
	}

	return rtrn;

}

static void
_build_traverse_line(struct xml_tree_node* head) {

	struct xml_tree_node *cur, *trav;

	cur = head;
	trav = head;

	for (;;) {
		if (cur->child != NULL) {
			trav->traverse = cur->child;
			trav = trav->traverse;
			cur = cur->child;
		} else {
			while (cur->next == NULL) {
				if (cur == head) {
					trav->traverse = NULL;
					return;
				}
				cur = cur->parent;
			}
			trav->traverse = cur->next;
			trav = trav->traverse;
			cur = cur->next;
		}
	}

}

void
xml_free_tree(struct xml_tree_node* head) {

	struct xml_tree_node *cur, *next;

	free(head->content_ptr);

	cur = head;

	while (cur != NULL) {
		next = cur->traverse;
		if (cur->props != NULL) {
			free(cur->props);
		}
		free(cur);
		cur = next;
	}

}

struct xml_tree_node*
xml_build_tree(char* xml) {

	struct xml_tree_node* head;
	char* xml_content;
	struct xml_tree_node* cur;
	char* curtok;
	char *text, *tag;

	if ((xml_content = _read_xml_file(xml)) == NULL) {
		fprintf(stderr, "%s: Could not parse\n", xml);
		return NULL;
	}

	if ((head = malloc(sizeof(struct xml_tree_node))) == NULL) {
		fprintf(stderr, "Could not allocate memory\n");
		free(xml_content);
		return NULL;
	}

	*head = null_node;
	head->content_ptr = xml_content;
	cur = head;

	curtok = strtok(head->content_ptr, "<");

	do {

		tag = curtok;
		if ((text = strchr(tag, '>')) == NULL) {
			continue;
		}
		*text = '\0';
		text++;

		while (*text == ' ') {
			text++;
		}

		if (*text == '\0') {
			text = NULL;
		}

		/* Ignore comments, CDATA, and PIs */
		if (*tag == '!' || *tag == '?') {
			;
		/* Current node has now ended, return to parent */
		} else if (_is_end_tag(tag, cur)) {
			cur = cur->parent;
		/* Single tag node */
		} else if (*(strchr(tag, '\0') - 1) == '/') {
			if ((cur = _make_child_node(cur)) == NULL) {
				goto die;
			}
			if (_parse_tag(tag, cur) == -1) {
				goto die;
			}
			cur = cur->parent;
		/* New child node */
		} else {
			if ((cur = _make_child_node(cur)) == NULL) {
				goto die;
			}
			if (_parse_tag(tag, cur) == -1) {
				goto die;
			}
		}

		if (text != NULL) {
			if ((cur = _make_child_node(cur)) == NULL) {
				goto die;
			}
			cur->text = text;
			cur = cur->parent;
		}

	} while ((curtok = strtok(NULL, "<")) != NULL);

	_build_traverse_line(head);

	return head;

die:
	_build_traverse_line(head);
	xml_free_tree(head);
	return NULL;

}

char*
xml_get_prop(struct xml_tree_node* node, char* propname) {

	for (struct xml_prop* p = node->props; p->name != NULL; p++) {
		if (strcmp(p->name, propname) == 0) {
			return p->value;
		}
	}

	return NULL;

}

/* Compare two strings. If one is NULL, return 1 */
int
xml_strcmpnul(char* s1, char* s2) {

	if (s1 == NULL || s2 == NULL) {
		return 1;
	}

	return strcmp(s1, s2);

}
