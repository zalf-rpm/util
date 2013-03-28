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

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <cmath>
#include <cassert>
#include <map>
#include <list>

#include <boost/foreach.hpp>
#include <boost/function.hpp>
#include <boost/algorithm/string/case_conv.hpp>

#include "tools/use-stl-algo-boost-lambda.h"

#include "climate/climate.h"
#include "db/abstract-db-connections.h"
#include "tools/coord-trans.h"
#include "tools/algorithms.h"
#include "tools/datastructures.h"
#include "tools/helper.h"

using namespace std;
using namespace Climate;
using namespace Tools;

using namespace Loki;

using boost::shared_ptr;

namespace
{
  struct L : public ObjectLevelLockable<L> {};
}

//------------------------------------------------------------------------------

string ClimateStation::toString() const
{
	ostringstream s;
	s << "climate-station: name: " << name() << " id: " <<  id()
	<< " geoCoord: " << geoCoord().toString();
	return s.str();
}

//------------------------------------------------------------------------------

ClimateSimulation::~ClimateSimulation()
{
  BOOST_FOREACH(ClimateStation* cs, _stations)
  {
    delete cs;
  }

  BOOST_FOREACH(ClimateScenario* s, _scenarios)
  {
    delete s;
  }

  BOOST_FOREACH(ClimateRealization* r, _realizations)
  {
    delete r;
  }
}

std::vector<LatLngCoord> ClimateSimulation::geoCoords() const
{
	std::vector<LatLngCoord> gcs(climateStations().size());
	for(unsigned int i = 0; i < gcs.size(); i++)
		gcs[i] = climateStations().at(i)->geoCoord();
	return gcs;
}

LatLngCoord ClimateSimulation::
climateStation2geoCoord(const string& stationName) const
{
  string lowerStationName = boost::to_lower_copy(stationName);
  BOOST_FOREACH(ClimateStation* cs, _stations)
  {
    if(boost::to_lower_copy(cs->name()).find(lowerStationName) != string::npos)
      return cs->geoCoord();
	}
	return LatLngCoord();
}

ClimateStation ClimateSimulation::
geoCoord2climateStation(const LatLngCoord& gc) const
{
  BOOST_FOREACH(ClimateStation* cs, _stations)
  {
    if(cs->geoCoord() == gc)
      return *cs;
	}
	return ClimateStation();
}

LatLngCoord ClimateSimulation::
getClosestClimateDataGeoCoord(const LatLngCoord& gc) const
{
	ClimateStation* closestCS = _stations.front();
	double minDist = gc.distanceTo(closestCS->geoCoord());
  BOOST_FOREACH(ClimateStation* cs, _stations)
  {
    double dist = gc.distanceTo(cs->geoCoord());
    if(dist < minDist)
    {
			minDist = dist;
      closestCS = cs;
		}
	}

	//cout << "closestCS: " << closestCS->toString() << endl;
	return closestCS->geoCoord();
}

ClimateStation ClimateSimulation::climateStation(const string& stationName) const
{
  string lowerStationName = boost::to_lower_copy(stationName);
  BOOST_FOREACH(ClimateStation* cs, _stations)
  {
    if(boost::to_lower_copy(cs->name()).find(lowerStationName) != string::npos)
      return *cs;
	}
	return ClimateStation();
}

ClimateScenario* ClimateSimulation::scenario(const string& name) const
{
  BOOST_FOREACH(ClimateScenario* s, _scenarios)
  {
    if(s->name() == name)
      return s;
	}
	return NULL;
}

ClimateScenario* ClimateSimulation::scenarioById(const string& id) const
{
  BOOST_FOREACH(ClimateScenario* s, _scenarios)
  {
    if(s->id() == id)
      return s;
  }
  return NULL;
}


//------------------------------------------------------------------------------

namespace
{
  bool cmpClimateStationPtrs(ClimateStation* left, ClimateStation* right)
  {
		return (*left) < (*right);
	}

  struct ToLower : public unary_function<char, void>
  {
		void operator()(char& c){ c = tolower(c); }
	};
}

StarSimulation::StarSimulation(Db::MysqlDB* con)
: ClimateSimulation("star", "Star", con)
{
  setClimateStations();

  ClimateScenario* cs = new ClimateScenario("---", this);
  _realizations.push_back(new StarRealization(this, cs, connection().clone()));
  cs->setRealizations(_realizations);
  _scenarios.push_back(cs);
}

void StarSimulation::setClimateStations()
{
  connection().select("select latitude, longitude, dat, bezeichnung, hnn, id "
                      "from klimades");

	MYSQL_ROW row;
	while((row = connection().getMysqlRow()) != 0)
  {
    string name(row[3]);
    capitalizeInPlace(name);
    //cout << "name: " << name << endl;
    ClimateStation* cs =
				new ClimateStation(atoi(row[5]),
													 LatLngCoord(std::atof(row[0]), std::atof(row[1])),
													 atof(row[4]), name, this);
    cs->setDbName(row[2]);
    _stations.push_back(cs);
  }

  sort(_stations.begin(), _stations.end(), cmpClimateStationPtrs);
  connection().freeResultSet();
}

ClimateScenario* StarSimulation::defaultScenario() const
{
  return _scenarios.back();
}

//------------------------------------------------------------------------------

Star2Simulation::Star2Simulation(Db::MysqlDB* con)
: ClimateSimulation("star2", "Star2", con)
{
  setClimateStations();
  setScenariosAndRealizations();
}

void Star2Simulation::setScenariosAndRealizations()
{
	string reals = Db::dbConnectionParameters().value("used-realizations", "star2", "1, 25, 50, 75, 100");
	vector<string> vsr = Tools::splitString(reals, ", ");
	vector<int> realizationNumbers;
	BOOST_FOREACH(string s, vsr) { realizationNumbers.push_back(atoi(s.c_str())); }

  Star2Scenario* s2s = new Star2Scenario("2k","2K", "2k_", this);
  Realizations rs;
  BOOST_FOREACH(int realizationNo, realizationNumbers)
  {
    rs.push_back(new Star2Realization(this, s2s, connection().clone(),
                                      realizationNo));
  }
  s2s->setRealizations(rs);
  _realizations.insert(_realizations.end(), rs.begin(), rs.end());
  _scenarios.push_back(s2s);

  rs.clear();
  s2s = new Star2Scenario("0k","0K", "0k_", this);
  BOOST_FOREACH(int realizationNo, realizationNumbers)
  {
    rs.push_back(new Star2Realization(this, s2s, connection().clone(),
                                      realizationNo));
  }
  s2s->setRealizations(rs);
  _realizations.insert(_realizations.end(), rs.begin(), rs.end());
  _scenarios.push_back(s2s);
}

void Star2Simulation::setClimateStations()
{
	connection().select("select lat, lon, name, id from station");// where klim = 1");

  MYSQL_ROW row;
	while((row = connection().getMysqlRow()) != 0)
  {
    string name(row[2]);
    for_each(name.begin(), name.end(), ToLower());
    capitalizeInPlace(name);
    //cout << "name: " << name << endl;
    ClimateStation* cs =
      new ClimateStation(atoi(row[3]),
                         LatLngCoord(std::atof(row[0]), std::atof(row[1])),
                         0.0, name, this);
    cs->setDbName("");
    _stations.push_back(cs);
  }

  sort(_stations.begin(), _stations.end(), cmpClimateStationPtrs);
  connection().freeResultSet();
}

ClimateScenario* Star2Simulation::defaultScenario() const
{
  return scenarioById("2k");
}

//------------------------------------------------------------------------------

Star2MeasuredDataSimulation::Star2MeasuredDataSimulation(Db::MysqlDB* con)
: ClimateSimulation("star2measured", "Star2m", con)
{
  setClimateStations();

  ClimateScenario* cs = new ClimateScenario("---", this);
  _realizations.push_back(new Star2MeasuredDataRealization
                          (this, cs, connection().clone()));
  cs->setRealizations(_realizations);
  _scenarios.push_back(cs);
}

