#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <limits.h>

#include "epub-html.h"
#include "epub-xml.h"
#include "unzip.h"

#ifndef EBREAD_VERSION
#  define EBREAD_VERSION "0.1"
#endif

#define PATHMAX 4096

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
	flag_t stdout;
	char* single_output_file;
};

static void
_print_usage(void) {

	printf("Usage: ebread [-oxVhuv] [-1 file] [-d dir] [-n name] [-i num] [-l num] EPUB\n");

}

static void
_print_version(void) {

	printf("ebread - %s\n", EBREAD_VERSION);

}

static void
_print_help(void) {

	_print_version();
	_print_usage();
	printf("Options:\n");
	printf(" -1 <file>  --single-file=<file>        Write all output to file.\n");
	printf(" -d <dir>   --output-directory=<dir>    Place output files in dir.\n");
	printf(" -n <name>  --name=<name>               Name output files.\n");
	printf(" -i <num>   --indent=<num>              Set output indent size (default is 4).\n");
	printf(" -l <num>   --line-length=<num>         Set output line length (default is 80).\n");
	printf(" -o         --stdout                    Write parsed text to stdout.\n");
	printf(" -x         --extract                   Extract epub contents, do no parsing.\n");
	printf(" -V         --verbose                   Enable verbose output.\n");
	printf(" -h         --help                      Print this help message.\n");
	printf(" -u         --usage                     Print usage message.\n");
	printf(" -v         --version                   Print program version.\n");

}

static int
_get_unzip_dir(char* uz_dir, struct ebread ebread) {

	char* eb;

	if (ebread.mode == PARSE) {

		strcpy(uz_dir, "/tmp/ebread-XXXXXX");

		if (mkdtemp(uz_dir) == NULL) {
			fprintf(stderr, "Could not create temporary extract directory\n");
			return -1;
		}

	} else if (ebread.mode == UNZIP && ebread.output_dir == NULL) {

		if (strchr(ebread.epub, '/') != NULL) {
			eb = strrchr(ebread.epub, '/') + 1;
		} else {
			eb = ebread.epub;
		}

		strcpy(uz_dir, eb);

		if (strchr(uz_dir, '.') != NULL) {
			*(strrchr(uz_dir, '.')) = '\0';
		}

		if (access(uz_dir, F_OK) == 0) {
			strcat(uz_dir, ".d");
		}

	} else if (ebread.mode == UNZIP && ebread.output_dir != NULL) {

		strcpy(uz_dir, ebread.output_dir);

	}

	if (*(strchr(uz_dir, '\0') - 1) != '/') {
		strcat(uz_dir, "/");
	}

	return 0;
}


/*
 * The name of the output directory will be the name of epub archive without the
 * epub extension, unless the -d option was used. Add the .d suffix to the
 * output directory if the epub has no suffix and a file of the same name exists
 * in the working directory.
 */
static void
_get_output_dir(char* out_dir, struct ebread ebread) {

	if (ebread.output_dir != NULL) {
		strcpy(out_dir, ebread.output_dir);
		strcat(out_dir, "/");
		return;
	}

	if (strchr(ebread.epub, '/') != NULL) {
		strcpy(out_dir, strrchr(ebread.epub, '/') + 1);
	} else {
		strcpy(out_dir, ebread.epub);
	}

	if (strchr(out_dir, '.') != NULL) {
		*(strrchr(out_dir, '.')) = '\0';
	}

	if (access(out_dir, F_OK) == 0) {
		strcat(out_dir, ".d");
	}

	strcat(out_dir, "/");

}

static void
_get_output_file(struct ebread ebread, char* outputf,
                 char* outputd, char* parsedf) {

	static int fc = 1; /* Used for -n filename numerical suffix */
	char fcstr[10];

	strcat(outputf, outputd);

	if (ebread.output_name != NULL) {

		char* fn = strrchr(outputf, '/');

		strcat(fn, ebread.output_name);

		sprintf(fcstr, "-%d", fc++);

		if (strchr(fn, '.') != NULL) {
			*(strchr(fn, '.')) = '\0';
			strcat(fn, fcstr);
			strcat(fn, strchr(ebread.output_name, '.'));
		} else {
			strcat(fn, fcstr);
		}

	} else {

		if (strchr(parsedf, '/') != NULL) {
			strcat(outputf, strrchr(parsedf, '/') + 1);
		} else {
			strcat(outputf, parsedf);
		}

		if (strchr(outputf, '.') != NULL) {
			*(strrchr(outputf, '.')) = '\0';
		}

		strcat(outputf, ".txt");

	}

}

