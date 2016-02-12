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
Climate::readClimateDataFromCSVFileViaHeaders(std::string pathToFile,
                                              CSVViaHeaderOptions options)
{
	pathToFile = fixSystemSeparator(pathToFile);
	ifstream ifs(pathToFile.c_str());
	if(!ifs.good())
	{
		cerr << "Could not open climate file " << pathToFile << "." << endl;
		return DataAccessor();
	}

	vector<ACD> header;
	string s;
	if(options.noOfHeaderLines > 0 && getline(ifs, s))
	{
		vector<string> r = splitString(s, options.separator);
		auto n2acd = name2acd();
		for(auto colName : r)
		{
			auto it2 = options.headerName2ACDName.find(colName);
			auto it = n2acd.find(it2 == options.headerName2ACDName.end()
			                     ? colName : it2->second);
			header.push_back(it == n2acd.end() ? skip : it->second);
		}
	}

	if(header.empty())
	{
		cerr
			<< "Couldn't match any column names to internally used names. "
			<< "Read CSV header line was: " << endl
			<< s << endl;
		return DataAccessor();
	}

	return readClimateDataFromCSVFile(pathToFile, 
																		options.separator,
																		header, 
																		options.startDate,
																		options.endDate,
																		options.noOfHeaderLines);

}


Climate::DataAccessor 
Climate::readClimateDataFromCSVFile(std::string pathToFile,
																		std::string separator,
																		std::vector<ACD> header,
																		Tools::Date startDate,
																		Tools::Date endDate,
																		size_t noOfHeaderLines)
{
	pathToFile = fixSystemSeparator(pathToFile);
	bool useLeapYears = startDate.useLeapYears();
	if(startDate.useLeapYears() != endDate.useLeapYears())
	{
		cerr
			<< "The start date " << (useLeapYears ? "uses " : "doesn't use ")
			<< "leap years, but end date "
			<< (useLeapYears ? "doesn't. " : "does. ")
			<< "Setting end year to " << (useLeapYears ? "also " : "not ")
			<< "use leap years." << endl;
		endDate.setUseLeapYears(startDate.useLeapYears());
	}

	if(header.empty())
		header = defaultHeader();

	bool isStartDateValid = startDate.isValid();
	bool isEndDateValid = endDate.isValid();

	ifstream ifs(pathToFile.c_str());
	if(!ifs.good()) 
	{
		cerr << "Could not open climate file " << pathToFile << "." << endl;
		return DataAccessor();
	}

	//we store all data in a map to also manage csv files with wrong order
	map<Date, map<ACD, double>> data;
	
	//skip header line(s) and 
	//save first header line to compare for repeated headers
	string headerLine;
	vector<string> startOfHeaderLines;
	while(noOfHeaderLines-- > 0)
	{
		getline(ifs, headerLine);
		startOfHeaderLines.push_back(headerLine.substr(0, 10));
	}
	
	string s;
	while(getline(ifs, s))
	{
		//skip (repeated) headers
		bool isRepeatedHeader = false;
		for(auto startOfHeaderLine : startOfHeaderLines)
		{
			if (s.substr(0, 10) == startOfHeaderLine)
			{
				isRepeatedHeader = true;
				break;
			}
		}

		if(isRepeatedHeader)
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
		try
		{
			for(size_t i = 0; i < hSize; i++)
			{
				ACD acdi = header.at(i);
				switch(acdi)
				{
				case day: date.setDay(stoul(r.at(i))); break;
				case month: date.setMonth(stoul(r.at(i))); break;
				case year: date.setYear(stoul(r.at(i))); break;
				case isoDate: date = Date::fromIsoDateString(r.at(i)); break;
				case deDate:
				{
					auto dmy = splitString(r.at(i), ".");
					if(dmy.size() == 3)
					{
						date.setDay(stoul(dmy.at(0)));
						date.setMonth(stoul(dmy.at(1)));
						date.setYear(stoul(dmy.at(2)));
					}
					break;
				}
				case skip: break; //ignore element
				default:
					vs[acdi] = stod(r.at(i));
				}
			}
		}
		catch(invalid_argument e)
		{
			cerr
				<< "Error converting one of the (climate) elements in the line: " << endl
				<< s << endl;
			return DataAccessor();
		}

		if(!useLeapYears
			 && date.day() == 29
			 && date.month() == 2)
			continue;

		if(isStartDateValid && date < startDate)
			continue;

		if(isEndDateValid && date > endDate)
			continue;

		//cout 
		//	<< "[" << date.day() << "." << date.month() << "." << date.year() 
		//	<< "] -> [";
		//for(auto p : vs)
		//	cout << "(" << acdNames().at(p.first) << ", " << p.second << ") ";
		//cout << "]" << endl;

		data[date] = vs;
	}

	if(!isStartDateValid && !data.empty())
		startDate = data.begin()->first;

	if(!isEndDateValid && !data.empty())
		endDate = data.rbegin()->first;
	
	int noOfDays = endDate - startDate + 1;
	if(data.size() < size_t(noOfDays))
	{
		cerr
			<< "Read timeseries data between " << startDate.toIsoDateString() 
			<< " and " << endDate.toIsoDateString() 
			<< " (" << noOfDays << " days) is incomplete. There are just "
			<< data.size() << " days in read dataset." << endl;
		return DataAccessor();
	}

	map<ACD, vector<double>> daData;
	for (Date d = startDate, ed = endDate; d <= ed; d++)
		for(auto p : data[d])
			daData[p.first].push_back(p.second);

	size_t sizes = 0;
	for(const auto& p : daData)
		sizes += p.second.size();

	if(sizes % daData.size() != 0)
	{
		cerr
			<< "At least one of the climate elements has less elements than the others."
			<< endl;
		return DataAccessor();
	}

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

