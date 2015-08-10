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

#ifndef CLIMATE_H_
#define CLIMATE_H_

#include <cstdlib>
#include <vector>
#include <map>
#include <list>
#include <set>
#include <string>
#include <cmath>
#include <utility>
#include <sstream>
#include <iostream>
#include <functional>
#include <mutex>

#include "tools/date.h"
#include "db/db.h"
#include "tools/coord-trans.h"
#include "tools/algorithms.h"
#include "tools/helper.h"
#include "tools/read-ini.h"

#include "climate/climate-common.h"

//! All climate data related classes.
namespace Climate
{
	class ClimateSimulation;

	/*!
	 * ClimateStation represents an abstract climate station. This means
	 * that usually it can be identified with a real climate station
	 * (eg. in Muencheberg, for the WettReg climate simulation), but it could
	 * also be, that a ClimateStation object is just some LatLngCoord geocoordinate
	 * which stands for a grid point in eg. the CLM climate simulation.
	 * In the end the ClimateRealization's climate data method just cares
	 * about geo coordinates.
	 */
  class ClimateStation
  {
	public:
    //! location of climate station, needed for precipitation correction of WettReg and CLM
		enum SL { f = 1, lg = 2, mg = 3, sg = 4 };

	public:
    ClimateStation() {}

		/*!
		 * create ClimateStation
		 * @param id
		 * @param geoPos latitude/longitude geoposition
		 * @param nn height above NN
		 * @param name a human readable/useable name
		 * @param simulation the climate simulation this station belongs to
		 * @return
		 */
		ClimateStation(int id, const Tools::LatLngCoord& geoPos, double nn,
                   const std::string& name, ClimateSimulation* simulation = 0)
      : _id(id),
        _name(name),
        _sl(mg),
        _geoCoord(geoPos),
        _nn(nn),
        _simulation(simulation)
    {}

    bool operator==(const ClimateStation& other) const { return _id == other._id; }

    bool operator<(const ClimateStation& other) const { return _name < other._name; }

    std::string name() const
    {
      return isPrecipStation() && _fullClimateReferenceStation
          ? _name + " (" + _fullClimateReferenceStation->name() + ")"
          : _name;
    }

		int id() const { return _id; }

		SL sl() const { return _sl; }

		void setSL(SL sl){ _sl = sl; }

		std::string dbName() const { return _dbName; }

		void setDbName(const std::string& dbn){ _dbName = dbn; }

		std::string toString() const;

    //! height over NN
    double nn() const { return _nn; }

    bool isPrecipStation() const { return _isPrecipStation; }
    void setIsPrecipStation(bool isPS) { _isPrecipStation = isPS; }

    Tools::LatLngCoord geoCoord() const { return _geoCoord; }

    //! rectangular coord in a give coordinate system
    Tools::RectCoord rcCoord(Tools::CoordinateSystem cs) const { return Tools::latLng2RC(geoCoord(), cs); }

		ClimateSimulation* simulation() const { return _simulation; }

    ClimateStation* fullClimateReferenceStation() const { return _fullClimateReferenceStation; }
    void setFullClimateReferenceStation(ClimateStation* fullClimateStation){ _fullClimateReferenceStation = fullClimateStation; }
	private: //state
		int _id;
		std::string _name;
		std::string _dbName;
		SL _sl;
		Tools::LatLngCoord _geoCoord;
		double _nn;
    ClimateSimulation* _simulation{NULL};
    bool _isPrecipStation{false};
    ClimateStation* _fullClimateReferenceStation{NULL};
	};

	//----------------------------------------------------------------------------
  	
	class ClimateRealization;
	class ZalfWettRegRealization;
	class StarRealization;

	class ClimateScenario;

	//! other name for std::vector of ClimateStation pointers
	typedef std::vector<ClimateStation*> Stations;

	//! other name for std::vector of ClimateScenario pointers
	typedef std::vector<ClimateScenario*> Scenarios;

	//! other name for std::vector of ClimateRealization pointers
	typedef std::vector<ClimateRealization*> Realizations;

