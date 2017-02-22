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
 * Analyser.cpp
 *
 *  Created on: 19/mar/2016
 *      Author: Marcello Traiola
 */

#include <fstream>
#include "control.h"
#include "my_utils.h"
#include <iostream>
#include <chrono>
#include <sys/stat.h>

using namespace std;
extern chrono::high_resolution_clock::time_point startTime;

/**
*constructor with parameters
*/
Analyzer::Analyzer(int level, vector<string> inputs,vector<string> outputs,multimap<string,vector<string> > minterms) :  graph(), nodeNames(graph),func(inputs,outputs,minterms), level(level){
	if(execParameters.verbose){
		cout<<"***FUNCTION PARAMETERS***"<<endl<<endl;
		cout<<"input: ";
		func.printInput();
		cout<<endl;
		cout<<"output: ";
		func.printOutput();
		cout<<endl;
		func.printFunction();
		cout<<endl;
		cout<<endl<<"***END FUNCTION PARAMETERS***"<<endl<<endl;
	}
}

//void Analyzer::analyzeFunctionFromXML(){
//	XMLDocument doc;
//	doc.LoadFile(this->file.c_str());
//
//	XMLElement* function = doc.FirstChildElement( "function" );
//	XMLElement* inputs = function->FirstChildElement( "inputs" );
//	XMLElement* outputs = function->FirstChildElement( "outputs" );
//	XMLElement* minterms = function->FirstChildElement( "minterms" );
//
//	//parse all inputs
//	for (XMLElement* e = inputs->FirstChildElement("input"); e!= NULL; e = e->NextSiblingElement("input")){
//		this->func.addInput(e->Attribute("id"));
//	}
//
//	//parse all outputs
//	for (XMLElement* e = outputs->FirstChildElement("output"); e!= NULL; e = e->NextSiblingElement("output")){
//		this->func.addOutput(e->Attribute("id"));
//	}
//
//	//parse all minterms
//	for (XMLElement* e = minterms->FirstChildElement("minterm"); e!= NULL; e = e->NextSiblingElement("minterm")){
//		func.addMinterm(e->Attribute("output"),e->Attribute("value"));
//	}
//
//	if(execParameters.verbose){
//		cout<<"***FUNCTION PARAMETERS***"<<endl<<endl;
//		cout<<"input: ";
//		this->func.printInput();
//		cout<<endl;
//		cout<<"output: ";
//		this->func.printOutput();
//		cout<<endl;
//		this->func.printFunction();
//		cout<<endl;
//		cout<<endl<<"***END FUNCTION PARAMETERS***"<<endl<<endl;
//	}
//}

/**
*Starting from file in .eqn format, extract the boolean function
*/
void Analyzer::analyzeFunctionFromEQN(){
	const string inputLabel = "INORDER";
	const string outputLabel = "OUTORDER";
	string line;
	ifstream myfile(this->file);
	if (myfile.is_open())
	{
		string expression;
		while ( getline (myfile,line) )
		{
			//skip the line if it's a comment
			if(line.length()>0 && line.at(0)=='#')
				continue;

			expression+=line;
			if(expression.find(";")!= std::string::npos){
				//'expression' contains a whole line

				//delete ';'
				expression = expression.substr(0,expression.find_last_of(';'));

				//divide left and right expressions
				int equal = expression.find_first_of ('=');
				string leftExpression = trim(expression.substr(0, equal));
				string rightExpression = trim(expression.substr(equal+1));

				if(leftExpression==inputLabel){
					//parsing inputs
					vector<string> inputs = tokenize(rightExpression," ");
					func.addInputs(inputs);
					for(vector<string>::const_iterator i = inputs.begin(); i!= inputs.end();i++){
						func.addInput("not_"+*i);
					}
				}
				else if(leftExpression==outputLabel){
					//parsing outputs
					func.addOutputs(tokenize(rightExpression," "));
				}
				else{
					//parsing minterms
					vector<string> minterms = tokenize(rightExpression,"+");
					for(vector<string>::iterator i = minterms.begin(); i!=minterms.end();i++){
						replace_substring(&*i, string("!"),string("not_"));
						replace_substring(&*i, string("*"),string(" "));
						trim(&*i);
						func.addMinterm(leftExpression,*i);
					}

				}
				//cout<<leftValue<<" = "<<rightExpression<<';'<<endl;
				expression.clear();
			}
			else if(*(expression.end()-1) == '\n' || *(expression.end()-1) == '\r'){
				//'expression' doesn't contain whole line
				//delete carriage return
				expression = expression.substr(0,expression.find_last_of('\n'));
				expression = expression.substr(0,expression.find_last_of('\r'));
				continue;
			}
		}
		myfile.close();
	}

	else cout << "Unable to open file";

	if(execParameters.verbose){
		cout<<"***FUNCTION PARAMETERS***"<<endl<<endl;
		cout<<"input: ";
		this->func.printInput();
		cout<<endl;
		cout<<"output: ";
		this->func.printOutput();
		cout<<endl;
		this->func.printFunction();
		cout<<endl;
		cout<<endl<<"***END FUNCTION PARAMETERS***"<<endl<<endl;
	}
}

