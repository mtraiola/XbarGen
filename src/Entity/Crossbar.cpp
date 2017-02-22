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
 * Crossbar.cpp
 *
 *  Created on: 19/mar/2016
 *      Author: Marcello Traiola
 */

#include "entities.h"
#include <iostream>
#include <cstdlib>
#include <fstream>
#include <array>
using namespace std;

/**
 * Constructor with parameters. Initializes the memristor matrix with 0 values
 * */
Crossbar::Crossbar(int numInput, int numOutput, int numMinterms) : numMemristors(0){
	for(int i=0; i<(1+numMinterms+numOutput);i++){
		vector<int> v;
		for(int j=0; j<(numInput+(numOutput*2));j++){
			v.push_back(0);
		}
		this->matrix.push_back(v);
	}
}

/**
 * Prints out the matrix through the std output
 * */
void Crossbar::printMatrix(){
	for(vector< vector<int> >::iterator i = this->matrix.begin(); i != this->matrix.end(); i++){
		for(vector<int>::iterator j = i->begin(); j != i->end(); j++){
			cout<<*j<<"\t";
		}
		cout<<endl;
	}
}

/**
 * Prints out the voltages for each nanowire, for each stage
 * */
void Crossbar::printVoltages(){
	for(map< string, map<string,string> >::iterator i = this->voltages.begin(); i != this->voltages.end(); i++){
		cout<<"Stage "<<i->first<<":"<<endl;
		for(map<string,string>::iterator j = i->second.begin(); j != i->second.end(); j++){
			cout<<j->first<<"="<<j->second<<endl;
		}
		cout<<endl;
	}
}

/**
 * This procedure generates VHDL version of the whole sub-Crossbar
 * */
void Crossbar::generateVHDLfiles(int level,vector<string> inputs, vector<string> outputs){
	generateCrossbarStructureFile(level);
	generateCrossbarFile(level,outputs.size());
	generateCrossbarControllerFile(level,inputs,outputs);
}

/**
 * This procedure generates the Crossbar's implementation VHDL file
 * */
void Crossbar::generateCrossbarFile(int level,int outSize){
	std::ofstream out(string("./crossbar_"+to_string(level)+".vhd").c_str());
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
			"use ieee.numeric_std.all;\n"
			"\n"
			"library memristor_lib;\n"
			"use memristor_lib.types.all;\n"
			"\n"
			"use work.crossbar_structure_"<<string(to_string(level).c_str())<<".all;\n"
			"\n"
			"entity "<<string("crossbar_"+to_string(level)).c_str()<<" is\n"
			"Port (\n"
			"Vpos : in  voltage_vector (0 to cb_width-1);\n"
			"Vneg : in  voltage_vector (0 to cb_height-1);\n"
			//			"output : out word\n"
			"output : out STD_LOGIC_VECTOR(0 to "<<outSize-1<<") --here we have "<<outSize<<" outputs\n"
			");\n"
			"end "<<string("crossbar_"+to_string(level)).c_str()<<";\n"
			"\n"
			"architecture Behavioral of "<<string("crossbar_"+to_string(level)).c_str()<<" is\n"
			"\n"
			"COMPONENT Memristor_behavioral_Snider_internal_clock\n"
			"PORT(\n"
			"Vpos : INOUT voltage;\n"
			"Vneg : INOUT voltage;\n"
			"Rout : OUT std_logic\n"
			");\n"
			"END COMPONENT;\n"
			"\n"
			"COMPONENT Memristor_behavioral_Snider_internal_clock_noOutput\n"
			"PORT(\n"
			"Vpos : INOUT voltage;\n"
			"Vneg : INOUT voltage\n"
			");\n"
			"END COMPONENT;\n"
			"\n"
			"signal verticalWires : voltage_vector (0 to cb_width-1);\n"
			"signal horizontalWires : voltage_vector (0 to cb_height-1);\n"
			"\n"
			"begin\n"
			"\n"
			"verticalWires <= Vpos;\n"
			"horizontalWires <= Vneg;\n"
			"righe : for i in cb_height-1 downto 0 generate\n"
			"\n"
			"colonne : for j in cb_width-1 downto 0 generate\n"
			"\n"
			"check: if (cb_structure(i)(j)=1) generate\n"
			"memristor_riga : Memristor_behavioral_Snider_internal_clock_noOutput PORT MAP(\n"
			"Vpos => verticalWires(j),\n"
			"Vneg => horizontalWires(i)\n"
			");\n"
			"end generate check;\n"
			"\n"
			"check_output: if (cb_structure(i)(j)>1) generate\n"
			"memristor_riga : Memristor_behavioral_Snider_internal_clock PORT MAP(\n"
			"Vpos => verticalWires(j),\n"
			"Vneg => horizontalWires(i),\n"
			"Rout => output(cb_structure(i)(j)-2)\n"
			");\n"
			"end generate check_output;\n"
			"\n"
			//			"set_high_imp: if (cb_structure(i)(j)='0') generate\n"
			//			"output(i)(j)<='Z';\n"
			//			"end generate set_high_imp;\n"
			//			"\n"
			"end generate colonne;\n"
			"\n"
			"end generate righe;\n"
			"\n"
			"end Behavioral;\n";
	std::cout.rdbuf(coutbuf); //reset to standard output again

}

