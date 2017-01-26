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
using namespace Climate;

CSVViaHeaderOptions::CSVViaHeaderOptions() 
	: separator(",") 
{}

CSVViaHeaderOptions::CSVViaHeaderOptions(json11::Json j)
{
	merge(j);
}

Tools::Errors CSVViaHeaderOptions::merge(json11::Json j)
{
	map<string, string> headerNames;
	for(auto p : j["header-to-acd-names"].object_items())
	{
		if(p.second.is_array() 
			 && p.second.array_items().size() == 3)
		{
			headerNames[p.first] = p.second[0].string_value();
			convert[p.first] = make_pair(p.second[1].string_value(), p.second[2].number_value());
		}
		else
			headerNames[p.first] = p.second.string_value();
	}

	set_iso_date_value(startDate, j, "start-date");
	set_iso_date_value(endDate, j, "end-date");

	set_string_valueD(separator, j, "csv-separator", ",");
	noOfHeaderLines = size_t(int_valueD(j, "no-of-climate-file-header-lines", 2));
	headerName2ACDName = headerNames;

	return{};
}

json11::Json CSVViaHeaderOptions::to_json() const
{
	J11Object headerNames;
	for(auto p : headerName2ACDName)
	{
		auto it = convert.find(p.first);
		if(it != convert.end())
			headerNames[p.first] = J11Array{p.second, it->second.first, it->second.second};
		else
			headerNames[p.first] = p.second;
	}

	J11Object convert_;
	for(auto p : convert)
		convert_[p.first] = J11Array{p.second.first, p.second.second};

	return json11::Json::object{
		 {"type", "CSVViaHeaderOptions"}
		,{"csv-separator", separator}
		,{"start-date", startDate.toIsoDateString()}
		,{"end-date", endDate.toIsoDateString()}
		,{"no-of-climate-file-header-lines", int(noOfHeaderLines)}
		,{"header-to-acd-names", headerNames}
	};
}

//-----------------------------------------------------------------------------