/**
* Generate the Graph of dependencies for the given input function.
*/
void Analyzer::createDependenciesGraph(int level){
	//initialize levels of nodes at 0
	ListDigraph::NodeMap<int> levels(graph,0);

	//starting from outputs build the dependency tree
	for(vector<string>::const_iterator i = func.outputs.begin(); i != func.outputs.end(); i++){
		verify_dependencies(*i);
	}

	//starting from inputs (which have level 0) build subsets of the boolean function
	for(vector<string>::const_iterator i = func.inputs.begin(); i != func.inputs.end(); i++){
		if((*i).find("not_") == std::string::npos){
			ListDigraph::NodeIt n = getVertexByName(&graph,&nodeNames,*i);
			if(n!=INVALID)
				build_levels(n,&levels,0);
		}
	}

	//build a map that, for each subset, has the corresponding terms of the function
	for (ListDigraph::NodeIt v(graph); v != INVALID; ++v){
		nodeLevels[levels[v]].push_back(v);
	}

	//generate dot file, if demanded
	if(execParameters.dot){
		generateDOTfromGraph(this->level,&graph,&nodeNames);
		if(execParameters.verbose){

			cout<<"***DOT GENERATION***"<<endl<<endl;
			cout<<"DOT file Generated";
			cout<<endl;
			cout<<endl<<"***END DOT GENERATION***"<<endl<<endl;
		}
	}

	if(execParameters.verbose){
		cout<<"***LEVELS***"<<endl<<endl;
		for(map <int, vector<ListDigraph::NodeIt> >::const_iterator i = nodeLevels.begin(); i != nodeLevels.end(); i++ ){
			cout<<"level "<<i->first<<"->";
			for(vector<ListDigraph::NodeIt>::const_iterator j = i->second.begin(); j != i->second.end(); j++ )
				cout<<nodeNames[*j]<<" ";
			cout<<endl;
		}
		cout<<endl<<"***END LEVELS***"<<endl<<endl;
	}
}

/**
* Recursive function for verifying dependencies between function terms
*/
ListDigraph::NodeIt Analyzer::verify_dependencies(string s){
	ListDigraph::NodeIt nodeIt = getVertexByName(&graph,&nodeNames,s);
	if(nodeIt == INVALID){
		nodeNames[graph.addNode()] = s;
		nodeIt = getVertexByName(&graph,&nodeNames,s);
	}
	for(multimap<string,vector<string> >::const_iterator i = func.minterms.begin(); i != func.minterms.end(); ++i){
		//if vertex has dependencies
		if(i->first==s){
			for(vector<string>::const_iterator j = i->second.begin() ; j != i->second.end(); ++j){
				string name = *j;
				replace_substring(&name,"not_","");
				ListDigraph::NodeIt targetNode = verify_dependencies(name);
				bool already_there = false;
				for (ListDigraph::OutArcIt a(graph, nodeIt); a!=INVALID; ++a){
					if(graph.target(a) == targetNode){
						already_there = true;
						break;
					}
				}
				if(!already_there)
					graph.addArc(nodeIt,targetNode);
			}
		}
	}
	return nodeIt;
}

