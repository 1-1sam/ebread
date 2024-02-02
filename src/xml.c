#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct xml_tree_node {
	char* name;
	struct xml_tree_node* next;
	struct xml_tree_node* prev;
	struct xml_tree_node* parent;
	struct xml_tree_node* child;
	struct xml_tree_node* preorder;
	char* attributes;
	char* text;
	char* content_ptr;
};

/* Used to initialize newly created nodes */
static struct xml_tree_node null_node = {
	.name = NULL,
	.next = NULL,
	.prev = NULL,
	.parent = NULL,
	.child = NULL,
	.preorder = NULL,
	.attributes = NULL,
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

static int
_parse_tag(char* tag, struct xml_tree_node* node) {

	char* name;
	char* attributes;

	name = tag + strspn(tag, " ");

	attributes = name + strcspn(tag, " ");
	attributes += strspn(attributes, " ");

	*(name + strcspn(name, " ")) = '\0';

	node->name = name;
	node->attributes = (*attributes != '\0') ? attributes : NULL;

	return 0;

}

static struct xml_tree_node*
_make_child_node(struct xml_tree_node* parent) {

	struct xml_tree_node* rtrn = parent;

	if (rtrn->child == NULL) {
		rtrn->child = malloc(sizeof(struct xml_tree_node));
		*(rtrn->child) = null_node;
		rtrn->child->parent = rtrn;
		rtrn = rtrn->child;
	} else {
		rtrn = rtrn->child;
		while (rtrn->next != NULL) {
			rtrn = rtrn->next;
		}
		rtrn->next = malloc(sizeof(struct xml_tree_node));
		*(rtrn->next) = null_node;
		rtrn->next->prev = rtrn;
		rtrn->next->parent = rtrn->parent;
		rtrn = rtrn->next;
	}

	return rtrn;

}

static void
_free_node(struct xml_tree_node* node) {

	if (node->child != NULL) {
		_free_node(node->child);
	}

	if (node->next != NULL) {
		_free_node(node->next);
	}

	free(node);

}

static struct xml_tree_node*
_get_preorder(struct xml_tree_node* node) {

	if (node->child != NULL) {
		node->preorder = node->child;
		_get_preorder(node->preorder);
	}

	if (node->next != NULL) {
		node->preorder = node->next;
		node = node->preorder;
		node = _get_preorder(node);
	}

	return node;

}

static void
_build_preorder(struct xml_tree_node* head) {

	struct xml_tree_node* cur = head;

	cur = _get_preorder(cur);

	cur->preorder = NULL;

}

struct xml_tree_node*
build_xml_tree(char* xml) {

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
		text = strchr(tag, '>') + 1;
		*(text - 1) = '\0';

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
			cur = _make_child_node(cur);
			_parse_tag(tag, cur);
			cur = cur->parent;
		/* New child node */
		} else {
			cur = _make_child_node(cur);
			_parse_tag(tag, cur);
		}

		if (text != NULL) {
			cur = _make_child_node(cur);
			cur->text = text;
			cur = cur->parent;
		}

	} while ((curtok = strtok(NULL, "<")) != NULL);

	_build_preorder(head);

	return head;

}

void
free_tree(struct xml_tree_node* head) {

	struct xml_tree_node *cur, *next;

	free(head->content_ptr);

	cur = head;

	while (cur != NULL) {
		next = cur->preorder;
		free(cur);
		cur = next;
	}

	//_free_node(head);

}
