/* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/*
Authors:
Michael Berg <michael.berg@zalf.de>

Maintainers:
Currently maintained by the authors.

This file is part of the util library used by models created at the Institute of
Landscape Systems Analysis at the ZALF.
Copyright (C) Leibniz Centre for Agricultural Landscape Research (ZALF)
*/

#include <fstream>
#include <cstdlib>
#include <iostream>

#include "values-file-loader.h"

using namespace Models;
using namespace std;

ValuesFileParameterMap::ValuesFileParameterMap(const string& pathToValuesFile)
: _pathToValuesFile(pathToValuesFile) {
	parseValuesFile();
}

void ValuesFileParameterMap::parseValuesFile(){
	ifstream ifs(pathToValuesFile().c_str(), ifstream::in);

	char line[1024];

	while (ifs.good()) {
		ifs.getline(line, 1024);
		parseAndInsert(string(line));
	}

	ifs.close();
}

void ValuesFileParameterMap::parseAndInsert(const string& line) {
	//if commented out, skip line
	if(line.find("//") == 0) return;

	size_t sepPos = line.find_first_of("=");
	if (sepPos == string::npos)
		return;
	string typeAndName = trim(line.substr(0, sepPos));
	size_t typeNameSepIndex = typeAndName.find_last_of(" *&");
	if (typeNameSepIndex == string::npos)
		return;
	string type = trim(typeAndName.substr(0, typeNameSepIndex + 1));
	string name = typeAndName.substr(typeNameSepIndex + 1);

	string valueAndRest = line.substr(sepPos + 1);
	string value = trim(trim(valueAndRest.
	                         substr(0, valueAndRest.find("//"))), "\"");

	//cout << "type: |" << type << "| name: |" << name << "| value: |" << value << "|" << endl;

	insert(make_pair(name, value));
}

string ValuesFileParameterMap::trim(const string& s, const string& whitespaces) {
	string str(s);
	size_t found;
	found = str.find_last_not_of(whitespaces);
	if (found != string::npos)
		str.erase(found + 1);
	found = str.find_first_not_of(whitespaces);
	if (found != string::npos)
		str.erase(0, found);
	return str;
}

/*
string ValuesFileParameterMap::value(const string& key,
                                     const string& def) const {
	map<string, string>::const_iterator it = find(key);
	return it == end() ? def : it->second;
}
*/

/*
//! Return value as integer.
int ValuesFileParameterMap::valueAsInt(const string& key, int def) const {
	MapType::const_iterator ci = find(key);
	return ci == end ? def : std::atoi(ci->second.c_str());
}

//! Return value as double.
double ValuesFileParameterMap::valueAsDouble(const string& key,
                                             double def) const {
	MapType::const_iterator ci = find(key);
	return ci == end() ? def : std::atof(ci->second.c_str());
}

//! Return value as bool.
bool ValuesFileParameterMap::valueAsBool(const string& key, bool def) const {
	MapType::const_iterator ci = find(key);
	return value(key) == "true";
}
*/
