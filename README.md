RS232
=====

A C C++ library for using rs232 protocole over Linux and Windows.
This project was originally a fork of [kranfix/rs232](https://github.com/kranfix/rs232).
All of the C cupport was dropped and huge code changes were made to make this a proper C++ library.

## Instalation

1) Download the source code:

```
git clone https://github.com/noah1510/rs232.git
```

2) Compile for your computer:


```
meson setup build
meson compile -C build
```

## Files

* Header: rs232.hpp
* main source file: rs232.cc
* For unix: rs232_linux.cc
* For windows: rs2323_win.cc


## Available Functions

In C++:

# Samples with an Arduino

For excecute the samples, first you must compile this code
in your Arduino:

- [Arduino tester](example/arduino_tester/arduino_tester.ino)

The sample code is compiled by default if there is a sample for your current platform.
