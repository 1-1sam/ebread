#include "xml.h"

typedef int flag_t;

struct html_rules {

};

struct html_content {
	char* tag;
	struct html_rules* rules;
};
