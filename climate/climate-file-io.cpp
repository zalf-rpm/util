/* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/*
Authors:
Michael Berg <michael.berg@zalf.de>

Maintainers:
Currently maintained by the authors.

This file is part of the util library used by models created at the Institute of
Landscape Systems Analysis at the ZALF.
Copyright (C) Leibniz Centre for Agricultural Landscape Research (ZALF)
*/

#include <map>
#include <sstream>
#include <iostream>
#include <fstream>
#include <cmath>
#include <utility>
#include <cassert>
#include <mutex>
#include <ctime>
#include <cstdlib>

#include "db/abstract-db-connections.h"
#include "climate/climate-common.h"
#include "tools/helper.h"
#include "tools/algorithms.h"

#include "carbiocial.h"
#include "tools/debug.h"
#include "core/monica-parameters.h"
#include "soil/conversion.h"
#include "core/simulation.h"
#include "io/database-io.h"
#include "io/hermes-file-io.h"
#include "run/production-process.h"

using namespace Db;
using namespace std;
using namespace Carbiocial;
using namespace Monica;
using namespace Tools;
using namespace Soil;

std::pair<const SoilPMs *, int>
Carbiocial::carbiocialSoilParameters(int profileId, int layerThicknessCm,
int maxDepthCm, GeneralParameters gps, string output_path, CentralParameterProvider centralParameterProvider)
{
	//cout << "getting soilparameters for STR: " << str << endl;
	int maxNoOfLayers = int(double(maxDepthCm) / double(layerThicknessCm));

	static mutex lockable;

	typedef int ProfileId;
	typedef int SoilClassId;

	typedef map<ProfileId, SoilPMsPtr> Map;
	typedef map<ProfileId, SoilClassId> PId2SCId;
	static bool initialized = false;
	static Map spss;
	static PId2SCId profileId2soilClassId;

	if (!initialized)
	{
    lock_guard<mutex> lock(lockable);

		if (!initialized)
		{
			DBPtr con(newConnection("carbiocial"));
			DBRow row;

			ostringstream s;
			s << "select id, count(horizon_id) "
				"from soil_profile_data "
				"where id not null "
				"group by id";
			con->select(s.str().c_str());

			map<int, int> id2layerCount;
			while (!(row = con->getRow()).empty())
				id2layerCount[satoi(row[0])] = satoi(row[1]);
			con->freeResultSet();

			ostringstream s2;
			s2 << "select id, horizon_id, soil_class_id, "
				"upper_horizon_cm, lower_horizon_cm, "
				"silt_percent, clay_percent, sand_percent, "
				"ph_kcl, c_org_percent, c_n, bulk_density_t_per_m3 "
				"from soil_profile_data "
				"where id not null "
				"order by id, horizon_id";
			con->select(s2.str().c_str());

			while (!(row = con->getRow()).empty())
			{
				ProfileId id = ProfileId(satoi(row[0]));

				//SoilClassId soilClassId = SoilClassId(satoi(row[2]));
				//if (profileId2soilClassId.find(id) == profileId2soilClassId.end())
				//	profileId2soilClassId[id] = soilClassId;

				Map::iterator spsi = spss.find(id);
				SoilPMsPtr sps;

				if (spsi == spss.end())
					spss.insert(make_pair(id, sps = SoilPMsPtr(new SoilPMs)));
				else
					sps = spsi->second;

				int hcount = id2layerCount[int(id)];
				int currenth = satoi(row[1]);

				int ho = sps->size()*layerThicknessCm;
				int hu = satoi(row[4]) ? satoi(row[4]) : maxDepthCm;
				int hsize = max(0, hu - ho);
				int subhcount = Tools::roundRT<int>(double(hsize) / double(layerThicknessCm), 0);
				if (currenth == hcount && (int(sps->size()) + subhcount) < maxNoOfLayers)
					subhcount += maxNoOfLayers - sps->size() - subhcount;

				SoilParameters p;
				if (!row[8].empty())
					p.vs_SoilpH = satof(row[8]);
				p.set_vs_SoilOrganicCarbon(row[9].empty() ? 0 : satof(row[9]) / 100.0);
				p.set_vs_SoilRawDensity(satof(row[11]));
				p.vs_SoilSandContent = satof(row[7]) / 100.0;
				p.vs_SoilClayContent = satof(row[6]) / 100.0;
				p.vs_SoilTexture = sandAndClay2KA5texture(p.vs_SoilSandContent, p.vs_SoilClayContent);
				p.vs_SoilStoneContent = 0.0;
        p.vs_Lambda = sandAndClay2lambda(p.vs_SoilSandContent, p.vs_SoilClayContent);

				// initialization of saturation, field capacity and perm. wilting point
				soilCharacteristicsKA5(p);

				bool valid_soil_params = p.isValid();
				if (!valid_soil_params)
				{
					cout << "Error in soil parameters. Aborting now simulation";
					exit(-1);
				}

				for (int i = 0; i < subhcount; i++)
					sps->push_back(p);
			}

			initialized = true;
		}
	}

	static SoilPMs nothing;
	Map::const_iterator ci = spss.find(profileId);

	if(activateDebug)
	{
		///////////////////////////////////////////////////////////
		// write soil information of each individual soil layer to file
		
		const SoilPMs* sps = ci->second.get();

		ostringstream soildata_file;
		soildata_file << profileId << ".txt";
		ofstream file;
		file.open((soildata_file.str()).c_str());
		if (file.fail()) {
			debug() << "Error while opening output file \"" << soildata_file.str().c_str() << "\"" << endl;
		}
		file << "Layer;Saturation [Vol-%];FC [Vol-%];PWP [Vol-%];BoArt;Sand;Clay;Dichte [kg m-3];LeachingDepth" << endl;
		
		int i = 0;
		for (auto p : *ci->second)
		{
			file << i++ << ";" << p.vs_Saturation * 100.0
					<< ";" << p.vs_FieldCapacity * 100.0
					<< ";" << p.vs_PermanentWiltingPoint * 100.0
					<< ";" << p.vs_SoilTexture.c_str()
					<< ";" << p.vs_SoilSandContent
					<< ";" << p.vs_SoilClayContent
					<< ";" << p.vs_SoilRawDensity()
					<< ";" << centralParameterProvider.userEnvironmentParameters.p_LeachingDepth
					<< endl;
		}
		file.close();
	}
	return ci != spss.end() ? make_pair(ci->second.get(), -1)//profileId2soilClassId[profileId])
		: make_pair(&nothing, -1);
}


