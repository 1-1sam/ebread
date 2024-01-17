#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

typedef int flag_t;

enum html_content_type {
	UNKNOWN,
	PARAGRAPH,
	HEADER,
	BREAK
};

struct html_tag {
	enum html_content_type type;
	const char* name;
};

static struct html_tag tags[] = {
	{ PARAGRAPH, "p" },
	{ HEADER, "h1" },
	{ HEADER, "h2" },
	{ HEADER, "h3" },
	{ HEADER, "h4" },
	{ HEADER, "h5" },
	{ HEADER, "h6" },
	{ BREAK, "br/" },
	{ UNKNOWN, NULL } /* Terminating element */
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

static enum html_content_type
_get_html_content_type(char* str) {

	/* Exit if given tag is an end tag */
	if (*str == '/') {
		return UNKNOWN;
	}

	for (int i = 0; tags[i].type != UNKNOWN; i++) {
		if (strcmp(str, tags[i].name) == 0) {
			return tags[i].type;
		}
	}

	return UNKNOWN;

}

static int
_is_end_tag(char* str, enum html_content_type type) {

	char* p = str;

	if (*p != '/') {
		return 0;
	}

	p++;

	for (int i = 0; tags[i].type != UNKNOWN; i++) {
		if (strcmp(p, tags[i].name) == 0 && type == tags[i].type) {
			return 1;
		}
	}

	return 0;

}

static int
_get_content_num(char* html) {

	int rtrn = 0;
	FILE* html_file;
	char* linebuf = NULL;
	size_t linebuf_size;
	enum html_content_type cur_tag_type = UNKNOWN;

	html_file = fopen(html, "r");

	while (getdelim(&linebuf, &linebuf_size, '>', html_file) != -1) {

		char *tag_start, *tag_end;

		tag_start = strchr(linebuf, '<');

		if (tag_start == NULL) {
			continue;
		}

		tag_start++;

		tag_end = tag_start;

		while (*tag_end != ' ' && *tag_end != '>') {
			tag_end++;
		}

		*tag_end = '\0';

		if (cur_tag_type == UNKNOWN) {
			cur_tag_type = _get_html_content_type(tag_start);
		} else if (_get_html_content_type(tag_start) == BREAK) {
			rtrn++;
		} else if (_is_end_tag(tag_start, cur_tag_type)) {
			rtrn++;
			cur_tag_type = UNKNOWN;
		}
	}

	fclose(html_file);
	free(linebuf);

	return rtrn;

}

static void
_add_indent(char* line, int indent) {

	for (int i = 0; i < indent; i++) {
		strcat(line, " ");
	}

}

/*
 * If this returns a parsed_html with a content_num of -1, that means something
 * went wrong.
 */
struct parsed_html
html_parse(char* html) {

	struct parsed_html parsed;
	FILE* html_file = fopen(html, "r");
	char* linebuf = NULL;
	size_t linelen;
	int cur = 0;
	int prevlen;
	int curlen = 1;

	parsed.content_num = _get_content_num(html);

	/* Nothing went wrong, this just means this page is blank. */
	if (parsed.content_num == 0) {
		return parsed;
	}

	parsed.content = malloc(sizeof(struct html_content) * parsed.content_num);

	if (parsed.content == NULL) {
		fprintf(stderr, "Failed to allocated memory\n");
		parsed.content_num = -1;
		return parsed;
	}

	parsed.content[cur].type = UNKNOWN;
	parsed.content[cur].text = NULL;
	parsed.content[cur].linebreak = 0;

	while (getdelim(&linebuf, &linelen,'>', html_file) != -1) {

		char *tag_start, *tag_end, *text_start;

		/* Newlines and tabs will be replaced with spaces; easier to format. */
		for (char* p = linebuf; *p != '\0'; p++) {
			if (*p == '\n' || *p == '\t') {
				*p = ' ';
			}
		}

		if ((tag_start = strchr(linebuf, '<')) == NULL) {
			continue;
		}

		tag_start++;

		tag_end = tag_start;

		while (*tag_end != ' ' && *tag_end != '>') {
			tag_end++;
		}

		if (parsed.content[cur].type == UNKNOWN) {
			*tag_end = '\0';
			parsed.content[cur].type = _get_html_content_type(tag_start);
			continue;
		}


		text_start = linebuf;
		*(tag_start - 1) = '\0';

		/* This gets rid of any leading white space. */
		while (*text_start == ' ') {
			text_start++;
		}

		prevlen = curlen - 1;
		curlen += strlen(text_start) + 1;

		parsed.content[cur].text = realloc(parsed.content[cur].text,
			sizeof(char) * curlen);

		/* TODO: Properly clean up allocated memory */
		if (parsed.content[cur].text == NULL) {
			fprintf(stderr, "Failed to allocate memory\n");
			parsed.content = NULL;
			parsed.content_num = -1;
			return parsed;
		}

		/* Initialize the new memory provided by realloc */
		for (int i = prevlen; i < curlen; i++) {
			parsed.content[cur].text[i] = 0;
		}

		strcat(parsed.content[cur].text, text_start);

		*tag_end = '\0';

		if (cur + 1 >= parsed.content_num) {
			break;
		}

		if (_get_html_content_type(tag_start) == BREAK) {
			parsed.content[cur++].linebreak = 1;
			parsed.content[cur].type = parsed.content[cur - 1].type;
			parsed.content[cur].text = NULL;
			parsed.content[cur].linebreak = 0;
			curlen = 1;
		} else if (_is_end_tag(tag_start, parsed.content[cur].type)) {
			cur++;
			parsed.content[cur].type = UNKNOWN;
			parsed.content[cur].text = NULL;
			parsed.content[cur].linebreak = 0;
			curlen = 1;
		}

	}

	free(linebuf);
	fclose(html_file);

	return parsed;
}

int
html_write_to_text(char* output, struct parsed_html html, int linelen,
                   int indent) {

	FILE* output_file = fopen(output, "a");
	char* curline;

	if (output_file == NULL) {
		fprintf(stderr, "Could not write to file: %s\n", output);
		return -1;
	}

	/* Must have room for a trailing space and null terminator */
	curline = calloc(linelen + 2, sizeof(char));

	if (curline == NULL) {
		fprintf(stderr, "Could not allocate memory\n");
		fclose(output_file);
		return -1;
	}

	for (int i = 0; i < html.content_num; i++) {

		char *lp, *null, *space;

		memset(curline, 0, linelen + 2);

		_add_indent(curline, indent);

		lp = html.content[i].text;
		null = strchr(html.content[i].text, '\0');

		while ((lp += strspn(lp, " ")) != null) {

			space = lp + strcspn(lp, " ");

			*space = '\0';

			if (strlen(lp) + strlen(curline) > (size_t) linelen) {

				fprintf(output_file, "%s\n", curline);

				memset(curline, 0, linelen + 2);

				_add_indent(curline, indent);

			/*
		 	* If a word is longer than linelen, split + hyphenate it across
		 	* multiple lines.
		 	*/
				while (strlen(lp) > (size_t) linelen - indent) {

					strncat(curline, lp, linelen - indent - 1);
					strcat(curline, "-");

					fprintf(output_file, "%s\n", curline);

					memset(curline, 0, linelen + 2);

					lp += linelen - indent - 1;

					_add_indent(curline, indent);

				}

			}

			strcat(curline, lp);
			strcat(curline, " ");

			if (space == null) {
				break;
			}

			*space = ' ';
			lp = space;

		}

		if (html.content[i].linebreak) {
			fprintf(output_file, "%s\n", curline);
		} else {
			fprintf(output_file, "%s\n\n", curline);
		}

	}

	free(curline);
	fclose(output_file);

	return 0;

}
