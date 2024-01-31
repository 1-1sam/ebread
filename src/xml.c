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
	char* attributes;
	char* text;
	char* content_ptr;
};

/* Used to initialize newly created nodes */
struct xml_tree_node null_node = {
	.name = NULL,
	.next = NULL,
	.prev = NULL,
	.parent = NULL,
	.child = NULL,
	.attributes = NULL,
	.text = NULL,
	.content_ptr = NULL,
};

/* Reads the entire file into a string. */
char*
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

int
_is_end_tag(char* tag, struct xml_tree_node* node) {

	if (*tag != '/' || node->name == NULL) {
		return 0;
	}

	if (strcmp(tag + 1, node->name) == 0) {
		return 1;
	}

	return 0;

}

int
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

struct xml_tree_node*
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

	return head;

}

void
free_tree(struct xml_tree_node* head) {

	free(head->content_ptr);

	_free_node(head);

}

void
print_names(struct xml_tree_node* node) {

	if (node->child != NULL) {
		print_names(node->child);
	}

	if (node->next != NULL) {
		print_names(node->next);
	}

	if (node->text != NULL) {
		printf("%s\n", node->text);
	}

}

int
main(int argc, char** argv) {

	char* xml_name;
	struct xml_tree_node* head;
	struct xml_tree_node* cur;

	if (argc != 2) {
		fprintf(stderr, "Invalid number of arguments\n");
		return 1;
	}

	xml_name = argv[1];

	head = build_xml_tree(xml_name);

	print_names(head);

	free_tree(head);

	return 0;


}
