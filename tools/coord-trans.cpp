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
#include <mutex>

#include "proj_api.h"

#ifndef NO_DB
#include "db/abstract-db-connections.h"
#endif
#include "coord-trans.h"
#include "tools/helper.h"

using namespace std;
using namespace Tools;
using namespace Db;

/*
string Tools::coordinateSystemToString(CoordinateSystem cs)
{
	switch(cs)
	{
	case GK5_EPSG31469: return "GK5_EPSG31469";
	case UTM21S_EPSG32721: return "UTM21S_EPSG32721";
	case LatLng_EPSG4326: return "LatLng_EPSG4326";
	case UndefinedCoordinateSystem: return "undefined coordinate system";
	default: ;
	}
	return "unknown coordinate system";
}
*/

/*
string Tools::coordinateSystemToShortString(CoordinateSystem cs)
{
	switch(cs)
	{
	case GK5_EPSG31469: return "GK5";
	case UTM21S_EPSG32721: return "UTM21S";
  case UTM32N_EPSG25832: return "UTM32N";
	case LatLng_EPSG4326: return "LatLng";
	case UndefinedCoordinateSystem: return "undefined";
	default: ;
	}
	return "unknown";
}
*/

string Tools::coordinateSystemToString(CoordinateSystem cs)
{
  if(cs.data)
    return cs.data->name;
  return string();
}

string Tools::coordinateSystemToShortString(CoordinateSystem cs)
{
  if(cs.data)
    return cs.data->shortName;
  return string();
}

CoordinateSystem Tools::shortStringToCoordinateSystem(string cs, CoordinateSystem def)
{
  static mutex lockable;

  typedef map<string, CoordinateSystem> M;
  static bool initialized = false;
  static M ss2cs;

  if (!initialized)
  {
    lock_guard<mutex> lock(lockable);

    if (!initialized)
    {
      DBPtr con(newConnection("coord-trans"));
      DBRow row;

      string query =
          "select id, name, short_name, source_conversion_factor, target_conversion_factor,  switch_2d_coordinates, params "
          "from proj4_conversion_params "
          "order by id";

      con->select(query.c_str());

      while (!(row = con->getRow()).empty())
      {
        int id = satoi(row[0]);

        auto csd = CoordinateSystemDataPtr(new CoordinateSystemData);
        csd->name = row[1];
        csd->shortName = row[2];

        CoordConversionParams ccps;
        ccps.sourceConversionFactor = satof(row[3]);
        ccps.targetConversionFactor = satof(row[4]);
        ccps.switch2DCoordinates = satob(row[5]);
        ccps.projectionParams = row[6];

        csd->proj4Params = ccps;

        ss2cs[Tools::toLower(csd->shortName)] = CoordinateSystem(id, csd);
      }

      initialized = true;
    }
  }

  M::const_iterator ci = ss2cs.find(Tools::toLower(cs));
  return ci != ss2cs.end() ? ci->second : def;
}

/*
CoordConversionParams Tools::coordConversionParams(CoordinateSystem cs)
{
	CoordConversionParams ccp;
	switch(cs)
	{
	case GK5_EPSG31469:
		ccp.sourceConversionFactor = 1.0;
		ccp.targetConversionFactor = 1.0;
		ccp.switch2DCoordinates = false;
    ccp.projectionParams =
        "+proj=tmerc +units=m +datum=potsdam "
        "+k=1 +lat_0=0 +lon_0=15d +x_0=5500000 "
        "+y_0=0 +ellps=bessel +towgs84=606.0,23.0,413.0";
		break;
	case UTM21S_EPSG32721:
		ccp.sourceConversionFactor = 1.0;
		ccp.targetConversionFactor = 1.0;
		ccp.switch2DCoordinates = false;
    ccp.projectionParams =
        "+proj=utm +zone=21 +south +ellps=WGS84 "
        "+datum=WGS84 +units=m +no_defs";
    break;
  case UTM32N_EPSG25832:
    ccp.sourceConversionFactor = 1.0;
    ccp.targetConversionFactor = 1.0;
    ccp.switch2DCoordinates = false;
    ccp.projectionParams =
        "+proj=utm +zone=32 +ellps=GRS80 "
        "+towgs84=0,0,0,0,0,0,0 +units=m +no_defs";
        //"+x_0=32000000 "
    break;

	case LatLng_EPSG4326:
		ccp.sourceConversionFactor = .0174532925199432958; //DEG to RAD
		ccp.targetConversionFactor = 57.29577951308232; //RAD to DEG
		ccp.switch2DCoordinates = true; //because conversion return long lat
		ccp.projectionParams = "+proj=longlat +ellps=WGS84 +datum=WGS84";
		break;
	case UndefinedCoordinateSystem:
	default: ;
		}
	return ccp;
}
*/

