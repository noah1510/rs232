RS232
=====

A C++17 library for using rs232 protocol on all platforms.
This project was originally a fork of [kranfix/rs232](https://github.com/kranfix/rs232).
All the C support was dropped and huge code changes were made to make this a proper C++ library.

## Installation

If you use meson, just add the following to your `meson.build` and clone the repo in you subprojects folder:

```meson
rs232_dep = dependency('rs232', version: '>=2.0.0')
```

For other build systems, you have to compile the library and run the meson install command.
The project generates pkg-config files, so you can use that to link the library with any build system.

## Usage

You can include the following two headers to use the library:

* 'rs232.hpp'
* 'rs232_native.hpp'

The first one is most likely what you want to use.
It provides high level access to the RS232 interface and allows for easy use of the library.
The second header provides access to the lower level functions.
It is not recommended to use this header directly, unless you know what you are doing.
That class does little to no error checking, so you have to do that yourself.
Some of the return values are the values given by the os, so you need some knowledge of C programming to use it.

For an example on how to use this library check the samples folder.
The interfaceTest sample also has the corresponding Arduino code in the example folder.