/**
 *	Recursive function for register the right subset for each term of the function
 * */
void Analyzer::build_levels(ListDigraph::Node u,ListDigraph::NodeMap<int>* levels,int lev){
	(*levels)[u] = max((*levels)[u],lev);
	int parentLev = lev+1;
	for (ListDigraph::InArcIt a(graph, u); a!=INVALID; ++a){
		build_levels(graph.source(a),levels,parentLev);
	}
}

/**
 * For each subset, generateCrossbar() creates an object Translator in order to generate
 * the corresponding memristor crossbar.
 * Since Translator is an extension of Analyzer class, it can manage boolean functions.
 * Then, since a subset of the input boolean function is still a boolean function,
 * each subset function is assigned to a Translator object
 * */
void Analyzer::generateCrossbar(){
	for(map <int, vector<ListDigraph::NodeIt> >::const_iterator i = nodeLevels.begin(); i != nodeLevels.end(); i++){

		if(i->first!=0){

			if(execParameters.verbose){
				cout<<"level: "<<i->first<<endl<<endl;
			}

			vector<string> inputs, outputs;
			multimap<string,vector<string> > minterms;
			for(vector<ListDigraph::NodeIt>::const_iterator j = i->second.begin(); j != i->second.end(); ++j){
				//build outputs
				outputs.push_back(nodeNames[*j]);

				//build inputs
				for(ListDigraph::OutArcIt a(graph, *j); a!=INVALID; ++a){
					if (std::find(inputs.begin(), inputs.end(), nodeNames[graph.target(a)]) == inputs.end()){
						inputs.push_back(nodeNames[graph.target(a)]);
						inputs.push_back("not_"+nodeNames[graph.target(a)]);
					}
				}

				//build minterms
				typedef multimap<string,vector<string> >::iterator mmit;
				std::pair <mmit, mmit> ret;
				ret = func.minterms.equal_range(nodeNames[*j]);
				minterms.insert(ret.first,ret.second);
			}
			Translator* tr;
			tr = new Translator(i->first,inputs,outputs,minterms);
			tr->func.countLiterals();
			tr->generateCrossbar();
			tr->generateVoltages();
			subAnalyzers.push_back(tr);

			if(execParameters.dot && execParameters.deepDot)
				tr->createDependenciesGraph(i->first);

			//#ifndef _DEBUG_
			//				//kill the child (otherwise it would re-execute the parent's code)
			//				exit(0);
			//			}
			//#endif _DEBUG_
		}
	}
}

/**
 * this procedure generates VHDL version of the whole circuit
 * */
void Analyzer::generateOutputVHDL(){
	for(vector<Analyzer*>::const_iterator i = this->subAnalyzers.begin() ; i != this->subAnalyzers.end(); ++i){
		(*i)->generateOutputVHDL();
	}
	generateStructuralOutputVHDL();
}

/**
 * this procedure generates VHDL structural file (all the crossbar connected togheter)
 * */