/*
CoordConversionParams Tools::GK5Params()
{
	CoordConversionParams ccp;
	ccp.sourceConversionFactor = 1.0;
	ccp.targetConversionFactor = 1.0;
	ccp.switch2DCoordinates = false;
	ccp.projectionParams = "+proj=tmerc +units=m +datum=potsdam "
												 "+k=1 +lat_0=0 +lon_0=15d +x_0=5500000 "
												 "+y_0=0 +ellps=bessel +towgs84=606.0,23.0,413.0";
	return ccp;
}

//UTM21S EPSG32721
CoordConversionParams Tools::UTM21SParams()
{
	CoordConversionParams ccp;
	ccp.sourceConversionFactor = 1.0;
	ccp.targetConversionFactor = 1.0;
	ccp.switch2DCoordinates = false;
	ccp.projectionParams = "+proj=utm +zone=21 +south +ellps=WGS84 "
												 "+datum=WGS84 +units=m +no_defs";
	return ccp;
}


//LatLng EPSG4326
CoordConversionParams Tools::LatLngParams()
{
	CoordConversionParams ccp;
	ccp.sourceConversionFactor = .0174532925199432958; //DEG to RAD
	ccp.targetConversionFactor = 57.29577951308232; //RAD to DEG
	ccp.switch2DCoordinates = true; //because conversion return long lat
	ccp.projectionParams = "+proj=longlat +ellps=WGS84 +datum=WGS84";
	return ccp;
}
*/

string RectCoord::toString() const {
	ostringstream s; s << "r: " << r << " h: " << h;
	return s.str();
}

//------------------------------------------------------------------------------

const double LatLngCoord::eps = 0.000001;

CoordinateSystem LatLngCoord::latLngCoordinateSystem()
{
  static mutex lockable;

  static bool initialized = false;
  static CoordinateSystem llc;

  if (!initialized)
  {
    lock_guard<mutex> lock(lockable);

    if (!initialized)
    {
      llc = shortStringToCoordinateSystem("LatLng");
      initialized = true;
    }
  }

  return llc;
}

string LatLngCoord::toString() const
{
	ostringstream s;
	s << "lat: " << lat << " lng: " << lng;
	return s.str();
}

std::string LatLngCoord::toCanonicalString(string lb, string f, string rb) const
{
	ostringstream s;
	s << lb << lat << f << lng << rb;
	return s.str();
}

bool LatLngCoord::operator==(const LatLngCoord& other) const
{
  return std::abs(lat-other.lat) < eps && std::abs(lng-other.lng) < eps;
}

bool LatLngCoord::operator<(const LatLngCoord & other) const
{
  return lat < other.lat ? true : lat == other.lat ? lng < other.lng : false;
}

LatLngCoord LatLngCoord::operator+(const LatLngCoord& other) const
{
	return LatLngCoord(lat + other.lat, lng + other.lng);
}

void LatLngCoord::operator+=(const LatLngCoord& other)
{
	lat += other.lat;
	lng += other.lng;
}

LatLngCoord LatLngCoord::operator*(double sf) const
{
	return LatLngCoord(lat*sf, lng*sf);
}

double LatLngCoord::distanceTo(const LatLngCoord & other) const
{
  return sqrt(((lat - other.lat)*(lat - other.lat))
              +((lng - other.lng)*(lng - other.lng)));
}

//------------------------------------------------------------------------------

