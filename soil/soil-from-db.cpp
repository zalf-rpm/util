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

#include <sstream>
#include <iostream>
#include <map>
#include <set>
#include <utility>
#include <mutex>

#include "tools/algorithms.h"
#include "db/abstract-db-connections.h"
#include "tools/helper.h"
#include "soil-from-db.h"
#include "conversion.h"

using namespace std;
using namespace Soil;
using namespace Db;
using namespace Tools;

SoilPMsPtr Soil::soilParameters(DBPtr con,
                                int profileId,
                                int layerThicknessCm,
                                int maxDepthCm)
{
	static SoilPMsPtr nothing = make_shared<SoilPMs>();

	int maxNoOfLayers = int(double(maxDepthCm) / double(layerThicknessCm));

	DBRow row;

	ostringstream s2;
	s2 << "select "
			      "id, "
			      "layer_depth_cm, "
			      "soil_organic_carbon_percent, "
			      "soil_raw_density_kg_per_m3, "
			      "sand_content_percent, "
			      "clay_content_percent, "
			      "ph_value, "
			      "soil_type "
			      "from soil_profile ";
	s2 << "where id = " << profileId << " ";
	s2 << "order by id, layer_depth_cm";

	con->select(s2.str());
	//cout << "query: " << s2.str() << endl;
	auto sps = make_shared<SoilPMs>();
	size_t currenth = 0;
	size_t hcount = con->getNumberOfRows();
	while(!(row = con->getRow()).empty())
	{
		int id = satoi(row[0]);
		currenth++;

		int ho = int(sps->size()) * layerThicknessCm;
		int hu = !row[1].empty() ? satoi(row[1]) : maxDepthCm;
		int hsize = max(0, hu - ho);
		int subhcount = Tools::roundRT<int>(double(hsize) / double(layerThicknessCm), 0);
		if(currenth == hcount && (sps->size() + subhcount) < maxNoOfLayers)
			subhcount += maxNoOfLayers - sps->size() - subhcount;

		//cout << "id: " << id << " ho: " << ho << " hu: " << hu << " hsize: " << hsize
		//<< " subhcount: " << subhcount << " currenth: " << currenth << endl;

		SoilParameters p;
		p.set_vs_SoilOrganicCarbon(satof(row[2]) / 100.);
		p.set_vs_SoilRawDensity(satof(row[3]));
		p.vs_SoilSandContent = satof(row[4]) / 100.0;
		p.vs_SoilClayContent = satof(row[5]) / 100.0;
		if(!row[6].empty())
			p.vs_SoilpH = satof(row[6]);
		if(row[7].empty())
		{
			auto et = sandAndClay2KA5texture(p.vs_SoilSandContent, p.vs_SoilClayContent);
			if(et.success())
				p.vs_SoilTexture = et.result;
			else
			{
				for(auto e : et.errors)
					cerr << e << endl;
				return nothing;
			}
		}
		else
			p.vs_SoilTexture = row[7];
		p.vs_SoilStoneContent = 0.0;
		p.vs_Lambda = sandAndClay2lambda(p.vs_SoilSandContent, p.vs_SoilClayContent);

		// initialization of saturation, field capacity and perm. wilting point
		soilCharacteristicsKA5(p);
		if(!p.isValid())
		{
			cerr << "Error in soil parameters for profileId: " << id << "!" << endl;
			return nothing;
		}

		for(int i = 0; i < subhcount; i++)
			sps->push_back(p);
	}

	return sps;
}

//------------------------------------------------------------------------------

SoilPMsPtr Soil::soilParameters(const string& abstractDbSchema,
                                int profileId,
                                int layerThicknessCm,
                                int maxDepthCm)
{
	return soilParameters(DBPtr(newConnection(abstractDbSchema)),
	                      profileId, layerThicknessCm, maxDepthCm);
}

