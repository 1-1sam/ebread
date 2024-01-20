# ebread
**ebread** is a Unix program that converts the XHTML conents of EPUB ebook
archives into formatted plaintext. Run `ebread -h` or `man 1 ebread` for
information on how to use **ebread**.

## Building
Building **ebread** requires the following:
* A C compiler
* make
* libxml2

**ebread** can be built on most Linux distributions, BSDs and macOS.

To build and install **ebread**, run the following:
```bash
make
make install # run as root if installing directly into system
make clean
```
By default, make will look in `/usr/include/libxml2` for libxml2 headers. If
your operating system installs them in another location, you may set the
`libxml_include_dir` variable when building.
```bash
make libxml_include_dir=/path/to/libxml2/headers
```

## Credit
[miniz](https://github.com/richgel999/miniz) - Unzipping

## License
**ebread** is licensed under the MIT license. View the *LICENSE* file for more
information.