void Analyzer::generateStructuralOutputVHDL(){

	string entity = (*tokenize(*(tokenize(this->file,"/").end()-1),".").begin());

	std::ofstream out(string("./"+entity+".vhd").c_str());
	std::streambuf *coutbuf = std::cout.rdbuf(); //save old buf
	std::cout.rdbuf(out.rdbuf()); //redirect std::cout


	cout<<"----------------------------------------------------------------------------------\n"
			"-- Engineer: 		Marcello Traiola\n"
			"--\n"
			"-- Create Date: ";
	time_t t = time(0);   // get time now
	struct tm * now = localtime( & t );
	cout << (now->tm_hour) << ':'
			<< (now->tm_min) << ':'
			<< (now->tm_sec) << ' '
			<<(now->tm_mday) << '/'
			<< (now->tm_mon + 1) << '/'
			<< (now->tm_year + 1900)
			<< endl;
	cout<<
			"-- Design Name: \n"
			"-- Module Name: \n"
			"-- Project Name: \n"
			"-- Revision 1.0 - File Created\n"
			"-- Notes - File generated using XbarGen tool by Marcello Traiola (marcellotraiola@gmail.com)\n"
			"----------------------------------------------------------------------------------\n"
			"library IEEE;\n"
			"use IEEE.STD_LOGIC_1164.ALL;\n"
			"\n"
			"library memristor_lib;\n"
			"use memristor_lib.types.all;\n"
			"\n"
			"entity "<<VHDLsintaxFilter(entity.c_str())<<" is\n"
			"Port ( \n";
	for(vector<string>::const_iterator i = func.inputs.begin(); i!= func.inputs.end();i++){
		if((*i).find("not_") == std::string::npos)
			cout<<VHDLsintaxFilter(*i)<<" : in  STD_LOGIC;\n";
	}
	for(vector<string>::const_iterator i = func.outputs.begin(); i!= func.outputs.end();i++){
		if(i!=func.outputs.end()-1)
			cout<<VHDLsintaxFilter(*i)<<" : out  STD_LOGIC;\n";
		else
			cout<<VHDLsintaxFilter(*i)<<" : out  STD_LOGIC\n";
	}
	cout<<	");\n"
			"end "<<VHDLsintaxFilter(entity.c_str())<<";\n"
			"\n"
			"architecture Behavioral of "<<VHDLsintaxFilter(entity.c_str())<<" is\n"
			"\n";

	vector<string> tempWires;
	string instances;

	for(vector<Analyzer*>::const_iterator i = this->subAnalyzers.begin() ; i != this->subAnalyzers.end(); ++i){
		//declare each crossbar
		cout<<"COMPONENT crossbar_controller_"<<(*i)->level<<"\n"
				"PORT(\n";
		instances+="Inst_Crossbar_"+to_string((*i)->level)+" : crossbar_controller_"+to_string((*i)->level)+" PORT MAP(\n";

		for(vector<string>::const_iterator j = (*i)->func.inputs.begin(); j!= (*i)->func.inputs.end();j++){
			if((*j).find("not_") == std::string::npos){
				cout<<VHDLsintaxFilter(*j)<<" : in  STD_LOGIC;\n";
				instances+=VHDLsintaxFilter(*j)+" => "+VHDLsintaxFilter(*j)+"_temp,\n";
			}
		}
		cout<<"en : in STD_LOGIC;\n";
		instances+="en => done_temp_"+to_string(((*i)->level)-1)+",\n";
		for(vector<string>::const_iterator j = (*i)->func.outputs.begin(); j != (*i)->func.outputs.end();j++){
			cout<<VHDLsintaxFilter(*j)<<" : out  STD_LOGIC;\n";

			instances+=VHDLsintaxFilter(*j)+" => "+VHDLsintaxFilter(*j)+"_temp,\n";

			tempWires.push_back(VHDLsintaxFilter(*j)+"_temp");
		}
		instances+="done => done_temp_"+to_string((*i)->level)+"\n"+");\n\n";

		cout<<"done : out STD_LOGIC\n"
				");\n"

				"END COMPONENT;\n\n";
	}
	for(vector<string>::const_iterator j = func.inputs.begin(); j!= func.inputs.end();j++)
		if((*j).find("not_") == std::string::npos)
			cout<<"signal "<<VHDLsintaxFilter(*j)<<"_temp : STD_LOGIC;\n";

	for(vector<string>::const_iterator j = tempWires.begin(); j!= tempWires.end();j++)
		cout<<"signal "<<VHDLsintaxFilter(*j)<<" : STD_LOGIC;\n";

	for(int i = 0; i <= subAnalyzers.size();++i)
		cout<<"signal done_temp_"<<i<<" : STD_LOGIC;\n";

	cout<<
			"\n"
			"begin\n"
			"\n"<<instances;

	vector<string> sensitivityList;
	for(vector<string>::const_iterator i = func.inputs.begin(); i != func.inputs.end(); i++){
		if((*i).find("not_") == std::string::npos)
			sensitivityList.push_back((VHDLsintaxFilter(*i)));
	}
	for(vector<string>::const_iterator i=sensitivityList.begin(); i != sensitivityList.end(); ++i){
		cout<<*i+"_temp"<<" <= "+*i+";\n";
	}
	for(vector<string>::const_iterator i = func.outputs.begin(); i != func.outputs.end(); i++){
		if (std::find(tempWires.begin(), tempWires.end(), VHDLsintaxFilter(*i)+"_temp") != tempWires.end())
			cout<<VHDLsintaxFilter(*i)<<" <= "+VHDLsintaxFilter(*i)+"_temp;\n";
		else
			cout<<VHDLsintaxFilter(*i)<<" <= '"<<func.minterms.find(*i)->second.front()<<"';\n";

	}
	cout<<"\nprocess(";
	for(vector<string>::const_iterator i=sensitivityList.begin(); i != sensitivityList.end(); ++i){
		if(i!=sensitivityList.end()-1)
			cout<<*i<<",";
		else
			cout<<*i;
	}
	cout<<")\n"
			"begin\n"
			"done_temp_0<='1';\n"
			"done_temp_0<='0' after 4 ns;\n"
			"end process;\n"
			"\n"
			"end Behavioral;\n";

	std::cout.rdbuf(coutbuf); //reset to standard output again
}