/*
vector<RECTCoord> Tools::latLng2RC(const vector<LatLngCoord>& lls)
{
	if(lls.empty())
		return vector<RECTCoord>();

	const char* rcConfig = "+proj=tmerc +units=m +datum=potsdam "
		"+k=1 +lat_0=0 +lon_0=15d +x_0=5500000 "
		"+y_0=0 +ellps=bessel +towgs84=606.0,23.0,413.0";
	//->"+proj=tmerc +units=m +datum=potsdam "
	//->"+k=1 +lat_0=0 +lon_0=15d +x_0=5500000 +y_0=0";
	const char* latLngConfig = "+proj=lonlat +datum=WGS84 +ellps=WGS84 "
		"+towgs84=0,0,0";
	//->"+proj=lonlat +datum=WGS84"

	unsigned int nocs = lls.size(); //no of coordinates
	double* xs = new double[nocs];
	double* ys = new double[nocs];
	double* zs = new double[nocs];

	for(unsigned int i = 0; i < nocs; i++){
		xs[i] = lls.at(i).lng * DEG_TO_RAD;
		ys[i] = lls.at(i).lat * DEG_TO_RAD;
		zs[i] = 0;
	}

	vector<RECTCoord> rcs(nocs);

	projPJ rcPJ = pj_init_plus(rcConfig);
	projPJ latLngPJ = pj_init_plus(latLngConfig);

	if(rcPJ && latLngPJ){
		int error = pj_transform(latLngPJ, rcPJ, nocs, 0, xs, ys, zs);
		if(error){
			cerr << "error: " << error << endl;
			return vector<RECTCoord>();
		}
	}

	for(unsigned int i = 0; i < nocs; i++)
		rcs[i] = RECTCoord(xs[i], ys[i]);

	if(latLngPJ) pj_free(latLngPJ);
	if(rcPJ) pj_free(rcPJ);

	delete[] xs;
	delete[] ys;
	delete[] zs;

	return !latLngPJ || !rcPJ ? vector<RECTCoord>() : rcs;
}

RECTCoord Tools::latLng2RC(LatLngCoord llc)
{
	auto v = latLng2RC(vector<LatLngCoord>(1, llc));
	return v.empty() ? RECTCoord() : v.front();
}

vector<LatLngCoord> Tools::RC2latLng(const vector<RECTCoord>& rcs)
{
	if(rcs.empty())
		return vector<LatLngCoord>();

	const char* rcConfig = "+proj=tmerc +units=m +datum=potsdam "
		"+k=1 +lat_0=0 +lon_0=15d +x_0=5500000\n"
		"+y_0=0 +ellps=bessel +towgs84=606.0,23.0,413.0";
	//->"+proj=tmerc +units=m +datum=potsdam "
	//->"+k=1 +lat_0=0 +lon_0=15d +x_0=5500000 +y_0=0";
	const char* latLngConfig = "+proj=lonlat +datum=WGS84 +ellps=WGS84 +towgs84=0,0,0";
	//->"+proj=lonlat +datum=WGS84"

	unsigned int nocs = rcs.size(); //no of coordinates
	double* xs = new double[nocs];
	double* ys = new double[nocs];
	double* zs = new double[nocs];

	for(unsigned int i = 0; i < nocs; i++){
		xs[i] = rcs.at(i).r;
		ys[i] = rcs.at(i).h;
		zs[i] = 0;
	}

	vector<LatLngCoord> lls(nocs);

	projPJ rcPJ = pj_init_plus(rcConfig);
	projPJ latLngPJ = pj_init_plus(latLngConfig);

	if(rcPJ && latLngPJ){
		int error = pj_transform(rcPJ, latLngPJ, nocs, 0, xs, ys, zs);
		if(error){
			cerr << "error: " << error << endl;
			return vector<LatLngCoord>();
		}

		for(unsigned int i = 0; i < nocs; i++)
			lls[i] = LatLngCoord(ys[i]*RAD_TO_DEG, xs[i]*RAD_TO_DEG);
	}

	if(latLngPJ) pj_free(latLngPJ);
	if(rcPJ) pj_free(rcPJ);

	delete[] xs;
	delete[] ys;
	delete[] zs;

	return !latLngPJ || !rcPJ ? vector<LatLngCoord>() : lls;
}

LatLngCoord Tools::RC2latLng(RECTCoord rcc)
{
	auto v = RC2latLng(vector<RECTCoord>(1, rcc));
	return v.empty() ? LatLngCoord() : v.front();
}
*/

bool Tools::contains(vector<LatLngCoord> tlTrBrBlRect, LatLngCoord llc)
{
	LatLngCoord tl = tlTrBrBlRect.at(0);
	//LatLngCoord tr = tlTrBrBlRect.at(1);
	LatLngCoord br = tlTrBrBlRect.at(2);
	//LatLngCoord bl = tlTrBrBlRect.at(3);

	return
			llc.lng >= tl.lng && llc.lat <= tl.lat &&
			llc.lng <= br.lng && llc.lat >= br.lat;
}
