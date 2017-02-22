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
 * Translator.cpp
 *
 *  Created on: 18/apr/2016
 *      Author: Marcello Traiola
 */

#include "control.h"

/**
 * This function creates both column and row indexes (in the Crossbar class) that are links
 * between boolean function elements (e.g. minterm, input, output)
 * and row/column nanowires within the crossbar
 * */
void Translator::create_index(){
	//create indexes for rows
	int j=0;
	for(vector<string>::const_iterator i = func.inputs.begin(); i != func.inputs.end(); i++){
		this->xbar->columnIndex.insert(make_pair(*i,j));
		j++;
	}
	for(vector<string>::const_iterator i = func.outputs.begin(); i != func.outputs.end(); i++){
		this->xbar->columnIndex.insert(make_pair(*i,j));
		j++;
		this->xbar->columnIndex.insert(make_pair(("not_"+*i),j));
		j++;
	}
	//create indexes for columns
	j=0;
	this->xbar->rowIndex.insert(make_pair("IL",j));
	j++;
	for(multimap<string,vector<string> >::const_iterator i = func.minterms.begin(); i != func.minterms.end(); ++i) {
		string v;
		for(vector<string>::const_iterator k = i->second.begin(); k != i->second.end(); k++) {
			if(k==i->second.end()-1)
				v +=*k;
			else
				v +=*k+"*";
		}
		if(this->xbar->rowIndex.count(v)==0){
			this->xbar->rowIndex.insert(make_pair(v,j));
			j++;
		}
	}
	for(vector<string>::const_iterator i = func.outputs.begin(); i != func.outputs.end(); i++){
		this->xbar->rowIndex.insert(make_pair(("not_"+*i),j));
		j++;
	}

	if(execParameters.verbose){
		//print indexes
		cout<<"***CROSSBAR INDEXES***"<<endl<<endl;

		cout<<"***ROWS***"<<endl<<endl;
		for(map<string, int>::const_iterator i = this->xbar->rowIndex.begin(); i != this->xbar->rowIndex.end(); i++)
		{
			cout<<i->first<<"->"<<i->second<<endl;
		}
		cout<<endl<<"***COLUMNS***"<<endl<<endl;
		for(map<string, int>::const_iterator i = this->xbar->columnIndex.begin(); i != this->xbar->columnIndex.end(); i++)
		{
			cout<<i->first<<"->"<<i->second<<endl;
		}

		cout<<endl<<"***END CROSSBAR INDEXES***"<<endl<<endl;
	}
}

/**
 * Starting from subset boolean function, generateCrossbar() creates an object Crossbar that
 * represent a model of the actual memristor crossbar implementing the sub-function
 * */
void Translator::generateCrossbar(){
	this->xbar = new Crossbar(this->func.getNumInput(),this->func.getNumOutput(), this->func.getNumMinterms_NoDuplicate());
	create_index();
	//generate IL (row 0)
	for(int i=0; i<this->func.getNumInput();i++){
		(this->xbar->matrix)[this->xbar->rowIndex.find("IL")->second][i] = 1;
	}

	//generate minterm rows
	for(multimap<string,vector<string> >::const_iterator i = this->func.minterms.begin(); i != this->func.minterms.end(); ++i) {
		//output column name
		string out = "not_"+i->first;

		//row name
		string row;
		for(vector<string>::const_iterator k = i->second.begin(); k != i->second.end(); k++) {
			if(k==i->second.end()-1)
				row +=*k;
			else
				row +=*k+"*";
		}
		//row num
		int rowNum = this->xbar->rowIndex.find(row)->second;

		//put memristor in (row,out)
		(this->xbar->matrix)[rowNum][this->xbar->columnIndex.find(out)->second] = 1;


		//put memristor in (row,inputs)
		for(vector<string>::const_iterator k = i->second.begin(); k != i->second.end(); k++) {
			(this->xbar->matrix)[rowNum][this->xbar->columnIndex.find(*k)->second] = 1;
		}
	}

	//generate outputs rows
	int i=2;
	for(string o : this->func.outputs){
		string o_mod = "not_"+o;
		(this->xbar->matrix)[this->xbar->rowIndex.find(o_mod)->second][this->xbar->columnIndex.find(o_mod)->second] = 1;
		(this->xbar->matrix)[this->xbar->rowIndex.find(o_mod)->second][this->xbar->columnIndex.find(o)->second] = i++;
	}

	if(execParameters.verbose){
		cout<<"***CROSSBAR***"<<endl<<endl;
		this->xbar->printMatrix();
		cout<<endl<<"***END CROSSBAR***"<<endl<<endl;
	}
}

