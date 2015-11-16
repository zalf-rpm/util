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

#include <map>
#include <sstream>
#include <iostream>
#include <fstream>
#include <cmath>
#include <utility>
#include <cassert>
#include <mutex>
#include <ctime>
#include <cstdlib>

#include "climate-file-io.h"

#include "climate/climate-common.h"
#include "tools/helper.h"
#include "tools/algorithms.h"
#include "tools/debug.h"

using namespace std;
using namespace Tools;

Climate::DataAccessor
Climate::readClimateDataFromCSVFileViaHeaders(const std::string& pathToFile,
																							const std::string& separator,
																							Tools::Date startDate,
																							Tools::Date endDate)
{
	ifstream ifs(pathToFile.c_str());
	if(!ifs.good())
	{
		cerr << "Could not open climate file " << pathToFile << ". Aborting now!" << endl;
		exit(1);
	}

	vector<ACD> header;
	string s;
	if(getline(ifs, s))
	{
		vector<string> r = splitString(s, separator);
		for(auto colName : r)
			for(int i = 0, size = acdNames().size(); i < size; i++)
				if(colName == acdNames().at(i))
					header.push_back(ACD(i));
	}

	if(header.empty())
	{
		cerr
			<< "Couldn't match any column names to internally used names. "
			<< "Read CSV header line was: " << endl
			<< s << endl
			<< "Aborting now!" << endl;
		exit(1);
	}

	return readClimateDataFromCSVFile(pathToFile, separator, header, 
																		startDate, endDate);

}


Climate::DataAccessor 
Climate::readClimateDataFromCSVFile(const std::string& pathToFile,
																		const std::string& separator,
																		std::vector<ACD> header,
																		Tools::Date startDate,
																		Tools::Date endDate)
{
	if(header.empty())
		header = defaultHeader();

	bool isStartDateValid = startDate.isValid();
	bool isEndDateValid = endDate.isValid();

	ifstream ifs(pathToFile.c_str());
	if(!ifs.good()) 
	{
		cerr << "Could not open climate file " << pathToFile << ". Aborting now!" << endl;
		exit(1);
	}

	//we store all data in a map to also manage csv files with wrong order
	map<Date, map<ACD, double>> data;
	
	string s;
	while(getline(ifs, s)) 
	{
		//skip (repeated) headers
		if (s.substr(0, 3) == "day")
			continue;

		vector<string> r = splitString(s, separator);
		size_t rSize = r.size();
		size_t hSize = header.size();
		if(rSize < header.size())
		{
			debug()
				<< "Skipping line: " << endl
				<< s << endl
				<< "because of less (" << r.size() << ") than expected ("
				<< header.size() << ") elements." << endl;
			continue;
		}

		Date date;
		map<ACD, double> vs;
		for(size_t i = 0; i < hSize; i++)
		{
			ACD acdi = header.at(i);
			switch(acdi)
			{
			case day: date.setDay(stoi(r.at(i))); break;
			case month: date.setMonth(stoi(r.at(i))); break;
			case year: date.setYear(stoi(r.at(i))); break;
			case isoDate: date = Date::fromIsoDateString(r.at(i)); break;
			case deDate:
			{
				auto dmy = splitString(r.at(i), ".");
				if(dmy.size() == 3)
				{
					date.setDay(stoi(dmy.at(0)));
					date.setMonth(stoi(dmy.at(1)));
					date.setYear(stoi(dmy.at(2)));
				}
				break;
			}
			case none: break; //ignore element
			default:
				vs[acdi] = stod(r.at(i));
			}
		}

		if(isStartDateValid && date < startDate)
			continue;

		if(isEndDateValid && date > endDate)
			continue;

		cout 
			<< "[" << date.day() << "." << date.month() << "." << date.year() 
			<< "] -> [";
		for(auto p : vs)
			cout << "(" << acdNames().at(p.first) << ", " << p.second << ") ";
		cout << "]" << endl;

		data[date] = vs;
	}

	if(!isStartDateValid && !data.empty())
		startDate = data.begin()->first;

	if(!isEndDateValid && !data.empty())
		endDate = data.rbegin()->first;
	
	int noOfDays = endDate - startDate;
	if(data.size() < size_t(noOfDays))
	{
		cerr
			<< "Read timeseries data between " << startDate.toIsoDateString() 
			<< " and " << endDate.toIsoDateString() 
			<< " (" << noOfDays << " days) is incomplete. There are just "
			<< data.size() << " days in read dataset. Aborting now!" << endl;
		exit(1);
	}

	map<ACD, vector<double>> daData;
	for (Date d = startDate, ed = endDate; d <= ed; d++)
		for(auto p : data[d])
			daData[p.first].push_back(p.second);

	Climate::DataAccessor da(startDate, endDate);
	da.addClimateData(Climate::tmin, daData[tmin]);
	da.addClimateData(Climate::tmax, daData[tmax]);
	da.addClimateData(Climate::tavg, daData[tavg]);
	da.addClimateData(Climate::precip, daData[precip]);
	da.addClimateData(Climate::globrad, daData[globrad]);
	da.addClimateData(Climate::relhumid, daData[relhumid]);
	da.addClimateData(Climate::wind, daData[wind]);

	return da;
}

