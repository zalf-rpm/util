#include <string>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <map>
#include <vector>

#include "db/abstract-db-connections.h"
#include "regionalization.h"
#include "grid/grid+.h"
#include "tools/time-measurement.h"
#include "tools/date.h"

using namespace std;
using namespace Climate;
using namespace Climate::Regionalization;
using namespace Grids;
using namespace Db;
using namespace boost;
using namespace Tools;

void createPrecipGrids(string folder, string sim, string scen,
                       string regionName, int km)
{
  GridPPtr dgm(new GridP("", GridP::ASCII,
                         string("../../data/grids/general/dgm_")+
                         regionName + "_100.asc"));

	Regionalization::Env env(precip);

	env.dgm = dgm.get();//->subGridClone(100, 100, 15, 15);
	env.fromYear = 1961;
	env.toYear = 2050;
	//env.acds = vector<ACD>(1, precip);
  ClimateSimulation* simulation = NULL;
  if(sim == "clm")
    simulation = new CLMSimulation(&newConnection("clm", true));
  else
    simulation = new WettRegSimulation(&newConnection("wettreg", true));
  env.realizations = simulation->scenario(scen)->realizations();
  env.borderSize = km;

	timeval before1 = startMeasurementViaTimeOfDay();
	AvgRealizationsResult rs = regionalizeAndAvgRealizationsSR(env);
	double secs1 = stopMeasurementViaTimeOfDay(before1);
	cout << "secs1: " << secs1 << endl;

  BOOST_FOREACH(const AvgRealizationsResult::value_type& p, rs)
  {
		Year year = p.first;
		GridPPtr g = p.second;

		ostringstream s;
		s << year << "_" << regionName << "_100.asc";
    cout << "writting ascii year: " << year << " to: "  << ((folder.empty() ? string() : folder + "/") + s.str()) << endl;
    g->writeAscii((folder.empty() ? string() : folder + "/") + s.str());
	}

  delete simulation;
	cout << "regionalization executed" << endl;
}

struct PrecipSum
{
  Date _from, _to;
  PrecipSum(Date from, Date to) : _from(from), _to(to) { }
  Regionalization::FuncResult operator()(DataAccessor da)
  {
    int startIndex = _from.dayOfYear() - 1;
    int endIndex = _to.dayOfYear() - 1;
    double value = 0;
    for(int i = startIndex; i < endIndex; i++)
      value += da.dataForTimestep(Climate::precip, i);

    Regionalization::FuncResult res;
    res[0] = value;
    return res;
  }
};

void fillGrowthPeriodPrecipCache(string real = "", Date start = Date(),
                                 Date end = Date(), string inputFile = "input.ini")
{
  //setting up model inputs
  IniParameterMap ipm(inputFile);
  string sim = ipm.value("fillGrowthPeriodPrecipCache", "simulation", "wettreg");
  string scen = ipm.value("fillGrowthPeriodPrecipCache", "scenario", "A1B");
  if(real.empty())
    real = ipm.value("fillGrowthPeriodPrecipCache", "realization", "");
  string pathToCache = ipm.value("fillGrowthPeriodPrecipCache", "pathToCache",
                                 "../data/hdfs/regionalized-climate-data/cache");
  int fromYear = ipm.valueAsInt("fillGrowthPeriodPrecipCache", "fromYear", 1986);
  int toYear = ipm.valueAsInt("fillGrowthPeriodPrecipCache", "toYear", 2100);
  int borderSize = ipm.valueAsInt("fillGrowthPeriodPrecipCache",
                                  "borderSize", 150);
  if(!start.isValid())
    start = ipm.valueAsRelativeDate("fillGrowthPeriodPrecipCache",
                                    "start", Date::relativeDate(10,5));
  if(!end.isValid())
    end = ipm.valueAsRelativeDate("fillGrowthPeriodPrecipCache",
                                  "end", Date::relativeDate(10,7));
  string regionName = ipm.value("fillGrowthPeriodPrecipCache",
                                "regionName", "uecker");

  GridPPtr dgm(new GridP("", GridP::ASCII,
                         string("../../data/grids/general/dgm_")+
                         regionName + "_100.asc"));

  Regionalization::Env renv(precip);
  renv.dgm = dgm.get();
  renv.fromYear = fromYear;
  renv.toYear = toYear;
  renv.borderSize = borderSize;
  //renv.acds = precip;
  ClimateSimulation* simulation = NULL;
  if(sim == "clm")
    simulation = new CLMSimulation(&newConnection("clm", true));
  else
    simulation = new WettRegSimulation(&newConnection("wettreg", true));
  if(real.empty())
    renv.realizations = simulation->scenario(scen)->realizations();
  else
  {
    renv.realizations.push_back(simulation->scenario(scen)->realization(real));
    cout << "loading only one realization: " << real << endl;
  }

  renv.cacheInfo.functionIdString = string("IrrigationNeed::runIrrigationNeed[") +
                                    start.toString() + "|" + end.toString() + "]";
  renv.cacheInfo.resultIds.push_back(0);
  renv.cacheInfo.pathToHdfCache = pathToCache;
  renv.cacheInfo.cacheData = true;
  renv.functionId = 0;
  renv.f = PrecipSum(start, end);

  timeval before1 = startMeasurementViaTimeOfDay();
  Regionalization::regionalize(renv);
  double secs1 = stopMeasurementViaTimeOfDay(before1);
  cout << "secs1: " << secs1 << endl;

  delete simulation;
  cout << "fill regionalization cache executed" << endl;
}