/*
const SoilPMsPtr Soil::soilParameters(const string& abstractDbSchema,
                                      int profileId,
                                      int layerThicknessCm,
                                      int maxDepthCm,
                                      bool loadSingleParameter)
{
	int maxNoOfLayers = int(double(maxDepthCm) / double(layerThicknessCm));

	static mutex lockable;

	typedef map<int, SoilPMsPtr> Map;
	typedef map<string, Map> Map2;
	static bool initialized = false;
	static Map2 spss2;

	//yet unloaded schema
	if(initialized && spss2.find(abstractDbSchema) == spss2.end())
		initialized = false;

	if(!initialized)
	{
		lock_guard<mutex> lock(lockable);

		if(!initialized)
		{
			DBPtr con(newConnection(abstractDbSchema));
			DBRow row;

			ostringstream s;
			s << "select id, count(id) "
					"from soil_profiles "
					"group by id";
			con->select(s.str().c_str());

			map<int, int> id2layerCount;
			while(!(row = con->getRow()).empty())
				id2layerCount[satoi(row[0])] = satoi(row[1]);
			con->freeResultSet();

			set<int> skip;

			ostringstream s2;
			s2 <<
			"select "
					"id, "
					"layer_depth_cm, "
					"soil_organic_carbon_percent, "
					"soil_raw_density_t_per_m3, "
					"sand_content_percent, "
					"clay_content_percent, "
					"ph_value, "
					"soil_type "
					"from soil_profiles ";
			if(loadSingleParameter)
				s2 << "where id = " << profileId << " ";
			s2 << "order by id, layer_depth_cm";

			Map& spss = spss2[abstractDbSchema];

			con->select(s2.str().c_str());
			int currenth = 0;
			while(!(row = con->getRow()).empty())
			{
				int id = satoi(row[0]);

				//Skip elements which are incomplete
				if(skip.find(id) != skip.end())
					continue;

				SoilPMsPtr sps = spss[id];
				if(!sps)
				{
					sps = spss[id] = SoilPMsPtr(new SoilPMs);
					currenth = 0;
				}

				int hcount = id2layerCount[id];
				currenth++;

				int ho = int(sps->size()) * layerThicknessCm;
				int hu = !row[1].empty() ? satoi(row[1]) : maxDepthCm;
				int hsize = max(0, hu - ho);
				int subhcount = Tools::roundRT<int>(double(hsize) / double(layerThicknessCm), 0);
				if(currenth == hcount && (int(sps->size()) + subhcount) < maxNoOfLayers)
					subhcount += maxNoOfLayers - sps->size() - subhcount;

				SoilParameters p;
				p.set_vs_SoilOrganicCarbon(satof(row[2]) / 100.);
				p.set_vs_SoilRawDensity(satof(row[3]));
				p.vs_SoilSandContent = satof(row[4]) / 100.0;
				p.vs_SoilClayContent = satof(row[5]) / 100.0;
				if(!row[6].empty())
					p.vs_SoilpH = satof(row[6]);
				if(row[7].empty())
					p.vs_SoilTexture = sandAndClay2KA5texture(p.vs_SoilSandContent, p.vs_SoilClayContent);
				else
					p.vs_SoilTexture = row[7];
				p.vs_SoilStoneContent = 0.0;
				p.vs_Lambda = sandAndClay2lambda(p.vs_SoilSandContent, p.vs_SoilClayContent);

				// initialization of saturation, field capacity and perm. wilting point
				soilCharacteristicsKA5(p);
				if(!p.isValid())
				{
					skip.insert(id);
					cout << "Error in soil parameters. Skipping profileId: " << id << endl;
					spss.erase(id);
					continue;
				}

				for(int i = 0; i < subhcount; i++)
					sps->push_back(p);
			}

			initialized = true;
		}
	}

	static SoilPMsPtr nothing = make_shared<SoilPMs>();
	auto ci2 = spss2.find(abstractDbSchema);
	if(ci2 != spss2.end())
	{
		Map& spss = ci2->second;
		Map::const_iterator ci = spss.find(profileId);
		return ci != spss.end() ? ci->second : nothing;
	}

	return nothing;
}
*/


//------------------------------------------------------------------------------