/**
 * generates a file with all statistics
 * */
void Analyzer::printOutputStats(){
	string entity = (*tokenize(*(tokenize(this->file,"/").end()-1),".").begin());

	std::ofstream out(string("./"+entity+"_stat.txt").c_str());
	std::streambuf *coutbuf = std::cout.rdbuf(); //save old buf
	std::cout.rdbuf(out.rdbuf()); //redirect std::cout

	cout<<"Inputs: "<<func.getNumInput()/2<<endl;
	cout<<"Outputs: "<<func.getNumOutput()<<endl;
	cout<<"Minterms: "<<getNumOfMinterms()<<endl;
	cout<<"Number of memristors of the circuit: "<<getNumMemristor()<<endl;
	cout<<"Total area of the circuit: k^2 * "<<getArea()<<" (where k^2 = area of a crossbar's cell)"<<endl;
	cout<<"Number of steps (memristor switching) to complete computation: "<<getNumOfComputationSteps()<<endl;
	cout<<"Number of crossbars: "<<getNumOfStages()<<endl;

	int* powCons = getPowerConsumption();
	cout<<"Estimated power consumption (worst case): "<<powCons[0] <<" * (Cup+Cdown)"<<endl;
	cout<<"Estimated error (worst case): "<<powCons[2]<<" * (Cup+Cdown)"<<endl;
	cout<<"Estimated power consumption (best case): "<<powCons[1]<<" * (Cup+Cdown)"<<endl;
	cout<<"Estimated error (best case): "<<powCons[3]<<" * (Cup+Cdown)"<<endl;

	auto time= chrono::high_resolution_clock::now() - startTime;
	cout<<"XbarGen exec time: "<<std::chrono::duration<double, std::milli>(time).count()<<" ms";
	std::cout.rdbuf(coutbuf); //reset to standard output again
	if(execParameters.verbose)
		cout<<"XbarGen exec time: "<<std::chrono::duration<double, std::milli>(time).count()<<" ms"<<endl;
}

/**
 * retrieves number of memeristor of the whole circuit
 * */
int Analyzer::getNumMemristor(){
	int numMemristor=0;
	for(vector<Analyzer*>::const_iterator i = this->subAnalyzers.begin() ; i != this->subAnalyzers.end(); ++i){
		numMemristor+=(*i)->getNumMemristor();
	}
	return numMemristor;
}

/**
 * retrieves area of the whole circuit
 * */
int Analyzer::getArea(){
	int area=0;
	for(vector<Analyzer*>::const_iterator i = this->subAnalyzers.begin() ; i != this->subAnalyzers.end(); ++i){
		area+=(*i)->getArea();
	}
	return area;
}

/**
 * retrieves the number of crossbars in the circuit
 * */
int Analyzer::getNumOfStages(){
	return this->subAnalyzers.size();
}

/**
 * retrieves the number of 'steps' needed so that the whole circuit can execute (execution time)
 * */