void Star2MeasuredDataSimulation::setClimateStations()
{
  connection().select("select lat, lon, name, id from station where klim = 1");

  MYSQL_ROW row;
	while((row = connection().getMysqlRow()) != 0)
  {
    string name(row[2]);
    for_each(name.begin(), name.end(), ToLower());
    capitalizeInPlace(name);
    //cout << "name: " << name << endl;
    ClimateStation* cs =
      new ClimateStation(atoi(row[3]),
                         LatLngCoord(std::atof(row[0]), std::atof(row[1])),
                         0.0, name, this);
    cs->setDbName("refzen");
    _stations.push_back(cs);
  }

  sort(_stations.begin(), _stations.end(), cmpClimateStationPtrs);
  connection().freeResultSet();
}

ClimateScenario* Star2MeasuredDataSimulation::defaultScenario() const
{
  return _scenarios.back();
}

//------------------------------------------------------------------------------

DDClimateDataServerSimulation::
DDClimateDataServerSimulation(const DDServerSetup& setupData, Db::MysqlDB* con)
	: ClimateSimulation(setupData.simulationId(), setupData.simulationName(), con),
		_setupData(setupData)
{
	if(_setupData.yearRange.isValid())
		_yearRange = _setupData.yearRange;

	setClimateStations();
	setScenariosAndRealizations();
}

void DDClimateDataServerSimulation::setScenariosAndRealizations()
{
	BOOST_FOREACH(string sid, _setupData.scenarioIds())
	{
		ClimateScenario* sc = new ClimateScenario(sid, this);
		Realizations rs;
		BOOST_FOREACH(string rid, _setupData.realizationIds())
		{
			rs.push_back(new DDClimateDataServerRealization(rid, this, sc,
																											connection().clone(),
																											_setupData));
		}
		sc->setRealizations(rs);
		_realizations.insert(_realizations.end(), rs.begin(), rs.end());
		_scenarios.push_back(sc);
	}
}

void DDClimateDataServerSimulation::setClimateStations()
{
	ostringstream ss;
	ss << "SELECT h.stat_id, h.stat_name, h.rwert5, h.hwert5, h.breite_dez, "
				"h.laenge_dez, h.nn, sl.dat_id, h.sl "
				"FROM "
		 << _setupData.headerDbName() << "." << _setupData.headerTableName() << " as h "
				"inner join "
		 <<  _setupData.stolistDbName() << "." << _setupData.stolistTableName() << " as sl "
				 "on h.stat_id = sl.stat_id "
				 "where sl.stat_ke = 'Klim' ";
	if(_setupData.useErrorTable())
		ss << "and sl.dat_id not in "
					"(SELECT distinct dat_id "
			 << "FROM " << _setupData.errorDbName() << "." << _setupData.errorTableName() << ") ";
	//excluded original wettreg 2006 stations with missing data or wrong data
	if(_setupData.simulationId() == "wettreg2006")
		ss << "and sl.dat_id not in (283, 385, 1120, 1623, 1861)";

	connection().select(ss.str().c_str());

	bool commaDotConversionChecked = false;
	bool convertCommaToDot = false;

	MYSQL_ROW row;
	while((row = connection().getMysqlRow()) != 0)
  {
		string name(row[1]);
		for_each(name.begin(), name.end(), ToLower());
		capitalizeInPlace(name);

    if(!commaDotConversionChecked)
    {
			convertCommaToDot = strchr(row[4], ',') != NULL;
			commaDotConversionChecked = true;
		}

		ClimateStation* cs =
			new ClimateStation(atoi(row[0]),
			                   convertCommaToDot
												 ? LatLngCoord(Tools::atof_comma(row[4]), Tools::atof_comma(row[5]))
                         : LatLngCoord(atof(row[4]), atof(row[5])),
												 (row[6] ? atof(row[6]) : 0.0), name, this);
		cs->setDbName(row[7]);
		cs->setSL(ClimateStation::SL(row[8] ? atoi(row[8]) : 1));
		_stations.push_back(cs);
		//cout << "wrname: " << name << " : " << _stations.back()->toString() << endl;
	}

	sort(_stations.begin(), _stations.end(), cmpClimateStationPtrs);
	connection().freeResultSet();
}

ClimateScenario* DDClimateDataServerSimulation::defaultScenario() const
{
	return scenario(defaultScenarioId());
}

YearRange DDClimateDataServerSimulation::availableYearRange()
{
  if(!_yearRange.isValid())
  {
    Lock lock(this);

		if(!_yearRange.isValid() &&
			 !_setupData.realizationIds().empty() &&
			 !climateStations().empty())
    {
			string firstRId = _setupData.realizationIds().front();
			ClimateStation* firstCS = climateStations().front();

			ostringstream ss;
			ss << "SELECT min(jahr), max(jahr) "
						"FROM "
				 << _setupData.dataDbName() << "." << _setupData.dataTableName() << " "
						"where szenario='" << defaultScenarioId() << "' and "
						"realisierung='" << firstRId << "' and "
						"dat_id = " << firstCS->dbName();

			connection().select(ss.str().c_str());

      MYSQL_ROW row;
			if((row = connection().getMysqlRow()) != 0)
        _yearRange = snapToRaster(YearRange(atoi(row[0]), atoi(row[1])));
    }
  }

  return _yearRange;
}

//------------------------------------------------------------------------------

CLMSimulation::CLMSimulation(Db::MysqlDB* con)
	: ClimateSimulation("clm20-9", "CLM20-9", con),
  _avgClimateStationsSet
	(boost::function<bool(const ClimateStation*, const ClimateStation*)>
	 (boost::lambda::bind(&ClimateStation::id, _1) < boost::lambda::bind(&ClimateStation::id, _2)))
{
	setClimateStations();
	setScenariosAndRealizations();
}

void CLMSimulation::setScenariosAndRealizations()
{
	string reals = Db::dbConnectionParameters().value("used-realizations", "clm20-9", "1, 2");
	vector<string> vsr = Tools::splitString(reals, ", ");

	ClimateScenario* sc = new ClimateScenario("A1B", this);
	Realizations rs;
	BOOST_FOREACH(string s, vsr)
	{
		rs.push_back(new CLMRealization(this, sc, s, connection().clone()));
	}
	sc->setRealizations(rs);
	_realizations.insert(_realizations.end(), rs.begin(), rs.end());
	_scenarios.push_back(sc);

	rs.clear();
	sc = new ClimateScenario("B1", this);
	BOOST_FOREACH(string s, vsr)
	{
		rs.push_back(new CLMRealization(this, sc, s, connection().clone()));
	}
	sc->setRealizations(rs);
	_realizations.insert(_realizations.end(), rs.begin(), rs.end());
	_scenarios.push_back(sc);
}

