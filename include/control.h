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
 * def.h
 *
 *  Created on: 19/mar/2016
 *      Author: Marcello Traiola
 */

#ifndef CONTROL_H_
#define CONTROL_H_

#include <cstring>
#include <set>
#include "entities.h"
#include <chrono>
#include <lemon/list_graph.h>
#include <lemon/bits/graph_extender.h>

using namespace std;
using namespace lemon;

static int numOfXbarStates = 7;

struct executionParameters{
	bool dot;
	bool deepDot;
	bool vhdl;
	bool stat;
	bool verbose;
};

extern executionParameters execParameters;
extern std::chrono::steady_clock::time_point beginTime;

/**
 * This class is expected to:
 * - analyze the input boolean function, in order to find its subsets
 * - create, for each subset, a 'Translator' object in order to translate
 * 		such subset, that indeed is still a boolean function, into a FBLC crossbar.
 * - generate, if demanded, the VHDL implementation of the entire circuit implementing the input function
 * - produce, if demanded,  the statistics either workload dependent either workload independent for the entire circuit
 */
class Analyzer{

private:
	string file;
	ListDigraph graph;
	ListDigraph::NodeMap<string> nodeNames;
	map <int, vector<ListDigraph::NodeIt> > nodeLevels;
	vector<Analyzer*> subAnalyzers;

	ListDigraph::NodeIt verify_dependencies(string s);
	void build_levels( ListDigraph::Node,ListDigraph::NodeMap<int>*,int);
	void generateStructuralOutputVHDL();
	int getNumOfStages();
	int getNumOfComputationSteps();
	int getNumOfMinterms();
	int* getPowerConsumption();

protected:
	Function func;
	int level;

	Analyzer(int ,vector<string>,
			vector<string> ,
			multimap<string,vector<string> > );
	virtual int getNumMemristor();
	virtual int getArea();
	virtual int* getOperativeMemristorPowerConsumption();

public:
	Analyzer(string file) :  file (file), graph(), nodeNames(graph),level(-1){};
	void analyzeFunctionFromXML();
	void analyzeFunctionFromEQN();
	void createDependenciesGraph(int = -1);
	void virtual generateCrossbar();
	void virtual generateOutputVHDL();
	void printOutputStats();
	void printFunction(){func.printFunction();}
	virtual ~Analyzer(){};
};

/**
 * This class is expected to:
 * - translate a 2level boolean function into a FBLC crossbar
 * - generate, if demanded, the VHDL implementation of such crossbar
*/

class Translator : public Analyzer{

private:

	Crossbar* xbar;

	void create_index();
	int getNumMemristor() override;
	int getArea() override;
	int* getOperativeMemristorPowerConsumption() override;


public:
	Translator(int level,
			vector<string> inputs,
			vector<string> outputs,
			multimap<string,vector<string> > minterms) : Analyzer(level,inputs, outputs,minterms), xbar(NULL) {};
	void generateCrossbar() override;
	void generateVoltages();
	void generateOutputVHDL() override;
	virtual ~Translator() {};
};

#endif /* CONTROL_H_ */
