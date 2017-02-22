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
 * entities.h
 *
 *  Created on: 26/mar/2016
 *      Author: Marcello Traiola
 */

#ifndef ENTITIES_H_
#define ENTITIES_H_

#include <vector>
#include <array>
#include <map>
#include <string>
#include <my_utils.h>

using namespace std;

/**
 * This class is the entity model of a boolean function; it contains:
 * - function inputs
 * - function outputs
 * - function minterms
 */
class Function{
	friend class Analyzer;
	friend class Translator;
	vector<string> inputs;
	vector<string> outputs;
	multimap<string,vector<string> > minterms;
	map<string, int> literalCount;

public:
	Function(){};
	Function(vector<string> inputs,
				vector<string> outputs,
				multimap<string,vector<string> > minterms) : inputs(inputs), outputs(outputs), minterms(minterms){};
	void addInput(string s) {inputs.push_back(s);}
	void addOutput(string s) {outputs.push_back(s);}
	void addInputs(vector<string> s);
	void addOutputs(vector<string> s);
	void printInput();
	void printOutput();
	void printFunction();
	void addMinterm(string,string);
	map<string, int> countLiterals();
	int getNumInput();
	int getNumOutput();
	int getNumMinterms();
	int getNumMinterms_NoDuplicate();
	map<string, int> getLiteralCount();
};

typedef vector< vector<int> > crossbarMatrix;
typedef map< string, map<string, string> > crossbarVoltages;

/**
 * This class is the entity model of a FBLC crossbar, which implements a boolean function; it contains:
 * - the memristor crossbar configuration (where are located the memristors)
 * - row index (link between boolean function element (e.g. minterm) and row number in the crossbar)
 * - column index (link between boolean function element (e.g. input) and column number in the crossbar)
 * - voltages: for each state of the FSM, each nanowire voltage is computed
 */
class Crossbar{
	friend class Translator;
private:
	crossbarMatrix matrix;
	map<string, int> rowIndex;
	map<string, int> columnIndex;
	crossbarVoltages voltages;
	int numMemristors;

	void generateVHDLfiles(int,vector<string>,vector<string>);
	void generateCrossbarStructureFile(int);
	void generateCrossbarControllerFile(int,vector<string>,vector<string>);
	void generateCrossbarFile(int ,int);
	string voltageFilter(string,string);

public:
	Crossbar(int numInput, int numOutput, int numMinterms);
	~Crossbar();
	void printMatrix();
	void printVoltages();
	unsigned int getHeight() {return matrix.size();}
	unsigned int getWidth() {return matrix[0].size();}
};

#endif /* ENTITIES_H_ */