void CLMSimulation::setClimateStations()
{
	connection().select
	("SELECT h.stat_id, h.stat_name, h.rwert5, h.hwert5, h.breite_dez, "
	 "h.laenge_dez, h.nn, sl.dat_id "
	 "FROM project_landcare.header_clm20 h "
	 "inner join project_landcare.clm20_stolist sl on h.stat_id = sl.stat_id");

  set<int> lats, lngs;

	MYSQL_ROW row;
	while((row = connection().getMysqlRow()) != 0)
  {
		ClimateStation* cs =
			new ClimateStation(atoi(row[0]), LatLngCoord(atof(row[4]), atof(row[5])),
			                   atof(row[6]), row[1], this);
		cs->setDbName(row[7]);
		_stations.push_back(cs);

    //find smallest and largest values
    lats.insert(int(cs->geoCoord().lat * 100.0));
    lngs.insert(int(cs->geoCoord().lng * 100.0));
	}

  //define position matrix which should be sparse (but isn't)
  //bearable in this case
  typedef SparseMatrix<ClimateStation*> LatLngPos;
  LatLngPos posMatrix(static_cast<ClimateStation*>(NULL));

  //put climatestations into position matrix
  BOOST_FOREACH(ClimateStation* cs, climateStations())
  {
    int lat = int(cs->geoCoord().lat * 100.0);
    int lng = int(cs->geoCoord().lng * 100.0);
    posMatrix.setValueAt(lat, lng, cs);
  }

  //reduce "sparse" position matrix to minimal one
  typedef StdMatrix<ClimateStation*> LatLngPos2;
  LatLngPos2 posMatrix2(lats.size(), lngs.size(),
                        static_cast<ClimateStation*>(NULL));
  int lat2 = 0;
  BOOST_FOREACH(int lat, lats)
  {
    int lng2 = 0;
    BOOST_FOREACH(int lng, lngs)
    {
      if(ClimateStation* cs =
         posMatrix.valueAt(lat, lng, static_cast<ClimateStation*>(NULL)))
        posMatrix2[lat2][lng2++] = cs;
    }
    lat2++;
  }

  //find surrounding climatestations and remember them in map
  for(int lat = 0, latRange = posMatrix2.rows(); lat < latRange; lat++)
  {
    for(int lng = 0, lngRange = posMatrix2.cols(); lng < lngRange; lng++)
    {
      ClimateStation* centerStation = posMatrix2[lat][lng];
      set<const ClimateStation*>& css = _avgClimateStationsSet[centerStation];

      //north-west
      if(lat > 0 && lng > 0)
        css.insert(posMatrix2[lat-1][lng-1]);

      //north
      if(lat > 0)
        css.insert(posMatrix2[lat-1][lng]);

      //west
      if(lng > 0)
        css.insert(posMatrix2[lat][lng - 1]);

      //self
      css.insert(centerStation);

      //south-west
      if(lat < latRange - 1 && lng > 0)
        css.insert(posMatrix2[lat+1][lng-1]);

      //south
      if(lat < latRange - 1)
        css.insert(posMatrix2[lat+1][lng]);

      //south-east
      if(lat < latRange - 1 && lng < lngRange - 1)
        css.insert(posMatrix2[lat+1][lng+1]);

      //east
      if(lng < lngRange - 1)
        css.insert(posMatrix2[lat][lng+1]);

      //north-east
      if(lat > 0 && lng < lngRange - 1)
        css.insert(posMatrix2[lat-1][lng+1]);
    }
  }

	sort(_stations.begin(), _stations.end(), cmpClimateStationPtrs);
	connection().freeResultSet();
}

ClimateScenario* CLMSimulation::defaultScenario() const
{
	return scenario("A1B");
}

set<const ClimateStation*>
    CLMSimulation::avgClimateStationSet(const ClimateStation* c)
{
  return value(_avgClimateStationsSet, c);
}

YearRange CLMSimulation::availableYearRange()
{
  if(!_yearRange.isValid())
  {
    Lock lock(this);

    if(!_yearRange.isValid())
    {
      connection().select("SELECT min(jahr), max(jahr) "
													"FROM clm20.clm20_data where szenario='A1B' and "
                          "realisierung='1' and dat_id = 1");

      MYSQL_ROW row;
			if((row = connection().getMysqlRow()) != 0)
        _yearRange = snapToRaster(YearRange(atoi(row[0]), atoi(row[1])));
    }
  }

  return _yearRange;
}

//------------------------------------------------------------------------------

ClimateRealization* ClimateScenario::realization(const string& name) const
{
  BOOST_FOREACH(ClimateRealization* r, realizations())
  {
    if(r->name() == name)
      return r;
  }
  return NULL;
}

//------------------------------------------------------------------------------

void ClimateRealization::fillCacheFor(const vector<AvailableClimateData>& acds,
                                      const LatLngCoord& gc,
                                      const Date& startDate,
                                      const Date& endDate)
{
	Lock lock(this);

	const LatLngCoord& cgc = simulation()->getClosestClimateDataGeoCoord(gc);
	vector<Cache>& cs = _geoCoord2cache[cgc];

	if(cs.size() < availableClimateDataSize())
		cs.resize(availableClimateDataSize());

	const ACDV& nicAcds = notInCache(cs, acds, startDate, endDate);
	const vector<ACDV>& cseAcds = commonStartEnd(cs, nicAcds, startDate, endDate);

	vector<ACDV>::const_iterator acdvi;
	for(acdvi = cseAcds.begin(); acdvi != cseAcds.end(); acdvi++)
		updateCaches(cs, *acdvi, cgc, startDate, endDate);
}

ACDV ClimateRealization::notInCache(const vector<Cache>& cs, const ACDV& acds,
                                    const Date& startDate,
                                    const Date& endDate) const
{
	ACDV res;
  for(ACDV::const_iterator acdi = acds.begin(); acdi != acds.end(); acdi++)
  {
		const Cache& c = cs.at(*acdi);
		if(!c.isInitialized() || c.startDate > startDate || c.endDate < endDate)
			res.push_back(*acdi);
	}
	return res;
}

vector<ACDV> ClimateRealization::commonStartEnd(const vector<Cache>& cs,
                                                const ACDV& acds,
                                                const Date& startDate,
                                                const Date& endDate) const
{
	vector<ACDV> res;
	typedef map<Date, vector<ACD> > MDVACD;
	typedef map<Date, MDVACD> MDMDVACD;
	MDMDVACD map;
  for(ACDV::const_iterator acdi = acds.begin(); acdi != acds.end(); acdi++)
  {
		const Cache& c = cs.at(*acdi);
		if(!c.isInitialized())
			map[startDate][endDate].push_back(*acdi);
		else
			map[c.startDate][c.endDate].push_back(*acdi);
	}

	MDMDVACD::const_iterator sdi;
	MDVACD::const_iterator edi;
  for(sdi = map.begin(); sdi != map.end(); sdi++)
    for(edi = sdi->second.begin(); edi != sdi->second.end(); edi++)
			res.push_back(vector<ACD>(edi->second.begin(), edi->second.end()));

	return res;
}

DataAccessor ClimateRealization::
dataAccessorFor(const ACDV& acds, const LatLngCoord& gc,
                const Date& startDate, const Date& endDate)
{
  //cout << "startDate: " << startDate.toString() <<
  //    " endDate: " << endDate.toString() << endl;

  //only try to fill caches if the realizations supports the requested range
  YearRange yr = simulation()->availableYearRange();
  if(yr.fromYear <= int(startDate.year()) && int(endDate.year()) <= yr.toYear)
  {
    fillCacheFor(acds, gc, startDate, endDate);

		Lock lock(this);

    const LatLngCoord& cgc = simulation()->getClosestClimateDataGeoCoord(gc);
    vector<Cache>& cs = _geoCoord2cache[cgc];

    int numberOfValues = startDate.numberOfDaysTo(endDate+1);
    DataAccessor bda(startDate, endDate);

    for(ACDV::const_iterator acdi = acds.begin(); acdi != acds.end(); acdi++)
    {
      Cache& c = cs[*acdi];
      unsigned int o = c.offsetFor(startDate);
      bda.addClimateData(*acdi, vector<double>(c._cache.begin()+o,
                                               c._cache.begin()+o+numberOfValues));
      //bda._data->push_back(vector<double>(c._cache.begin()+o,
      //		c._cache.begin()+o+numberOfValues));
      //bda._acd2dataIndex[int(*acdi)] = bda._data->size() - 1;
    }

    return bda;
  }

  return DataAccessor();
}

unsigned int Cache::getNewOffsetIndexFor(const Tools::Date& start)
{
	int delta = offsetFor(start);
	offsets.push_back(delta < 0 ? 0 : delta);
	return offsets.size() - 1;
}

