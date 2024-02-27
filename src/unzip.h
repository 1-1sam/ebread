#define ZIP_PATH_MAX 260

void uz_rm_tree(char* path);
int  uz_make_path(char* path);

/* Unzips contents of epub to outputdir */
/* NOTE: output_dir must end with a slash character */
int uz_unzip_epub (char* epub, char* output_dir);