int Analyzer::getNumOfComputationSteps(){
	return this->subAnalyzers.size() * numOfXbarStates;
}

/**
 * retrieves the number of minterms of the boolean function
 * */
int Analyzer::getNumOfMinterms(){
	int minterms=0;
	for(vector<Analyzer*>::const_iterator i = this->subAnalyzers.begin() ; i != this->subAnalyzers.end(); ++i){
		minterms+=(*i)->func.getNumMinterms_NoDuplicate();
	}
	return minterms;
}

/**
 * retrieves the power consumption bounds and estimated errors for the circuit
 * */
int* Analyzer::getPowerConsumption(){
	/*** ESTIMATING POWER CONSUMPTION ***/
	//for each crossbar
	int* powCons = new int[4];
	memset(powCons,0,4*sizeof(int));

	for(vector<Analyzer*>::const_iterator i = this->subAnalyzers.begin(); i != this->subAnalyzers.end();i++){

		//			cout<<"level: "<<(*i)->level<<endl;
		//			cout<<"input: "<<(*i)->func.getNumInput()/2<<endl;
		//			cout<<"output: "<<(*i)->func.getNumOutput()<<endl;

		//calculate the pow cons for operative memristors
		int* opMem = (*i)->getOperativeMemristorPowerConsumption();

		//calculate the pow cons for worst and best case
		powCons[0] += (*i)->func.getNumInput()/2 + (*i)->func.getNumOutput() + opMem[0];
		powCons[1] += (*i)->func.getNumInput()/2 + (*i)->func.getNumOutput() + opMem[1];

		//get the error for the worst case
		powCons[2] += opMem[2];
		//get the error for the best case
		powCons[3] += opMem[3];


	}
	return powCons;

}

/**
 * retrieves the power consumption estimation for memristors which are dependent on workload
 * (this function is implemented only in Translator class)
 * */
int* Analyzer::getOperativeMemristorPowerConsumption(){
	return NULL;
}


//	struct literal{
//		string name;
//		bool check;
//		int qta;
//	};
//
//	unordered_map<string,pair< literal*,literal*> > pointers;
//
//	//create 2 lists: input in negative and positive forms
//	multimap<int, literal* > orderedLiteralCount;
//	multimap<int, literal* > orderedLiteralCountNegate;
//
//	for(map<string,int>::const_iterator i = this->func.literalCount.begin(); i != this->func.literalCount.end();i++){
//
//		literal *l = new literal;
//		l->name=(*i).first;l->check=false;l->qta=(*i).second;
//
//		unordered_map<string,pair< literal*,literal*> >::iterator it;
//
//		if((*i).first.find("not", 0, 3)!=string::npos){
//			it = pointers.find(l->name.substr(4,string::npos ));
//			orderedLiteralCountNegate.insert(pair<int, literal*>((*i).second,l));
//			if(it == pointers.end())
//				pointers.insert(pair <string,pair< literal*,literal*> > ( l->name.substr(4,string::npos ),pair<literal*,literal*>(NULL,l)));
//			else
//				(*it).second.second = l;
//
//		}
//		else{
//			it = pointers.find(l->name);
//			orderedLiteralCount.insert(pair<int, literal*>((*i).second,l));
//			if(it == pointers.end())
//				pointers.insert(pair <string,pair< literal*,literal*> > ( l->name,pair<literal*,literal*>(l,NULL)));
//			else
//				(*it).second.first = l;
//		}
//	}
//	/*now we have 2 lists with the number of occurrence of each literal in the crossbar
//	 * we have to select the input vector that make switch the biggest(smallest) number of memristors */
//
//	/*NmNandMax is the max number of NAND memristors that could be switched at the same time using a specific input vector*/
//	int NmNandMax = 0;
//	vector<string> notcheck;
//	for(multimap<int, literal* >::reverse_iterator i = orderedLiteralCount.rbegin(); i != orderedLiteralCount.rend(); i++){
//		if(this->func.literalCount["not_"+(*i).second->name] <= (*i).first){
//			NmNandMax += (*i).first;
//			(*i).second->check = true;
//			notcheck.push_back((*i).second->name);
//		}
//		else{
//			NmNandMax += this->func.literalCount["not_"+(*i).first];
//			(*pointers.find((*i).second->name)).second.second->check = true;
//			notcheck.push_back((*pointers.find((*i).second->name)).second.second->name);
//		}
//	}
//
//	int NmAnd=0;
//	for(multimap<string,vector<string> >::const_iterator i = this->func.minterms.begin(); i != this->func.minterms.end();i++){
//		bool found = true;
//		for(vector<string>::const_iterator j = (*i).second.begin(); j != (*i).second.end();j++){
//			if(find (notcheck.begin(), notcheck.end(), *j) != notcheck.end()){
//				found = false;
//				break;
//			}
//		}
//		if(found)
//			NmAnd++;
//	}
//
//	return NmNandMax + NmAnd;

