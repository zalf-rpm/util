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

#include <iostream>
#include <sstream>
#include <algorithm>
#include <assert.h>
#include <limits>

#include "climate/climate-common.h"
#include "tools/algorithms.h"

using namespace std;
using namespace Climate;
using namespace Tools;

//------------------------------------------------------------------------------

string Climate::availableClimateData2CLMDBColName(AvailableClimateData col)
{
  switch(col)
  {
  case day: return "tag";
  case month: return "monat";
  case year: return "jahr";
  case tmin: return "tn";
  case tavg: return "tm";
  case tmax: return "tx";
  case precip: return "rr_corr";
  case precipOrig: return "rr";
  case globrad: return "0";
  case relhumid: return "rf";
  case airpress: return "pp";
  case sunhours: return "sd";
  case cloudamount: return "nn";
  case vaporpress: return "dd";
  case wind: return "ff";
  default: ;
	}
	return "error";
}

string Climate::availableClimateData2WerexColName(AvailableClimateData col)
{
  switch(col)
  {
  case day: return "tag";
  case month: return "mo";
  case year: return "jahr";
  case tmin: return "tmin";
  case tavg: return "tmit";
  case tmax: return "tmax";
  case precip: return "rr";
  case precipOrig: return "rr";
  case globrad: return "0";
  case relhumid: return "rf";
  case airpress: return "pp";
  case sunhours: return "sd";
  case cloudamount: return "nn";
  case vaporpress: return "dd";
  case wind: return "ff";
  default: ;
	}
	return "error";
}

string Climate::availableClimateData2WettRegDBColName(AvailableClimateData col)
{
  switch(col)
  {
  case day: return "tag";
  case month: return "mo";
  case year: return "jahr";
  case tmin: return "tmin";
  case tavg: return "tmit";
  case tmax: return "tmax";
  case precip: return "nied";
  case precipOrig: return "nied";
  case globrad: return "stra";
  case relhumid: return "relf";
  case airpress: return "ludr";
  case sunhours: return "sonn";
  case cloudamount: return "bewo";
  case vaporpress: return "dadr";
  case wind: return "wind";
  default: ;
	}
	return "error";
}

pair<string, int>
Climate::availableClimateData2CarbiocialDBColNameAndScaleFactor(AvailableClimateData col)
{
	switch(col)
	{
	case day: return make_pair("day", 0);
	case month: return make_pair("month", 0);
	case year: return make_pair("year", 0);
	case tmin: return make_pair("t_min_times_10", 10);
	case tavg: return make_pair("t_avg_times_10", 10);
	case tmax: return make_pair("t_max_times_10", 10);
	case precip: return make_pair("precipitation_mm_times_10", 10);
	case precipOrig: return make_pair("precipitation_mm_times_10", 10);
	case globrad: return make_pair("global_radiation_mj_per_m2_times_100", 100);
	case relhumid: return make_pair("relative_humidity_percent_times_10", 10);
	//case airpress: return make_pair("ludr", 0);
	//case sunhours: return make_pair("sonn", 0);
	//case cloudamount: return make_pair("bewo", 0);
	//case vaporpress: return make_pair("dadr", 0);
	case wind: return make_pair("windspeed_m_per_s_times_10", 10);
	default: ;
	}
	return make_pair("error", 0);
}

string
Climate::availableClimateData2UserSqliteDBColNameAndScaleFactor(AvailableClimateData col)
{
  switch(col)
  {
  case day: return "day";
  case month: return "month";
  case year: return "year";
  case tmin: return "t_min";
  case tavg: return "t_avg";
  case tmax: return "t_max";
  case precip: return "precipitation_mm";
  case precipOrig: return "precipitation_mm";
  case globrad: return "global_radiation_mj_per_m2";
  case relhumid: return "relative_humidity_percent";
  //case airpress: return "ludr";
  //case sunhours: return "sonn";
  //case cloudamount: return "bewo";
  //case vaporpress: return "dadr";
  case wind: return "windspeed_m_per_s";
  default: ;
  }
  return "error";
}

std::string Climate::availableClimateData2Name(AvailableClimateData col)
{
	switch(col)
	{
	case day: return "Tag";
	case month: return "Monat";
	case year: return "Jahr";
	case tmin: return "Tmin";
	case tavg: return "Tmit";
	case tmax: return "Tmax";
	case precip: return "Niederschlag";
	case precipOrig: return "Niederschlag (unkorrigiert)";
	case globrad: return "Globlstrahlung";
	case relhumid: return "relative Luftfeuchte";
	case airpress: return "Luftdruck";
	case sunhours: return "Sonnenscheindauer";
	case cloudamount: return "Bewölkungsgrad";
	case vaporpress: return "Dampfdruck";
	case wind: return "Windgeschwindigkeit";
	case co2: return "CO2";
	default: ;
	}
	return "unknown";
}