/**
* Method for starting a simulation with coordinates from ascii grid.
* A configuration object is passed that stores all relevant
* information e.g. location, output path etc.
*
* @param simulation_config Configuration object that stores all simulation information
*/
std::map<int, double> Carbiocial::runCarbiocialSimulation(const CarbiocialConfiguration* simulation_config)
{
	//std::cout << "Start Carbiocial cluster Simulation" << std::endl;
	std::string input_path = simulation_config->getInputPath();
	std::string output_path = simulation_config->getOutputPath();

	// read in ini - file ------------------------------------------
	IniParameterMap ipm(simulation_config->getPathToIniFile());

	std::string crop_rotation_file = ipm.value("files", "croprotation");
	std::string fertilisation_file = ipm.value("files", "fertiliser");

	//std::cout << "soil_file: " << soil_file.c_str() << "\t" << ipm.value("files", "soil").c_str() << endl;
	//std::cout << "crop_rotation_file: " << crop_rotation_file.c_str() << endl;
	//std::cout << "fertilisation_file: " << fertilisation_file.c_str() << endl << endl;

	//site configuration
	double latitude = ipm.valueAsDouble("site_parameters", "latitude", -1.0);
	//  double slope = ipm.valueAsDouble("site_parameters", "slope", -1.0);
	//  double heightNN = ipm.valueAsDouble("site_parameters", "heightNN", -1.0);
	double CN_ratio = ipm.valueAsDouble("site_parameters", "soilCNRatio", -1.0);
	double atmospheric_CO2 = ipm.valueAsDouble("site_parameters", "atmospheric_CO2", -1.0);
	double wind_speed_height = ipm.valueAsDouble("site_parameters", "wind_speed_height", -1.0);
	double leaching_depth = ipm.valueAsDouble("site_parameters", "leaching_depth", -1.0);
	//  double critical_moisture_depth = ipm.valueAsDouble("site_parameters", "critical_moisture_depth", -1.0);
	//double ph = ipm.valueAsDouble("site_parameters", "pH", -1.0);

	//  std::cout << "latitude: " << latitude << endl;
	//  std::cout << "slope: " << slope << endl;
	//  std::cout << "heightNN: " << heightNN << endl;
	//  std::cout << "CN_ratio: " << CN_ratio << endl;
	//  std::cout << "atmospheric_CO2: " << atmospheric_CO2 << endl;
	//  std::cout << "wind_speed_height: " << wind_speed_height << endl;
	//  std::cout << "leaching_depth: " << leaching_depth << endl;
	//  std::cout << "critical_moisture_depth: " << critical_moisture_depth << endl;
	//  std::cout << "ph: " << ph << endl << endl;

	// general parameters
	bool n_response = ipm.valueAsInt("general_parameters", "nitrogen_response_on", 1) == 1;
	bool water_deficit_response = ipm.valueAsInt("general_parameters", "water_deficit_response_on", 1) == 1;
	bool emergence_flooding_control = ipm.valueAsInt("general_parameters", "emergence_flooding_control_on", 1) == 1;
	bool emergence_moisture_control = ipm.valueAsInt("general_parameters", "emergence_moisture_control_on", 1) == 1;
	//  std::cout << "n_response: " << n_response << endl;
	//  std::cout << "water_deficit_response: " << water_deficit_response << endl << endl;

	// initial values
	double init_FC = ipm.valueAsDouble("init_values", "init_percentage_FC", -1.0);

	//  std::cout << "init_FC: " << init_FC << endl;
	// ---------------------------------------------------------------------

	CentralParameterProvider centralParameterProvider = readUserParameterFromDatabase();
	centralParameterProvider.userEnvironmentParameters.p_AthmosphericCO2 = atmospheric_CO2;
	centralParameterProvider.userEnvironmentParameters.p_WindSpeedHeight = wind_speed_height;
	centralParameterProvider.userEnvironmentParameters.p_LeachingDepth = leaching_depth;
	centralParameterProvider.userInitValues.p_initPercentageFC = init_FC;

	SiteParameters siteParams;
	siteParams.vs_Latitude = simulation_config->getLatitude();
	siteParams.vs_Slope = 0.01;
	siteParams.vs_HeightNN = simulation_config->getElevation();
	siteParams.vs_Soil_CN_Ratio = CN_ratio; // TODO: xeh CN_Ratio aus Bodendatei auslesen

	//cout << "Site parameters " << endl;
	//cout << siteParams.toString().c_str() << endl;

	double layer_thickness = centralParameterProvider.userEnvironmentParameters.p_LayerThickness;
	double profile_depth = layer_thickness * double(centralParameterProvider.userEnvironmentParameters.p_NumberOfLayers);
	double max_mineralisation_depth = 0.4;

	GeneralParameters gps = GeneralParameters(layer_thickness);
	gps.ps_ProfileDepth = profile_depth;
	gps.ps_MaxMineralisationDepth = max_mineralisation_depth;
	gps.pc_NitrogenResponseOn = n_response;
	gps.pc_WaterDeficitResponseOn = water_deficit_response;
	gps.pc_EmergenceFloodingControlOn = emergence_flooding_control;
	gps.pc_EmergenceMoistureControlOn = emergence_moisture_control;

	//soil data
	const SoilPMs* sps;

	//  std::string project_id = simulation_config->getProjectId();
	//  std::string lookup_project_id = simulation_config->getLookupProjectId();

	pair<const SoilPMs*, int> p =
		Carbiocial::carbiocialSoilParameters(simulation_config->getProfileId(),
		int(layer_thickness * 100), int(profile_depth * 100), 
		 gps, output_path, centralParameterProvider);
	sps = p.first;

	//no soil available, return no yields
	if (sps->empty())
		return std::map<int, double>();

	//	sps = soilParametersFromCarbiocialFile(soil_file, gps, ph);

	//cout << "Groundwater min:\t" << centralParameterProvider.userEnvironmentParameters.p_MinGroundwaterDepth << endl;
	//cout << "Groundwater max:\t" << centralParameterProvider.userEnvironmentParameters.p_MaxGroundwaterDepth << endl;
	//cout << "Groundwater month:\t" << centralParameterProvider.userEnvironmentParameters.p_MinGroundwaterDepthMonth << endl;

	// climate data
	Climate::DataAccessor climateData = climateDataFromCarbiocialFiles(simulation_config->getClimateFile(), centralParameterProvider, 
	latitude, simulation_config);
	//cout << "Return from climateDataFromMacsurFiles" << endl;

	// fruchtfolge
	vector<ProductionProcess> ff = cropRotationFromHermesFile(input_path + crop_rotation_file);
	//cout << "Return from cropRotationFromHermesFile" << endl;
	
	// fertilisation
	attachFertiliserApplicationsToCropRotation(ff, input_path + fertilisation_file);
	//cout << "Return from attachFertiliserApplicationsToCropRotation" << endl;
	//  for(const ProductionProcess& pv : ff)
	//    cout << "pv: " << pv.toString() << endl;
	
	//build up the monica environment
	Env env(sps, centralParameterProvider);
	env.general = gps;
	env.pathToOutputDir = output_path;
	env.setMode(MODE_CARBIOCIAL_CLUSTER);
	env.site = siteParams;
	env.writeOutputFiles = simulation_config->writeOutputFiles;

	env.da = climateData;
	env.cropRotation = ff;

	if (ipm.valueAsInt("automatic_irrigation", "activated") == 1) 
	{
		double amount = ipm.valueAsDouble("automatic_irrigation", "amount", 0.0);
		double treshold = ipm.valueAsDouble("automatic_irrigation", "treshold", 0.15);
		double nitrate = ipm.valueAsDouble("automatic_irrigation", "nitrate", 0.0);
		double sulfate = ipm.valueAsDouble("automatic_irrigation", "sulfate", 0.0);
		//  std::cout << "use_automatic_irrigation: " << use_automatic_irrigation << endl;
		//  std::cout << "amount: " << amount << endl;
		//  std::cout << "treshold: " << treshold << endl;
		//  std::cout << "nitrate: " << nitrate << endl;
		//  std::cout << "sulfate: " << sulfate << endl << endl;

		env.useAutomaticIrrigation = true;
		env.autoIrrigationParams = AutomaticIrrigationParameters(amount, treshold, nitrate, sulfate);
	}
	//cout << env.toString() << endl;
	//cout << "Before runMonica" << endl;
	Monica::Result res = runMonica(env);
	//cout << "After runMonica" << endl;

	std::map<int, double> year2yield;
	int ey = env.da.endDate().year();
	for (int i = 0, size = res.pvrs.size(); i < size; i++)
	{
		int year = ey - size + 1 + i;
		double yield = res.pvrs[i].pvResults[primaryYieldTM] / 10.0;
		debug() << "year: " << year << " yield: " << yield << " tTM" << endl;
		year2yield[year] = Tools::round(yield, 3);
	}

	return year2yield;
}

