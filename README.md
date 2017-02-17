
#XbarGen
##Overview
XbarGen is an open-source software system for synthesizing memristor-based digital circuits.

Specifically, XbarGen executes a translation of a boolean function into a digital circuit. Particularly, it targets a specific memristor-based crossbar architecture.

The underlying idea behind XbarGen is performing a design space exploration of memristor-based crossbar architectures.

XbarGen has been developed from scratch, due to the lack of an automated process of translation from a given boolean function to a memristor-based crossbar architecture.

XbarGen is part of a far-reaching project, yet at its very early stage, which aims to realize a Computer-Aided-Design (CAD) software system that targets memristor architectures.

XbarGen is licensed with the GNU General Public License (GNU GPL). Please visit http://www.gnu.org/licenses/ to see a copy of the license.

Although XbarGen is a project in which different people are involved, we kindly ask you to direct bug reports, questions and other comments to Marcello Traiola (traiola@lirmm.fr) or to the Bitbucket issue tracker.

##Building
Before getting started, there are some requirements that are expected to be satisfied:

###Requirements
- We strongly suggest a unix-like system for compiling and using XbarGen;
- We used CMake, for building the Makefile. The oldest version with which we tested is the 2.8.12.2 but it should reasonably work also with other versions;
- Finally g++ is mandatory, for compiling XbarGen. The oldest version with which we tested is the 4.8.4 (Ubuntu - 2013) but it should reasonably work also with other versions.
- Make sure that the paths of 'cmake' and 'g++' binary folders are included within the PATH environment variable
To compile XbarGen, you need to download the source code, open the shell, move to the downloaded folder and run the following commands:
```
$ mkidr build 
$ cd build 
$ cmake .. 
$ make
```

In case you receive compilation errors, they should be self-explanatory.

If the compilation succeeds, the same directory contains the executables, namely `XbarGen`.

##Documentation
XbarGen is released with documentation. In particular, the source code comes along with Doxygen documentation.

It is possible to generate XbarGen's internal documentation

###Requirements
- Install Doxygen (oldest version tested: 1.8.9.1)
- Install Graphviz (oldest version tested: 2.36)
- Make sure that the paths of 'doxygen' and 'dot' binary folders are included within the PATH environment variable
CMake will eventually prepare the an entry for generating a Doxygen documentation project and you just need to type:
```
$ make doc
```
in the building folder to generate it. The documentation folder will be created in the building directory's parent folder.
