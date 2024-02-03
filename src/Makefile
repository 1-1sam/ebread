ebread_cflags := -g -O2 -std=c99 -D_GNU_SOURCE -Wall -Wextra -pedantic $(CFLAGS)
ebread_objects := $(patsubst %.c,%.o,$(wildcard *.c))
ebread_ldflags := $(LDFLAGS)

ifdef ebread_version
  ebread_cflags := $(ebread_cflags) -DEBREAD_VERSION=\"$(ebread_version)\"
endif

all: ebread

ebread: $(ebread_objects)
	$(CC) $(ebread_cflags) $(ebread_objects) $(LDFLAGS) -o ebread

$(ebread_objects): %.o: %.c
	$(CC) -c $(ebread_cflags) $< -o $@

clean:
	rm ebread $(ebread_objects)
