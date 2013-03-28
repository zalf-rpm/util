/**
File platform.cpp - platform specific code for SAMT

Created 9.3.2008, Maximilian Matthe

Authors:
Maximilian Matthe

Maintainers:
Currently maintained by the authors.

This file is part of the util library used by models created at the Institute of
Landscape Systems Analysis at the ZALF.
Copyright (C) 2007-2013, Leibniz Centre for Agricultural Landscape Research (ZALF)

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/*
	File platform.cpp - platform specific code for SAMT

	Created 9.3.2008, Maximilian Matthe
*/

#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <map>

#include "platform.h"

using namespace std;

// windows: replaces / with \\, Linus does nothing
#ifdef WIN32
void ReplaceSeparators(char* str)
{
  /* Replace / with \\ */
	char* ptr = str;
	while(*ptr != 0)
	{
		if(*ptr == '/')
			*ptr = '\\';
		ptr++;
	}
}
#else
#define ReplaceSeparators(x)
#endif

// Returns the filename of the temporary hdf file xxx.h5
// function is needed because some windows cmdline-tools do not understand / as path separator
/*
	LINUX: returns getenv("SAMTHOME")/hdf/xxx.h5
	WINDOWS: returns getOption("SAMTHOME")\hdf\xxx.h5
*/
const char* getOutTempHDF()
{
	static char result[512];
	strcpy(result, getOption("SAMTHOME"));
	strcat(result, "/hdf/xxx.h5");
	ReplaceSeparators(result);
	return result;
}

// Returns absolute path of yyy.h5
const char* getInTempHDF()
{
	static char result[512];
	strcpy(result, getOption("SAMTHOME"));
	strcat(result, "/hdf/yyy.h5");
	ReplaceSeparators(result);
	return result;
}

// Function reads the option file samt.conf (must be in cwd) and tries to return the needed value.
/* Format of file:
KEY=Value
KEY=Value
.
.
.
*/
const char* getOptionFromFile(const char* optionname)
{
	static char result[512];
	result[0] = 0;  // Make result empty string

	cerr << "Loading option " << optionname << "... ";

	// locations where the file samt.conf can be found
	std::vector<std::string> filenames;
	filenames.push_back("samt.conf");
	filenames.push_back("../samt.conf");
	filenames.push_back("../../samt.conf");
	filenames.push_back("../../../samt.conf");
	filenames.push_back("~/samt.conf");
	filenames.push_back("~/samt/samt.conf");

	// look for file in multiple locations
	std::ifstream infile;
	for(size_t i = 0; i < filenames.size(); i++)
	{
		infile.close();
		infile.clear();
		infile.open(filenames[i].c_str());
		if(infile.good())
		{
			//cerr << "Found option file in " << filenames[i] << endl;
			break;
		}
	}

	if(!infile)
	{
		cerr << "\nFile samt.conf not found. Please create and set values appropriate. " << endl;
		return result; // empty string
	}

	while(!infile.eof())
	{
		std::string line;
		std::getline(infile, line);

		if(line.size() == 0)
			continue;

		size_t f = line.find('=');
		if(f != line.npos && f < line.size() - 1)
		{
			std::string key = line.substr(0, f);
			std::string val = line.substr(f+1);

			if(key == optionname)
			{
				strcpy(result, val.c_str());
				cerr << result << endl;
				return result;
			}
		}
	}

	cerr << "NOT FOUND!" << endl;
	return result;  // empty string
}

#ifdef WIN32
// platform-specific for windows



// Eigentlich h�tte ich lieber std::string als R�ckgabewert, der kann aber leider
// nicht in QString konvertiert werden. QString basiert auf QT, diese datei hier
// nicht unbedingt, bleibt nur char*
const char* getOption(const char* optionname)
{
	return getOptionFromFile(optionname);
}

const char* getCommand(const char* commandname)
{
	static char result[512];
	std::map<std::string, std::string> commands;
	commands.insert(std::pair<std::string, std::string>("rm", "del"));
	commands.insert(std::pair<std::string, std::string>("rm ", "del "));
	commands.insert(std::pair<std::string, std::string>("sync", "echo .")); // sync not needed in win32

	std::map<std::string, std::string>::iterator it = commands.find(commandname);
	if(it == commands.end())
	{
		cerr << "Command not found: " << commandname << endl;
		strcpy(result, "echo ");
		return result;
	}

	strcpy(result, it->second.c_str());
	return result;
}
#else
//#endif
//#ifdef unix
const char* getOption(const char* optionname)
{
	const char* opt = std::getenv(optionname);
	if(opt == NULL || std::strcmp(opt, "") == 0)
		return getOptionFromFile(optionname);	// if environment not set, try reading from a settings file
	else
		return opt;
}

const char* getCommand(const char* commandname)
{
	return commandname;
}
#endif