/**
 * This procedure generates the Crossbar's structure VHDL file
 * */
void Crossbar::generateCrossbarStructureFile(int level){

	std::ofstream out(string("./crossbar_structure_"+to_string(level)+".vhd").c_str());
	std::streambuf *coutbuf = std::cout.rdbuf(); //save old buf
	std::cout.rdbuf(out.rdbuf()); //redirect std::cout

	cout<<"--\n"
			"--	Package File\n"
			"--\n"
			"--	Purpose: This package defines Crossbar constants and structure\n"
			"--\n"
			"--   This file is generated using XbarGen tool by Marcello Traiola (marcellotraiola@gmail.com)\n"
			"--\n"
			"\n"
			"library IEEE;\n"
			"use IEEE.STD_LOGIC_1164.all;\n"
			"\n"
			"package "<<string("crossbar_structure_"+to_string(level)).c_str()<<" is\n"
			"\n"
			"constant cb_height : integer := "<<this->getHeight()<<";\n"
			"constant cb_width : integer := "<<this->getWidth()<<";\n"
			"\n"
//			"type word is array(0 to cb_height-1) of std_logic_vector(0 to cb_width-1);\n"
			"type matrix_row is array (0 to cb_width-1) of integer\n;"
			"\n"
			"type matrix is array (0 to cb_height-1) of matrix_row;\n"
			"\n"
//			"type matrix is array (0 to cb_height-1) of std_logic_vector(0 to cb_width-1);\n"
//			"\n"
			"constant cb_structure : matrix := (\n";

	for(vector< vector<int> >::iterator i = this->matrix.begin(); i != this->matrix.end(); i++){
		cout<<"\t\t\t\t\t(";
		for(vector<int>::iterator j = i->begin(); j != i->end(); j++){
			if(j!=(i->end())-1)
				cout<<*j<<",";
			else
				cout<<*j;
		}
		if(i!=this->matrix.end()-1)
			cout<<"),"<<endl;
		else
			cout<<")"<<endl;
	}
	cout<<
			");\n"
			"end "<<string("crossbar_structure_"+to_string(level)).c_str()<<";";

	std::cout.rdbuf(coutbuf); //reset to standard output again
}

/**
 * This procedure generates the Crossbar's controller VHDL file (FSM)
 * */
