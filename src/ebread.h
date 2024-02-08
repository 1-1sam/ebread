typedef int flag_t;

struct ebread {
	char* epub;
	enum { RUN, NORUN, ERROR } run_state;
	enum { PARSE, UNZIP } mode;
	char* output_dir;
	char* output_name;
	flag_t verbose;
	unsigned long linelen;
	unsigned long indent;
	int stdout;
	char* output_file;
};

struct ebread ebread_init(int argc, char** argv);
int           ebread_run(struct ebread init);