//! for now we just make the cache grow infinitely
void ClimateRealization::updateCaches(vector<Cache>& cs, ACDV acds,
                                      const LatLngCoord& gc,
                                      const Date& startDate,
                                      const Date& endDate)
{
	const Cache& exampleCache = cs.at(acds.front());

	bool isNewCache = !exampleCache.isInitialized();

	Date sd = startDate, ed = endDate;

  if(!isNewCache)
  {
		//the cache contains already the the whole needed range of data
		if(startDate >= exampleCache.startDate && endDate <= exampleCache.endDate)
			return;

		//we got no sparse vectors, so have to extend the endDate until the
		//existing start date (could potentially be many elements
		//(might have to change)
		if(endDate < exampleCache.startDate || endDate <= exampleCache.endDate)
			ed = exampleCache.startDate - 1;

		if(startDate > exampleCache.endDate || startDate >= exampleCache.startDate)
			sd = exampleCache.endDate + 1;
	}

	//cout << "executing query" << endl;
	map<ACD, vector<double>*> acd2ds = executeQuery(acds, gc, sd, ed);
	map<ACD, vector<double>*>::const_iterator dsi;
  for(dsi = acd2ds.begin(); dsi != acd2ds.end(); dsi++)
  {
		vector<double>* ds = dsi->second;
		Cache& c = cs[dsi->first];
		unsigned int rowCount = ds->size();

		int lowerSlice = isNewCache ? rowCount : sd.numberOfDaysTo(c.startDate);
		int upperSlice = isNewCache ? 0 : c.endDate.numberOfDaysTo(ed);

		//update offsets (not necessary if the cache is new)
    if(!isNewCache)
    {
			int oldOffset = sd.numberOfDaysTo(c.startDate);
			if(oldOffset < 0)
				oldOffset = 0;

			//now update the offsets of all userHandles with the same location
			for(unsigned int i = 0; i < c.offsets.size(); i++)
				c.offsets[i] += oldOffset;
		}

		//prepend lower slice of rows
    if(lowerSlice > 0)
    {
			c._cache.insert(c._cache.begin(), ds->begin(), ds->begin() + lowerSlice);
			c.startDate = sd;

			if(isNewCache)
				c.endDate = ed;
		}
		//append upper slice of rows
    if(upperSlice > 0)
    {
			c._cache.insert(c._cache.end(), ds->end() - upperSlice, ds->end());
			c.endDate = ed;
		}

		//took ownership of data-vector
		delete ds;
	}
}

//------------------------------------------------------------------------------

//!helper functions to access and parse db-result set
namespace
{
	typedef ClimateStation::SL SL;

	//! month = -1 means fkorr value for whole year
  double fkorr(SL sl, int month = 0)
  {
		if(month < 0 || month > 12)
			return 0.0;

		static L lockable;
		static map<SL, map<int, double> > m;
		static bool initialized = false;
		if(!initialized)
		{
			L::Lock lock(lockable);

			if(!initialized)
			{
				SL f = ClimateStation::f;
				m[f][1]=31.6; m[f][2]=33.5; m[f][3]=26.9; m[f][4]=18.3;
				m[f][5]=12.5; m[f][6]=10.4; m[f][7]=10.8; m[f][8]=10.5;
				m[f][9]=12.6; m[f][10]=15.5; m[f][11]=21.8; m[f][12]=26.5;
				m[f][-1]=18.2;

				SL lg = ClimateStation::lg;
				m[lg][1]=23.3; m[lg][2]=24.5; m[lg][3]=20.3; m[lg][4]=15.1;
				m[lg][5]=11.1; m[lg][6]=9.8; m[lg][7]=10.0; m[lg][8]=9.5;
				m[lg][9]=11.5; m[lg][10]=12.7; m[lg][11]=16.8; m[lg][12]=19.8;
				m[lg][-1]=14.6;

				SL mg = ClimateStation::mg;
				m[mg][1]=17.3; m[mg][2]=17.9; m[mg][3]=15.5; m[mg][4]=12.7;
				m[mg][5]=10.1; m[mg][6]=8.8; m[mg][7]=9.1; m[mg][8]=8.5;
				m[mg][9]=10.2; m[mg][10]=11.0; m[mg][11]=13.3; m[mg][12]=15.0;
				m[mg][-1]=12.0;

				SL sg = ClimateStation::sg;
				m[sg][1]=11.5; m[sg][2]=11.8; m[sg][3]=10.7; m[sg][4]=10.0;
				m[sg][5]=8.6; m[sg][6]=7.7; m[sg][7]=8.0; m[sg][8]=7.5;
				m[sg][9]=8.7; m[sg][10]=8.8; m[sg][11]=9.5; m[sg][12]=10.3;
				m[sg][-1]=9.3;

				initialized = true;
			}
		}

		return m[sl][month];
	}

	//kind of precipitation
	enum PArt { Regen = 0, Mischniederschlag = 2, Schnee = 3 };

	//kind of precipitation extended
	enum PArtPlus { rs = 0, rw = 1, mn = Mischniederschlag, s = Schnee };

  PArtPlus createPArtPlus(PArt pa, int month)
  {
    if(pa == Regen)
    {
			if(4 <= month && month <= 9)
				return rs;
			else
				return rw;
		}
		return PArtPlus(pa);
	}

  PArt PArt4tmit(double tmit, bool forSaxony = false)
  {
    if(forSaxony)
    {
      return tmit > 3.0 ?
          Regen : -0.4 <= tmit && tmit <= 3.0 ?
          Mischniederschlag : Schnee;
		}

    return tmit > 3.0 ?
        Regen : -0.7 <= tmit && tmit <= 3.0 ?
        Mischniederschlag : Schnee;
	}

	//! b koefficient
  double bKoeff(SL sl, PArtPlus pap)
  {
		static L lockable;
		static map<SL, map<PArtPlus, double> > m;
		static bool initialized = false;
		if(!initialized)
		{
			L::Lock lock(lockable);

			if(!initialized)
			{
				SL f = ClimateStation::f;
				m[f][rs]=0.345; m[f][rw]=0.34; m[f][mn]=0.535; m[f][s]=0.72;

				SL lg = ClimateStation::lg;
				m[lg][rs]=0.31; m[lg][rw]=0.28; m[lg][mn]=0.39; m[lg][s]=0.51;

				SL mg = ClimateStation::mg;
				m[mg][rs]=0.28; m[mg][rw]=0.24; m[mg][mn]=0.305; m[mg][s]=0.33;

				SL sg = ClimateStation::sg;
				m[sg][rs]=0.245; m[sg][rw]=0.19; m[sg][mn]=0.185; m[sg][s]=0.21;

				initialized = true;
			}
		}

		return m[sl][pap];

	}

	//! epsilon koefficient
  double epsilonKoeff(PArtPlus pap)
  {
		double res = 0;
    switch(pap)
    {
    case rs: res = 0.38; break;
    case rw: res = 0.46; break;
    case mn: res = 0.55; break;
    case s: res = 0.82; break;
		}
		return res;
	}


  struct Fun
  {
		virtual ~Fun(){}
		virtual double operator()(const MYSQL_ROW& row) const = 0;
	};

  struct ParseAsDouble : public Fun
  {
		int _pos;
		ParseAsDouble(int pos) : _pos(pos) {}
		virtual ~ParseAsDouble(){}
    double operator()(const MYSQL_ROW& row) const
    {
			return std::atof(row[_pos]);
		}
	};

  struct CalcStarGlobrad : public Fun
  {
    int _pos; bool _asMJpm2pd;
    CalcStarGlobrad(int pos, bool asMJpm2pd = true) :
        _pos(pos), _asMJpm2pd(asMJpm2pd) {}
		virtual ~CalcStarGlobrad(){}
    double operator()(const MYSQL_ROW& row) const
    {
      //100.0*100.0/1000000.0 -> 1/100
      //double gr = std::atof(row[_pos])*4.1868;
      double gr = std::atof(row[_pos]);
      return _asMJpm2pd ? gr / 100.0 : gr;
		}
	};

  struct CalcWettRegGlobrad : public Fun
  {
		int _posSun, _posYd; double _lat;
		CalcWettRegGlobrad(int posSun, int posYd, double lat)
		: _posSun(posSun), _posYd(posYd), _lat(lat) {}
		virtual ~CalcWettRegGlobrad(){}
    double operator()(const MYSQL_ROW& row) const //[MJ/m²/d]
    {
			return Tools::sunshine2globalRadiation(atoi(row[_posYd]),
			                                       atof(row[_posSun]),
			                                       _lat);
		}
	};

	struct CalcRemoGlobrad : public Fun
	{
		int _posCloudAmount, _posDoy;
		double _lat, _hnn;
		CalcRemoGlobrad(int posCloudAmount, int posDoy, double lat, double heightNN)
			: _posCloudAmount(posCloudAmount),
				_posDoy(posDoy),
				_lat(lat),
				_hnn(heightNN)
		{}
		virtual ~CalcRemoGlobrad(){}
		double operator()(const MYSQL_ROW& row) const //[MJ/m²/d]
		{
			return Tools::cloudAmount2globalRadiation(atoi(row[_posDoy]),
																								atof(row[_posCloudAmount]),
																								_lat, _hnn);
		}
	};

