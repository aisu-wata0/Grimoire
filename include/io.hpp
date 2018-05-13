#pragma once

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <string.h>
#include <cstdint>
#include <ios>
#include <algorithm>

namespace gm
{
using namespace std;


void clearln(istream& in){
	in.clear();
	in.ignore(numeric_limits<streamsize>::max(), '\n');
}

template<class type>
void read(istream& in, type& value,
	ostream& out = NULL, string error_msg = "")
{
	while (not(in >> value)) {
		out << error_msg;
		clearln(in);
	}
}

template<class type>
void readln(istream& in, type& value,
	ostream& out = NULL, string error_msg = "")
{
	read(in, value, out, error_msg);
	clearln(in);
}

template<class type>
bool inRange(type value, type min, type max){
	return ((min < value) && (value < max));
}

template<class type>
void readInRange(istream& in, type& value, type min, type max,
	ostream& out = NULL,
	string err_range_msg = "", string err_valid_msg = "")
{
	bool valid = false;
	while(!valid){
		read(in, value, out, err_valid_msg);
		if(inRange(value, min, max))
			valid = true;
		else out << err_range_msg << endl;
	}
}

void readline(istream& in, string& response){
	while (getline(in, response) && response.empty());
}

void printAscii(string filename){
	ifstream inFile(filename);
	string buffer;
	while(getline(inFile, buffer)){
		cout << buffer << endl;
	}
}


}
