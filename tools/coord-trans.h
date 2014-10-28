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
#include <cassert>

#include "proj_api.h"

namespace Tools
{
	enum CoordinateSystem
	{
		LatLng_EPSG4326 = 0,
		UTM21S_EPSG32721,
    GK5_EPSG31469,
    UTM32N_EPSG25832,
		UndefinedCoordinateSystem
	};

	std::string coordinateSystemToString(CoordinateSystem cs);
	std::string coordinateSystemToShortString(CoordinateSystem cs);
  CoordinateSystem shortStringToCoordinateSystem(std::string cs, CoordinateSystem def = UndefinedCoordinateSystem);

	template<typename T>
	struct Coord2D
	{
		Coord2D() : coordinateSystem(UndefinedCoordinateSystem) {}
		Coord2D(CoordinateSystem cs) : coordinateSystem(cs) {}
		virtual T firstDimension() const = 0;
		virtual T secondDimension() const = 0;
		CoordinateSystem coordinateSystem;
	};

	struct RectCoord : public Coord2D<double>
	{
		RectCoord() : Coord2D(UndefinedCoordinateSystem), r(0), h(0) { }

		RectCoord(CoordinateSystem cs) : Coord2D(cs), r(0), h(0) { }

//		RectCoord(double r, double h) : Coord2D(GK5_EPSG31469), r(r), h(h) { }

		RectCoord(CoordinateSystem cs, double r, double h)
			: Coord2D(cs), r(r), h(h) { }

		virtual double firstDimension() const { return r; }
		virtual double secondDimension() const { return h; }

		RectCoord operator+(const RectCoord& other) const
		{
			assert(coordinateSystem == other.coordinateSystem);
			return RectCoord(coordinateSystem, r + other.r, h + other.h);
		}

		RectCoord operator-(const RectCoord & other) const
		{
			return (*this) + (other*-1);
		}

		RectCoord operator+(double value) const
		{
			return RectCoord(coordinateSystem, r + value, h + value);
		}

		RectCoord operator-(double value) const
		{
			return (*this) + (-1*value);
		}

		RectCoord operator*(double value) const
		{
			return RectCoord(coordinateSystem, r*value, h*value);
		}

		RectCoord operator/(double value) const
		{
			return (*this) * (1.0 / value);
		}

