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

#ifndef COORDTRANS_H_
#define COORDTRANS_H_

#include <vector>
#include <cmath>

namespace Tools
{
	struct GK5Coord
	{
    GK5Coord() : r(0), h(0) { }

    GK5Coord(double r, double h) : r(r), h(h) { }

		GK5Coord operator+(const GK5Coord & other) const
		{
			return GK5Coord(r + other.r, h + other.h);
		}

		GK5Coord operator-(const GK5Coord & other) const
		{
			return (*this) + (other*-1);
		}

		GK5Coord operator*(double value) const
		{
			return GK5Coord(r*value, h * value);
		}

		GK5Coord operator/(double value) const
		{
			return (*this) * (1.0 / value);
		}

		double distanceTo(const GK5Coord & other) const
		{
			return std::sqrt(((r - other.r)*(r - other.r))
											+((h - other.h)*(h - other.h)));
		}

		std::string toString() const;

		bool isZero() const
		{
      return int(r) == 0 && int(h) == 0;
    }

		double r;
		double h;
	};

	//----------------------------------------------------------------------------

	/*! A Geocoordinate Pair (latitude, longitude)
	 * geocoordinates
	 */
	struct LatLngCoord
	{
    LatLngCoord() : lat(0), lng(0) {}

		LatLngCoord(double lat, double lng) : lat(lat), lng(lng) {}

    bool operator==(const LatLngCoord& other) const;

    bool operator<(const LatLngCoord & other) const;

		LatLngCoord operator+(const LatLngCoord& other) const;

		LatLngCoord operator-(const LatLngCoord& other) const
		{
			return (*this) + (other * -1);
		}

		void operator+=(const LatLngCoord& other);

		void operator-=(const LatLngCoord& other){ (*this) += (other * -1); }

		LatLngCoord operator*(double scaleFactor) const;

    double distanceTo(const LatLngCoord & other) const;

		std::string toCanonicalString(std::string leftBound = std::string(),
																	std::string fill = std::string(),
																	std::string rightBound = std::string()) const;

		std::string toString() const;

		bool isZero() const { return int(lat) == 0 && int(lng) == 0; }

		double lat;
		double lng;
		static const double eps;
	};

	//----------------------------------------------------------------------------

	//! convert lat lng geocoords in decimal degrees to gk5
	/*!
	 * returns empty vector on error
	 */
	std::vector<GK5Coord> latLng2GK5(const std::vector<LatLngCoord>& lls);

	GK5Coord latLng2GK5(LatLngCoord llc);

	//! convert gk5 coords to lat lng geocoords in decimal degrees
	/*!
	 * returns empty vector on error
	 */
	std::vector<LatLngCoord> GK52latLng(const std::vector<GK5Coord>& gk5s);

	LatLngCoord GK52latLng(GK5Coord gk5c);

}

#endif
