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
 * utils.cpp
 *
 *  Created on: 08/apr/2016
 *      Author: Marcello Traiola
 */
#include <my_utils.h>
#include <iostream>
using namespace std;

unordered_map<string,char> VHDL_Reserved_Words;
unordered_map<string,string> badStringMap;


/**
 * returns the trimmed string 'str' without modifying the original one
 */
string trim(string str)
{
	size_t first = str.find_first_not_of(' ');
	size_t last = str.find_last_not_of(' ');
	if(str.size()>0)
		return str.substr(first, (last-first+1));
	else
		return "";
}

/**
 * trims the original string 'str'
 */
void trim(string* str)
{
	size_t first = (*str).find_first_not_of(' ');
	size_t last = (*str).find_last_not_of(' ');
	if(str->size()>0)
		(*str) = (*str).substr(first, (last-first+1));
}

/**
 * returns a vector of strings where each element is a part of the input string 's' such that
 * each element is separated from the following one by the string 'delim'.
 */
vector<string> tokenize(string s,const char* delim){
	vector<string> v;
	int first=0,last=0;
	while(last!=string::npos){
		last = s.find(delim,first);
		string str = s.substr(first, last-first);
		if(str.size()>0)
			v.push_back(str);
		first = last+1;
	}
	return v;
}

/**
 * replaces with 'new_string' each occurrence of 'old_string' within the input string 's'
 */
void replace_substring(string* s, const string old_string, const string new_string){
	size_t index = 0;
	while (true) {

		index = (*s).find(old_string, index);
		if (index == std::string::npos) break;

		/* Make the replacement. */
		(*s).replace(index, old_string.length(), new_string);

		/* Advance index forward so the next iteration doesn't pick it up as well. */
		index+=old_string.length();
	}
}

/**
 * retrieves, from the set of Nodes 'n', the one named as string 's'
 */
ListDigraph::NodeIt getVertexByName(ListDigraph* g,ListDigraph::NodeMap<string> *n, string s){
	for(ListDigraph::NodeIt i(*g); i != INVALID; ++i){
		if((*n)[i] == s)
			return i;
	}
	return INVALID;
}

/**
 * generates a file .dot of the graph composed by the set of nodes 'nodeNames'
 */
void generateDOTfromGraph(int level, ListDigraph* g,ListDigraph::NodeMap<string>* nodeNames){
	typedef ListDigraph::ArcIt EdgeIt;
	typedef ListDigraph::NodeIt NodeIt;

	string file("./dependency_graph"+ (level!=-1? "_"+to_string(level):"") +".dot");
	std::ofstream out(file.c_str());
	std::streambuf *coutbuf = std::cout.rdbuf(); //save old buf
	std::cout.rdbuf(out.rdbuf()); //redirect std::cout

	cout << "digraph lemon_dot_example {" << endl;
	cout << "  node [ shape=ellipse, fontname=Helvetica, fontsize=10 ];" << endl;
	for(NodeIt n(*g); n!=INVALID; ++n) {
		string name = (*nodeNames)[n];
		replace_substring(&name,"not_","");
		cout << name << " [ label=\"" << name << "\" ]; " << endl;
	}
	cout << "  edge [ shape=ellipse, fontname=Helvetica, fontsize=10 ];" << endl;
	for(EdgeIt e(*g); e!=INVALID; ++e) {
		string sourceName = (*nodeNames)[(*g).source(e)], targetName = (*nodeNames)[(*g).target(e)];
		replace_substring(&sourceName,"not_","");
		replace_substring(&targetName,"not_","");
		cout <<sourceName << " -> " << targetName << " [ label=\"" << (*g).id(e) << "\" ]; " << endl;
	}
	cout << "}" << endl;
	std::cout.rdbuf(coutbuf); //reset to standard output again

	//automatically open Graphviz on MacOS
	//	chmod(file.c_str(),strtol("0655", 0, 8));
	//	if (fork()==0) {
	//		system(string("/Applications/Graphviz.app/Contents/MacOS/Graphviz "+file+" &").c_str());
	//		exit(0);
	//	}

}

/**
 * starting from a string 's', expected to be a VHDL name (e.g. instance of a signal), returns its 'clean' version
 * (not allowed characters and names are deleted or replaced)
 */
string VHDLsintaxFilter(string s){
	if(badStringMap.find(s) != badStringMap.end()){
		s = badStringMap.find(s)->second;
	}
	else{
		string initial = s;
		while(s.find("[")!= std::string::npos){
			s.replace(s.find("["),1,"_");
		}
		while(s.find("]")!= std::string::npos){
			s.replace(s.find("]"),1,"_");
		}
		while(s.find("<")!= std::string::npos){
			s.replace(s.find("<"),1,"_");
		}
		while(s.find(">")!= std::string::npos){
			s.replace(s.find(">"),1,"_");
		}
		while(s.find(".")!= std::string::npos){
			s.replace(s.find("."),1,"_");
		}
		while(s.find("__")!= std::string::npos){
			s.replace(s.find("__"),2,"_");
		}
		while(s.back()=='_' || s.front()=='_' || s.front()=='-'|| isdigit(s.front())){
			if(s.back()=='_'){
				s.erase(--s.end());
			}
			if(s.front()=='_'){
				s.erase(s.begin());
			}
			if(s.front()=='-'){
				s.erase(s.begin());
			}
			while(isdigit(s.front())){
				s.erase(s.begin());
			}
		}
		if(!VHDL_Reserved_Words.empty()){
			if(VHDL_Reserved_Words.find(s) != VHDL_Reserved_Words.end())
				s = "XbG_" + s;
		}

		if(s.empty()){
			//get a 7 random character string (lowercase)
			for (int i=0;i<7;i++)
				s += rand() % 25 + 97;
		}
		badStringMap.insert(make_pair(initial,s));
	}
	return s;
}

/**
 * load the VHDL 'reserved keywords' from file VHDLrsrvdWords.dat
 */
bool loadVHDLReservedWords(){
	string line;
	ifstream myfile("./VHDLrsrvdWords.dat");
	if (myfile.is_open())
	{
		while ( getline (myfile,line) )
		{
			VHDL_Reserved_Words.insert(make_pair(trim(line),NULL));
		}
		myfile.close();
		return true;
	}
	else cout << "Unable to open file";
	return false;
}

