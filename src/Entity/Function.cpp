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
 * Funzione.cpp
 *
 *  Created on: 19/mar/2016
 *      Author: Marcello Traiola
 */

#include "entities.h"
#include <iostream>
#include <cstring>
#include <set>

void Function::addInputs(vector<string> s){
	for(vector<string>::const_iterator i = s.begin(); i!= s.end(); i++){
		this->inputs.push_back(*i);
	}
}

void Function::addOutputs(vector<string> s){
	for(vector<string>::const_iterator i = s.begin(); i!= s.end(); i++){
		this->outputs.push_back(*i);
	}
}

void Function::printInput(){
	for(vector<string>::const_iterator i = this->inputs.begin(); i != this->inputs.end(); ++i) {
		cout << *i << " ";
	}
}

void Function::printOutput(){
	for(vector<string>::const_iterator i = this->outputs.begin(); i != this->outputs.end(); ++i) {
		cout << *i << " ";
	}
}

void Function::printFunction(){
	string prec("");
	for(multimap<string,vector<string> >::const_iterator i = this->minterms.begin(); i != this->minterms.end(); ++i) {
		if(i->first != prec)
			cout << endl <<i->first << "=(";
		else
			cout << "+(";
		prec = i->first;
		for(vector<string>::const_iterator j = i->second.begin(); j != i->second.end(); ++j) {
			if(j+1 != i->second.end())
				cout << *j << "*";
			else
				cout << *j<<")";
		}
	}
}

void Function::addMinterm(string out,string minterm){
	vector<string> v;
	char* pch = strtok ((char*)minterm.c_str()," ");
	while (pch != NULL)
	{
		v.push_back(string(pch));
		pch = strtok (NULL, " ");
	}
	this->minterms.insert(make_pair(out,v));
}

map<string, int> Function::countLiterals(){
	set<string> counter;

	for(multimap<string,vector<string> >::const_iterator i = this->minterms.begin(); i != this->minterms.end(); ++i) {
		string minterm;

		for(vector<string>::const_iterator j = i->second.begin(); j != i->second.end(); ++j) {
			if(j+1 != i->second.end())
				minterm+=*j+"*";
			else
				minterm+=*j;
		}

		pair<set<string>::iterator,bool> result = counter.insert(minterm);
		if(result.second){
			for(vector<string>::const_iterator j = i->second.begin(); j != i->second.end(); ++j) {
				map<string, int>::iterator it = this->literalCount.find(*j);
				if(it != this->literalCount.end())
					(*it).second++;
				else
					this->literalCount.insert(pair<string,int>(*j,1));
			}
		}

	}

	return this->literalCount;

}

int Function::getNumInput(){
	return this->inputs.size();
}

int Function::getNumOutput(){
	return this->outputs.size();
}

int Function::getNumMinterms(){
	return this->minterms.size();
}

int Function::getNumMinterms_NoDuplicate(){
	//a set can't contain duplicates
	set<string> counter;

	for(multimap<string,vector<string> >::const_iterator i = this->minterms.begin(); i != this->minterms.end(); ++i) {
		string minterm;

		for(vector<string>::const_iterator j = i->second.begin(); j != i->second.end(); ++j) {
			if(j+1 != i->second.end())
				minterm+=*j+"*";
			else
				minterm+=*j;
		}

		counter.insert(minterm);

	}

	return counter.size();
}

map<string, int> Function::getLiteralCount(){
	return this->literalCount;
}