	//! represents a climatesimulation as WettReg, CLM or Star
	/*!
	 * A ClimateSimulation top of the hierarchy for getting climate data.
	 * A ClimateSimulation consists potentially of different ClimateScenarios
	 * and every ClimateScenario consists potentially of a bunch of
	 * ClimateRealizations. Usually these classes are just abstract superclasses
	 * and for the particular simulations exist derived classes which implement
	 * the particular specifics of the simulation. Sometimes eg. Star doesn't have
	 * ClimateScenarios and only one ClimateRealization, then things can be simpler.
	 *
	 * The user of these classes usually instantiates a particular ClimateSimulation
	 * object (eg. WettRegSimulation), gets from the object a sought for
	 * ClimateScenario and might then get from the choosen scenario one or more
	 * ClimateRealization objects. Once a particular ClimateRealization object
	 * is available, it can be asked for a DataAccessor object (climate data) in
	 * a particular time range with a requested amount of different climate data
	 * elements.
	 */
  class ClimateSimulation
  {
	public:
		//! takes ownerwhip of connection
    ClimateSimulation(const std::string& id, const std::string& name,
											Db::DB* connection) :
    _name(name), _id(id), _connection(connection) {}

		virtual ~ClimateSimulation();

		//! name of this particular climate simulation
		std::string name() const { return _name; }

    //! id of this particular climate simulation
    std::string id() const { return _id; }

		//! all scenarios available for this climate simulation
		const Scenarios& scenarios(){ return _scenarios; }

		/*!
		 * get a climate scenario by name
		 * @param name of ClimateScenario (eg. A1B)
		 * @return the requested climate scenario or NULL if not available
		 */
    ClimateScenario* scenario(const std::string& name) const;

    ClimateScenario* scenarioById(const std::string& id) const;

		/*!
		 * @return returns a default scenario (usually needed in connection with
		 * UI usage, eg. first selected element in a Combobox with all
		 * ClimateScenarios of a some particular ClimateSimulation)
		 */
		virtual ClimateScenario* defaultScenario() const = 0;

		//! all climate stations available for this simulation
    const Stations& climateStations() const { return _stations; }

		//! get a station via its id
    ClimateStation climateStation(const std::string& stationName) const;

		//! all geoCoordinates for this simulation
		virtual std::vector<Tools::LatLngCoord> geoCoords() const;

		//! get geo coord from a given climate station id
		Tools::LatLngCoord
    climateStation2geoCoord(const std::string& stationName) const;

		//! get climatestation by a given geocoord
		ClimateStation geoCoord2climateStation(const Tools::LatLngCoord& gc) const;

		/*!
		 * gets the closest geocoordinate where climate data (a climatestation)
		 * is availabel for the given parameter
		 * @param gc geocoordinate
		 * @return closest geocoordinate with climate data for given parameter
		 */
		Tools::LatLngCoord
		getClosestClimateDataGeoCoord(const Tools::LatLngCoord& gc) const;

		/*!
		 * @return DB connection used to access climate data for this simulation
		 * (right now all realizations of a simulation use the
		 * same database connection, this one)
		 */
		Db::DB& connection() const { return *(_connection.get()); }

    virtual YearRange availableYearRange() { return _yearRange; }

	protected: //state
		//! all climate stations available for this simulation
		Stations _stations;

		//! the list of scenarios for this simulation
		Scenarios _scenarios;

		//! the list of realizations for this simulation (for different scenarios)
		Realizations _realizations;

    YearRange _yearRange;

    std::mutex _lockable;
	private: //state
		//! the simulations name
		std::string _name;

    //! the simulation id
    std::string _id;

		//! connection to db
		Db::DBPtr _connection;
	};

	//----------------------------------------------------------------------------

  ClimateSimulation* createSimulationFromSetupData(const Tools::IniParameterMap& dbParams,
                                              const std::string& abstractSchema);

	struct DDServerSetup
	{
    DDServerSetup() {}

    DDServerSetup(std::map<std::string, std::string> setupSectionMap);

		DDServerSetup(std::string simulationId, std::string simulationName,
									std::string headerTableName, std::string stolistTableName,
									std::string dataDbName, std::string dataTableName,
									std::string errorTableName = std::string(),
									std::string headerDbName = "project_landcare")
			: _simulationId(simulationId),
				_simulationName(simulationName),
				_headerDbName(headerDbName),
				_headerTableName(headerTableName),
				_stolistTableName(stolistTableName),
				_dataDbName(dataDbName),
				_dataTableName(dataTableName),
				_errorTableName(errorTableName)
		{}

		std::string simulationId() const { return _simulationId; }
		std::string simulationName() const { return _simulationName; }

    std::string headerDbName() const
    {
      return _headerDbName.empty() ? dataDbName() : _headerDbName;
    }
		std::string headerTableName() const { return _headerTableName; }

		std::string stolistDbName() const
		{
			return _stolistDbName.empty() ? headerDbName() : _stolistDbName;
		}
		std::string stolistTableName() const { return _stolistTableName; }

