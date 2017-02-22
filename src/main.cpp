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

/*
 * main.cpp
 *
 *  Created on: 19/mar/2016
 *      Author: Marcello Traiola
 */
#include "control.h"
#include <iostream>
#include <sys/wait.h>
#include <chrono>
#include <my_utils.h>

using namespace std;

executionParameters execParameters;
chrono::high_resolution_clock::time_point startTime;

bool evaluate(string s);
string usage();

int main (int argc, char *argv[]){
	if(argc>1){
		int file=0;
		for(int i=1; i<argc;i++){
			//evaluate option
			if(evaluate(string(argv[i])))
				file=i;
		}
		if(file!=0){
			//if user wants statistics
			if(execParameters.stat){
				//start taking time
				startTime = chrono::high_resolution_clock::now();
			}
			string str(argv[file]);
			Analyzer an(str);

			//the analyzer extracts a model of the input function
			an.analyzeFunctionFromEQN();

			//the analyzer explores the function's subsets
			an.createDependenciesGraph();

			//for each subset, the analyzer generates the corresponding crossbar
			an.generateCrossbar();

			//if user wants the vhdl implementation of the circuit
			if(execParameters.vhdl){
				if(!loadVHDLReservedWords())
					cout<<"WARNING: VHDL reserved words are not loaded\n";
				//generate the VHDL output
				an.generateOutputVHDL();
			}
			//if user wants statistics
			if(execParameters.stat)
				//print out statistics
				an.printOutputStats();
		}
		else
			//print help
			cout<<usage()<<endl;

	}
	else
		//print help
		cout<<usage()<<endl;

	return 0;
}
/**
*returns the usage guide
*/
string usage(){
	return 	"Usage:\n"
			"\tXbarGen <filename.eqn> [--help] [--graph] [--dgraph] [--stat] [--vhdl] [--verbose]\n"
			"\n"
			"\n"
			"\tOptions:\n"
			"\t--help     Show this screen.\n"
			"\t--graph    Produce the dependencies' graph of the boolean function (.dot format).\n"
			"\t--dgraph   If --graph is set, produce dependencies' graph of each 'level'(*) of the function.\n"
			"\t--stat     Produce a textual file with some statistics about the circuit.\n"
			"\t--vhdl     Produce a memristor based crossbar behavioral implementation of the given function (VHDL language).\n"
			"\t--verbose  Print informations about the translation's process.\n";
}

/**
 * evaluate 's' for understanding whether it is an option or the input file
 * if it's an option, activate such option
 */
bool evaluate(string s){
	if(s.at(0)=='-'){
		if(s=="--help")
			cout<<usage();
		else if(s=="--graph")
			execParameters.dot = true;
		else if(s=="--dgraph")
			execParameters.deepDot = true;
		else if(s=="--stat")
			execParameters.stat = true;
		else if(s=="--vhdl")
			execParameters.vhdl = true;
		else if(s=="--verbose")
			execParameters.verbose = true;
		else
			cout<<s<<" ignored\n";
		return false;
	}
	return true;
}


