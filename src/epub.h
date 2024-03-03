/*
 * The spine is a structure in an epub root file that lists the xhtml content
 * files of the epub using IDs. Each ID has a respective content file listed in
 * the root file's manifest. We will use the spine to determine which files to
 * parse.
 */
struct spine {
	char** hrefs;
	int hrefnum;
};

/*
 * Get the path to the epub's root file. rootdir is the directory that the epub
 * was unzipped to. If the root file is found, it is written to rootfile.
 */
int epub_get_rootfile(char* rootfile, char* rootdir);

/* Get spine in rootfile, which we will use to find what xhtml files to parse */
/* NOTE: Spine should be freed using epub_free_spine when no longer in use. */
struct spine epub_get_spine(char* rootfile);

/* Free spine created by epub_get_spine */
void epub_free_spine(struct spine spine);

/*
 * Parse html, write to output. linelen specifies maximum output line length
 * (including indent spaces) and indent the number of spaces an indent has.
 * There are some things html2text is not capable of doing (as of right now):
 * - Does not read {un}ordered lists properly. html2text just treats each item
 *   as a normal piece of text with a linebreak at the end.
 * - No special formatting. <p>, <h1>, <li>, etc. all look the same. Bold
 *   and italics are ignored.
 * - Links in <a> tags are ignored.
 * - Anything relating to CSS is ignored.
 */
int epub_html2text(char* html, char* output, int linelen, int indent);