std::string Climate::availableClimateData2unit(AvailableClimateData col)
{
	switch(col)
	{
	case day: return "d";
	case year: return "a";
	case tmin: return "°C";
	case tavg: return "°C";
	case tmax: return "°C";
	case precip: return "mm";
	case precipOrig: return "mm";
	case sunhours: return "h";
	case wind: return "m/s";
	case co2: return "ppm";
	default: ;
	}
	return "";
}

/*
const ACDV& acds()
{
	static ACDV v;
	static bool isInitialized = false;
  if(!isInitialized)
  {
		ACD t[] = { day, month, year, tmin, tavg, tmax, precip, precipOrig, globrad,
		            wind, sunhours, cloudamount, relhumid, airpress, vaporpress, co2 };
		v.insert(v.end(), t, t + availableClimateDataSize());
		isInitialized = true;
	}
	return v;
}
*/

//------------------------------------------------------------------------------

YearRange Climate::snapToRaster(YearRange yr, int raster)
{
  int from = yr.fromYear;
  int to = yr.toYear;

  int fmod = from % raster;
  if(fmod != 1)
    from = from + (fmod == 0 ? 0 : raster - fmod) + 1;
  int tmod = to % raster;
  if(tmod != 0)
    to = to - tmod;

  return YearRange(from, to);
}

//------------------------------------------------------------------------------

DataAccessor::DataAccessor()
	: _data(new VVD)
  , _acd2dataIndex(availableClimateDataSize(), -1)
  , _fromStep(0)
  , _numberOfSteps(0)
{}

DataAccessor::DataAccessor(const Tools::Date& startDate,
													 const Tools::Date& endDate)
  : _startDate(startDate)
  , _endDate(endDate)
  , _data(new VVD)
  , _acd2dataIndex(availableClimateDataSize(), -1)
  , _fromStep(0)
	, _numberOfSteps(0)
{}

DataAccessor::DataAccessor(const DataAccessor& other)
  : _startDate(other._startDate)
  ,  _endDate(other._endDate)
  , _data(other._data)
  , _acd2dataIndex(other._acd2dataIndex)
  , _fromStep(other._fromStep)
  , _numberOfSteps(other._numberOfSteps)
{}

DataAccessor::DataAccessor(json11::Json j)
{
	merge(j);
}

Errors DataAccessor::merge(json11::Json j)
{
	for(const auto& acd2data : j["data"].object_items())
		addOrReplaceClimateData(ACD(stoi(acd2data.first)), double_vector(acd2data.second));

	set_iso_date_value(_startDate, j, "startDate");
	set_iso_date_value(_endDate, j, "endDate");

	return{};
}

json11::Json DataAccessor::to_json() const
{
	J11Object data;
	int acd = 0;
	for(auto i : _acd2dataIndex)
	{
		if(i >= 0)
		{
			data[to_string(acd)] = toPrimJsonArray(dataAsVector(ACD(acd)));
		}
		acd++;
	}

	return J11Object
	{{"type", "DataAccessor"}
	,{"data", data}
	,{"startDate", startDate().toIsoDateString()}
	,{"endDate", endDate().toIsoDateString()}
	};
}

double DataAccessor::dataForTimestep(AvailableClimateData acd,
																		 size_t stepNo,
																		 double def) const
{
	short cacheIndex = _acd2dataIndex.at(int(acd));
	return cacheIndex < 0 ? def : _data->at(cacheIndex).at(_fromStep + stepNo);
}

Maybe<double> DataAccessor::dataForTimestepM(AvailableClimateData acd,
																						 size_t stepNo) const
{
	short cacheIndex = _acd2dataIndex.at(int(acd));
	return cacheIndex < 0 ? Maybe<double>() : _data->at(cacheIndex).at(_fromStep + stepNo);
}

map<Climate::ACD, double>
DataAccessor::allDataForStep(size_t stepNo,
														 double latitude) const
{
	map<ACD, double> m;

	for(size_t k = 0, size = _acd2dataIndex.size(); k < size; k++)
	{
		ACD acd = ACD(int(k));
		auto v = dataForTimestepM(acd, stepNo);
		if(v.isValue())
			m[acd] = v.value();
		else if(acd == globrad)
		{
			auto v2 = dataForTimestepM(sunhours, stepNo);
			if(v2.isValue())
				m[acd] = Tools::sunshine2globalRadiation(dateForStep(stepNo).julianDay(),
																								 v2.value(),
																								 latitude,
																								 true);
		}
	}

	return m;
}

vector<double> DataAccessor::dataAsVector(AvailableClimateData acd) const
{
	short cacheIndex = _acd2dataIndex.at(int(acd));
	return cacheIndex < 0
		? vector<double>()
		: vector<double>(_data->at(cacheIndex).begin() + _fromStep,
										 _data->at(cacheIndex).begin() + _fromStep + noOfStepsPossible());
}

