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
#include <assert.h>
#include <sys/stat.h> // stat
#ifdef WIN32
#include <direct.h>   // _mkdir
#endif

#include "helper.h"

using namespace Tools;
using namespace std;

bool Tools::printPossibleErrors(const Errors& es, bool includeWarnings)
{
	if (es.failure())
		for (auto e : es.errors)
			std::cerr << e << std::endl;

	if (includeWarnings)
		for (auto w : es.warnings)
			std::cerr << w << std::endl;

	return es.success();
};

//-----------------------------------------------------------------------------

bool Tools::stob(const std::string& s, bool def)
{
	if (s.empty())
		return def;

	auto start = toLower(s).at(0);

	switch (start)
	{
	case 't': case '1':
		return true;
	case 'f': case '0':
		return false;
	default:
		return def;
	}
}

EResult<string> Tools::readFile(string path)
{
	EResult<string> res;
	path = fixSystemSeparator(path);

	ifstream ifs;
	ifs.open(path);
	if (ifs.good())
	{
		for (string line; getline(ifs, line);)
			res.result += line;
	}
	else
		res.errors.push_back(string("Couldn't open file: '") + path + "'");

	ifs.close();
	return res;
}

pair<string, string> Tools::splitPathToFile(const string& pathToFile)
{
	size_t found = pathToFile.find_last_of("/\\");
	return found == string::npos
		? make_pair("", pathToFile)
		: make_pair(pathToFile.substr(0, found + 1),
			pathToFile.substr(found + 1));
}

bool Tools::isAbsolutePath(const std::string& path)
{
	size_t found = path.find_first_of("/:");
	//unix absolute path
	if (found == 0 && path.at(0) == '/')
		return true;
	//absolute windows path
	else if (found == 1
		&& path.at(1) == ':'
		&& (path.at(2) == '\\'
			|| path.at(2) == '/'))
		return true;

	return false;
}

string Tools::fixSystemSeparator(string path)
{
#ifdef WIN32
	auto pos = path.find("/");
	while (pos != string::npos)
	{
		path.replace(pos, 1, "\\");
		pos = path.find("/", pos + 1);
	}
	pos = path.find("\\\\");
	while (pos != string::npos)
	{
		path.replace(pos, 2, "\\");
		pos = path.find("\\\\", pos + 1);
	}
#else
	auto pos = path.find("//");
	while (pos != string::npos)
	{
		path.replace(pos, 2, "/");
		pos = path.find("//", pos + 1);
	}
#endif
	return path;
}
//-----------------------------------------------------------------------------

bool Tools::ensureDirExists(const string& path)
{
	string fullpath = fixSystemSeparator(path);

#ifdef WIN32
	int ret = _mkdir(fullpath.c_str());
#else
	mode_t mode = 0755;
	int ret = mkdir(fullpath.c_str(), mode);
#endif
	if (ret != 0)
	{
		switch (errno)
		{
		case ENOENT:
		{
			// parent didn't exist, try to create parents(recursive)
			fullpath = rimRight(fullpath, "/\\");
			size_t pos = fullpath.find_last_of('/');
			if (pos == std::string::npos)
			{
#ifdef WIN32
				pos = fullpath.find_last_of('\\');
				if (pos == std::string::npos)
				{
					return false;
				}
#else
				return false;
#endif
			}
			if (!ensureDirExists(fullpath.substr(0, pos)))
			{
				return false;
			}
			// after creating the parents, create folder
#if defined(_WIN32)
			return 0 == _mkdir(fullpath.c_str());
#else 
			return 0 == mkdir(fullpath.c_str(), mode);
#endif
		}
		case EEXIST:
			// directory already exist, check if it is valid 
			return directoryExist(fullpath);
		default:
			return false;
		}
	}

	return true;
}

//------------------------------------------------------------------------------
/**
Removes all characters in charSet from the right side of the string.
*/
std::string 
Tools::rimRight(const std::string& str, const std::string& charSet)
{
	assert(!charSet.empty());
	string strResult;
	if (!str.empty())
	{
		size_t charSetLen = charSet.length();
		size_t thisIndex = str.length() - 1;
		bool stopped = false;
		while (!stopped && (thisIndex >= 0))
		{
			size_t charSetIndex;
			bool match = false;
			for (charSetIndex = 0; charSetIndex < charSetLen; charSetIndex++)
			{
				if (str[thisIndex] == charSet[charSetIndex])
				{
					// a match
					match = true;
					break;
				}
			}
			if (!match)
			{
				// stop if no match
				stopped = true;
			}
			else
			{
				// a match, advance to next character
				--thisIndex;
			}
		}
		strResult = str.substr(0, thisIndex + 1);
	}
	else
	{
		strResult = str;
	}
	return strResult;
}

//-----------------------------------------------------------------------------

bool Tools::directoryExist(const std::string& path)
{
	string fullpath = fixSystemSeparator(path);
#ifdef WIN32
	struct _stat info;
	if (_stat(fullpath.c_str(), &info) != 0)
	{
		return false;
	}
	return (info.st_mode & _S_IFDIR) != 0;
#else 
	struct stat info;
	if (stat(fullpath.c_str(), &info) != 0)
	{
		return false;
	}
	return (info.st_mode & S_IFDIR) != 0;
#endif
}

//-----------------------------------------------------------------------------

string Tools::replace(std::string s, std::string findStr, std::string replStr)
{
	auto count = findStr.size();
	size_t pos = s.find(findStr);
	while (pos != std::string::npos)
	{
		s.replace(pos, count, replStr);
		pos = s.find(findStr);
	}
	return s;
}

std::string Tools::replaceEnvVars(std::string path)
{
	string startToken = "${";
	string endToken = "}";
	auto startPos = path.find(startToken);
	while (startPos != std::string::npos)
	{
		auto endPos = path.find(endToken, startPos + 1);
		if (endPos != std::string::npos)
		{
			auto nameStart = startPos + 2;
			auto envVarName = path.substr(nameStart, endPos - nameStart);
			auto envVarContent = getenv(envVarName.c_str());
			if (envVarContent)
			{
				path.replace(startPos, endPos + 1 - startPos, envVarContent);
				startPos = path.find(startToken);
			}
			else
				startPos = path.find(startToken, endPos + 1);
		}
	}

	return path;
}

