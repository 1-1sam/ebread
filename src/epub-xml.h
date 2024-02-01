struct spine {
	char** hrefs;
	int hrefnum;
};

int xml_get_rootfile(char* rootfile, char* rootdir);

struct spine xml_get_spine(char* rootfile);
