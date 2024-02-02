#include <stdio.h>
#include <string.h>
#include <stdlib.h>

enum html_content_type {
	UNKNOWN,
	PARAGRAPH,
	HEADER,
	BREAK
};

struct html_content {
	enum html_content_type type;
	char* text;
	int linebreak;
};

struct parsed_html {
	int content_num;
	struct html_content* content;
};

struct parsed_html html_parse(char* html);
int                html_write_to_text(char* output, struct parsed_html html,
                                      int linelen, int indent);
