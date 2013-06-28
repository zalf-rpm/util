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

#include <string>
#include <cstring>
#include <sstream>

#include <boost/foreach.hpp>

#include "tools/use-stl-algo-boost-lambda.h"

#include "regionalization.h"
#include "tools/coord-trans.h"
#include "tools/date.h"
#include "db/abstract-db-connections.h"
#include "tools/helper.h"
#include "tools/algorithms.h"

#define LOKI_OBJECT_LEVEL_THREADING
#include "loki/Threads.h"

using namespace std;
using namespace Climate;
using namespace Climate::Regionalization;
using namespace Grids;
using namespace Tools;

namespace
{
  struct L : public Loki::ObjectLevelLockable<L> {};

  struct X
  {
    X()  { }
		X(ClimateStation cs, RectCoord rc, vector<double> vs)
			: station(cs), rc(rc), values(vs) { }
		ClimateStation station;
		RectCoord rc;
		vector<double> values;
		vector<double> residua;
  };

  struct RegressionResult
  {
		RegressionResult() { }
		vector<double> m, n, r2;
	};
	
  RegressionResult regression(const vector<X>& stations)
  {
    RegressionResult res;

    if(stations.empty())
			return res;

    int valuesSize = stations.front().values.size();

    double nn_quer = 0.0;
    vector<double> values_quer(valuesSize, 0.0);
    BOOST_FOREACH(const X& x, stations)
    {
      nn_quer += x.station.nn();
      values_quer += x.values;
//			cout << "nn=" << x.station.nn()
//					<< " values=" << toString(x.values) << endl;
		}
    double noOfStations = stations.size();
    nn_quer /= noOfStations;
    values_quer /= noOfStations;
//		cout << "nn_quer=" << nn_quer
//				<< " values_quer=" << toString(values_quer) << endl;

    vector<double> var_nn_values(valuesSize, 0.0), var_values(valuesSize, 0.0);
    double var_nn = 0.0;
    BOOST_FOREACH(const X& x, stations)
    {
      double nn_diff = x.station.nn() - nn_quer;
      vector<double> values_diff = x.values - values_quer;
      var_nn += nn_diff * nn_diff;
      var_values += values_diff * values_diff;
      var_nn_values += values_diff * nn_diff;
//			cout << "nn_diff=" << nn_diff
//					<< " values_diff=" << toString(values_diff) << endl;
		}
    res.m = var_nn_values / var_nn;
    res.n = values_quer - (res.m * nn_quer);
    var_nn /= noOfStations - 1.0;
    var_values /= noOfStations - 1.0;
    var_nn_values /= noOfStations - 1.0;
    res.r2 = (var_nn_values * var_nn_values) / (var_values * var_nn);

//		cout << "var_nn=" << var_nn << " var_values=" << toString(var_values)
//				<< " var_nn_values=" << toString(var_nn_values)
//				<< " m=var_nn_values/var_nn=" << toString(var_nn_values) << "/"
//				<< var_nn << "=" << toString(res.m)
//				<< " n=values_quer-(res.m*nn_quer)=" << toString(values_quer)
//				<< "-" << "(" << toString(res.m) << "*" << nn_quer
//				<< ")=" << toString(res.n)
//				<< " r2=(var_nn_values*var_nn_values)/(var_values*var_nn)="
//				<< "(" << toString(var_nn_values) << "*" << toString(var_nn_values) << ")/("
//				<< toString(var_values) << "*" << var_nn << "="
//				<< toString(res.r2) << endl;
    return res;
	}