/**
* Read climate information from macsur file
*/
Climate::DataAccessor Carbiocial::climateDataFromCarbiocialFiles(const std::string& pathToFile,
	const CentralParameterProvider& cpp, double latitude, const CarbiocialConfiguration* simulationConfig)
{
	bool reorderData = simulationConfig->create2013To2040ClimateData;
	map<int, map<int, map<int, vector<int>>>> year2month2day2oldDMY;
	if (reorderData)
	{
		string pathToReorderingFile = simulationConfig->pathToClimateDataReorderingFile;
		
		cout << "pathToReorderingFile: " << pathToReorderingFile << endl;
		cout << "reorderData: " << reorderData << endl;

		ifstream ifs(pathToReorderingFile.c_str());
		if (!ifs.good()) {
			cerr << "Could not open file " << pathToReorderingFile << " . Aborting now!" << endl;
			exit(1);
		}

		string s;
		while (getline(ifs, s)) 
		{
			istringstream iss(s);
			string arrow;
			int ty(0), tm(0), td(0);
			vector<int> from(3);
			
			iss >> td >> tm >> ty >> arrow >> from[0] >> from[1] >> from[2];

			//cout << "arrow: " << arrow << " to[0]: " << to[0] << " to[1]: " << to[1] << " to[2]: " << to[2] << " fd: " << fd << " fm: " << fm << " fy: " << fy << endl;
			
			if(ty > 0 && tm > 0 && td > 0)
				year2month2day2oldDMY[ty][tm][td] = from;
		}
	}

	//cout << "climateDataFromMacsurFiles: " << pathToFile << endl;
	Climate::DataAccessor da(simulationConfig->getStartDate(), simulationConfig->getEndDate());

	Date startDate = simulationConfig->getStartDate();
	Date endDate = simulationConfig->getEndDate();
	int dayCount = endDate - startDate + 1;
//	cout << "startDate: " << startDate.toString() << endl;
//	cout << "endDate: " << endDate.toString() << endl;
	
	ifstream ifs(pathToFile.c_str(), ios::binary);
	if (!ifs.good()) {
		cerr << "Could not open file " << pathToFile.c_str() << " . Aborting now!" << endl;
		exit(1);
	}

	//if have to store all data before adding them to the DataAccessor
	//because the climate files may contain duplicate entries and might
	//not be fully ordered due to problems during processing
	typedef vector<double> Values;
	typedef map<int, Values> DailyValues;
	typedef map<int, DailyValues> MonthlyValues;
	typedef map<int, MonthlyValues> YearlyValues;
	YearlyValues data;
	
	string s;
	while (getline(ifs, s)) 
	{
		//skip (repeated) headers
		if (s.substr(0, 3) == "day")
			continue;

		vector<string> r = splitString(s, ",");
		if(r.size() < 11)
			continue;

		unsigned int day = satoi(r.at(0));
		unsigned int month = satoi(r.at(1));
		unsigned int year = satoi(r.at(2));

		if (!reorderData)
		{
			if (year < startDate.year())
				continue;
			else if (month < startDate.month())
				continue;
			else if (day < startDate.day())
				continue;

			if (year > endDate.year())
				continue;
			else if (month > endDate.month())
				continue;
			else if (day > endDate.day())
				continue;
		}
		
		//double tmin, tavg, tmax ,precip, globrad, relhumid, windspeed;
		vector<double> d;
		
		d.push_back(satof(r.at(4))); //tmin
		d.push_back(satof(r.at(5))); //tavg
		d.push_back(satof(r.at(6))); //tmax
		d.push_back(satof(r.at(7))); //precip
		d.push_back(satof(r.at(8))); //globrad
		d.push_back(satof(r.at(9))); //relhumid
		d.push_back(satof(r.at(10))); //windspeed

		//cout << "[" << day << "." << month << "." << year << "] -> [";
		//for(auto v : d)
		//	cout << v << " ";
		//cout << "]" << endl;

		data[year][month][day] = d;
	}
	
	cout << endl;

	vector<double> tmins, tavgs, tmaxs, precips, globrads, relhumids, winds;

	int daysAdded = 0;
	for (Date d = startDate, ed = endDate; d <= ed; d++)
	{
		//cout << "date: " << d.toString() << endl;
		int year = d.year();
		int month = d.month();
		int day = d.day();
		
		if (reorderData && year >= 2013 && year <= 2040)
		{
			auto from = year2month2day2oldDMY[year][month][day];
			if (!from.empty())
			{
				year = from[2];
				month = from[1];
				day = from[0];
			}
		}
	
		auto v = data[year][month][day];

		//cout << "[" << day << "." << month << "." << year << "] -> [";
		//for(auto vv : v)
		//	cout << vv << " ";
		//cout << "]" << endl;
		
		if(v.size() < 7)
    {
			cerr << "Error: no, or too few, climate elements at date [dd.mm.yyyy]: "
				<< day << "." << month << "." << year << " available." << endl;
			
			cerr << "[" << day << "." << month << "." << year << "] -> [";
			for(auto vv : v)
				cerr << vv << " ";
			cerr << "]" << endl;
			
			Date pd(d);
			pd--;
			
			if(pd < startDate)
			{
				cout << "Stopping." << endl;
				exit(1);
			}
			else
				cout << "Continuing, but using data from previous day!" << endl;
			
      v = data[pd.year()][pd.month()][pd.day()];
		}
		
		tmins.push_back(v.at(0));
		tavgs.push_back(v.at(1));
		tmaxs.push_back(v.at(2));
		precips.push_back(v.at(3));
		globrads.push_back(v.at(4));
		relhumids.push_back(v.at(5));
		winds.push_back(v.at(6));

		daysAdded++;
	}

	//  cout << "daysAdded: " << daysAdded << endl;
	if (daysAdded != dayCount) {
		cout << "Wrong number of days in " << pathToFile.c_str() << " ." << " Found " << daysAdded << " days but should have been "
			<< dayCount << " days. Aborting." << endl;
		exit(1);
	}

	da.addClimateData(Climate::tmin, tmins);
	da.addClimateData(Climate::tmax, tmaxs);
	da.addClimateData(Climate::tavg, tavgs);
	da.addClimateData(Climate::precip, precips);
	da.addClimateData(Climate::globrad, globrads);
	da.addClimateData(Climate::relhumid, relhumids);
	da.addClimateData(Climate::wind, winds);

	return da;
}