//	/*now it's possible that, even if NmNandMax is the max number of NAND memristors that could be switched
//	 *  at the same time, a particular configuration could make switch the AND memristor in a way that the sum
//	 *  of the NAND memristors and the AND ones is bigger.
//	 *  So now it's time to check that fact*/
//
//	int NmAnd = 0, NmNand=NmNandMax, NmNandtemp;
//
//	//I won't go under NmNandMax
//	bool change = true;
//	while(NmNand + NmAnd >= NmNandMax){
//		NmNandtemp = NmNand;
//		NmNand = 0;
//		literal* tmp = NULL;
//		if(change){
//			bool done = false;
//			for(multimap<int, literal* >::reverse_iterator i = orderedLiteralCount.rbegin(); i != orderedLiteralCount.rend(); i++){
//				if(!((*i).second->check) && !done){
//					if(tmp){
//						tmp->check = false;
//						unordered_map<string,pair< literal*,literal*> >::iterator it = pointers.find(tmp->name);
//						if((*it).second.first != tmp)
//							(*it).second.first->check = true;
//						else
//							(*it).second.second->check = true;
//					}else{
//
//					}
//					(*i).second->check = true;
//					unordered_map<string,pair< literal*,literal*> >::iterator it = pointers.find(tmp->name);
//					if((*it).second.first != (*i).second)
//						(*it).second.first->check = true;
//					else
//						(*it).second.second->check = true;
//					done = true;
//				}
//				else{
//					if(!done)
//						NmNand += (*i).second->qta;
//					else{
//						unordered_map<string,pair< literal*,literal*> >::iterator it = pointers.find((*i).second->name);
//						if((*it).second.first != (*i).second)
//							NmNand += (*it).second.first->qta;
//						else
//							NmNand += (*it).second.second->qta;
//					}
//					tmp = (*i).second;
//				}
//			}
//			if(NmNand == NmNandtemp)
//				change = false;
//		}
//		else{
//			bool done = false;
//			for(multimap<int, literal* >::reverse_iterator i = orderedLiteralCountNegate.rbegin(); i != orderedLiteralCountNegate.rend(); i++){
//				if(!((*i).second->check) && !done){
//					if(tmp){
//						tmp->check = false;
//						unordered_map<string,pair< literal*,literal*> >::iterator it = pointers.find(tmp->name);
//						if((*it).second.first != tmp)
//							(*it).second.first->check = true;
//						else
//							(*it).second.second->check = true;
//					}
//					(*i).second->check = true;
//					unordered_map<string,pair< literal*,literal*> >::iterator it = pointers.find(tmp->name);
//					if((*it).second.first != (*i).second)
//						(*it).second.first->check = true;
//					else
//						(*it).second.second->check = true;
//					done = true;
//				}
//				else{
//					if(!done)
//						NmNand += (*i).second->qta;
//					else{
//						unordered_map<string,pair< literal*,literal*> >::iterator it = pointers.find((*i).second->name);
//						if((*it).second.first != (*i).second)
//							NmNand += (*it).second.first->qta;
//						else
//							NmNand += (*it).second.second->qta;
//					}
//					tmp = (*i).second;
//				}
//			}
//			if(NmNand == NmNandtemp)
//				change = true;
//		}
//
//	}