DataAccessor DataAccessor::cloneForRange(size_t fromStep,
																				 size_t numberOfSteps) const
{
	//cout << "cloneForRange fromStep: " << fromStep <<
	//" numberOfSteps: " << numberOfSteps << endl;
	if(!isValid()
		 || fromStep > noOfStepsPossible()
		 || (fromStep + numberOfSteps) > noOfStepsPossible())
		return DataAccessor(); //numberOfSteps = fromStep = 0;

	DataAccessor clone(*this);
	clone._fromStep += fromStep;
	clone._numberOfSteps = numberOfSteps;
	clone._startDate = clone._startDate + clone._fromStep;
	clone._endDate = clone._startDate + numberOfSteps - 1;
	return clone;
}

void DataAccessor::addClimateData(AvailableClimateData acd,
																	const vector<double>& data)
{
	if(!_data->empty())
		assert(_numberOfSteps = data.size());

	_data->push_back(data);
	_acd2dataIndex[int(acd)] = short(_data->size() - 1);
	_numberOfSteps = _data->empty() ? 0 : _data->front().size();
}

void DataAccessor::prependOrAppendClimateData(DataAccessor other,
																							bool replaceOverlappingData)
{
	if(_data->empty())
	{
		(*this) = other;
		return;
	}

	if((startDate() > other.endDate() + 1) ||
		 (endDate() < other.startDate() - 1))
	{
		cout
			<< "Can't prepend or append climate data because together data don't give a continuous timeseries. "
			<< "current: (" << startDate().toIsoDateString() << " - " << endDate().toIsoDateString() << ") " 
			<< "new: (" << other.startDate().toIsoDateString() << " - " << other.endDate().toIsoDateString() << ")."
			<< "New climate data won't be appended or prepended." << endl;
	}
	//insert all of others data before this' data
	else if(startDate() == other.endDate() + 1)
	{
		for(int i = 0; i < _acd2dataIndex.size(); i++)
		{
			auto acd = ACD(i);
			short index = _acd2dataIndex[i];
			if(index < 0)
				continue;
			auto& di = (*_data)[index];
			const auto& odai = other._data->at(index);

			di.insert(di.begin(), odai.begin(), odai.end());
		}
		_startDate = other.startDate();
		_numberOfSteps = _data->empty() ? 0 : _data->front().size();
	}
	//insert all of others data after this' data
	else if(endDate() + 1 == other.startDate())
	{
		for(int i = 0; i < _acd2dataIndex.size(); i++)
		{
			auto acd = ACD(i);
			short index = _acd2dataIndex[i];
			if(index < 0)
				continue;
			auto& di = (*_data)[index];
			const auto& odai = other._data->at(index);

			di.insert(di.end(), odai.begin(), odai.end());
		}
		_endDate = other.endDate();
		_numberOfSteps = _data->empty() ? 0 : _data->front().size();
	}
	//all of this' data will be overwritten
	else if(startDate() >= other.startDate() && endDate() <= other.endDate())
	{
		(*this) = other;
		return;
	}
	else 
	{
		int prependCount = startDate() - other.startDate();
		int appendCount = other.endDate() - endDate();
		
		for(int i = 0; i < _acd2dataIndex.size(); i++)
		{
			auto acd = ACD(i);
			short index = _acd2dataIndex[i];
			if(index < 0)
				continue;
			auto& di = (*_data)[index];
			const auto& odai = other._data->at(index);

			// insert the new elements before
			if(prependCount > 0)
				di.insert(di.begin(), odai.begin(), odai.begin() + prependCount);

			if(replaceOverlappingData)
			{
				auto dii = di.begin() + abs(prependCount);
				auto odaii = odai.begin() + (prependCount < 0 ? 0 : prependCount);
				for(; dii != di.end() && odaii != odai.end(); dii++, odaii++)
					*dii = *odaii;
			}

			// insert the new elements after the old ones
			if(appendCount > 0)
				di.insert(di.end(), odai.begin() + odai.size() - appendCount, odai.end());
		}
		if(prependCount > 0)
			_startDate = other.startDate();
		if(appendCount > 0)
			_endDate = other.endDate();
		_numberOfSteps = _data->empty() ? 0 : _data->front().size();
	}
}

void DataAccessor::addOrReplaceClimateData(AvailableClimateData acd,
                                           const vector<double>& data)
{
  int index = _acd2dataIndex[int(acd)];
	if(index < 0)
	{
		addClimateData(acd, data);
	}
	else
	{
		if(!_data->empty())
			assert(_numberOfSteps = data.size());

		(*_data)[index] = data;
	}
}