/**
 * For the object Crossbar, compute, for each state of the FBLC FSM, each nano-wire voltage
 * */
void Translator::generateVoltages(){

	//****generate stage INA (reset)****
	map<string,string> INA;

	//horizontal
	for(int i=0;i<this->xbar->getHeight();i++)
		INA.insert(make_pair("XbG_H"+to_string(i),"Vw"));

	//vertical
	for(int i=0;i<this->xbar->getWidth();i++)
		INA.insert(make_pair("XbG_V"+to_string(i),"zero"));

	this->xbar->voltages.insert(make_pair("A_INA",INA));


	//****generate stage RI (SFC)****
	map<string,string> RI;

	//horizontal
	RI.insert(make_pair("XbG_H0","zero"));
	for(int i=1;i<this->xbar->getHeight();i++)
		RI.insert(make_pair("XbG_H"+to_string(i),"Vr"));

	//vertical
	for(vector<string>::const_iterator i = this->func.inputs.begin(); i!= this->func.inputs.end(); i++)
		RI.insert(make_pair("XbG_V"+to_string(this->xbar->columnIndex.find(*i)->second),"Z"));
	for(vector<string>::const_iterator i = this->func.outputs.begin(); i!= this->func.outputs.end(); i++){
		RI.insert(make_pair("XbG_V"+to_string(this->xbar->columnIndex.find(*i)->second),"Vr"));
		RI.insert(make_pair("XbG_V"+to_string(this->xbar->columnIndex.find("not_"+(*i))->second),"Vr"));
	}

	this->xbar->voltages.insert(make_pair("B_RI",RI));


	//****generate stage CFM (MFC)****
	map<string,string> CFM;

	//horizontal
	CFM.insert(make_pair("XbG_H0","Vw"));
	for(multimap<string,vector<string> >::const_iterator i = this->func.minterms.begin(); i!= this->func.minterms.end(); i++){
		string toFind;
		for(vector<string>::const_iterator j = i->second.begin(); j != i->second.end(); ++j) {
			if(j+1 != i->second.end())
				toFind+=*j+"*";
			else
				toFind+=*j;
		}
		CFM.insert(make_pair("XbG_H"+to_string(this->xbar->rowIndex.find(toFind)->second),"zero"));
	}
	for(vector<string>::const_iterator i = this->func.outputs.begin(); i!= this->func.outputs.end(); i++){
		CFM.insert(make_pair("XbG_H"+to_string(this->xbar->rowIndex.find("not_"+(*i))->second),"Vr"));
	}

	//vertical
	for(vector<string>::const_iterator i = this->func.inputs.begin(); i!= this->func.inputs.end(); i++)
		CFM.insert(make_pair("XbG_V"+to_string(this->xbar->columnIndex.find(*i)->second),"Z"));
	for(vector<string>::const_iterator i = this->func.outputs.begin(); i!= this->func.outputs.end(); i++){
		CFM.insert(make_pair("XbG_V"+to_string(this->xbar->columnIndex.find(*i)->second),"Vr"));
		CFM.insert(make_pair("XbG_V"+to_string(this->xbar->columnIndex.find("not_"+(*i))->second),"Vr"));
	}

	this->xbar->voltages.insert(make_pair("C_CFM",CFM));


	//****generate stage EVM (SFNAND)****
	map<string,string> EVM;

	//horizontal
	EVM.insert(make_pair("XbG_H0","Vr"));
	for(multimap<string,vector<string> >::const_iterator i = this->func.minterms.begin(); i!= this->func.minterms.end(); i++){
		string toFind;
		for(vector<string>::const_iterator j = i->second.begin(); j != i->second.end(); ++j) {
			if(j+1 != i->second.end())
				toFind+=*j+"*";
			else
				toFind+=*j;
		}
		EVM.insert(make_pair("XbG_H"+to_string(this->xbar->rowIndex.find(toFind)->second),"Z"));
	}
	for(vector<string>::const_iterator i = this->func.outputs.begin(); i!= this->func.outputs.end(); i++){
		EVM.insert(make_pair("XbG_H"+to_string(this->xbar->rowIndex.find("not_"+(*i))->second),"Vr"));
	}

	//vertical
	for(vector<string>::const_iterator i = this->func.inputs.begin(); i!= this->func.inputs.end(); i++)
		EVM.insert(make_pair("XbG_V"+to_string(this->xbar->columnIndex.find(*i)->second),"Vr"));
	for(vector<string>::const_iterator i = this->func.outputs.begin(); i!= this->func.outputs.end(); i++){
		EVM.insert(make_pair("XbG_V"+to_string(this->xbar->columnIndex.find(*i)->second),"Vr"));
		EVM.insert(make_pair("XbG_V"+to_string(this->xbar->columnIndex.find("not_"+(*i))->second),"Vw"));
	}

	this->xbar->voltages.insert(make_pair("D_EVM",EVM));


	//****generate stage EVR (AND)****
	map<string,string> EVR;

	//horizontal
	EVR.insert(make_pair("XbG_H0","Vr"));
	for(multimap<string,vector<string> >::const_iterator i = this->func.minterms.begin(); i!= this->func.minterms.end(); i++){
		string toFind;
		for(vector<string>::const_iterator j = i->second.begin(); j != i->second.end(); ++j) {
			if(j+1 != i->second.end())
				toFind+=*j+"*";
			else
				toFind+=*j;
		}
		EVR.insert(make_pair("XbG_H"+to_string(this->xbar->rowIndex.find(toFind)->second),"Vw"));
	}
	for(vector<string>::const_iterator i = this->func.outputs.begin(); i!= this->func.outputs.end(); i++){
		EVR.insert(make_pair("XbG_H"+to_string(this->xbar->rowIndex.find("not_"+(*i))->second),"zero"));
	}

	//vertical
	for(vector<string>::const_iterator i = this->func.inputs.begin(); i!= this->func.inputs.end(); i++)
		EVR.insert(make_pair("XbG_V"+to_string(this->xbar->columnIndex.find(*i)->second),"Vr"));
	for(vector<string>::const_iterator i = this->func.outputs.begin(); i!= this->func.outputs.end(); i++){
		EVR.insert(make_pair("XbG_V"+to_string(this->xbar->columnIndex.find(*i)->second),"Vr"));
		EVR.insert(make_pair("XbG_V"+to_string(this->xbar->columnIndex.find("not_"+(*i))->second),"Z"));
	}

	this->xbar->voltages.insert(make_pair("E_EVR",EVR));


	//****generate stage INR (INV)****
	map<string,string> INR;

	//horizontal
	INR.insert(make_pair("XbG_H0","Vr"));
	for(multimap<string,vector<string> >::const_iterator i = this->func.minterms.begin(); i!= this->func.minterms.end(); i++){
		string toFind;
		for(vector<string>::const_iterator j = i->second.begin(); j != i->second.end(); ++j) {
			if(j+1 != i->second.end())
				toFind+=*j+"*";
			else
				toFind+=*j;
		}
		INR.insert(make_pair("XbG_H"+to_string(this->xbar->rowIndex.find(toFind)->second),"Vr"));
	}
	for(vector<string>::const_iterator i = this->func.outputs.begin(); i!= this->func.outputs.end(); i++){
		INR.insert(make_pair("XbG_H"+to_string(this->xbar->rowIndex.find("not_"+(*i))->second),"Z"));
	}

	//vertical
	for(vector<string>::const_iterator i = this->func.inputs.begin(); i!= this->func.inputs.end(); i++)
		INR.insert(make_pair("XbG_V"+to_string(this->xbar->columnIndex.find(*i)->second),"Vr"));
	for(vector<string>::const_iterator i = this->func.outputs.begin(); i!= this->func.outputs.end(); i++){
		INR.insert(make_pair("XbG_V"+to_string(this->xbar->columnIndex.find(*i)->second),"Vw"));
		INR.insert(make_pair("XbG_V"+to_string(this->xbar->columnIndex.find("not_"+(*i))->second),"Vr"));
	}

	this->xbar->voltages.insert(make_pair("F_INR",INR));

	if(execParameters.verbose){
		cout<<"***VOLTAGES***"<<endl<<endl;
		this->xbar->printVoltages();
		cout<<endl<<"***END VOLTAGES***"<<endl<<endl;
	}
}

