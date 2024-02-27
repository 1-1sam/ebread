/*
 * Procedures for unzipping epub zip archives using miniz.
 */
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fts.h>

#include "miniz.h"
#include "unzip.h"

#define PATHMAX 4095

/* Magic bits used by zip archives. */
static char epub_magic[] = { 0x50, 0x4B, 0x03, 0x04 };

static int
_is_epub(char* filename) {

	FILE* file;
	char magic[4];

	file = fopen(filename, "r");

	fread(magic, sizeof(char), sizeof(magic), file);

	fclose(file);

	if (memcmp(magic, epub_magic, sizeof(magic)) == 0) {
		return 1;
	}

	return 0;

}

/*
 * uz_rm_tree does not check whether we have permission to delete a file as it
 * will only be used to delete files that ebread itself created.
 */
void
uz_rm_tree(char* path) {

	char* pargv[] = { path, NULL };
	FTS* fts;
	FTSENT* cur;

	fts = fts_open(pargv, FTS_NOSTAT, NULL);

	while ((cur = fts_read(fts)) != NULL) {

		switch (cur->fts_info) {
		case FTS_DP:
		case FTS_DNR:
			rmdir(cur->fts_accpath);
			break;
		default:
			unlink(cur->fts_accpath);
			break;
		}

	}

	fts_close(fts);

}

int
uz_make_path(char* path) {

	char* slash;
	char* end;

	slash = path;
	end = strchr(path, '\0');

	for (;;) {

		slash += strspn(slash, "/");
		slash += strcspn(slash, "/");

		*slash = '\0';

	/* EEXIST means the directory already exists, which is to be expected. */
		if (mkdir(path, 0777) == -1 && errno != EEXIST) {
			return -1;
		}

		if (slash == end) {
			break;
		}

		*slash = '/';
	}

	return 0;

}

int
uz_unzip_epub(char* epub, char* output_dir) {

	mz_zip_archive ep_archive;
	int filenum;
	char zip_filename[ZIP_PATH_MAX + 1];
	char unzipped_path[PATHMAX + 1];
	char* last_slash;

	if (!_is_epub(epub)) {
		fprintf(stderr, "%s: Not an epub.\n", epub);
		return -1;
	}

	mz_zip_zero_struct(&ep_archive);

	mz_zip_reader_init_file(&ep_archive, epub, 0);

	filenum = mz_zip_reader_get_num_files(&ep_archive);

	for (int i = 0; i < filenum; i++) {

		memset(unzipped_path, 0, PATHMAX + 1);

		mz_zip_reader_get_filename(&ep_archive, i, zip_filename, ZIP_PATH_MAX);

		strncat(unzipped_path, output_dir, PATHMAX);
		strncat(unzipped_path, zip_filename, PATHMAX - strlen(unzipped_path));

		/* This should give us the full path of the file's parent directory */
		last_slash = strrchr(unzipped_path, '/');
		*last_slash = '\0';

		if (uz_make_path(unzipped_path) == -1) {

			fprintf(stderr, "Error creating extract directory: %s\n",
				unzipped_path);

			mz_zip_reader_end(&ep_archive);

			uz_rm_tree(output_dir);

			return -1;
		}

		*last_slash = '/';

		mz_zip_reader_extract_file_to_file(&ep_archive, zip_filename,
			unzipped_path, 0);

	}

	mz_zip_reader_end(&ep_archive);

	return 0;

}