  vector<const ClimateStation*> filterClimateStations(ClimateSimulation* sim,
                                                      GridMetaData gmd,
                                                      int borderSize)
  {
    static L lockable;
		typedef map<GridMetaData, vector<const ClimateStation*> > M2;
		typedef map<ClimateSimulation*, M2> M1;
		static M1 m;

    struct X
    {
			static vector<const ClimateStation*>
          tryToFind(M1& m, ClimateSimulation* sim, GridMetaData gmd)
      {
				M1::const_iterator ci = m.find(sim);
        if(ci != m.end())
        {
					M2::const_iterator ci2 = ci->second.find(gmd);
          if(ci2 != ci->second.end())
						return ci2->second;
				}
				return vector<const ClimateStation*>();
			}
		};

		{
			L::Lock lock(lockable);
			const vector<const ClimateStation*> fcss = X::tryToFind(m, sim, gmd);
			if(!fcss.empty())
				return fcss;
		}

		RCRect extendedRegion = gmd.rcRect();
		int bs = borderSize * 1000; //m
		extendedRegion.tl = extendedRegion.tl + RectCoord(-bs, bs);
		extendedRegion.br = extendedRegion.br + RectCoord(bs, -bs);
		cout << "region: " << gmd.rcRect().toString()
				 << " with border-size-increment of " << borderSize << " [km] -> "
				 << " extendedRegion: " << extendedRegion.toString() << endl;

		vector<const ClimateStation*> filteredStations;
    BOOST_FOREACH(const ClimateStation* cs, sim->climateStations())
    {
			if(extendedRegion.contains(cs->rcCoord()))
      {
				cout << "----> included: " << cs->name() << " rcc: " << cs->rcCoord().toString() << endl;
				filteredStations.push_back(cs);
			}
		}

		{
			L::Lock lock(lockable);
			const vector<const ClimateStation*> fcss = X::tryToFind(m, sim, gmd);
			//nobody else inserted them already
			if(fcss.empty())
				m[sim][gmd] = filteredStations;
		}

		cout << "used " << filteredStations.size() << " climate-stations" << endl;
		return filteredStations;
	}
	
  string acdsToString(set<ACD> acds)
  {
    ostringstream s;
    BOOST_FOREACH(ACD acd, acds)
    {
      s << int(acd) << "_";
    }
    string res = s.str();
    res.erase(res.length()-1);
    return res;
  }

}

int Regionalization::borderSizeIncrementKM(int newGlobalValue)
{
	static L lockable;
	static int globalValue = defaultBorderSize;

	if(newGlobalValue > 0)
	{
		L::Lock lock(lockable);
		globalValue = newGlobalValue;
	}

	return globalValue;
}

boost::function<FuncResult(DataAccessor) >
Regionalization::defaultFunctionWith(AvailableClimateData acd)
{
	return boost::lambda::bind(defaultFunction, acd, _1);
}

FuncResult Regionalization::defaultFunction(AvailableClimateData acd,
                                            DataAccessor da)
{
	int steps = da.noOfStepsPossible();
	double v = 0;
  for(int i = 0; i < steps; i++)
		v += da.dataForTimestep(acd, i);
	v /= (acd == precip ? 1 : steps);

	map<ResultId, double> m;
	m.insert(make_pair(0, v));
	return m;
}

int Regionalization::uniqueFunctionId(const string& fid)
{
	static L lockable;
	static map<string, int> idCounts;
	static int idCount = 0;

	{
		L::Lock lock(lockable);
		if(idCounts.find(fid) == idCounts.end())
			idCounts[fid] = idCount++;
	}

//	cout << "uniqueFunctionId(" << fid << ") = " << idCounts[fid] << endl;
	return idCounts[fid];
}

void Regionalization::preloadClimateData(ClimateRealization* r,
																				 GridMetaData gmd,  vector<ACD> acds,
																				 int fromYear, int toYear,
																				 boost::function<void(int, int)> callback,
                                         int borderSize)
{
	vector<const ClimateStation*> climateStations =
			filterClimateStations(r->simulation(), gmd,
														borderSize < 0 ? borderSizeIncrementKM() : borderSize);
	int nocs = climateStations.size();
	int csCount = 0;
  BOOST_FOREACH(const ClimateStation* cs, climateStations)
  {
//		cout << "Filling climate data cache for:"
//				<< " ClimateStation: " << cs->name()
//				<< " ClimateRealization: " << r->name()
//				<< " GridMetaData: " << gmd.toString()
//				<< " ACDs: [ ";
//		BOOST_FOREACH(ACD acd, acds)
//		{
//			cout << acd << " ";
//		}
//		cout << " ] fromYear: " << fromYear << " toYear: " << toYear
//				<< " borderSize: " << borderSize << "km" << endl;
		r->fillCacheFor(acds, cs->geoCoord(), Date(1, 1, fromYear),
                    Date(31, 12, toYear));
		if(callback)
			callback(++csCount, nocs);
	}
}

