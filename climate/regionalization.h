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

// c++ version of the interpolation algorithm based on
// Mrs. Dr. Haentzschel TU-Dresden
// Idea: use a elevation model to construct a lineare regression function 
// over some weather station near the region of interest with an R2
// add an inverse distance interpolation using: 
// inv *(1-R2)+ regression*R2

#ifndef __REGIONALIZATION__
#define __REGIONALIZATION__

#pragma warning(disable:4503)

#include <vector>
#include <string>
#include <map>

#include "grid/grid+.h"
#include "climate.h"

namespace Climate
{
  namespace Regionalization
  {
		int borderSizeIncrementKM(int newGlobalValue = -1 /*km*/);
    const int defaultBorderSize = 100;

		typedef int ResultId;

		typedef std::map<ResultId, double> FuncResult;

		FuncResult defaultFunction(AvailableClimateData acd, DataAccessor da);

		std::function<FuncResult(DataAccessor)> defaultFunctionWith(AvailableClimateData acd);

		int uniqueFunctionId(const std::string& functionIdentifier);

		void preloadClimateData(ClimateRealization* realization,
														Grids::GridMetaData gmd, std::vector<ACD> acds,
														int fromYear, int toYear,
														std::function<void(int, int) > stateCallback =
														std::function<void(int, int)>(),
														int borderSize = -1);

    struct CacheInfo
    {
      CacheInfo() : cacheData(false) {}
      bool cacheData;
			std::string pathToHdfCache;
			std::string functionIdString;
      std::vector<ResultId> resultIds;
    };

    struct Env
    {
			Env()
        : dgm(NULL), fromYear(0), toYear(0), yearSlice(1),
					borderSize(borderSizeIncrementKM()), functionId(0) { }

			Env(AvailableClimateData acd)
        : dgm(NULL), acds(1, acd), fromYear(0), toYear(0), yearSlice(1),
				borderSize(borderSizeIncrementKM()), functionId(0), f(defaultFunctionWith(acd)) { }

			const Grids::GridP* dgm;
			std::vector<AvailableClimateData> acds;
			int fromYear;
			int toYear;
			//! how many years of continous climate data are needed for the given function
			int yearSlice;
			int borderSize; //km
			Climate::Realizations realizations;
			int functionId;

      CacheInfo cacheInfo;

			//! function being applied to a complete year
			std::function < FuncResult(DataAccessor) > f;
		};

    typedef std::map<int, std::vector<Grids::GridPPtr> > Result;
		typedef std::map<ResultId, Result> Results;
		Results regionalize(Env env);

		//! regionalize but return the first and sole result
    inline Result regionalizeSR(Env env)
    {
			return regionalize(env).begin()->second;
		}

    typedef std::map<int, Grids::GridPPtr> AvgRealizationsResult;
		typedef std::map<ResultId, AvgRealizationsResult> AvgRealizationsResults;
		AvgRealizationsResults regionalizeAndAvgRealizations(Env env);

		//! regionalize and avg realizations and return the first and sole result
    inline AvgRealizationsResult regionalizeAndAvgRealizationsSR(Env env)
    {
			return regionalizeAndAvgRealizations(env).begin()->second;
		}
	}
}

#endif 