  struct CalcCorrWRAndCLMPrecip : public Fun
  {
		int _posPrecip; int _posTavg; int _posMonth; SL _sl;
		CalcCorrWRAndCLMPrecip(int posPrecip, int posTavg, int posMonth, SL sl)
		: _posPrecip(posPrecip), _posTavg(posTavg), _posMonth(posMonth), _sl(sl) {}
		virtual ~CalcCorrWRAndCLMPrecip(){}
    double operator()(const MYSQL_ROW& row) const
    {
			int month = atoi(row[_posMonth]);
			PArtPlus pap = createPArtPlus(PArt4tmit(atof(row[_posTavg])), month);
			double P = atof(row[_posPrecip]);
			double b = bKoeff(_sl, pap);
			double epsilon = epsilonKoeff(pap);
			return P+b*pow(P, epsilon);
		}
	};
}

//------------------------------------------------------------------------------

DataAccessor StarRealization::
dataAccessorFor(const vector<AvailableClimateData>& acds,
                        const string& stationName,
                        const Date& startDate,
                        const Date& endDate)
{
	return ClimateRealization::
	dataAccessorFor(acds, simulation()->climateStation2geoCoord(stationName),
	                startDate, endDate);
}

map<ACD, vector<double>*>
StarRealization::executeQuery(const ACDV& acds,
                              const LatLngCoord& gc, const Date& startDate,
                              const Date& endDate) const
{
	const ClimateStation& cs = simulation()->geoCoord2climateStation(gc);

	ostringstream query; query << "select ";
	int c = 0;
	vector<Fun*> fs;

  for(ACDV::const_iterator acdi = acds.begin(); acdi != acds.end(); acdi++)
  {
    ACD acd = *acdi;
    switch(acd)
    {
    case Climate::globrad:
      query << availableClimateData2StarDBColName(acd);
      fs.push_back(new CalcStarGlobrad(c++));
      break;
    default:
      query << availableClimateData2StarDBColName(acd);
      fs.push_back(new ParseAsDouble(c++));
		}
		query << (acdi+1 != acds.end() ? ", " : " ");
	}

  string dbDate =
      "concat(jahr, \'-\', "
      "if(mo<10,concat(\'0\',mo),mo), \'-\', "
      "if(tag<10,concat(\'0\',tag),tag))";

	query <<
  "from " << cs.dbName() << " "
  "where " << dbDate << " >= " << startDate.toMysqlString() << " "
  "and " << dbDate << " <= " << endDate.toMysqlString() << " "
  "and not (mo = 2 and tag = 29) "
	"order by jahr, mo, tag";

  //cout << "query: " << query.str() << endl;
  connection().select(query.str().c_str());

	int rowCount = connection().getNumberOfRows();
	map<ACD, vector<double>*> acd2ds;
  BOOST_FOREACH(ACD acd, acds)
  {
    acd2ds[acd] = new vector<double>(rowCount);
  }

	MYSQL_ROW row;
	int count = 0;
	while((row=connection().getMysqlRow())!=0) {
		int c = 0;
    BOOST_FOREACH(ACD acd, acds)
    {
      (*(acd2ds[acd]))[count] = (*(fs.at(c++)))(row);
    }
		count++;
	}

	for(unsigned int i = 0; i < fs.size(); i++)
		delete fs.at(i);

	return acd2ds;
}

//------------------------------------------------------------------------------

DataAccessor Star2Realization::
dataAccessorFor(const vector<AvailableClimateData>& acds,
                        const string& stationName,
                        const Date& startDate,
                        const Date& endDate)
{
  return ClimateRealization::
  dataAccessorFor(acds, simulation()->climateStation2geoCoord(stationName),
                  startDate, endDate);
}

map<ACD, vector<double>*>
Star2Realization::executeQuery(const ACDV& acds,
                               const LatLngCoord& gc, const Date& startDate,
                               const Date& endDate) const
{
  const ClimateStation& cs = simulation()->geoCoord2climateStation(gc);

  ostringstream query, query2; query << "select ";
  int c = 0;
  vector<Fun*> fs;

  for(ACDV::const_iterator acdi = acds.begin(); acdi != acds.end(); acdi++)
  {
    ACD acd = *acdi;
    switch(acd)
    {
    case Climate::globrad:
      query << availableClimateData2StarDBColName(acd);
      fs.push_back(new CalcStarGlobrad(c++));
      break;
    default:
      query << availableClimateData2StarDBColName(acd);
      fs.push_back(new ParseAsDouble(c++));
    }
    query << ", ";//(acdi+1 != acds.end() ? ", " : " ");
  }
  query << " tag as _tag, mo as _mo, jahr as _jahr ";

  string dbDate =
      "concat(jahr, \'-\', "
      "if(mo<10,concat(\'0\',mo),mo), \'-\', "
      "if(tag<10,concat(\'0\',tag),tag))";

  query2 << query.str();

	query << "from " << scenario()->id() << "_" << id() << " "
					 "where " << dbDate << " >= " << startDate.toMysqlString() << " "
					 "and " << dbDate << " <= " << endDate.toMysqlString() << " "
					 "and not (mo = 2 and tag = 29) "
					 "and id = " << cs.id();

	query2 << "from refzen "
						"where " << dbDate << " >= " << startDate.toMysqlString() << " "
						"and " << dbDate << " <= " << endDate.toMysqlString() << " "
						"and not (mo = 2 and tag = 29) "
						"and id = " << cs.id();

  query << " union " << query2.str() << " "
					 "order by _jahr, _mo, _tag";

  //cout << "query: " << query.str() << endl;
  connection().select(query.str().c_str());

  int rowCount = connection().getNumberOfRows();
  map<ACD, vector<double>*> acd2ds;
  BOOST_FOREACH(ACD acd, acds)
  {
    acd2ds[acd] = new vector<double>(rowCount);
  }

  MYSQL_ROW row;
  int count = 0;
	while((row=connection().getMysqlRow())!=0) {
    int c = 0;
    BOOST_FOREACH(ACD acd, acds)
    {
      (*(acd2ds[acd]))[count] = (*(fs.at(c++)))(row);
    }
    count++;
  }

  for(unsigned int i = 0; i < fs.size(); i++)
    delete fs.at(i);

  return acd2ds;
}

//------------------------------------------------------------------------------

DataAccessor Star2MeasuredDataRealization::
dataAccessorFor(const vector<AvailableClimateData>& acds,
                        const string& stationName,
                        const Date& startDate,
                        const Date& endDate)
{
  return ClimateRealization::
  dataAccessorFor(acds, simulation()->climateStation2geoCoord(stationName),
                  startDate, endDate);
}

map<ACD, vector<double>*>
    Star2MeasuredDataRealization::executeQuery(const ACDV& acds,
                                               const LatLngCoord& gc,
                                               const Date& startDate,
                                               const Date& endDate) const
{
  const ClimateStation& cs = simulation()->geoCoord2climateStation(gc);

  ostringstream query; query << "select ";
  int c = 0;
  vector<Fun*> fs;

  for(ACDV::const_iterator acdi = acds.begin(); acdi != acds.end(); acdi++)
  {
    ACD acd = *acdi;
    switch(acd)
    {
    case Climate::globrad:
      query << availableClimateData2StarDBColName(acd);
      fs.push_back(new CalcStarGlobrad(c++));
      break;
    default:
      query << availableClimateData2StarDBColName(acd);
      fs.push_back(new ParseAsDouble(c++));
    }
    query << (acdi+1 != acds.end() ? ", " : " ");
  }

  string dbDate =
      "concat(jahr, \'-\', "
      "if(mo<10,concat(\'0\',mo),mo), \'-\', "
      "if(tag<10,concat(\'0\',tag),tag))";

	query << "from " << cs.dbName() << " "
					 "where " << dbDate << " >= " << startDate.toMysqlString() << " "
					 "and " << dbDate << " <= " << endDate.toMysqlString() << " "
					 "and not (mo = 2 and tag = 29) "
					 "and id = " << cs.id() << " "
					 "order by jahr, mo, tag";

  //cout << "query: " << query.str() << endl;

  connection().select(query.str().c_str());

  int rowCount = connection().getNumberOfRows();
  map<ACD, vector<double>*> acd2ds;
  BOOST_FOREACH(ACD acd, acds)
  {
    acd2ds[acd] = new vector<double>(rowCount);
  }

  MYSQL_ROW row;
  int count = 0;
	while((row=connection().getMysqlRow())!=0) {
    int c = 0;
    BOOST_FOREACH(ACD acd, acds)
    {
      (*(acd2ds[acd]))[count] = (*(fs.at(c++)))(row);
    }
    count++;
  }

  for(unsigned int i = 0; i < fs.size(); i++)
    delete fs.at(i);

  return acd2ds;
}