/**
 * this procedure generates VHDL version of the assigned Crossbar invoking the corresponding
 * function on the managed Crossbar object
 * */
void Translator::generateOutputVHDL(){
	this->xbar->generateVHDLfiles(level,func.inputs,func.outputs);
}

/**
 * retrieves the number of minterms of the assigned sub-function
 * */
int Translator::getNumMemristor(){
	int n=0;
	for(vector< vector<int> >::const_iterator i = (this->xbar->matrix).begin(); i!= (this->xbar->matrix).end(); i++){
		for(vector<int>::const_iterator j = i->begin(); j!= i->end(); j++){
			if(*j!=0)
				n++;
		}
	}
	return n;
}

/**
 * retrieves the area of the assigned Crossbar
 * */
int Translator::getArea(){
	return this->xbar->getHeight()*this->xbar->getWidth();
}

/**
 * retrieves the power consumption estimation for memristors which are dependent on workload
 * */
int* Translator::getOperativeMemristorPowerConsumption(){

	struct literal{
		string name;
		bool check;
		int qta;
	};

	unordered_map<string,pair< literal*,literal*> > pointers;

	//create 2 lists: input in negative and positive forms -- UNUSED
	multimap<int, literal* > orderedLiteralCount;
	multimap<int, literal* > orderedLiteralCountNegate;

	for(map<string,int>::const_iterator i = this->func.literalCount.begin(); i != this->func.literalCount.end();i++){

		literal *l = new literal;
		l->name=(*i).first;l->check=false;l->qta=(*i).second;

		unordered_map<string,pair< literal*,literal*> >::iterator it;

		if((*i).first.find("not", 0, 3)!=string::npos){
			it = pointers.find(l->name.substr(4,string::npos ));
			orderedLiteralCountNegate.insert(pair<int, literal*>((*i).second,l));
			if(it == pointers.end())
				pointers.insert(pair <string,pair< literal*,literal*> > ( l->name.substr(4,string::npos ),pair<literal*,literal*>(NULL,l)));
			else
				(*it).second.second = l;
		}
		else{
			it = pointers.find(l->name);
			orderedLiteralCount.insert(pair<int, literal*>((*i).second,l));
			if(it == pointers.end())
				pointers.insert(pair <string,pair< literal*,literal*> > ( l->name,pair<literal*,literal*>(l,NULL)));
			else
				(*it).second.first = l;
		}
	}
	/*now we have 2 lists with the number of occurrence of each literal in the crossbar
	 * we have to select the input vector that make switch the biggest(smallest) number of memristors -- UNUSED*/

	/*NmNandMax is the max number of NAND memristors that could be switched at the same time using a specific input vector*/
	/*NmNandMin is the min number of NAND memristors that could be switched at the same time using a specific input vector*/
	int NmNandMax = 0;
	int NmNandMin = 0;
	set<string> notchecked;
	set<string> checked;
	for(vector<string>::const_iterator i = this->func.inputs.begin(); i != this->func.inputs.end(); i++){

		string lpos, lneg;
		//found a negation of a literal
		if((*i).find("not", 0, 3)!=string::npos){
			//create its 'positive'
			lpos = (*i).substr(4,string::npos);
			lneg = (*i);
		}
		else{
			lpos = (*i);
			lneg = "not_" + (*i);
		}
		//if the occurrences of the negative form are more than the ones of the positive, take it
		if(this->func.literalCount[lneg] > this->func.literalCount[lpos]){
			pair<set<string>::iterator,bool> result = notchecked.insert(lpos);
			checked.insert(lneg);
			if(result.second){
				NmNandMax += this->func.literalCount[lneg];
				NmNandMin += this->func.literalCount[lpos];
				if(pointers[lneg].second)
					pointers[lneg].second->check = true;
			}
		}
		//otherwise, take the positive one
		else{
			pair<set<string>::iterator,bool> result = notchecked.insert(lneg);
			checked.insert(lpos);
			if(result.second){
				NmNandMax += this->func.literalCount[lpos];
				NmNandMin += this->func.literalCount[lneg];
				if(pointers[lpos].second)
					pointers[lpos].second->check = true;
			}
		}
	}

	/*since a 'selected' inputs are set as '0' and since the NAND operation gives '0' only if takes all 1s as input
	 * we stored unselected inputs (set as '1') in order to check if there is any minterm that contains all of them.
	 * if some minterms satisfy these conditions, they give 0 as output (so they're considered as 'commutations' -> powerCons)
	 * those who are
	 * */
	int NmAndWorst = 0, NmAndWorstError = 0, NmAndBest = 0;
	for(multimap<string,vector<string> >::const_iterator i = this->func.minterms.begin(); i != this->func.minterms.end();i++){
		bool foundWorst = true;
		bool foundBest = true;
		for(vector<string>::const_iterator j = (*i).second.begin(); j != (*i).second.end();j++){
			if(find(notchecked.begin(), notchecked.end(), *j) == notchecked.end() && foundWorst == true){
				foundWorst = false;
				NmAndWorstError++;
			}
			if(find(checked.begin(), checked.end(), *j) == checked.end() && foundBest == true)
				foundBest = false;

			if(!foundWorst && !foundBest)
				break;
		}
		if(foundWorst)
			NmAndWorst++;
		if(foundBest)
			NmAndBest++;
	}

	int* toRet = new int[4];
	toRet[0] = NmNandMax + NmAndWorst;
	toRet[1] = NmNandMin + NmAndBest;
	toRet[2] = NmAndWorstError;
	toRet[3] = NmAndBest;
	return toRet;
}
