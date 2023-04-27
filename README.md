# BitcoinHD PoST C++ wrap library

This is a library used to wrap chia PoST libraries.

## Requirements

* cmake version 3.5 or higher

* gcc or clang and supports c++14

## Build instructions

* Use git to clone this repo to local storage device `git clone https://github.com/bhdone/bhd_vdf`

* Get into the dir of project root and create a new sub-dir `build`

* Get inside the dir `build` and use cmake to configure the project `cmake ..`

* Use command `make -j7` to make binaries

### Download MPIR

GMP or MPIR should be installed on your system, if you didn't have the library installed, you can do the configure with arg `-DDOWNLOAD_EXTERNAL_MPIR=1` and the project will download MPIR from internet for you.

### Build test

Configure with arg `-DBUILD_TEST=1` to build the test cases.
