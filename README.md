[![Build Status](https://travis-ci.org/ticktac-project/uppaal-to-tchecker.svg?branch=travis-conf)](https://travis-ci.org/ticktac-project/uppaal-to-tchecker)
[![MIT license](https://img.shields.io/badge/License-MIT-blue.svg)](https://lbesson.mit-license.org/)

# uppaal-to-tchecker

As its name suggests `uppaal-to-tchecker` (or `utot` for short) is a tool used 
to translate files produced by [Uppaal] to the new timed-automata 
model-checker [TChecker]. `utot` does not support the whole specification 
language of Uppaal but it should be sufficient for lots of cases; see 
[this document](doc/translation.md) for details. 


`uppaal-to-tchecker` is part of the [Ticktac Project](http://ticktac-project.github.io) and is partially supported by funds ANR-18-CE40-0015.

## Table of contents

* [Compilation and installation](#compilation-and-installation)
* [Usage](#usage)

## Compilation and installation

### Prerequisites

The compilation of `utot` requires:
* [CMake](https://cmake.org) >= 3.10 
* A C++ compiler compliant with the C++11 standard.
* The [UTAP library](http://people.cs.aau.dk/~adavid/utap/) should be installed. 
`utot` sources come with a clone of `libutap` 0.91 source code. If the build 
script fails to find the library on the build host, it tries to compile its own
sources.
* [LibXml2](http://xmlsoft.org/) is required to use by UTAP.

### Compilation

We recommend to compile `utot` in some sub-directory to maintain source 
directory clean. For instance create a sub-directory _build_ from the root
directory of `utot` sources and invoke `cmake` from _build_.

Using `-D` flags, You can specify some options to the configuration process:
* `-DCMAKE_CXX_COMPILER=`*some C++ compiler* to enforce the C++ compiler
* `-DCMAKE_INSTALL_PREFIX=`*absolute path to the installation direct* to specify
the installation directory. By default, `utot` is installed in your _local_ 
system tree (e.g /usr/local). This option permits to install `utot` in a 
directory where you will not need system administrator rights.

In the following example, `utot` will be installed in the directory 
`${HOME}/mysofts/bin`:

    $ mkdir build
    $ cd build  
    $ cmake -DCMAKE_INSTALL_PREFIX=${HOME}/mysofts/ ..
    -- The CXX compiler identification is GNU 8.3.0
    -- Checking whether CXX compiler has -isysroot
    -- Checking whether CXX compiler has -isysroot - yes
    -- Checking whether CXX compiler supports OSX deployment target flag
    -- Checking whether CXX compiler supports OSX deployment target flag - yes
    -- Check for working CXX compiler: /opt/local/bin/c++
    -- Check for working CXX compiler: /opt/local/bin/c++ -- works
    ...
    
The `cmake` command yields a *Makefile* file in your *build* directory. Now 
simply compile and install `utot` with `make && make install`:

    $ make -j && make install
    Scanning dependencies of target utot
    [ 20%] Building CXX object src/CMakeFiles/utot.dir/utot.cpp.o
    [ 40%] Building CXX object src/CMakeFiles/utot.dir/utot-translate.cc.o
    [ 60%] Building CXX object src/CMakeFiles/utot.dir/utot-expr.cc.o
    [ 80%] Building CXX object src/CMakeFiles/utot.dir/utot-decl.cc.o
    [100%] Linking CXX executable utot
    [100%] Built target utot
    $

## Usage

`utot` accepts several options; use -h to display the usage message:

    $ utot -h
    usage: utot [options] [uppaal-input-file] [tchecker-output-file]
    where options are: 
    --debug, -d 	 enable debug traces
    --erase, -e 	 erase output file if it exists
    --help, -h 	 display this help message.
    --verbose, -V 	 increase the level of verbosity
    --version, -v 	 display version number
    --xml 		 enforce XML as input format
    --xta 		 enforce XTA as input language
    --ta 		 enforce TA as input language
    --sysname id 	 specify the label of teh system
    -- 		 specify the end of options (if necessary)
    
    If no input file is specified, the standard input is used.
    If several 'xta', 'xml' or 'ta' options are used the last one prevails.
    
Usually `utot` is invoked with two arguments an input and an output files. If 
none is given, the program reads its standard input and print the result on the 
standard output. If only one file is specified, it is assumed to be the input
file. 

[Uppaal] supports several file format (see 
[UTAP documentation](http://people.cs.aau.dk/%7Eadavid/utap/syntax.html) for 
details). `utot` recognizes the format of its input file according to its 
filename extension: `xta`, `xml` or `ta`. If the input file has no such 
extension or if `utot` reads its standard input, one has to specify the format 
using one of the options: `--xta`, `--xml` or `--ta`.

If the specified output file already exists, `utot` does not erase its content;
this behavior can be changed using option `-e`.

By default, the input filename is used to populate the _system_ line of the 
[TChecker] output (if no input filename is specified the string "_System_" is 
used). The user can specify another name for the system using `--sysname` 
option.

[Uppaal]: http://www.uppaal.org "Uppaal"
[TChecker]: http://github.com/ticktac-project/tchecker "TChecker"