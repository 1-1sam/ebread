#include "ebread.h"

int
main(int argc, char** argv) {

	struct ebread ebread;

	ebread = ebread_init(argc, argv);

	if (ebread.run_state == ERROR) {
		return 1;
	}

	if (ebread.run_state == NORUN) {
		return 0;
	}

	ebread_run(ebread);

	return 0;

}
