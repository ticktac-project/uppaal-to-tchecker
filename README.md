[![Build Status](https://travis-ci.org/ticktac-project/uppaal-to-tchecker.svg?branch=travis-conf)](https://travis-ci.org/ticktac-project/uppaal-to-tchecker)
[![MIT license](https://img.shields.io/badge/License-MIT-blue.svg)](https://lbesson.mit-license.org/)

# uppaal-to-tchecker

As its name suggests `uppaal-to-tchecker` (or `utot` for short) is a tool used 
to translate files containing models supported by [Uppaal](http://www.uppaal.org) to the new 
timed-automata model-checker [TChecker](http://github.com/ticktac-project/tchecker).

`uppaal-to-tchecker` is part of the [Ticktac Project](http://ticktac-project.github.io) and is partially supported by funds ANR-18-CE40-0015.

## Table of contents

* [Compilation and installation]()
* [Usage]()

## Compilation and installation

### Prerequisites

The compilation of `utot` requires:
* [CMake](https://cmake.org) >= 3.10 
* a C++ compiler supporting the C++11 standard.
* The [UTAP library](http://people.cs.aau.dk/~adavid/utap/) should be installed. 
`utot` sources come with a clone of `libutap` 0.91. If the build script fails to 
find the library on the compilation host, it tries to compile its own sources.
* [LibXml2](http://xmlsoft.org/) is required by UTAP.

### Compilation

We recommend to compile `utot` in some sub-directory to maintain sources clean. 
For instance create a sub-directory _build_ from the root directory of `utot` 
source and invoke `cmake` from _build_.

Using `-D` flags, You can specify some options to the configuration process:
* `-DCMAKE_CXX_COMPILER=`*some C++ compiler*
* `-DCMAKE_C_COMPILER=`*some C compiler*
* `-DCMAKE_INSTALLPREFIX=`*absolute path to the installation direct*

The last option permits to install `utot` in a directory where you do not need
administrator rights.

    $ mkdir build
    $ cd build  
    $ cmake ..
    -- The CXX compiler identification is GNU 8.3.0
    -- Checking whether CXX compiler has -isysroot
    -- Checking whether CXX compiler has -isysroot - yes
    -- Checking whether CXX compiler supports OSX deployment target flag
    -- Checking whether CXX compiler supports OSX deployment target flag - yes
    -- Check for working CXX compiler: /opt/local/bin/c++
    -- Check for working CXX compiler: /opt/local/bin/c++ -- works
    ...
    
The `cmake` command yields a *Makefile* file in your *build* directory. Now 
simply compile `utot` with `make`:

    $ make 
    Scanning dependencies of target utot
    [ 20%] Building CXX object src/CMakeFiles/utot.dir/utot.cpp.o
    [ 40%] Building CXX object src/CMakeFiles/utot.dir/utot-translate.cc.o
    [ 60%] Building CXX object src/CMakeFiles/utot.dir/utot-expr.cc.o
    [ 80%] Building CXX object src/CMakeFiles/utot.dir/utot-decl.cc.o
    [100%] Linking CXX executable utot
    [100%] Built target utot
    $

### Tests

Before installing the binary file from src/`utot`, you should run the test-suite
using the `test` target. Tests are only available if `tchecker` is found by `cmake`.

    make test 
    Running tests...
    Test project .../build
            Start   1: utot-build
      1/123 Test   #1: utot-build .......................   Passed    0.17 sec
            Start   2: utot-build
      2/123 Test   #2: utot-build .......................   Passed    0.18 sec
            Start   3: ad94@
      3/123 Test   #3: ad94@ ............................   Passed    0.11 sec
            Start   4: critical-region-async@3
      4/123 Test   #4: critical-region-async@3 ..........   Passed    0.62 sec
    ...    

## Usage