struct ebread
ebread_init(int argc, char** argv) {

	int c;
	/* TODO: Perhaps make ebread a global...? */
	struct ebread ebread = {
		.epub = NULL,
		.run_state = RUN,
		.mode = PARSE,
		.output_dir = NULL,
		.output_name = NULL,
		.verbose = 0,
		.linelen = 80,
		.indent = 4,
		.stdout = 0,
		.single_output_file = NULL,
	};

	struct option opts[] = {
		{ "single-file", required_argument, 0, '1' },
		{ "indent", required_argument, 0, 'i' },
		{ "line-length", required_argument, 0, 'l' },
		{ "stdout", no_argument, 0, 'o' },
		{ "output-directory", required_argument, 0, 'd' },
		{ "name", required_argument, 0, 'n' },
		{ "extract", no_argument, 0, 'x' },
		{ "verbose", no_argument, 0, 'V' },
		{ "help", no_argument, 0, 'h' },
		{ "usage", no_argument, 0, 'u' },
		{ "version", no_argument, 0, 'v' },
		{ 0, 0, 0, 0 }
	};

	while ((c = getopt_long(argc, argv, "1:i:l:od:n:xVhuv", opts, NULL)) != -1) {
		switch (c) {
		case '1':
			ebread.single_output_file = optarg;
			break;
		case 'i':
			ebread.indent = strtoul(optarg, NULL, 10);
			break;
		case 'l':
			ebread.linelen = strtoul(optarg, NULL, 10);
			break;
		case 'o':
			ebread.stdout = 1;
			break;
		case 'd':
			ebread.output_dir = optarg;
			break;
		case 'n':
			if (strchr(ebread.output_name = optarg, '/') != NULL) {
				fprintf(stderr, "Filename contains '/': %s\n",
					ebread.output_name);
				ebread.run_state = ERROR;
			}
			break;
		case 'x':
			ebread.mode = UNZIP;
			break;
		case 'V':
			ebread.verbose = 1;
			break;
		case 'h':
			_print_help();
			ebread.run_state = NORUN;
			return ebread;
		case 'u':
			_print_usage();
			ebread.run_state = NORUN;
			return ebread;
		case 'v':
			_print_version();
			ebread.run_state = NORUN;
			return ebread;
		case '?':
			ebread.run_state = ERROR;
			return ebread;
		default:
			fprintf(stderr, "Error parsing options\n");
			ebread.run_state = ERROR;
			return ebread;
		}
	}

	if (optind < argc) {
		ebread.epub = argv[optind];
	} else {
		_print_help();
		ebread.run_state = ERROR;
	}

	/* 0 or ULONG_MAX means something went wrong with the strtoul conversion. */
	if (ebread.linelen == 0 || ebread.linelen == ULONG_MAX) {
		ebread.linelen = 80;
	}

	if (ebread.indent == ULONG_MAX) {
		ebread.indent = 0;
	}

	/*
	 * 2 is the minimum line length because each line must have room for at
	 * least 1 character and a hyphen.
	 */
	if (ebread.linelen <= 2) {
		ebread.linelen = 80;
	}

	if (ebread.indent >= ebread.linelen - 2) {
		ebread.indent = 0;
	}

	return ebread;

}

int
ebread_run(struct ebread init) {

	char uz_dir[PATHMAX + 1];
	char out_dir[PATHMAX + 1];
	char rootfile[ZIP_PATH_MAX + 1] = { 0 };
	struct spine spine;
	char content_dir[ZIP_PATH_MAX + 1];
	char cur_file[PATHMAX + 1];
	char cur_out[PATHMAX + 1];

	if (access(init.epub, R_OK) == -1) {
		fprintf(stderr, "Could not open %s\n", init.epub);
		return 1;
	}

	if (_get_unzip_dir(uz_dir, init) == -1) {
		return 1;
	}

	if (uz_unzip_epub(init.epub, uz_dir) == -1) {
		fprintf(stderr, "Error extracting %s\n", init.epub);
		return 1;
	}

	/* Nothing else to be done in UNZIP mode :-) */
	if (init.mode == UNZIP) {
		return 0;
	}

	if (init.stdout) {
		strcpy(cur_out, "/dev/stdout");
	} else if (init.single_output_file != NULL) {

		if (strchr(init.single_output_file, '/') != NULL) {

			*(strrchr(init.single_output_file, '/')) = '\0';

			if (uz_make_path(init.single_output_file) == -1) {
				fprintf(stderr, "Could not create output directory: %s\n",
					out_dir);
				return 1;
			}

			*(strchr(init.single_output_file, '\0')) = '/';
		}

		strcpy(cur_out, init.single_output_file);

	} else if (init.single_output_file == NULL) {
		_get_output_dir(out_dir, init);
		if (uz_make_path(out_dir) == -1) {
			fprintf(stderr, "Could not create output directory: %s\n", out_dir);
			return 1;
		}
	}

	if (xml_get_rootfile(rootfile, uz_dir) == -1) {
		fprintf(stderr, "Could not find rootfile in %s\n", init.epub);
		return 1;
	}

	spine = xml_get_spine(rootfile);

	if (spine.hrefs == NULL) {
		fprintf(stderr, "Could not parse rootfile in %s\n", init.epub);
		return 1;
	}

/*
 * The content directory is the directory where all the html pages are stored.
 * The rootfile is also in the content directory, so we'll use it to copy it
 * over to cont_dir.
 */
	if (strchr(rootfile, '/') != NULL) {
		*(strrchr(rootfile, '/')) = '\0';
		strcpy(content_dir, rootfile);
		*(strchr(rootfile, '\0')) = '/';
	} else {
		strcpy(content_dir, uz_dir);
	}

	for (int i = 0; i < spine.hrefnum; i++) {

		memset(cur_file, 0, ZIP_PATH_MAX);
		sprintf(cur_file, "%s/%s", content_dir, spine.hrefs[i]);

		if (init.single_output_file == NULL && !init.stdout) {
			memset(cur_out, 0, PATHMAX);
			_get_output_file(init, cur_out, out_dir, spine.hrefs[i]);
		}

		html_write_to_file(cur_file, cur_out, init.linelen, init.indent);

	}

	for (int i = 0; i < spine.hrefnum; i++) {
		free(spine.hrefs[i]);
	}
	free(spine.hrefs);

	if (init.verbose) {
		printf("Deleting temporary extract directory: %s\n", uz_dir);
	}

/*
 * TODO:
 * Sometimes uz_rm_tree never gets to delete the uz_dir when using the -o,
 * option and piping sometimes, this is because ebread recieves a SIGPIPE. Is
 * there a way around this?
 */
	uz_rm_tree(uz_dir);

	return 0;

}

