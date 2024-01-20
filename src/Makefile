libxml_include_dir ?= /usr/include/libxml2

ebread_cflags := -g -O2 -std=c99 -D_GNU_SOURCE -Wall -Wextra -pedantic -I$(libxml_include_dir) $(CFLAGS)
ebread_objects := $(patsubst %.c,%.o,$(wildcard *.c))
ebread_ldflags := $(LDFLAGS)
ebread_ldlibs := -lxml2

ifdef ebread_version
  ebread_cflags := $(ebread_cflags) -DEBREAD_VERSION=\"$(ebread_version)\"
endif

all: ebread

ebread: $(ebread_objects)
	$(CC) $(ebread_cflags) $(ebread_objects) $(LDFLAGS) $(ebread_ldlibs) -o ebread

$(ebread_objects): %.o: %.c
	$(CC) -c $(ebread_cflags) $< -o $@

clean:
	rm ebread $(ebread_objects)
