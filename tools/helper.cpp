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

#include <cstdlib>
#include <iostream>
#include <string>
#include <fstream>

#include "helper.h"

using namespace Tools;
using namespace std;

bool Tools::stob(const std::string& s, bool def)
{
  if(s.empty())
    return def;

  auto start = toLower(s).at(0);

  switch(start)
  {
  case 't': case '1':
    return true;
  case 'f': case '0':
    return false;
  default:
    return def;
  }
}

string Tools::readFile(string path)
{
	string s;
	path = fixSystemSeparator(path);

	ifstream ifs;
	ifs.open(path);
	if(ifs.good())
	{
		for(string line; getline(ifs, line);)
			s += line;
	}
	ifs.close();

	return s;
}

pair<string, string> Tools::splitPathToFile(const string& pathToFile)
{
	size_t found = pathToFile.find_last_of("/\\");
	return make_pair(pathToFile.substr(0,found+1),
	                 pathToFile.substr(found+1));
}

bool Tools::isAbsolutePath(const std::string& path)
{
	size_t found = path.find_first_of("/\\");
	//unix absolute path
	if(found == 0 && path.at(0) == '/')
		return true;
	//absolute windows path
	else if(found == 2
	        && path.at(1) == ':'
	        && path.at(2) == '\\')
		return true;

	return false;
}

string Tools::fixSystemSeparator(string path)
{
#ifdef WIN32
	auto pos = path.find("/");
	while(pos != string::npos)
	{
		path.replace(pos, 1, "\\");
		pos = path.find("/", pos + 1);
	}
#endif
	return path;
}

string Tools::ensureDirExists(const string& path)
{
#ifdef WIN32
  string mkdir("mkdir ");
#else
  string mkdir("mkdir -p ");
#endif

	string fullCmd = fixSystemSeparator(mkdir + path);
	
	int res = system(fullCmd.c_str());
	//cout << "res: " << res << endl;

	return path;
}
