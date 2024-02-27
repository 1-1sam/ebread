/*
 * This is the default path max for zip files in Windows. I'd assume epub
 * creators would not exceed this limit.
 */
#define ZIP_PATH_MAX 260

/* Basically just rm -r */
void uz_rm_tree(char* path);

/* Basically just mkdir -p */
int  uz_make_path(char* path);

/* Unzips contents of epub to outputdir */
/* NOTE: output_dir must end with a slash character */
int uz_unzip_epub (char* epub, char* output_dir);