		double distanceTo(const RectCoord & other) const
		{
			return std::sqrt(((r - other.r)*(r - other.r)) + ((h - other.h)*(h - other.h)));
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
	struct LatLngCoord : public Coord2D<double>
	{
		LatLngCoord() : Coord2D(LatLng_EPSG4326), lat(0), lng(0) {}

		LatLngCoord(CoordinateSystem)
			: Coord2D(LatLng_EPSG4326), lat(0), lng(0)
		{}

		LatLngCoord(double lat, double lng)
			: Coord2D(LatLng_EPSG4326), lat(lat), lng(lng)
		{}

		LatLngCoord(CoordinateSystem, double lat, double lng)
			: Coord2D(LatLng_EPSG4326), lat(lat), lng(lng)
		{}

		virtual double firstDimension() const { return lat; }
		virtual double secondDimension() const { return lng; }

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

	struct CoordConversionParams
	{
		double sourceConversionFactor;
		double targetConversionFactor;
		bool switch2DCoordinates;
		std::string projectionParams;
	};

	CoordConversionParams coordConversionParams(CoordinateSystem cs);

	template<typename SourceCoordType, typename TargetCoordType>
	std::vector<typename TargetCoordType>
	sourceProj2targetProj(const std::vector<SourceCoordType>& sourceCoords,
												CoordinateSystem targetCoordinateSystem);

	template<typename SourceCoordType, typename TargetCoordType>
	TargetCoordType singleSourceProj2targetProj(SourceCoordType s,
																							CoordinateSystem targetCoordinateSystem);

//	template<typename SP, typename TP>
//	std::vector<typename TP::CoordType>
//	sourceProj2targetProj(const std::vector<typename SP::CoordType>& sourceCoords);

//	template<typename SP, typename TP>
//	typename TP::CoordType sourceProj2targetProj(typename SP::CoordType s);

	//Lat/Lng to GK5

//	inline std::vector<RectCoord> latLng2GK5(const std::vector<LatLngCoord>& lls)
//	{ return sourceProj2targetProj<LatLng_EPSG4326_Params, GK5_Params>(lls); }

//	inline RectCoord latLng2GK5(LatLngCoord llc)
//	{ return sourceProj2targetProj<LatLng_EPSG4326_Params, GK5_Params>(llc); }


	inline std::vector<RectCoord> latLng2RC(const std::vector<LatLngCoord>& lls,
																					CoordinateSystem targetCoordinateSystem
																					= GK5_EPSG31469)
	{
		return sourceProj2targetProj<LatLngCoord, RectCoord>(lls, targetCoordinateSystem);
	}

	inline RectCoord latLng2RC(LatLngCoord llc,
														 CoordinateSystem targetCoordinateSystem
														 = GK5_EPSG31469)
	{
		return singleSourceProj2targetProj<LatLngCoord, RectCoord>(llc, targetCoordinateSystem);
	}

//	inline std::vector<RectCoord> latLng2RC(const std::vector<LatLngCoord>& lls)
//	{ return sourceProj2targetProj<LatLng_EPSG4326_Params, GK5_Params>(lls); }

//	inline RectCoord latLng2RC(LatLngCoord llc)
//	{ return sourceProj2targetProj<LatLng_EPSG4326_Params, GK5_Params>(llc); }

	//Gk5 to Lat/Lng
//	inline std::vector<LatLngCoord> GK52latLng(const std::vector<RectCoord>& rcs)
//	{ return sourceProj2targetProj<GK5_Params, LatLng_EPSG4326_Params>(rcs); }

//	inline LatLngCoord GK52latLng(RectCoord rcc)
//	{ return sourceProj2targetProj<GK5_Params, LatLng_EPSG4326_Params>(rcc); }

	inline std::vector<LatLngCoord> RC2latLng(const std::vector<RectCoord>& rcs)
	{
		return sourceProj2targetProj<RectCoord, LatLngCoord>(rcs, LatLng_EPSG4326);
	}

	inline LatLngCoord RC2latLng(RectCoord rcc)
	{
		return singleSourceProj2targetProj<RectCoord, LatLngCoord>(rcc, LatLng_EPSG4326);
	}

	//Lat/Lng to UTM21S
//	inline std::vector<UTM21SCoord> latLng2UTM21S(const std::vector<LatLngCoord>& lls)
//	{ return sourceProj2targetProj<LatLng_EPSG4326_Params, UTM21S_EPSG32721_Params>(lls); }

//	inline UTM21SCoord latLng2UTM21S(LatLngCoord llc)
//	{ return sourceProj2targetProj<LatLng_EPSG4326_Params, UTM21S_EPSG32721_Params>(llc); }

	//UTM21S to Lat/Lng
//	inline std::vector<LatLngCoord> UTM21S2latLng(const std::vector<UTM21SCoord>& utms)
//	{ return sourceProj2targetProj<UTM21S_EPSG32721_Params, LatLng_EPSG4326_Params>(utms); }

//	inline LatLngCoord UTM21S2latLng(UTM21SCoord utmc)
//	{ return sourceProj2targetProj<UTM21S_EPSG32721_Params, LatLng_EPSG4326_Params>(utmc); }

	bool contains(std::vector<LatLngCoord> tlTrBrBlRect, LatLngCoord point);
}

//template implementations
//-------------------------------

template<typename SCT, typename TCT>
std::vector<TCT>
Tools::sourceProj2targetProj(const std::vector<SCT>& scs, CoordinateSystem targetCS)
{
	if(scs.empty())
		return std::vector<TCT>();

	CoordConversionParams sccp = coordConversionParams(scs.front().coordinateSystem);
	CoordConversionParams tccp = coordConversionParams(targetCS);

	unsigned int nocs = (unsigned int)scs.size(); //no of coordinates
	double* xs = new double[nocs];
	double* ys = new double[nocs];
	double* zs = new double[nocs];

	double scv = sccp.sourceConversionFactor;
	bool ss2Dc = sccp.switch2DCoordinates;
	for(unsigned int i = 0; i < nocs; i++)
	{
		xs[i] = ss2Dc ? scs.at(i).secondDimension() * scv
									: scs.at(i).firstDimension() * scv;
		ys[i] = ss2Dc ? scs.at(i).firstDimension() * scv
									: scs.at(i).secondDimension() * scv;
		zs[i] = 0;
	}

	projPJ sourcePJ = pj_init_plus(sccp.projectionParams.c_str());
	projPJ targetPJ = pj_init_plus(tccp.projectionParams.c_str());

	if(sourcePJ && targetPJ){
		int error = pj_transform(sourcePJ, targetPJ, nocs, 0, xs, ys, zs);
		if(error)
		{
			std::cerr << "error: " << error << std::endl;
			return std::vector<TCT>();
		}
	}

	std::vector<TCT> tcs(nocs);
	double tcv = tccp.targetConversionFactor;
	bool ts2Dc = tccp.switch2DCoordinates;
	for(unsigned int i = 0; i < nocs; i++)
		tcs[i] = ts2Dc ? TCT(targetCS, ys[i]*tcv, xs[i]*tcv)
									 : TCT(targetCS, xs[i]*tcv, ys[i]*tcv);

	if(sourcePJ)
		pj_free(sourcePJ);
	if(targetPJ)
		pj_free(targetPJ);

	delete[] xs;
	delete[] ys;
	delete[] zs;

	return !sourcePJ || !targetPJ ? std::vector<TCT>() : tcs;
}

template<typename SCT, typename TCT>
TCT Tools::singleSourceProj2targetProj(SCT sc, CoordinateSystem targetCS)
{
	auto v = sourceProj2targetProj<SCT, TCT>(std::vector<SCT>(1, sc), targetCS);
	return v.empty() ? TCT(targetCS) : v.front();
}




#endif