int main(int argc, char** argv)
{
	setlocale(LC_ALL, "");
	setlocale(LC_NUMERIC, "C");

	//use the non-default db-conections-core.ini
	dbConnectionParameters("db-connections.ini");

	CarbiocialConfiguration cc;
  	
	bool activateDebugOutput = true; bool activateDebugOutputSet = false;
	bool writeOutputFiles = false; bool writeOutputFilesSet = false;
	
	int row = 0; //bool rowSet = false;
	int col = 0; //bool colSet = false;
	int profileId = 1; bool profileIdSet = false;

	string startDate = "1981-01-01"; bool startDateSet = false;
	string endDate = "2012-12-31"; bool endDateSet = false;

	string pathToIniFile = "../carbiocial-project/monica/run-local-example-inputs/maize.ini"; 

	string pathToClimateDataFile = string("/media/archiv/md/berg/carbiocial/climate-data-years-1981-2012-rows-0-2544/") + "row-" + to_string(row) + "/col-" + to_string(col) + ".asc"; bool pathToClimateDataFileSet = false;
	string pathToClimateDataReorderingFile = "/media/archiv/md/berg/carbiocial/final_order_dates_l9_sres_a1b_2013-2040.dat";

	//time_t t = time(NULL);
	//char mbstr[100];
	//strftime(mbstr, sizeof(mbstr), "%Y-%m-%d", std::localtime(&t));
    
	string outputPath = "../carbiocial-project/monica/run-local-example-outputs/"; bool outputPathSet = false; //+ mbstr + "/"; 
	string inputPath = "../carbiocial-project/monica/run-local-example-inputs/"; bool inputPathSet = false;

	if(argc == 2 || (argc > 1 && (argc-1) % 2 == 1))
	{
		cout << "usage: monica " << endl
			<< "\t (in general type case can be ignored)" << endl
			<< "\t -h or --help (get this help text)" << endl
			<< "\t[" << endl
			<< "\t| pathToIniFile: " << pathToIniFile << " (path to MONICA ini file)" << endl
			<< "\t| startDate: " << startDate << " [yyyy-mm-dd]" << endl
			<< "\t| endDate: " << endDate << " [yyyy-mm-dd]" << endl
			//<< "\t| row: " << row << endl
			//<< "\t| col: " << col << endl
			<< "\t| profileId: " << profileId << endl
			<< "\t| activateDebugOutput: " << (activateDebugOutput ? "true" : "false") << " [true | false] (should monica output debug information (on screen))" << endl
			<< "\t| writeOutputFiles: " << (writeOutputFiles ? "true" : "false") << " [true | false] (should monica write output files (rmout, smout))" << endl
			<< "\t| pathToClimateDataFile: " << pathToClimateDataFile <<  endl
			<< "\t| pathToInputs: " << inputPath << " (path to input data, e.g. monica.ini)" <<  endl
			<< "\t| pathToOutputs: " << outputPath << " (path to output data, e.g. rmout, smout)" << endl
			<< "\t]*" << endl;
			
			return 0;
	}
	else
	{
		for(int i = 1; i < argc; i+=2)
		{
			//cout << "argv[" << i << "] = |" << argv[i] 
			//		<< "| argv[" << (i+1) << "] = |" << argv[i+1] << "|" << endl;
			
			if(toLower(string(argv[i])) == trim("pathtoinifile:"))
			{
				pathToIniFile = trim(argv[i+1]);
				//iniFileSet = true;
			}
			else if(toLower(string(argv[i])) == trim("startdate:"))
			{
				startDate = trim(argv[i+1]);
				startDateSet = true;
			}
			else if(toLower(string(argv[i])) == trim("enddate:"))
			{
				endDate = trim(argv[i+1]);
				endDateSet = true;
			}
			//else if(toLower(string(argv[i])) == trim("row:"))
			//{
			//	row = stoi(argv[i+1]);
			//	rowSet = true;
			//}
			//else if(toLower(string(argv[i])) == trim("col:"))
			//{
			//	col = stoi(argv[i+1]);
			//	colSet = true;
			//}
			else if(toLower(string(argv[i])) == trim("profileid:"))
			{
				profileId = stoi(argv[i+1]);
				profileIdSet = true;
			}
			else if(toLower(string(argv[i])) == trim("activatedebugoutput:"))
			{
				activateDebugOutput = stob(argv[i+1]);
				activateDebugOutputSet = true;
			}
			else if(toLower(string(argv[i])) == trim("writeoutputfiles:"))
			{
				writeOutputFiles = stob(argv[i+1]);
				writeOutputFilesSet = true;
			}
			else if(toLower(string(argv[i])) == trim("pathtoclimatedatafile:"))
			{
				pathToClimateDataFile = trim(argv[i+1]);
				pathToClimateDataFileSet = true;
			}
			else if(toLower(string(argv[i])) == trim("pathtoinputs:"))
			{
				inputPath = trim(argv[i+1]);
				inputPathSet = true;
			}
			else if(toLower(string(argv[i])) == trim("pathtooutputs:"))
			{
				outputPath = trim(argv[i+1]);
				outputPathSet = true;
			}
		}
	}
	
	IniParameterMap ipm(pathToIniFile);
	if(!startDateSet)
		startDate = ipm.value("carbiocial", "startDate", startDate);
	if(!endDateSet)
		endDate = ipm.value("carbiocial", "endDate", endDate);
	//if(!rowSet)
	//	row = ipm.valueAsInt("carbiocial", "row", row);
	//if(!colSet)
	//	col = ipm.valueAsInt("carbiocial", "col", col);
	if(!profileIdSet)
		profileId = ipm.valueAsInt("carbiocial", "profileId", profileId);
	if(!activateDebugOutputSet)
		activateDebugOutput = ipm.valueAsBool("carbiocial", "activateDebugOutput", activateDebugOutput);
	if(!writeOutputFilesSet)
		writeOutputFiles = ipm.valueAsBool("carbiocial", "writeOutputFiles", writeOutputFiles);
	if(!pathToClimateDataFileSet)
		pathToClimateDataFile = ipm.value("carbiocial", "pathToClimateDataFile", pathToClimateDataFile);
	if(!inputPathSet)
		inputPath = ipm.value("carbiocial", "inputPath", inputPath);
	if(!outputPathSet)
		outputPath = ipm.value("carbiocial", "outputPath", outputPath);
	
	cout << "starting monica with the following parameters:" << endl;
	cout << "\t| pathToIniFile: " << pathToIniFile << endl
			<< "\t| startDate: " << startDate << endl
			 << "\t| endDate: " << endDate << endl
			 //<< "\t| row: " << row << endl
			 //<< "\t| col: " << col << endl
			 << "\t| activateDebugOutput: " << (activateDebugOutput ? "true" : "false") << endl
			 << "\t| writeOutputFiles: " << (writeOutputFiles ? "true" : "false") << endl
			 << "\t| pathToClimateDataFile: " << pathToClimateDataFile << endl
			 << "\t| pathToInputs: " << inputPath << endl
			 << "\t| pathToOutputs: " << outputPath << endl;
	
	cc.setInputPath(inputPath);
	cc.setPathToIniFile(pathToIniFile);
	//cc.pathToClimateDataReorderingFile = pathToClimateDataReorderingFile;
	//cc.create2013To2040ClimateData = true;
	//cc.setCropName(crop);
 
	cc.setStartDate(startDate);
	cc.setEndDate(endDate);
	cc.setClimateFile(pathToClimateDataFile);
	//cc.setClimateFile(pathToCarbiocialData+"input_data/row-0/col-0.asc");
	//cc.setRowId(row);
	//cc.setColId(col);
	cc.setProfileId(profileId);

#ifdef WIN32
	string mkdir = "mkdir ";
#else
	string mkdir = "mkdir -p ";
#endif
	system((mkdir + outputPath).c_str());
	cc.setOutputPath(outputPath);

	cc.writeOutputFiles = writeOutputFiles;
	activateDebug = activateDebugOutput;
	
	cc.setLatitude(ipm.valueAsDouble("carbiocial", "latitude", -9.41));
	cc.setElevation(ipm.valueAsDouble("carbiocial", "elevation", 300.0));
		
	auto year2yields = runCarbiocialSimulation(&cc);
	cout << "results [year -> yield]: " << endl;
	for(auto y2y : year2yields)
		cout << y2y.first << " -> " << y2y.second << endl;
	cout << endl;
	
	return 0;
}