		std::string dataDbName() const { return _dataDbName; }
		std::string dataTableName() const { return _dataTableName;  }

    std::vector<std::string> scenarioIds() const { return _scenarioIds; }
    std::vector<std::string> realizationIds() const { return _realizationIds; }

    bool useErrorTable() const { return !errorTableName().empty(); }
		std::string errorDbName() const
		{
			return _errorDbName.empty() ? dataDbName() : _errorDbName;
		}
		std::string errorTableName() const { return _errorTableName; }

    YearRange yearRange;

    bool setupComplete() const { return _setupComplete; }

  private:
		std::string _simulationId;
		std::string _simulationName;

		std::string _headerDbName;
		std::string _headerTableName;

    std::string _stolistDbName;
		std::string _stolistTableName;

    std::string _dataDbName;
		std::string _dataTableName;

    std::string _errorDbName;
    std::string _errorTableName;

    std::vector<std::string> _scenarioIds;
    std::vector<std::string> _realizationIds;

    bool _setupComplete{false};
	};

	class DDClimateDataServerSimulation : public ClimateSimulation
	{
	public:
		DDClimateDataServerSimulation(const DDServerSetup& setupData,
																	Db::DB* connection);

		virtual ClimateScenario* defaultScenario() const;

		std::string defaultScenarioId() const
		{
			return _setupData.scenarioIds().empty() ? "" : _setupData.scenarioIds().front();
		}

		virtual YearRange availableYearRange();

	private:
		//! loads all climate stations
		void setClimateStations();
		//! set all scenarios for wettreg
		void setScenariosAndRealizations();

		DDServerSetup _setupData;
	};

	//----------------------------------------------------------------------------

  class UserSqliteDBSimulation : public ClimateSimulation
	{
	public:
    UserSqliteDBSimulation(Db::DB* connection);

		virtual ClimateScenario* defaultScenario() const;

		virtual YearRange availableYearRange() { return YearRange(1996, 2025); }

	private:
		//! loads all climate stations
		void setClimateStations();
	};

	//----------------------------------------------------------------------------

	//! Star simulation data
	/*!
	 * Access to star simulation data, Zalf only.
	 * Note: Star has only one (implicit) scenario, as well as only one
	 * realization.
	 */
  class StarSimulation : public ClimateSimulation
  {
	public:
		StarSimulation(Db::DB* connection);

		virtual ClimateScenario* defaultScenario() const;

    virtual YearRange availableYearRange() { return YearRange(1951, 2055); }

	private:
		//! loads all climate stations
		void setClimateStations();
	};

  //----------------------------------------------------------------------------

  class Star2Simulation : public ClimateSimulation
  {
  public:
		Star2Simulation(Db::DB* connection);

    virtual ClimateScenario* defaultScenario() const;

    virtual YearRange availableYearRange() { return YearRange(1951, 2060); }

  private:
    //! loads all climate stations
    void setClimateStations();
    void setScenariosAndRealizations();
  };

  //----------------------------------------------------------------------------

  class Star2MeasuredDataSimulation : public ClimateSimulation
  {
  public:
		Star2MeasuredDataSimulation(Db::DB* connection);

    virtual ClimateScenario* defaultScenario() const;

    virtual YearRange availableYearRange() { return YearRange(1951, 2006); }

  private:
    //! loads all climate stations
    void setClimateStations();
  };


	//----------------------------------------------------------------------------

	//! CLM Simulation
	/*!
	 * Access to the CLM simulation, at the Zalf or TU-Dresden.
	 * Note, that CLM only has 1 realization, but 2 scenarios (A1B, B1).
	 */
  class CLMSimulation : public ClimateSimulation
  {
	public:
		CLMSimulation(Db::DB* connection);

		virtual ClimateScenario* defaultScenario() const;

    std::set<const ClimateStation*> avgClimateStationSet(const ClimateStation* c);

    virtual YearRange availableYearRange();

  private: //methods
		//! loads all climate stations
		void setClimateStations();
		void setScenariosAndRealizations();

  private: //state
    typedef std::map<const ClimateStation*, std::set<const ClimateStation*>,
    std::function<bool(const ClimateStation*, const ClimateStation*)>> M;
    M _avgClimateStationsSet;
	};

	//----------------------------------------------------------------------------

