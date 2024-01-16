#include <stdio.h>
#include <sys/stat.h>
#include <errno.h>

#define ZIP_PATH_MAX 260

void uz_rm_tree(char* path);
int  uz_make_path(char* path);
int  uz_unzip_epub (char* epub, char* output_dir);
