/*
*   This file is part of XbarGen
*   XbarGen is an open-source software system for synthesizing memristor-based crossbar architectures. 
*
*    Copyright (C) 2016-2017  Marcello Traiola
*
*    This program is free software: you can redistribute it and/or modify
*    it under the terms of the GNU Affero General Public License as
*    published by the Free Software Foundation, either version 3 of the
*    License, or (at your option) any later version.
*
*    This program is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*    GNU Affero General Public License for more details.
*
*    You should have received a copy of the GNU Affero General Public License
*    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/*! \mainpage XbarGen
 *
 * \section intro_sec Introduction
 *
 * XbarGen is an open-source software system for synthesizing memristor-based digital circuits.

Specifically, XbarGen executes a translation of a boolean function into a digital circuit. Particularly, it targets a specific memristor-based crossbar architecture.

The underlying idea behind XbarGen is performing a design space exploration of memristor-based crossbar architectures.

XbarGen has been developed from scratch, due to the lack of an automated process of translation from a given boolean function to a memristor-based crossbar architecture.

XbarGen is part of a far-reaching project, yet at its very early stage, which aims to realize a Computer-Aided-Design (CAD) software system that targets memristor architectures.

XbarGen is licensed with the GNU Affero General Public License (GNU AGPL). Please visit http://www.gnu.org/licenses/ to see a copy of the license.

Although XbarGen is a project in which different people are involved, we kindly ask you to direct bug reports, questions and other comments to <a href="http://www.lirmm.fr/~traiola/">Marcello Traiola</a> or to the GitHub issue tracker.
 *
 * \section prereq Prerequisites
 *Before getting started, there are some requirements that are expected to be satisfied.
*<ul>
*<li> We strongly suggest a unix-like system for compiling and using XbarGen;
*<li> We used CMake, for building the Makefile. The oldest version with which we tested is the 2.8.12.2 but it should reasonably work also with other versions;
*<li> Finally g++, for compiling XbarGen. The oldest version with which we tested is the 4.8.4 (Ubuntu - 2013) but it should reasonably work also with other versions.

 * \section build Building
 *
To compile XbarGen, you need to download the source code, open the shell, move to the downloaded folder and run the following commands:<br/>
$ mkidr build<br/>
$ cd build<br/>
$ cmake ..<br/>
$ make<br/>
In case you receive compilation errors, they should be self-explanatory.

If the compilation succeeds, the same directory contains the executables, namely XbarGen.
*/