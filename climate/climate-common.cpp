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

#include <iostream>
#include <sstream>
#include <algorithm>
#include <assert.h>

#include "climate/climate-common.h"

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
	default: ;
	}
	return "unbekannt";
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
	default: ;
	}
	return "";
}

const ACDV& acds()
{
	static ACDV v;
	static bool isInitialized = false;
  if(!isInitialized)
  {
		ACD t[] = { day, month, year, tmin, tavg, tmax, precip, precipOrig, globrad,
		            wind, sunhours, cloudamount, relhumid, airpress, vaporpress };
		v.insert(v.end(), t, t + availableClimateDataSize());
		isInitialized = true;
	}
	return v;
}

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
  : _data(new VVD),
    _acd2dataIndex(availableClimateDataSize(), -1),
    _fromStep(0),
    _numberOfSteps(0)
{}

DataAccessor::DataAccessor(const Tools::Date& startDate,
													 const Tools::Date& endDate)
  : _startDate(startDate),
    _endDate(endDate),
    _data(new VVD),
    _acd2dataIndex(availableClimateDataSize(), -1),
    _fromStep(0),
    _numberOfSteps(0)
{}

DataAccessor::DataAccessor(const DataAccessor& other)
  : _startDate(other._startDate),
    _endDate(other._endDate),
    _data(other._data),
    _acd2dataIndex(other._acd2dataIndex),
    _fromStep(other._fromStep),
    _numberOfSteps(other._numberOfSteps)
{}

double DataAccessor::dataForTimestep(AvailableClimateData acd,
                                     size_t stepNo) const
{
  short cacheIndex = _acd2dataIndex.at(int(acd));
  return cacheIndex < 0 ? 0.0 : _data->at(cacheIndex).at(_fromStep + stepNo);
}

vector<double> DataAccessor::dataAsVector(AvailableClimateData acd) const
{
  short cacheIndex = _acd2dataIndex.at(int(acd));
  return cacheIndex < 0 ? vector<double>()
                        :vector<double>(_data->at(cacheIndex).begin()+_fromStep,
                                        _data->at(cacheIndex).begin()+_fromStep+noOfStepsPossible());
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
	_acd2dataIndex[int(acd)] = _data->size() - 1;
	_numberOfSteps = _data->empty() ? 0 : _data->front().size();
}

void DataAccessor::addOrReplaceClimateData(AvailableClimateData acd,
                                           const vector<double>& data)
{
  int index = _acd2dataIndex[int(acd)];
  if(index < 0)
    addClimateData(acd, data);
  else
    (*_data)[index] = data;
}