void example()
{
	bool createHdf = false;
	string regionName = "uecker";

  GridPPtr dgm(new GridP("", GridP::ASCII,
                         "../../data/grids/general/dgm_uecker_100.asc"));

	Regionalization::Env env(tavg);

	env.dgm = dgm->subGridClone(100, 100, 15, 15);
	env.fromYear = 1961;
	env.toYear = 1962;
	//env.acds = vector<ACD>(1, tavg);
	//env.simulation = new CLMSimulation(&newConnection("clm", true));
	WettRegSimulation* wrs = new WettRegSimulation(&newConnection("wettreg", true));
	env.realizations = wrs->scenario("A1B")->realizations();
	env.borderSize = 150;

	timeval before1 = startMeasurementViaTimeOfDay();
	Results rs = regionalize(env);
	double secs1 = stopMeasurementViaTimeOfDay(before1);
	cout << "secs1: " << secs1 << endl;

	timeval before2 = startMeasurementViaTimeOfDay();
	rs = regionalize(env);
	double secs2 = stopMeasurementViaTimeOfDay(before2);
	cout << "secs2: " << secs2 << endl;
  BOOST_FOREACH(const Results::value_type& p, rs)
  {
		ResultId rid = p.first;
		const Result& res = p.second;

    for(int year = env.fromYear; year <= env.toYear; year++)
    {
			Result::const_iterator ci = res.find(year);

      if(ci != res.end())
      {
        for(int i = 0, size = ci->second.size(); i < size; i++)
        {
					GridPPtr g = ci->second.at(i);
          if(createHdf)
          {
						ostringstream ptf, dsn;
						ptf << "res-" << rid << "-" << i << ".h5";
						dsn << year;
						cout << "writting hdf year: " << year << " realization: " << i << endl;
						g->writeHdf(ptf.str(), dsn.str(), regionName, 0);
          }
          else
          {
						ostringstream s;
						s << "res-" << rid << "-" << year << "-" << i << ".asc";
						cout << "writting ascii year: " << year << " realization: " << i << endl;
						g->writeAscii(s.str());
					}
				}
			}
		}
	}

	delete env.dgm;
	cout << "regionalization executed" << endl;
	return;
}

int main(int argc, char **argv)
{
  if(argc > 3)
    fillGrowthPeriodPrecipCache(argv[1],
                                fromMysqlString(argv[2]).toRelativeDate(),
                                fromMysqlString(argv[3]).toRelativeDate(),
				argc == 5 ? argv[4] : "input.ini");
  else
    fillGrowthPeriodPrecipCache();

  return 0;


  string folder = argc > 1 ? argv[1] : "";
  string sim = argc > 2 ? argv[2] : "wettreg";
  string scen = argc > 3 ? argv[3] : "A1B";
  string region = argc > 4 ? argv[4] : "uecker";
  int km = argc > 5 ? atoi(argv[5]) : 150;

  cout << "running with folder: " << folder << " sim: " << sim
      << " scen: " << scen << " region: " << region << " km: " << km << endl;

  createPrecipGrids(folder, sim, scen, region, km);

  return 0;
}
