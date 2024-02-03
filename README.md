# ebread
**ebread** is a Unix program that converts the XHTML conents of EPUB ebook
archives into formatted plaintext. Run `ebread -h` or `man 1 ebread` for
information on how to use **ebread**.

## Building
Building **ebread** requires the following:
* A C compiler
* GNU make

**ebread** can be built on most Linux distributions, BSDs and macOS.

To build and install **ebread**, run the following:
```bash
make
make install # run as root if installing directly into system
make clean
```

## Credit
[miniz](https://github.com/richgel999/miniz) - Unzipping

## License
**ebread** is licensed under the MIT license. View the *LICENSE* file for more
information.