//------------------------------------------------------------------------------

DataAccessor DDClimateDataServerRealization::
dataAccessorFor(const vector<AvailableClimateData>& acds,
								const string& stationName, const Date& startDate,
								const Date& endDate)
{
	return ClimateRealization::
			dataAccessorFor(acds, simulation()->climateStation2geoCoord(stationName),
											startDate, endDate);
}

map<ACD, vector<double>*>
DDClimateDataServerRealization::executeQuery(const ACDV& acds,
																						 const LatLngCoord& gc,
																						 const Date& startDate,
																						 const Date& endDate) const
{
	const ClimateStation& cs = simulation()->geoCoord2climateStation(gc);

	string dbDate =
			"concat(jahr, \'-\', "
			"if(monat<10,concat(\'0\',monat),monat), \'-\', "
			"if(tag<10,concat(\'0\',tag),tag))";

	ostringstream query; query << "select ";
	int c = 0;
	vector<Fun*> fs;
	for(ACDV::const_iterator acdi = acds.begin(); acdi != acds.end(); acdi++)
	{
		ACD acd = *acdi;
		switch(acd)
		{
		case Climate::globrad:
		{
			if(_setupData.simulationId() == "remo")
			{
				query << "nn, dayofyear(" << dbDate << ") as dy";
				int posCloudAmount = c++; int posDoy = c++;
				fs.push_back(new CalcRemoGlobrad(posCloudAmount, posDoy,
																				 cs.geoCoord().lat, cs.nn()));
			}
			else
			{
				query << "sd, dayofyear(" << dbDate << ") as dy";
				int posSun = c++; int posYd = c++;
				fs.push_back(new CalcWettRegGlobrad(posSun, posYd, cs.geoCoord().lat));
			}
			break;
		}
		case Climate::precip:
		{
			if(_setupData.simulationId() == "remo")
			{
				query << "rr_drift";
				fs.push_back(new ParseAsDouble(c++));
			}
			else
			{
				query << "rr, tm, monat";
				int posPrecip = c++; int posTavg = c++; int posMonat = c++;
				fs.push_back(new CalcCorrWRAndCLMPrecip(posPrecip, posTavg, posMonat,
																								cs.sl()));
			}
			break;
		}
		case Climate::sunhours:
		{
			if(_setupData.simulationId() == "remo")
				query << "0 as sd";
			else
				query << availableClimateData2CLMDBColName(acd);
			fs.push_back(new ParseAsDouble(c++));
			break;
		}
		default:
			query << availableClimateData2CLMDBColName(acd);
			fs.push_back(new ParseAsDouble(c++));
			break;
		}
		query << (acdi+1 != acds.end() ? ", " : " ");
	}
	query << "from "
				<< _setupData.dataDbName() << "." << _setupData.dataTableName() << " "
					 "where szenario = '" << _scenario->name() << "' "
					 "and realisierung = '" << id() << "' "
					 "and dat_id = " << cs.dbName() << " "
					 "and " << dbDate << " >= " << startDate.toMysqlString() << " "
					 "and " << dbDate << " <= " << endDate.toMysqlString() << " "
					 "and not (monat = 2 and tag = 29) "
					 "order by jahr, monat, tag";

	//cout << "select: " << query.str() << endl;
	connection().select(query.str().c_str());

	int rowCount = connection().getNumberOfRows();
	map<ACD, vector<double>*> acd2ds;
	BOOST_FOREACH(ACD acd, acds)
	{
		acd2ds[acd] = new vector<double>(rowCount);
	}

	MYSQL_ROW row;
	int count = 0;
	while((row=connection().getMysqlRow())!=0)
	{
		int c = 0;
		BOOST_FOREACH(ACD acd, acds)
		{
			(*(acd2ds[acd]))[count] = (*(fs.at(c++)))(row);
		}
		count++;
	}

	for(unsigned int i = 0; i < fs.size(); i++)
		delete fs.at(i);

	return acd2ds;
}

//------------------------------------------------------------------------------

DataAccessor CLMRealization::
dataAccessorFor(const vector<AvailableClimateData>& acds,
                const string& stationName, const Date& startDate,
                const Date& endDate)
{
	return ClimateRealization::
	dataAccessorFor(acds, simulation()->climateStation2geoCoord(stationName),
	                startDate, endDate);
}

map<ACD, vector<double>*>
CLMRealization::executeQuery(const ACDV& acds,
                             const LatLngCoord& gc,
                             const Date& startDate,
                             const Date& endDate) const
{
	const ClimateStation& cs = simulation()->geoCoord2climateStation(gc);

	string dbDate =
      "concat(jahr, \'-\', "
      "if(monat<10,concat(\'0\',monat),monat), \'-\', "
      "if(tag<10,concat(\'0\',tag),tag))";

	ostringstream query; query << "select ";
	int c = 0;
	vector<Fun*> fs;
  for(ACDV::const_iterator acdi = acds.begin(); acdi != acds.end(); acdi++)
  {
    ACD acd = *acdi;
    switch(acd)
    {
    case Climate::globrad:
      {
        query << "avg(sd), avg(dayofyear(" << dbDate << ")) as dy";
				int posSun = c++; int posYd = c++;
				fs.push_back(new CalcWettRegGlobrad(posSun, posYd, cs.geoCoord().lat));
				break;
			}
    case day:
    case month:
    case year:
      query << availableClimateData2CLMDBColName(acd);
      fs.push_back(new ParseAsDouble(c++));
      break;
    case precip:
      query << "avg(if("
          << availableClimateData2CLMDBColName(precip) << " < -998, "
          << availableClimateData2CLMDBColName(precipOrig) << ", "
          << availableClimateData2CLMDBColName(precip) << "))";
      fs.push_back(new ParseAsDouble(c++));
      break;
    default:
      query << "avg(" << availableClimateData2CLMDBColName(acd) << ")";
      fs.push_back(new ParseAsDouble(c++));
      break;
    }
    query << (acdi+1 != acds.end() ? ", " : " ");
	}
  CLMSimulation* sim = static_cast<CLMSimulation*>(simulation());
  ostringstream stationList;
  stationList << "(";
  BOOST_FOREACH(const ClimateStation* c, sim->avgClimateStationSet(&cs))
  {
    stationList << c->dbName() << ",";
  }
  string sl = stationList.str();
  sl.at(sl.length()-1) = ')';

	query <<
					 "from clm20.clm20_data "
					 "where szenario = '" << _scenario->name() << "' "
					 "and realisierung = '" << _realizationNo << "' "
					 "and dat_id in " << sl << " "//cs.dbName() << " "
					 "and " << dbDate << " >= " << startDate.toMysqlString() << " "
					 "and " << dbDate << " <= " << endDate.toMysqlString() << " "
					 "and not (monat = 2 and tag = 29) "
					 "group by szenario, realisierung, tag, monat, jahr "
					 "order by jahr, monat, tag";

  //cout << "select: " << query.str() << endl;
	connection().select(query.str().c_str());

	int rowCount = connection().getNumberOfRows();
	map<ACD, vector<double>*> acd2ds;
  BOOST_FOREACH(ACD acd, acds)
  {
    acd2ds[acd] = new vector<double>(rowCount);
  }

	MYSQL_ROW row;
	int count = 0;
	while((row=connection().getMysqlRow())!=0)
  {
		int c = 0;
    BOOST_FOREACH(ACD acd, acds)
    {
      (*(acd2ds[acd]))[count] = (*(fs.at(c++)))(row);
    }
		count++;
	}

	for(unsigned int i = 0; i < fs.size(); i++)
		delete fs.at(i);

	return acd2ds;
}