	//! represents a scenario assumed for a climate simulation
  class ClimateScenario
  {
	public:
		ClimateScenario()
    : _realizations(Realizations()), _name("---"), _id(_name), _simulation(0) {}
		ClimateScenario(const std::string& name, ClimateSimulation* simulation,
                    const Realizations& realizations = Realizations()) :
    _realizations(realizations), _name(name), _id(_name),
    _simulation(simulation) {}
    ClimateScenario(const std::string& id, const std::string& name,
                    ClimateSimulation* simulation,
                    const Realizations& realizations = Realizations()) :
    _realizations(realizations), _name(name), _id(id),
    _simulation(simulation) {}

		virtual ~ClimateScenario(){}

		//! get the name of this scenario
		std::string name() const { return _name; }

    //! get id of scenario
    std::string id() const { return _id; }

		//! return the realizations available for this scenario
    Realizations realizations() const { return _realizations; }

    ClimateRealization* realization(const std::string& name) const;

    void setRealizations(Realizations rs)
    {
			_realizations.insert(_realizations.end(), rs.begin(), rs.end());
		}

		//! return the simulation this scenario belongs to
		ClimateSimulation* simulation() const { return _simulation; }

	private: //state
		//! the list of realizations
		Realizations _realizations;

		//! the scenarios name
		std::string _name;

    //! the scenario id
    std::string _id;

		//! belongs to this simulation
		ClimateSimulation* _simulation;
	};

	//----------------------------------------------------------------------------

  class Star2Scenario : public ClimateScenario
  {
  public:
    Star2Scenario(const std::string& id, const std::string& name,
                  const std::string& dbPrefix, ClimateSimulation* simulation,
                  const Realizations& realizations = Realizations()) :
    ClimateScenario(id, name, simulation, realizations), _prefix(dbPrefix) {}

    virtual ~Star2Scenario(){}

    //! the suffix needed for db access
    std::string dbPrefix() const { return _prefix; }

  private: //state
    std::string _prefix;
  };

	//----------------------------------------------------------------------------

	//! a cache structure for some data (e.g. avg temperature)
  struct Cache
  {
		Cache() {}

    Cache(unsigned int num) : _cache(num) {}

		//! the start date for the all the current values
		Tools::Date startDate;
		Tools::Date endDate;

		//! offset into cache
		std::vector<unsigned int> offsets;

		unsigned int getNewOffsetIndexFor(const Tools::Date& start);
		unsigned int offsetFor(const Tools::Date& start) const
    {
			return startDate.numberOfDaysTo(start);
		}

    bool isInitialized() const
    {
			return startDate.isValid() && endDate.isValid();
		}

		double at(unsigned int index) const { return _cache.at(index); }

    size_t size() const { return _cache.size(); }

	private:
		std::vector<double> _cache;
		friend class ClimateRealization;
	};

	//----------------------------------------------------------------------------

	//! encapsulates efficient access to climate database
	class ClimateRealization
  {
  public:
		ClimateRealization(const std::string& id, ClimateSimulation* simulation,
											 ClimateScenario* s, Db::DB* connection) :
		_id(id), _con(connection), _simulation(simulation), _scenario(s){}

    virtual ~ClimateRealization(){}

    //! just fills the cache, but doesn't return any data
    void fillCacheFor(const std::vector<AvailableClimateData>& acds,
											const Tools::LatLngCoord& geoCoord,
											const Tools::Date& startDate,
											const Tools::Date& endDate);

    //! get data deep copied without references to climate cache
    DataAccessor dataAccessorFor(const std::vector<AvailableClimateData>& acds,
																 const Tools::LatLngCoord& geoCoord,
																 const Tools::Date& startDate,
																 const Tools::Date& endDate);

    ClimateSimulation* simulation() const { return _simulation; }

    ClimateScenario* scenario() const { return _scenario; }

		virtual std::string id() const { return _id; }

		virtual std::string name() const {  return _name.empty() ? id() : _name; }

		virtual void setName(std::string newName){ _name = newName; }

  protected:
		Db::DB& connection() const { return *(_con.get()); }

    //! caller takes care of returned pointer to data-vector
    virtual std::map<ACD, std::vector<double>*>
				executeQuery(const ACDV& acds, const Tools::LatLngCoord& geoCoord,
										 const Tools::Date& startDate,
										 const Tools::Date& endDate) const = 0;

