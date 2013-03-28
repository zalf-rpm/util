/**
Authors:
Michael Berg <michael.berg@zalf.de>

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

#include <vector>
#include <sstream>

#include <list>

#include "agricultural-helper.h"
#include "tools/helper.h"

using namespace Tools;
using namespace std;

vector<int> Tools::splitSttCode(int sttCode)
{
	if(sttCode > 693 || sttCode < 100)
		return vector<int>();

	vector<int> res(3);
	int _1 = res[0] = int(double(sttCode) / 100.0);
	int _2 = res[1] = int(double(sttCode % 100) / 10.0);
	int _3 = res[2] = sttCode - (_1 * 100) - (_2 * 10);
	if(_3 > 3) return vector<int>();

	return res;
}

string Tools::sttFromCode(int sttCode)
{
	const vector<int>& sttc = splitSttCode(sttCode);
	if(sttc.empty())
		return "";

	ostringstream stt;

	switch(sttc.at(0))
	{
	case 1: stt << "D"; break;
	case 2: stt << "M"; break;
	case 3: stt << "Al"; break;
	case 4: stt << "Lö"; break;
	case 5: stt << "V"; break;
	case 6: stt << "K"; break;
	}

	stt << sttc.at(1);

	switch(sttc.at(2))
	{
	case 1: stt << "a"; break;
	case 2: stt << "b"; break;
	case 3: stt << "c"; break;
	}

	return stt.str();
}

vector<string> Tools::sttsFromCode(int sttCode)
{
	vector<string> stts;

	const vector<int>& sttc = splitSttCode(sttCode);
	if(sttc.empty())
		return stts;

	ostringstream stt;

	switch(sttc.at(0))
	{
		case 1: stt << "D"; break;
		case 2: stt << "M"; break;
		case 3: stt << "Al"; break;
		case 4: stt << "Lö"; break;
		case 5: stt << "V"; break;
		case 6: stt << "K"; break;
	}
	stts.push_back(stt.str());

	stt << sttc.at(1);
	stts.push_back(stt.str());

	switch(sttc.at(2))
	{
		case 1: stt << "a"; break;
		case 2: stt << "b"; break;
		case 3: stt << "c"; break;
	}
	stts.push_back(stt.str());

	return stts;
}

int Tools::sttCodeFromStt(string stt)
{
	stt = Tools::toLower(stt);
	int sttCode = 0;

	int i = 0;
	char c = stt.at(i);
	if(c == 'd')
		sttCode += 100;
	else if(c == 'm')
		sttCode += 200;
	else if(c == 'a')
	{
		i++;
		if(stt.at(i) == 'l')
			sttCode += 300;
		else
			return 0;
	}
	else if(c == 'l')
	{
		i++;
		if(stt.at(i) == 'ö')
			sttCode += 400;
		else
			return 0;
	}
	else if(c == 'v')
		sttCode += 500;
	else if(c == 'k')
		sttCode += 600;
	else
		return 0;

	i++;
	sttCode += 10 * atoi(stt.substr(i,1).c_str());

	i++;
	c = stt.at(i);
	if(c == 'a')
		sttCode += 1;
	else if(c == 'b')
		sttCode += 2;
	else if(c == 'c')
		sttCode += 3;
	else
		return 0;

	return sttCode;
}
