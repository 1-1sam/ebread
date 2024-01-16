libxml_include_dir ?= /usr/include/libxml2

ebread_cflags := -g -O2 -std=c99 -D_GNU_SOURCE -Wall -Wextra -pedantic $(CFLAGS)
ebread_objects := $(patsubst %.c,%.o,$(wildcard *.c))
ebread_ldflags := $(LDFLAGS)
ebread_ldlibs := -lxml2

ifdef libxml_include_dir
  ebread_cflags := $(ebread_cflags) -I$(libxml_include_dir)
endif

all: ebread

ebread: $(ebread_objects)
	$(CC) $(ebread_cflags) $(ebread_objects) $(LDFLAGS) $(ebread_ldlibs) -o ebread

$(ebread_objects): %.o: %.c
	$(CC) -c $(ebread_cflags) $< -o $@

clean:
	rm ebread $(ebread_objects)