    std::mutex _lockable;
  private: //methods
    //! create list of acds not completely in cache
    ACDV notInCache(const std::vector<Cache>& cs, const ACDV& acds,
										const Tools::Date& startDate,
										const Tools::Date& endDate) const;
    //! create list of all lists with common start and end in cache
    std::vector<ACDV> commonStartEnd(const std::vector<Cache>& cs,
                                     const ACDV& acds,
																		 const Tools::Date& startDate,
																		 const Tools::Date& endDate) const;
    //! adjust the cache for acds with common start/end date
    void updateCaches(std::vector<Cache>& cs, ACDV acds,
											const Tools::LatLngCoord& geoCoord,
											const Tools::Date& startDate,
											const Tools::Date& endDate);

  private:
		std::string _id;
		std::string _name;

    //! the connection to a climate database
		Db::DBPtr _con;
    ClimateSimulation* _simulation;
    ClimateScenario* _scenario;

		std::map<Tools::LatLngCoord, std::vector<Cache> > _geoCoord2cache;

//    friend void testClimate();
	};

	//----------------------------------------------------------------------------

	class StarSimulation;

  class StarRealization : public ClimateRealization
  {
	public:
		StarRealization(StarSimulation* simulation, ClimateScenario* s,
										Db::DB* connection) :
    ClimateRealization("1", simulation, s, connection) {}

		virtual ~StarRealization(){}

		DataAccessor dataAccessorFor(const std::vector<AvailableClimateData>& acds,
		                             const std::string& stationName,
																 const Tools::Date& startDate,
																 const Tools::Date& endDate);

	protected:
		virtual std::map<ACD, std::vector<double>*>
				executeQuery(const ACDV& acds, const Tools::LatLngCoord& geoCoord,
										 const Tools::Date& startDate,
										 const Tools::Date& endDate) const;
	};

  //----------------------------------------------------------------------------

  class UserSqliteDBRealization : public ClimateRealization
	{
	public:
    UserSqliteDBRealization(UserSqliteDBSimulation* simulation, ClimateScenario* s,
													Db::DB* connection) :
			ClimateRealization("1", simulation, s, connection) {}

    virtual ~UserSqliteDBRealization(){}

		DataAccessor dataAccessorFor(const std::vector<AvailableClimateData>& acds,
																 const std::string& stationName,
																 const Tools::Date& startDate,
																 const Tools::Date& endDate);

	protected:
		virtual std::map<ACD, std::vector<double>*>
		executeQuery(const ACDV& acds, const Tools::LatLngCoord& geoCoord,
								 const Tools::Date& startDate,
								 const Tools::Date& endDate) const;
	};

	//----------------------------------------------------------------------------

  class Star2Realization : public ClimateRealization
  {
  public:
    Star2Realization(Star2Simulation* simulation, Star2Scenario* s,
										 Db::DB* connection, int realizationNo) :
		ClimateRealization(Tools::toString(realizationNo), simulation, s,
                       connection){}

    virtual ~Star2Realization(){}

    DataAccessor dataAccessorFor(const std::vector<AvailableClimateData>& acds,
                                 const std::string& stationName,
																 const Tools::Date& startDate,
																 const Tools::Date& endDate);

  protected:
    virtual std::map<ACD, std::vector<double>*>
				executeQuery(const ACDV& acds, const Tools::LatLngCoord& geoCoord,
										 const Tools::Date& startDate,
										 const Tools::Date& endDate) const;
  };

  //----------------------------------------------------------------------------

  class Star2MeasuredDataRealization : public ClimateRealization
  {
  public:
    Star2MeasuredDataRealization(Star2MeasuredDataSimulation* simulation,
                                 ClimateScenario* s,
																 Db::DB* connection) :
    ClimateRealization("measured", simulation, s, connection) {}

    virtual ~Star2MeasuredDataRealization(){}

    DataAccessor dataAccessorFor(const std::vector<AvailableClimateData>& acds,
                                 const std::string& stationName,
																 const Tools::Date& startDate,
																 const Tools::Date& endDate);

  protected:
    virtual std::map<ACD, std::vector<double>*>
				executeQuery(const ACDV& acds, const Tools::LatLngCoord& geoCoord,
										 const Tools::Date& startDate,
										 const Tools::Date& endDate) const;
  };

	//----------------------------------------------------------------------------

	class DDClimateDataServerRealization : public ClimateRealization
	{
	public:
		DDClimateDataServerRealization(std::string id,
																	 DDClimateDataServerSimulation* simulation,
																	 ClimateScenario* s, Db::DB* connection,
																	 const DDServerSetup& setupData)
			: ClimateRealization(id, simulation, s, connection), _scenario(s),
				_setupData(setupData)
		{}

		virtual ~DDClimateDataServerRealization(){}

