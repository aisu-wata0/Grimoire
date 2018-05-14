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

void clearln(std::istream& in){
	in.clear();
	in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

template<class type>
void read(std::istream& in, type& value,
	std::ostream& out = NULL, std::string error_msg = "")
{
	while (not(in >> value)) {
		out << error_msg;
		clearln(in);
	}
}

template<class type>
void readln(std::istream& in, type& value,
	std::ostream& out = NULL, std::string error_msg = "")
{
	read(in, value, out, error_msg);
	clearln(in);
}

template<class type>
bool inRange(type value, type min, type max){
	return ((min < value) && (value < max));
}

template<class type>
void readInRange(std::istream& in, type& value, type min, type max,
	std::ostream& out = NULL,
	std::string err_range_msg = "", std::string err_valid_msg = "")
{
	bool valid = false;
	while(!valid){
		read(in, value, out, err_valid_msg);
		if(inRange(value, min, max))
			valid = true;
		else out << err_range_msg << std::endl;
	}
}

void readline(std::istream& in, std::string& response){
	while (getline(in, response) && response.empty());
}

void printAscii(std::string filename){
	std::ifstream inFile(filename);
	std::string buffer;
	while(std::getline(inFile, buffer)){
		std::cout << buffer << std::endl;
	}
}


}