//------------------------------------------------------------------------------

ClimateDataManager& Climate::climateDataManager()
{
	static ClimateDataManager cdm;
  static L lockable;
  static bool initialized = false;
  if(!initialized)
  {
    L::Lock lock(lockable);

    if(!initialized)
    {
			const Names2Values& n2vs =
          Db::dbConnectionParameters().values("active-climate-db-schemas");
			set<string> s;
      transform(n2vs.begin(), n2vs.end(), inserter(s, s.begin()),
								boost::lambda::bind(&Names2Values::value_type::first, _1));
			cdm.loadAvailableSimulations(s);
      initialized = true;
    }
  }

	return cdm;
}

void ClimateDataManager::loadAvailableSimulations(set<string> ass)
{
	using namespace Db;
	if(ass.find("clm20-9") != ass.end())
		_simulations.push_back(new CLMSimulation(toMysqlDB(newConnection("clm20-9"))));
	if(ass.find("clm20") != ass.end())
		_simulations.push_back(newDDClm20());
  if(ass.find("star") != ass.end())
		_simulations.push_back(new StarSimulation(toMysqlDB(newConnection("star"))));
  if(ass.find("star2") != ass.end())
  {
		_simulations.push_back(new Star2Simulation(toMysqlDB(newConnection("star2"))));
		_simulations.push_back(new Star2MeasuredDataSimulation(toMysqlDB(newConnection("star2"))));
  }
	if(ass.find("wettreg2006") != ass.end())
	{
		//put in front to designate the default
		_simulations.insert(_simulations.begin(), newDDWettReg2006());
	}
	if(ass.find("wettreg2010") != ass.end())
	{
		_simulations.push_back(newDDWettReg2010());
	}
	if(ass.find("remo") != ass.end())
		_simulations.push_back(newDDRemo());
	if(ass.find("werex4") != ass.end())
		_simulations.push_back(newDDWerex4());
	if(ass.find("echam5") != ass.end())
		_simulations.push_back(newDDEcham5());
	if(ass.find("hrm3") != ass.end())
	{
		_simulations.push_back(newDDHrm3(YearRange(1971, 2000)));
		_simulations.push_back(newDDHrm3(YearRange(2041, 2070)));
	}
	if(ass.find("cru") != ass.end())
		_simulations.push_back(newDDCru());
}

ClimateDataManager::~ClimateDataManager()
{
	BOOST_FOREACH(ClimateSimulation* sim, _simulations)
	{
		delete sim;
	}
}

vector<ClimateSimulation*> ClimateDataManager::allClimateSimulations() const
{
	return _simulations;
	vector<ClimateSimulation*> css;
}

ClimateSimulation* ClimateDataManager::defaultSimulation() const
{
	return _simulations.empty() ? NULL : _simulations.front();
}

//------------------------------------------------------------------------------

DDClimateDataServerSimulation* Climate::newDDWettReg2006(string userRs)
{
	DDServerSetup setup("wettreg2006", "WettReg2006", "header", "wettreg_stolist",
											"wettreg2006", "wettreg_data", "wettreg_fehler_regklam");
	setup._scenarioIds.push_back("A1B");
	setup._scenarioIds.push_back("A2");
	setup._scenarioIds.push_back("B1");

	string rs = userRs.empty() ? Db::dbConnectionParameters().value("used-realizations", "wettreg2006", "tro_a, nor_a, feu_a") : userRs;
	vector<string> vsr = Tools::splitString(rs, ", ");
	BOOST_FOREACH(string s, vsr) { setup._realizationIds.push_back(s); }

	return new DDClimateDataServerSimulation(setup, Db::toMysqlDB(Db::newConnection("wettreg2006")));
}

DDClimateDataServerSimulation* Climate::newDDWettReg2010(string userRs)
{
	DDServerSetup setup("wettreg2010", "WettReg2010", "header", "wettreg2010_stolist",
											"wettreg2010", "wettreg2010_data");
	setup._scenarioIds.push_back("A1B");
	setup._scenarioIds.push_back("B1");
	setup._scenarioIds.push_back("A2");

	string rs = userRs.empty() ? Db::dbConnectionParameters().value("used-realizations", "wettreg2010", "00, 55, 99") : userRs;
	vector<string> vsr = Tools::splitString(rs, ", ");
	BOOST_FOREACH(string s, vsr) { setup._realizationIds.push_back(s); }

	return new DDClimateDataServerSimulation(setup, Db::toMysqlDB(Db::newConnection("wettreg2010")));
}

DDClimateDataServerSimulation* Climate::newDDRemo()
{
	DDServerSetup setup("remo", "REMO", "header_remo", "remo_stolist",
											"remo", "remo_data", "remo_fehler_regklam");
	setup._scenarioIds.push_back("A1B");
	setup._scenarioIds.push_back("B1");
	setup._realizationIds.push_back("1");
	return new DDClimateDataServerSimulation(setup, Db::toMysqlDB(Db::newConnection("remo")));
}

DDClimateDataServerSimulation* Climate::newDDWerex4(string userRs)
{
	DDServerSetup setup("werex4", "WEREX4", "header", "werex4_stolist",
											"werex4", "werex4_data", "werex4_fehler_regklam");
	setup._scenarioIds.push_back("A1B");
	setup._scenarioIds.push_back("A2");
	setup._scenarioIds.push_back("B1");

	string rs = userRs.empty() ? Db::dbConnectionParameters().value("used-realizations", "werex4", "tro, nor, feu") : userRs;
	vector<string> vsr = Tools::splitString(rs, ", ");
	BOOST_FOREACH(string s, vsr) { setup._realizationIds.push_back(s); }

	return new DDClimateDataServerSimulation(setup, Db::toMysqlDB(Db::newConnection("werex4")));
}

DDClimateDataServerSimulation* Climate::newDDClm20(string userRs)
{
	DDServerSetup setup("clm20", "CLM20", "header_clm20", "clm20_stolist",
											"clm20", "clm20_data", "clm20_fehler_regklam");
	setup._scenarioIds.push_back("A1B");
	setup._scenarioIds.push_back("B1");

	string rs = userRs.empty() ? Db::dbConnectionParameters().value("used-realizations", "clm20", "1, 2") : userRs;
	vector<string> vsr = Tools::splitString(rs, ", ");
	BOOST_FOREACH(string s, vsr) { setup._realizationIds.push_back(s); }

	return new DDClimateDataServerSimulation(setup, Db::toMysqlDB(Db::newConnection("clm20")));
}

DDClimateDataServerSimulation* Climate::newDDEcham5(string userRs)
{
	DDServerSetup setup("echam5", "ECHAM5", "header_echam5", "echam5_stolist",
											"project_mexiko", "echam5_data", string(), "project_mexiko");
	setup._scenarioIds.push_back("A1B");

	string rs = userRs.empty() ? Db::dbConnectionParameters().value("used-realizations", "echam5", "1") : userRs;
	vector<string> vsr = Tools::splitString(rs, ", ");
	BOOST_FOREACH(string s, vsr) { setup._realizationIds.push_back(s); }

	return new DDClimateDataServerSimulation(setup, Db::toMysqlDB(Db::newConnection("echam5")));
}

DDClimateDataServerSimulation* Climate::newDDHrm3(YearRange yr, string userRs)
{
	ostringstream ss;
	ss << "HRM3-" << yr.fromYear << "/" << yr.toYear;
	DDServerSetup setup("hrm3", ss.str(), "header_hrm3", "hrm3_stolist",
											"project_mexiko", "hrm3_data", string(), "project_mexiko");
	setup.yearRange = yr;
	setup._scenarioIds.push_back("A2");

	string rs = userRs.empty() ? Db::dbConnectionParameters().value("used-realizations", "hrm3", "1") : userRs;
	vector<string> vsr = Tools::splitString(rs, ", ");
	BOOST_FOREACH(string s, vsr) { setup._realizationIds.push_back(s); }

	return new DDClimateDataServerSimulation(setup, Db::toMysqlDB(Db::newConnection("hrm3")));
}