		DataAccessor dataAccessorFor(const std::vector<AvailableClimateData>& acds,
																 const std::string& stationName,
																 const Tools::Date& startDate,
																 const Tools::Date& endDate);

	protected:
		virtual std::map<ACD, std::vector<double>*>
				executeQuery(const ACDV& acds, const Tools::LatLngCoord& geoCoord,
										 const Tools::Date& startDate,
										 const Tools::Date& endDate) const;

	private: //state
		ClimateScenario* _scenario;
		DDServerSetup _setupData;
	};

	//----------------------------------------------------------------------------

  class CLMRealization : public ClimateRealization
  {
	public:
		CLMRealization(CLMSimulation* simulation, ClimateScenario* s,
									 std::string realizationNo, Db::DB* connection) :
    ClimateRealization(realizationNo, simulation, s, connection),
    _scenario(s), _realizationNo(realizationNo) { }

		virtual ~CLMRealization(){}

		DataAccessor dataAccessorFor(const std::vector<AvailableClimateData>& acds,
		                             const std::string& stationName,
																 const Tools::Date& startDate,
																 const Tools::Date& endDate);

	protected:
		virtual std::map<ACD, std::vector<double>*>
				executeQuery(const ACDV& acds, const Tools::LatLngCoord& geoCoord,
										 const Tools::Date& startDate,
										 const Tools::Date& endDate) const;

	private: //state
		//! realization belongs to this scenario
		ClimateScenario* _scenario;
		std::string _realizationNo;
	};

	//----------------------------------------------------------------------------

  class ClimateDataManager
  {
	public:
		~ClimateDataManager();

    void loadAvailableSimulations(std::set<std::string> availableSimulations =
                                  std::set<std::string>());

		std::vector<ClimateSimulation*> allClimateSimulations() const;

		ClimateSimulation* defaultSimulation() const;
	private:
		std::vector<ClimateSimulation*> _simulations;
	};

	//! has to be called the first time with valid db initparameters
	ClimateDataManager& climateDataManager();

//	DDClimateDataServerSimulation* newDDWettReg2006(std::string userRs = std::string());
//	DDClimateDataServerSimulation* newDDWettReg2010(std::string userRs = std::string());
//	DDClimateDataServerSimulation* newDDRemo();
//	DDClimateDataServerSimulation* newDDWerex4(std::string userRs = std::string());
//	DDClimateDataServerSimulation* newDDWerex5_eh5_l1(std::string userRs = std::string());
//	DDClimateDataServerSimulation* newDDWerex5_eh5_l1_clm(std::string userRs = std::string());
//	DDClimateDataServerSimulation* newDDWerex5_eh5_l2(std::string userRs = std::string());
//	DDClimateDataServerSimulation* newDDWerex5_eh5_l2_clm(std::string userRs = std::string());
//	DDClimateDataServerSimulation* newDDWerex5_eh5_l3(std::string userRs = std::string());
//	DDClimateDataServerSimulation* newDDWerex5_eh5_l3_racmo(std::string userRs = std::string());
//	DDClimateDataServerSimulation* newDDWerex5_eh5_l3_remo(std::string userRs = std::string());
//	DDClimateDataServerSimulation* newDDWerex5_hc3c_l1_a1b(std::string userRs = std::string());
//	DDClimateDataServerSimulation* newDDWerex5_hc3c_l1_e1(std::string userRs = std::string());
//	DDClimateDataServerSimulation* newDDClm20(std::string userRs = std::string());
//	DDClimateDataServerSimulation* newDDEcham5(std::string userRs = std::string());
//	DDClimateDataServerSimulation* newDDEcham6(std::string userRs = std::string());
//	DDClimateDataServerSimulation* newDDHrm3(YearRange yr, std::string userRs = std::string());
//	DDClimateDataServerSimulation* newDDCru(std::string userRs = std::string());
//  DDClimateDataServerSimulation* newDDDwdNrw(std::string userRs = std::string());


	//----------------------------------------------------------------------------

	inline double potentialEvaporationTW(double globRad_Jpcm2,
                                       double tavg, double fk = 1)
  {
		return (globRad_Jpcm2+93*fk)*(tavg+22)/(150*(tavg+123));
	}

	inline double climaticWaterBalanceTW(double precip_mm, double globRad_Jpcm2,
                                       double tavg, double fk = 1)
  {
		return precip_mm - potentialEvaporationTW(globRad_Jpcm2, tavg, fk);
	}

	//----------------------------------------------------------------------------

	void testClimate();
}

#endif
