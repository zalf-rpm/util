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

#include <proj_api.h>
#include <iostream>
#include <sstream>

#include "coord-trans.h"

using namespace std;
using namespace Tools;

string GK5Coord::toString() const {
	ostringstream s; s << "r: " << r << " h: " << h;
	return s.str();
}

//------------------------------------------------------------------------------

const double LatLngCoord::eps = 0.000001;

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

vector<GK5Coord> Tools::latLng2GK5(const vector<LatLngCoord>& lls)
{
	if(lls.empty())
		return vector<GK5Coord>();

	const char* gk5Config = "+proj=tmerc +units=m +datum=potsdam "
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

	vector<GK5Coord> gk5s(nocs);

	projPJ gk5PJ = pj_init_plus(gk5Config);
	projPJ latLngPJ = pj_init_plus(latLngConfig);

	if(gk5PJ && latLngPJ){
		int error = pj_transform(latLngPJ, gk5PJ, nocs, 0, xs, ys, zs);
		if(error){
			cerr << "error: " << error << endl;
			return vector<GK5Coord>();
		}
	}

	for(unsigned int i = 0; i < nocs; i++)
		gk5s[i] = GK5Coord(xs[i], ys[i]);

	if(latLngPJ) pj_free(latLngPJ);
	if(gk5PJ) pj_free(gk5PJ);

	delete[] xs;
	delete[] ys;
	delete[] zs;

	return !latLngPJ || !gk5PJ ? vector<GK5Coord>() : gk5s;
}

GK5Coord Tools::latLng2GK5(LatLngCoord llc)
{
	auto v = latLng2GK5(vector<LatLngCoord>(1, llc));
	return v.empty() ? GK5Coord() : v.front();
}

vector<LatLngCoord> Tools::GK52latLng(const vector<GK5Coord>& gk5s)
{
	if(gk5s.empty())
		return vector<LatLngCoord>();

	const char* gk5Config = "+proj=tmerc +units=m +datum=potsdam "
		"+k=1 +lat_0=0 +lon_0=15d +x_0=5500000\n"
		"+y_0=0 +ellps=bessel +towgs84=606.0,23.0,413.0";
	//->"+proj=tmerc +units=m +datum=potsdam "
	//->"+k=1 +lat_0=0 +lon_0=15d +x_0=5500000 +y_0=0";
	const char* latLngConfig = "+proj=lonlat +datum=WGS84 +ellps=WGS84 +towgs84=0,0,0";
	//->"+proj=lonlat +datum=WGS84"

	unsigned int nocs = gk5s.size(); //no of coordinates
	double* xs = new double[nocs];
	double* ys = new double[nocs];
	double* zs = new double[nocs];

	for(unsigned int i = 0; i < nocs; i++){
		xs[i] = gk5s.at(i).r;
		ys[i] = gk5s.at(i).h;
		zs[i] = 0;
	}

	vector<LatLngCoord> lls(nocs);

	projPJ gk5PJ = pj_init_plus(gk5Config);
	projPJ latLngPJ = pj_init_plus(latLngConfig);

	if(gk5PJ && latLngPJ){
		int error = pj_transform(gk5PJ, latLngPJ, nocs, 0, xs, ys, zs);
		if(error){
			cerr << "error: " << error << endl;
			return vector<LatLngCoord>();
		}

		for(unsigned int i = 0; i < nocs; i++)
			lls[i] = LatLngCoord(ys[i]*RAD_TO_DEG, xs[i]*RAD_TO_DEG);
	}

	if(latLngPJ) pj_free(latLngPJ);
	if(gk5PJ) pj_free(gk5PJ);

	delete[] xs;
	delete[] ys;
	delete[] zs;

	return !latLngPJ || !gk5PJ ? vector<LatLngCoord>() : lls;
}

LatLngCoord Tools::GK52latLng(GK5Coord gk5c)
{
	auto v = GK52latLng(vector<GK5Coord>(1, gk5c));
	return v.empty() ? LatLngCoord() : v.front();
}