Climate::DataAccessor
Climate::readClimateDataFromCSVInputStreamViaHeaders(istream& is,
																										 CSVViaHeaderOptions options)
{
	if(!is.good())
	{
		cerr << "Input stream not good!" << endl;
		return DataAccessor();
	}
		
	vector<ACD> header;
	map<ACD, function<double(double)>> convert;
	string s;
	if(options.noOfHeaderLines > 0 && getline(is, s))
	{
		vector<string> r = splitString(s, options.separator);
		//remove possible \r at the end of the last element, when reading windows files under linux
		if(r.back().back() == '\r')
			r.back().pop_back();
		auto n2acd = name2acd();
		for(auto colName : r)
		{
			auto tcn = trim(colName);
			auto replColName = options.headerName2ACDName[tcn];
			auto acd = n2acd[replColName.empty() ? tcn : replColName];
			header.push_back(acd == 0 ? skip : acd);

			if(!options.convert.empty())
			{
				auto it = options.convert.find(tcn);
				if(it != options.convert.end())
				{
					auto acd = n2acd[replColName.empty() ? tcn : replColName];
					auto op = it->second.first;
					double value = it->second.second;
					if(op == "*")
						convert[acd] = [=](double d){ return d * value; };
					else if(op == "/")
						convert[acd] = [=](double d){ return d / value; };
					else if(op == "+")
						convert[acd] = [=](double d){ return d + value; };
					else if(op == "-")
						convert[acd] = [=](double d){ return d - value; };
				}
			}
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

	return readClimateDataFromCSVInputStream(is.seekg(0),
																					 options.separator,
																					 header,
																					 options.startDate,
																					 options.endDate,
																					 options.noOfHeaderLines,
																					 convert);
}

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

	return readClimateDataFromCSVInputStreamViaHeaders(ifs, options);
}

Climate::DataAccessor
Climate::readClimateDataFromCSVStringViaHeaders(std::string csvString,
																								CSVViaHeaderOptions options)
{
	istringstream iss(csvString);
	if(!iss.good())
	{
		cerr << "Could not access input string stream!" << endl;
		return DataAccessor();
	}

	return readClimateDataFromCSVInputStreamViaHeaders(iss, options);
}

//-----------------------------------------------------------------------------

Climate::DataAccessor
Climate::readClimateDataFromCSVInputStream(std::istream& is,
																					 std::string separator,
																					 std::vector<ACD> header,
																					 Tools::Date startDate,
																					 Tools::Date endDate,
																					 size_t noOfHeaderLines, 
																					 std::map<ACD, std::function<double(double)>> convert)
{
	if(!is.good())
	{
		cerr << "Input stream not good!" << endl;
		return DataAccessor();
	}
		
	if(header.empty())
		header = defaultHeader();

	bool isStartDateValid = startDate.isValid();
	bool isEndDateValid = endDate.isValid();
	
	//we store all data in a map to also manage csv files with wrong order
	map<Date, map<ACD, double>> data;

	//skip header line(s) and 
	//save first header line to compare for repeated headers
	string headerLine;
	vector<string> startOfHeaderLines;
	while(noOfHeaderLines-- > 0)
	{
		getline(is, headerLine);
		startOfHeaderLines.push_back(headerLine.substr(0, 10));
	}

	string s;
	while(getline(is, s))
	{
		//skip (repeated) headers
		bool isRepeatedHeader = false;
		for(auto startOfHeaderLine : startOfHeaderLines)
		{
			if(s.substr(0, 10) == startOfHeaderLine)
			{
				isRepeatedHeader = true;
				break;
			}
		}

		if(isRepeatedHeader)
			continue;

		vector<string> r = splitString(s, separator);
		//remove possible \r at the end of the last element, when reading windows files under linux
		if(r.back().back() == '\r')
			r.back().pop_back();
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
				bool doConvert = !convert.empty() && convert[acdi];
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
				{
					auto v = stod(r.at(i));
					vs[acdi] = doConvert ? convert[acdi](v) : v;
				}
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

		if(isStartDateValid && date < startDate)
			continue;

		if(isEndDateValid && date > endDate)
			continue;

		//cout 
		//	<< "[" << date.day() << "." << date.month() << "." << date.year() 
		//	<< "] -> [";
		//for(auto p : vs)
		//	cout << "(" << availableClimateData2Name(p.first) << ", " << p.second << ") ";
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
		cout
			<< "Read timeseries data between " << startDate.toIsoDateString()
			<< " and " << endDate.toIsoDateString()
			<< " (" << noOfDays << " days) is incomplete. There are just "
			<< data.size() << " days in read dataset." << endl;
		return DataAccessor();
	}
		
	map<ACD, vector<double>> daData;
	for(Date d = startDate, ed = endDate; d <= ed; d++)
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
	if(!daData[globrad].empty())
		da.addClimateData(Climate::globrad, daData[globrad]);
	else if(!daData[sunhours].empty())
		da.addClimateData(Climate::sunhours, daData[sunhours]);
	da.addClimateData(Climate::relhumid, daData[relhumid]);
	da.addClimateData(Climate::wind, daData[wind]);

	return da;
}


Climate::DataAccessor 
Climate::readClimateDataFromCSVFile(std::string pathToFile,
																		std::string separator,
																		std::vector<ACD> header,
																		Tools::Date startDate,
																		Tools::Date endDate,
																		size_t noOfHeaderLines,
																		std::map<ACD, std::function<double(double)>> convert)
{
	pathToFile = fixSystemSeparator(pathToFile);
	ifstream ifs(pathToFile.c_str());
	if(!ifs.good()) 
	{
		cerr << "Could not open climate file " << pathToFile << "." << endl;
		return DataAccessor();
	}

	return readClimateDataFromCSVInputStream(ifs, 
																					 separator, 
																					 header, 
																					 startDate, 
																					 endDate, 
																					 noOfHeaderLines);
}

Climate::DataAccessor
Climate::readClimateDataFromCSVString(std::string csvString,
																			std::string separator,
																			std::vector<ACD> header,
																			Tools::Date startDate,
																			Tools::Date endDate,
																			size_t noOfHeaderLines,
																			std::map<ACD, std::function<double(double)>> convert)
{
	istringstream iss(csvString);
	if(!iss.good())
	{
		cerr << "Could not access input string stream!" << endl;
		return DataAccessor();
	}

	return readClimateDataFromCSVInputStream(iss, 
																					 separator, 
																					 header, 
																					 startDate, 
																					 endDate, 
																					 noOfHeaderLines);
}

