struct spine {
	char** hrefs;
	int hrefnum;
};

int epub_get_rootfile(char* rootfile, char* rootdir);

struct spine epub_get_spine(char* rootfile);

int epub_html2text(char* html, char* output, int linelen, int indent);