namespace
{
  template <typename Pair>
  struct Contains
  {
		RCRect _r;
		Contains(GridMetaData gmd) : _r(gmd.rcRect()) {}
    bool operator()(Pair p) const
    {
			return p.first.rcRect().contains(_r);
    }
  };
}

Results Regionalization::regionalize(Env env)
{
//	cout << "entering Climate::regionalize acds: ( ";
//	BOOST_FOREACH(ACD acd, env.acds)
//	{
//		cout << acd << " ";
//	}
//	cout << ") functionId: " << env.functionId
//			<< " functionIdString: " << env.cacheInfo.functionIdString
//			<< " from: " << env.fromYear << " to: " << env.toYear << endl;

  static L memoryCacheLockable;
  static L diskCacheLockable;
	typedef string SimulationId;
	typedef string ScenarioId;
	typedef string RealizationName;
	typedef int FunctionId;
  typedef int Year;
	typedef map<Year, GridPPtr> Year2Res;
	typedef map<ResultId, Year2Res> ResId2Res;
	typedef map<FunctionId, ResId2Res> FuncId2Res;
	typedef map<set<ACD>, FuncId2Res> ACD2Res;
	typedef map<RealizationName, ACD2Res> Real2Res;
	typedef map<ScenarioId, Real2Res> Scen2Res;
	typedef map<SimulationId, Scen2Res> Sim2Res;
	typedef map<GridMetaData, Sim2Res> GMD2Res;
	static GMD2Res cache;

	Results res;

	const Realizations& realizations = env.realizations;
	if(realizations.empty())
		return res;
//	cout << "number of realizations used: " << realizations.size() << endl;

	ClimateRealization* someRealization = realizations.front();
	ClimateScenario* scen = someRealization->scenario();
	ClimateSimulation* sim = scen->simulation();

	typedef map<ClimateRealization*, set<Year> > Real2Years;
	Real2Years realization2years;
  BOOST_FOREACH(ClimateRealization* r, realizations)
  {
		realization2years[r] =  Tools::range<set<Year> >(env.fromYear, env.toYear);
	}

	GridMetaData gmd(env.dgm->gridPtr());

	set<ACD> acdsSet(env.acds.begin(), env.acds.end());

	{
		//lock the cache, so only one thread can change it
    L::Lock lock(memoryCacheLockable);

    //check if cache contains the same grid size already
    GMD2Res::const_iterator ci = cache.find(gmd);
    if(ci == cache.end())
      ci = find_if(cache.begin(), cache.end(),
                   Contains<GMD2Res::value_type>(gmd));
    if(ci != cache.end())
    {
			bool gmdIsSubregion = ci->first.rcRect().contains(gmd.rcRect()) &&
                            ci->first != gmd;
			//if so check for the correct simulation
			Sim2Res::const_iterator ci2 = ci->second.find(sim->id());
      if(ci2 != ci->second.end())
      {
				//if so check for the correct scenario
				Scen2Res::const_iterator ci3 = ci2->second.find(scen->id());
        if(ci3 != ci2->second.end())
        {
          BOOST_FOREACH(ClimateRealization* real, realizations)
          {
						RealizationName realn = real->name();
						set<Year>& years = realization2years[real];
						//if so check for the correct realization
						Real2Res::const_iterator ci4 = ci3->second.find(realn);
            if(ci4 != ci3->second.end())
            {
							//if so check for the correct data element
							ACD2Res::const_iterator ci5 = ci4->second.find(acdsSet);
              if(ci5 != ci4->second.end())
              {
								//and check whether the function yielding the requested data
								//is the same (via the generic handchoosen function id (right now))
								FuncId2Res::const_iterator ci6 = ci5->second.find(env.functionId);
                if(ci6 != ci5->second.end())
                {
									const ResId2Res& rs = ci6->second;
									//now find all possible years for the given data element
									//and load only really necessary ones
                  if(!rs.empty())
                  {
										//assume that for one set of results all the years are the same
										//as they should have been calculated at the same time
										const Year2Res& r = rs.begin()->second;
										//add known years to result and remove them from list of
										//all requested years
                    for(int year = env.fromYear; year <= env.toYear; year++)
                    {
											Year2Res::const_iterator ci7 = r.find(year);
                      if(ci7 != r.end())
                      {
                        BOOST_FOREACH(const ResId2Res::value_type& p, rs)
                        {
													ResultId rid = p.first;
													const Year2Res& r = p.second;
													Year2Res::const_iterator ci8 = r.find(year);
													assert(ci8 != r.end());
                          GridPPtr g = ci8->second;
                          if(gmdIsSubregion)
                          {
                            pair<Row, Col> rc =
                                rowColInGrid(gmd, gmd.topLeftCorner());
                            g = GridPPtr(g->subGridClone(rc.first, rc.second,
                                                         gmd.nrows, gmd.ncols));
                          }
                          res[rid][year].push_back(g);
												}
												years.erase(year);
											}
										}
										//if we found all years in cache, we can remove
										//the realization for the list of data to regionalize
										if(years.empty())
											realization2years.erase(real);
									}
								}
							}
						}
					}
          //if all data are in cache, then there are no more realizations
          //in the map
          if(realization2years.empty())
          {
//            cout << "all requested regionalized data are available in cache" << endl;
            return res;
          }
				}
			}
		}

  }

  {
    L::Lock lock(diskCacheLockable);

    //if data are not yet in the cache, try to load them from a hdf
    if(env.cacheInfo.cacheData)
    {
      string pathToHdfCache = env.cacheInfo.pathToHdfCache;
      string functionIdString = env.cacheInfo.functionIdString;

      BOOST_FOREACH(Real2Years::value_type p, realization2years)
      {
        ClimateRealization* r = p.first;
        set<Year>& years = p.second;

        //assume that the cache contains for every year all the resultids
        BOOST_FOREACH(Year year, years)
        {
          bool foundYear = false;
          BOOST_FOREACH(ResultId rid, env.cacheInfo.resultIds)
          {
            ostringstream pathToHdf;
            pathToHdf << pathToHdfCache;
            if(*(pathToHdfCache.rbegin()) != '/')
              pathToHdf << "/";
            pathToHdf << gmd.toCanonicalString("_") << "/"
                << sim->name() << "/" << scen->name() << "/"
                << r->id() << "/" << acdsToString(acdsSet) << "/"
                << functionIdString << "/" << rid << ".hdf";
            ostringstream dsn;
            dsn << year;
//            cout << "trying to load: year: " << year << " path: " << pathToHdf.str() << endl;
            GridPPtr g = GridPPtr(new GridP(dsn.str(), GridP::HDF, pathToHdf.str()));
            if(g->isValid())
            {
//              cout << "found grid in hdf" << endl;
              foundYear = true;
              {
                L::Lock lock(memoryCacheLockable);
                cache[gmd][sim->id()][scen->id()][r->id()][acdsSet]
                    [env.functionId][rid][year] = g;
              }
              res[rid][year].push_back(g);
            }
          }
          if(foundYear)
            years.erase(year);
        }

        if(years.empty())
          realization2years.erase(r);
      }
      if(realization2years.empty())
      {
//        cout << "all requested regionalized data have been loaded from the hdf" << endl;
        return res;
      }
    }
	}

	//now regionalize climate data
	vector<const ClimateStation*> climateStations =
			filterClimateStations(sim, gmd, env.borderSize);

  if(climateStations.empty())
    return res;

  BOOST_FOREACH(Real2Years::value_type p, realization2years)
  {
		ClimateRealization* r = p.first;
//		cout << "calculating realization: " << r->name() << endl;
		set<Year> years = p.second;

		typedef map<Year, vector<X> > XS;
		XS year2xs;

    BOOST_FOREACH(const ClimateStation* cs, climateStations)
    {
			DataAccessor da = r->dataAccessorFor(env.acds, cs->geoCoord(),
                                           Date(1, 1, env.fromYear),
                                           Date(31, 12, env.toYear));

      for(int k = 0, to = env.toYear - env.fromYear + 1 - (env.yearSlice - 1);
      k < to; k++)
      {
				//skip years which have already been calculated and are available
				//in the cache
				int currentYear = env.fromYear + k;
        if(years.find(currentYear) != years.end())
        {
					DataAccessor yda = da.cloneForRange(k * 365, 365 * env.yearSlice);
					const FuncResult& vals = env.f(yda);

					vector<double> values;
          BOOST_FOREACH(FuncResult::value_type p, vals)
          {
						values.push_back(p.second);
					}

					//cache also rc coordinate of station
					year2xs[currentYear].push_back(X(*cs, cs->rcCoord(), values));
//					cout << "current year: " << currentYear << " station: " << cs->name();
//					BOOST_FOREACH(FuncResult::value_type p, vals)
//					{
//						cout << " [id: " << p.first << "|value: " << p.second << "]";
//					}
//					cout << endl;
				}
			}
		}

		//get results for current realization
		//is basically the same as the avg realization results
		AvgRealizationsResults newRes;
    BOOST_FOREACH(XS::value_type p, year2xs)
    {
      int year = p.first;
//			cout << year << "|" << r->name() << " " << endl;

			vector<X>& xs = year2xs[year];

      //if less than three stations, don't do the regression, but
      //simply average the two stations or take the values of the one stationso/one station/s
      bool moreThanTwoStations = xs.size() > 2;

      int noOfResults = xs.front().values.size();
      vector<GridPPtr> gs(noOfResults);
      for(int i = 0, size = gs.size(); i < size; i++)
        gs[i] = GridPPtr(env.dgm->clone());

      RegressionResult rr;
      if(moreThanTwoStations)
      {
        rr = regression(xs);

        // inverse distance and regression
        BOOST_FOREACH(X& x, xs)
        {
          x.residua = x.values - ((rr.m * x.station.nn()) + rr.n);
//					cout << "residua=values-((m*nn)+n)=" << toString(x.values)
//							 << "-" << "((" << toString(rr.m) << "*" << x.station.nn() << ")+"
//							 << toString(rr.n) << "=" << toString(x.residua) << endl;
        }
      }

			GridPPtr g = gs.front();
			double cellSize = g->cellSize();
			double r = g->gridPtr()->xcorner + (cellSize / 2);
			double h = g->gridPtr()->ycorner + (double(g->rows()) * cellSize)-(cellSize / 2.0);
      for(int i = 0, rs = g->rows(); i < rs; i++)
      {
        for(int j = 0, cs = g->cols(); j < cs; j++)
        {
          if(g->isDataField(i, j))
          {
            if(moreThanTwoStations)
            {
              double sum = 0.0;
              vector<double> sumz(noOfResults, 0.0);

              BOOST_FOREACH(const X& x, xs)
              {
								double dist = x.rc.distanceTo(RectCoord(r + (cellSize * j),
                                                        h - (cellSize * i)));
                if(dist > 1.0)
                {
                  sum += 1.0 / (dist * dist);
                  sumz += x.residua / (dist * dist);
                }
              }
              for(int k = 0; k < noOfResults; k++)
              {
                double dgm = env.dgm->dataAt(i, j);
                double m = rr.m[k];
                double n = rr.n[k];
                double oldValue = dgm * m + n + sumz[k]/sum;
								gs[k]->setDataAt(i, j, float(oldValue));
//								cout << "dataAt(" << i << "," << j << ")="
//										 << "dgm*m+n+sumz/sum="
//										 << dgm << "*"
//										 << m << "+" << n << "+"
//										 << sumz[k] << "/" << sum << "="
//										 << oldValue
//										 << endl;
              }
            }
            else
            {
              for(int k = 0; k < noOfResults; k++)
              {
                if(xs.size() == 2)
                {
                  const X& f = xs.at(0);
                  const X& s = xs.at(1);
									RectCoord cellRC = RectCoord(r+(cellSize*j), h-(cellSize*i));
									double df = f.rc.distanceTo(cellRC);
									double ds = s.rc.distanceTo(cellRC);
                  double fv = df/(df+ds)*f.values[k];
                  double sv = ds/(df+ds)*s.values[k];
									gs[k]->setDataAt(i, j, float(fv + sv));
//									cout << "dataAt(" << i << "," << j << ")="
//											 << "(fv + sv="
//											 << fv << "+" << sv << "="
//											 << (fv + sv)
//											 << endl;
								}
                else
                {
									gs[k]->setDataAt(i, j, float(xs.at(0).values[k]));
//									cout << "dataAt(" << i << "," << j << ")="
//											 << "value=" << xs.at(0).values[k] << endl;
								}

              }
            }
					}
				}
			}
      for(int k = 0; k < noOfResults; k++)
      {
        ResultId rid = env.cacheInfo.resultIds[k];
				newRes[rid][year] = gs.at(k);
//				cout << "newRes[rid][year]=newRes[" << rid << "][" << year << "]=" << gs.at(k)->dataAt(0,0) << endl;
			}
//			cout << "year: " << year << endl;
		}
//		cout << endl;

		//store the newly loaded data in cache
		{
      //lock the cache, so only one thread can change it
      //we have to lock until all date in newRes have been
      //stored, because the cache-code at the start of this function
      //assumes for instance that all years are being stored at the same time
      //and thus are available at the same time
      L::Lock lock(memoryCacheLockable);
      BOOST_FOREACH(const AvgRealizationsResults::value_type& p, newRes)
      {
				ResultId rid = p.first;
				const AvgRealizationsResult& arr = p.second;
        BOOST_FOREACH(const AvgRealizationsResult::value_type& p2, arr)
        {
					Year year = p2.first;
					GridPPtr g = p2.second;
          {

            cache[gmd][sim->id()][scen->id()][r->id()][acdsSet]
                [env.functionId][rid][year] = g;
          }

					if(env.cacheInfo.cacheData)
          {
            string pathToHdfCache = env.cacheInfo.pathToHdfCache;
            string functionIdString = env.cacheInfo.functionIdString;

            ostringstream pathToHdf;
            pathToHdf << pathToHdfCache;
            if(*(pathToHdfCache.rbegin()) != '/')
              pathToHdf << "/";
						pathToHdf << gmd.toCanonicalString("_") << "/"
								<< sim->name() << "/" << scen->name() << "/"
								<< r->id() << "/" << acdsToString(acdsSet) << "/"
								<< functionIdString << "/" << rid << ".hdf";
//						pathToHdf << gmd.toCanonicalString("_") << "/"
//								<< sim->name() << "/" << scen->name() << "/"
//								<< r->id() << "/" << acdsToString(acdsSet) << "/"
//								<< functionIdString << "/" << rid << "-" << year << ".ascii";
						ostringstream dsn;
            dsn << year;

            //should be ok to store separately because the memory cache
            //is anyway only used during this runtime-session
            L::Lock lock(diskCacheLockable);
//            cout << "writing into hdf: year: " << year << " path: " << pathToHdf.str() << endl;
						g->writeHdf(pathToHdf.str(), dsn.str(), "", -1);
//						cout << "writeAscii path: " << pathToHdf.str() << endl;
//						g->writeAscii(pathToHdf.str());
          }
				}
			}
		}

		//add potentially missing years to result
    BOOST_FOREACH(AvgRealizationsResults::value_type p, newRes)
    {
			ResultId rid = p.first;
			const AvgRealizationsResult& arr = p.second;
      BOOST_FOREACH(const AvgRealizationsResult::value_type& p2, arr)
      {
				Year year = p2.first;
				GridPPtr g = p2.second;
				res[rid][year].push_back(g);
			}
		}

		//and next realization
	}
	
//	cout << "leaving Climate::regionalize" << endl;

	return res;
}

AvgRealizationsResults Regionalization::regionalizeAndAvgRealizations(Env env)
{
	AvgRealizationsResults res;

	const Results& rs = regionalize(env);
  BOOST_FOREACH(const Results::value_type& p, rs)
  {
		ResultId rid = p.first;
    BOOST_FOREACH(const Result::value_type& p2, p.second)
    {
      res[rid][p2.first] = Grids::average(p2.second);
		}
	}

	return res;
}