DDClimateDataServerSimulation* Climate::newDDCru(string userRs)
{
	DDServerSetup setup("cru", "CRU", "header_cru", "cru_stolist",
											"project_mexiko", "cru_data", string(), "project_mexiko");
	setup._scenarioIds.push_back("CRU");

	string rs = userRs.empty() ? Db::dbConnectionParameters().value("used-realizations", "cru", "3.1") : userRs;
	vector<string> vsr = Tools::splitString(rs, ", ");
	BOOST_FOREACH(string s, vsr) { setup._realizationIds.push_back(s); }

	return new DDClimateDataServerSimulation(setup, Db::toMysqlDB(Db::newConnection("cru")));
}


//------------------------------------------------------------------------------

void Climate::testClimate()
{
	//typedef Climate<AllDBData> Climate;

	vector<string> stations;
	/*
	typedef StarClimate<DBData> StarClimate;
 	StarClimate climate;
	stations.push_back("drd_");
	stations.push_back("ang_");
	stations.push_back("pre_");
	stations.push_back("mub_");
	stations.push_back("gri_");
	stations.push_back("bro_");
	//*/

	/*
	typedef WettRegClimate<AllDBData> WettRegClimate;
	WettRegClimate climate(WettRegClimate::Scenario::A1B(), WettRegClimate::dry);
	stations.push_back("DRESDEN");
	stations.push_back("ANGERMUENDE");
	stations.push_back("MUENCHEBERG");
	stations.push_back("BROCKEN");
	//*/

	/*
	vector<string>::const_iterator sit;
	for(sit = stations.begin(); sit != stations.end(); sit++){
		string station(*sit);
		cout << "station: " << station << endl;

		Date s0(1,1,2000); Date e0(31,1,2000);
		cout << "caching range: " << s0.toMysqlString()
		<< " - " << e0.toMysqlString() << endl;
		Climate::LWDataAccessor da0 = climate.dataAccessorFor(station, s0, e0);
		const Climate::Cache& c = da0._cache;
		cout << "c.startDate: " << c.startDate.toMysqlString()
		<< " c.endDate: " << c.endDate.toMysqlString() << endl;
		assert(c.startDate == s0 && c.endDate == e0);
		cout << "c.size(): " << c.size() << endl;
		assert(c.size() == 31);
		cout << "da0.offset: " << da0.offset() << endl;
		assert(da0.offset() == 0);

		Date s1(1,1,2002); Date e1(31,12,2002);
		cout << "caching range: " << s1.toMysqlString()
		<< " - " << e1.toMysqlString() << endl;
		Climate::LWDataAccessor da1 = climate.dataAccessorFor(station, s1, e1);
		cout << "c.startDate: " << c.startDate.toMysqlString()
		<< " c.endDate: " << c.endDate.toMysqlString() << endl;
		assert(c.startDate == s0 && c.endDate == e1);
		cout << "c.size(): " << c.size() << endl;
		assert(c.size() == 31 + (365 - 31) + 365 + 365);
		cout << "da1.offset: " << da1.offset() << endl;
		assert(da1.offset() == 31 + (365 - 31) + 365);
		cout << "da0.offset: " << da0.offset() << endl;
		assert(da0.offset() == 0);

		Date s2(20,12,1999); Date e2(25,12,1999);
		cout << "caching range: " << s2.toMysqlString()
		<< " - " << e2.toMysqlString() << endl;
		Climate::LWDataAccessor da2 = climate.dataAccessorFor(station, s2, e2);
		cout << "c.startDate: " << c.startDate.toMysqlString()
		<< " c.endDate: " << c.endDate.toMysqlString() << endl;
		assert(c.startDate == s2 && c.endDate == e1);
		cout << "c.size(): " << c.size() << endl;
		assert(c.size() == 6 + 6 + 31 + (365 - 31) + 365 + 365);
		cout << "da2.offset: " << da2.offset() << endl;
		assert(da2.offset() == 0);
		cout << "da1.offset: " << da1.offset() << endl;
		assert(da1.offset() == 6 + 6 + 31 + (365 - 31) + 365);
		cout << "da0.offset: " << da0.offset() << endl;
		assert(da0.offset() == 6 + 6);

		Date s3(10,12,1999); Date e3(25,12,2002);
		cout << "caching range: " << s3.toMysqlString()
		<< " - " << e3.toMysqlString() << endl;
		Climate::LWDataAccessor da3 = climate.dataAccessorFor(station, s3, e3);
		cout << "c.startDate: " << c.startDate.toMysqlString()
		<< " c.endDate: " << c.endDate.toMysqlString() << endl;
		assert(c.startDate == s3 && c.endDate == e1);
		cout << "c.size(): " << c.size() << endl;
		assert(c.size() == 10 + 6 + 6 + 31 + (365 - 31) + 365 + 365);
		cout << "da3.offset: " << da3.offset() << endl;
		assert(da3.offset() == 0);
		cout << "da2.offset: " << da2.offset() << endl;
		assert(da2.offset() == 10);
		cout << "da1.offset: " << da1.offset() << endl;
		assert(da1.offset() == 10 + 6 + 6 + 31 + (365 - 31) + 365);
		cout << "da0.offset: " << da0.offset() << endl;
		assert(da0.offset() == 10 + 6 + 6);

		Date s4(1,1,2001); Date e4(15,2,2003);
		cout << "caching range: " << s4.toMysqlString()
		<< " - " << e4.toMysqlString() << endl;
		Climate::LWDataAccessor da4 = climate.dataAccessorFor(station, s4, e4);
		cout << "c.startDate: " << c.startDate.toMysqlString()
		<< " c.endDate: " << c.endDate.toMysqlString() << endl;
		assert(c.startDate == s3 && c.endDate == e4);
		cout << "c.size(): " << c.size() << endl;
		assert(c.size() == 10 + 6 + 6 + 31 + (365 - 31) + 365 + 365 + 46);
		cout << "da4.offset: " << da4.offset() << endl;
		assert(da4.offset() == 10 + 6 + 6 + 365);
		cout << "da3.offset: " << da3.offset() << endl;
		assert(da3.offset() == 0);
		cout << "da2.offset: " << da2.offset() << endl;
		assert(da2.offset() == 10);
		cout << "da1.offset: " << da1.offset() << endl;
		assert(da1.offset() == 10 + 6 + 6 + 31 + (365 - 31) + 365);
		cout << "da0.offset: " << da0.offset() << endl;
		assert(da0.offset() == 10 + 6 + 6);

		Date s5(30,6,2001); Date e5(14,12,2002);
		cout << "caching range: " << s5.toMysqlString()
		<< " - " << e5.toMysqlString() << endl;
		Climate::LWDataAccessor da5 = climate.dataAccessorFor(station, s5, e5);
		cout << "c.startDate: " << c.startDate.toMysqlString()
		<< " c.endDate: " << c.endDate.toMysqlString() << endl;
		assert(c.startDate == s3 && c.endDate == e4);
		cout << "c.size(): " << c.size() << endl;
		assert(c.size() == 10 + 6 + 6 + 31 + (365 - 31) + 365 + 365 + 46);
		cout << "da5.offset: " << da5.offset() << endl;
		assert(da5.offset() == 10 + 6 + 6 + 365 + 31 + 28 + 31 + 30 + 31 + 29);
		cout << "da4.offset: " << da4.offset() << endl;
		assert(da4.offset() == 10 + 6 + 6 + 365);
		cout << "da3.offset: " << da3.offset() << endl;
		assert(da3.offset() == 0);
		cout << "da2.offset: " << da2.offset() << endl;
		assert(da2.offset() == 10);
		cout << "da1.offset: " << da1.offset() << endl;
		assert(da1.offset() == 10 + 6 + 6 + 31 + (365 - 31) + 365);
		cout << "da0.offset: " << da0.offset() << endl;
		assert(da0.offset() == 10 + 6 + 6);
	}
	*/
}