void Crossbar::generateCrossbarControllerFile(int level,vector<string> inputs, vector<string> outputs){

	std::ofstream out(string("./controller_"+to_string(level)+".vhd").c_str());
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
			"use work."<<string("crossbar_structure_"+to_string(level)).c_str()<<".all;\n"
			"\n"
			"entity "<<string("crossbar_controller_"+to_string(level)).c_str()<<" is\n"
			"Port ( \n";
	for(vector<string>::const_iterator i = inputs.begin(); i!= inputs.end();i++){
		if((*i).find("not_") == std::string::npos)
			cout<<VHDLsintaxFilter(*i)<<" : in  STD_LOGIC;\n";
	}
	cout<<"en : in STD_LOGIC;\n";
	for(vector<string>::const_iterator i = outputs.begin(); i!= outputs.end();i++){
		cout<<VHDLsintaxFilter(*i)<<" : out  STD_LOGIC;\n";
	}
	cout<<"done : out STD_LOGIC\n"
			");\n"
			"end "<<string("crossbar_controller_"+to_string(level)).c_str()<<";\n"
			"\n"
			"architecture Behavioral of "<<string("crossbar_controller_"+to_string(level)).c_str()<<" is\n"
			"\n"
			"COMPONENT "<<string("crossbar_"+to_string(level)).c_str()<<"\n"
			"PORT(\n"
			"Vpos : IN voltage_vector(0 to "<<getWidth()-1<<");\n"
			"Vneg : IN voltage_vector(0 to "<<getHeight()-1<<");\n"
			//			"output : OUT word\n"
			"output : out STD_LOGIC_VECTOR(0 to "<<outputs.size()-1<<") --here we have "<<outputs.size()<<" outputs\n"
			");\n"
			"END COMPONENT;\n"
			"\n"
			"signal Vpos_temp : voltage_vector(0 to "<<getWidth()-1<<");\n"
			"signal Vneg_temp: voltage_vector(0 to "<<getHeight()-1<<");\n"
			//			"signal output_temp : word;\n"
			"signal output_temp : STD_LOGIC_VECTOR(0 to "<<outputs.size()-1<<");\n"
			"\n";
	for(int i=0; i<getWidth();i++){
		cout<<"	alias XbG_V"<<i<<" : voltage is Vpos_temp("<<i<<");\n";
	}
	for(int i=0; i<getHeight();i++){
		cout<<"	alias XbG_H"<<i<<" : voltage is Vneg_temp("<<i<<");\n";
	}
	int j=0;
	for(vector<string>::const_iterator i = outputs.begin(); i!= outputs.end();i++){
		//		cout<<"alias "<<*i<<"_tmp : std_logic is output_temp("<<rowIndex.find("not_"+*i)->second<<")("<<columnIndex.find(*i)->second<<");\n";
		cout<<"alias "<<VHDLsintaxFilter(*i)<<"_tmp : std_logic is output_temp("<<j++<<");\n";
	}

	cout<<"type FSMstate is (IDLE";
	for(map< string, map<string, string> >::const_iterator i = voltages.begin(); i != voltages.end(); i++){
		cout<<","<<i->first;
	}
	cout<<
			");\n"
			"\n"
			"signal state, next_state : FSMstate := IDLE;\n"
			"\n"
			"signal clk : std_logic := '0';\n"
			"\n"
			"constant clk_period : time := 1 ns;\n"
			"\n"
			"begin\n"
			"\n"
			"Inst_Crossbar : "<<string("crossbar_"+to_string(level)).c_str()<<" PORT MAP(\n"
			"Vpos => Vpos_temp,\n"
			"Vneg => Vneg_temp,\n"
			"output => output_temp\n"
			");\n"
			"\n";

	for(vector<string>::const_iterator i = outputs.begin(); i!= outputs.end();i++){
		cout<<VHDLsintaxFilter(*i)<<"<="<<VHDLsintaxFilter(*i)<<"_tmp;\n";
	}
	cout<<
			"\n"
			"-- Clock process definitions\n"
			"clk_process : process (clk)\n"
			"begin\n"
			"clk <= not(clk) after clk_period/2; --only behavioral simulation\n"
			"end process;\n"
			"\n"
			"change_state: process (clk)\n"
			"begin\n"
			"if(clk'event and clk='1') then\n"
			"state <= next_state;   --state change.\n"
			"end if;\n"
			"end process;\n"
			"\n"
			"FSM: process(state,";
	for(vector<string>::const_iterator i = inputs.begin(); i!= inputs.end();i++){
		if((*i).find("not_") == std::string::npos)
			cout<<VHDLsintaxFilter(*i)<<",";
	}
	cout<<
			"en)\n"
			"begin\n"
			"\n"
			"case state is\n"
			"\n"
			"when IDLE =>\n"
			"\n"
			"done<='0' after clk_period;\n"
			"\n"
			"if(en='1') then\n"
			"next_state<=A_INA;\n"
			"else\n"
			"next_state<=IDLE;\n"
			"end if;\n"
			"\n";

	for(map< string, map<string, string> >::const_iterator i = voltages.begin(); i != voltages.end();i++){
		if(i->first.find("A_") == string::npos){
			cout<<
					"\n"
					"next_state<="<<i->first<<";\n"
					"\n";
		}
		cout<<
				"when "<<i->first<<" =>\n"
				"\n";
		for(map<string, string>::const_iterator j = i->second.begin(); j != i->second.end();j++){
			cout<<voltageFilter(i->first,j->first)<<endl;
		}
	}


	cout<<
			"\n"
			"done<='1' after clk_period;\n"
			"\n"
			"next_state<=IDLE;\n"
			"\n"
			"end case;\n"
			"\n"
			"end process;\n"
			"\n"
			"end Behavioral;\n";

	std::cout.rdbuf(coutbuf); //reset to standard output again
}

/**
 * This function adapt "logic" voltages that we computed to VHDL compliant signals.
 * N.B. Actually, a "definition file" is provided with our memristor VHDL model
 * (e.g. here we use signals like Vr, Vw, etc that are actually translated to VHDL compliant
 * signals, within the definition file)
 * */
string Crossbar::voltageFilter(string stage, string voltage){
	map<string, string>::const_iterator tension = voltages.find(stage)->second.find(voltage);
	if(tension->second=="Z"){
		string isZ;
		/***TEMPORARY***
		 * If we are within the RI stage (ReceiveInputs), we assign the input values "manually"
		 * to memristors within "IL" input register
		 * even though they should come from the previous crossbar
		 */
		if(stage.find("RI") != string::npos){
			map<int, string> reverseIndexColumn;
			for(map<string,int>::const_iterator i = columnIndex.begin(); i!= columnIndex.end();i++){
				reverseIndexColumn.insert(make_pair(i->second,i->first));
			}
			if(reverseIndexColumn.find(stoi(voltage.substr(5,string::npos)))->second.find("not_") != string::npos)
				isZ+="if "+VHDLsintaxFilter((reverseIndexColumn.find(stoi(voltage.substr(5,string::npos)))->second).substr(4,string::npos))+"='0' then "+voltage+"<=Vw_neg; else "+voltage+"<=Vw; end if;\n";
			else
				isZ+="if "+VHDLsintaxFilter(reverseIndexColumn.find(stoi(voltage.substr(5,string::npos)))->second)+"='1' then "+voltage+"<=Vw_neg; else "+voltage+"<=Vw; end if;\n";
		}
		else if((--(voltages.find(stage)))->second.find(voltage)->second.find("Vw") != string::npos){
			isZ+=tension->first+"<=Vr;\n";
			isZ+=tension->first+"<=(others=>'Z') after 1 ps;";
		}
		else
			isZ+=tension->first+"<=(others=>'Z');";
		return isZ;
	}
	return tension->first+"<="+tension->second+";";
}
