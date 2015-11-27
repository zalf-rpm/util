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

#include "helper.h"

using namespace Tools;
using namespace std;

bool Tools::satob(const std::string& s, bool def)
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

string Tools::fixSystemSeparator(std::string path)
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

void Tools::ensureDirExists(const std::string& path)
{
#ifdef WIN32
  string mkdir("mkdir ");
#else
  string mkdir("mkdir -p ");
#endif

	string fullCmd = fixSystemSeparator(mkdir + path);
	
	int res = system(fullCmd.c_str());
	//cout << "res: " << res << endl;
}
