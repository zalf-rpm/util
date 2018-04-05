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
#include "tools/json11-helper.h"
#include "tools/debug.h"

using namespace std;
using namespace Soil;
using namespace Db;
using namespace Tools;
using namespace json11;

json11::Json Soil::jsonSoilParameters(DBPtr con,
																			int profileId)
{
	DBRow row;

	enum { 
		id = 0, 
		layer_depth, 
		soil_organic_carbon,
		soil_organic_matter,
		bulk_density, 
		raw_density,
		sand,
		clay, 
		ph, 
		KA5_texture_class, 
		permanent_wilting_point, 
		field_capacity, 
		saturation, 
		soil_water_conductivity_coefficient, 
		sceleton, 
		soil_ammonium, 
		soil_nitrate, 
		c_n, 
		initial_soil_moisture ,
		layer_description,
		is_in_groundwater,
		is_impenetrable
	};

	ostringstream oss;
	oss <<
		"select "
		"id, "
		"layer_depth, "
		"soil_organic_carbon, "
		"soil_organic_matter, "
		"bulk_density, "
		"raw_density, "
		"sand, "
		"clay, "
		"ph, "
		"KA5_texture_class, "
		"permanent_wilting_point, "
		"field_capacity, "
		"saturation, "
		"soil_water_conductivity_coefficient, "
		"sceleton, "
		"soil_ammonium, "
		"soil_nitrate, "
		"c_n, "
		"initial_soil_moisture, "
		"layer_description, "
		"is_in_groundwater, "
		"is_impenetrable "
		"from soil_profile "
		"where id = " << profileId << " "
		"order by id, layer_depth";

	con->select(oss.str());
	//cout << "query: " << oss.str() << endl;

	J11Array layers;
	double prev_depth = 0;
	while(!(row = con->getRow()).empty())
	{
		J11Object layer = {{"type", "SoilParameters"}};
		if(!row[layer_depth].empty())
		{
			double depth = stof(row[layer_depth]);
			layer["Thickness"] = J11Array{depth - prev_depth, "m"};
			prev_depth = depth;
		}

		if(!row[KA5_texture_class].empty())
			layer["KA5TextureClass"] = row[KA5_texture_class];

		if(!row[sand].empty())
			layer["Sand"] = J11Array{stof(row[sand]) / 100.0, "% [0-1]"};

		if(!row[clay].empty())
			layer["Clay"] = J11Array{stof(row[clay]) / 100.0, "% [0-1]"};

		if(!row[ph].empty())
			layer["pH"] = stof(row[ph]);

		if(!row[sceleton].empty())
			layer["Sceleton"] = J11Array{stof(row[sceleton]) / 100.0, "vol% [0-1]"};

		if(!row[soil_organic_carbon].empty())
			layer["SoilOrganicCarbon"] = J11Array{stof(row[soil_organic_carbon]), "mass% [0-100]"};
		else if(!row[soil_organic_matter].empty())
			layer["SoilOrganicMatter"] = J11Array{stof(row[soil_organic_matter]) / 100.0, "mass% [0-1]"};

		if(!row[bulk_density].empty())
			layer["SoilBulkDensity"] = J11Array{stof(row[bulk_density]), "kg m-3"};
		else if(!row[raw_density].empty())
			layer["SoilRawDensity"] = J11Array{stof(row[raw_density]), "kg m-3"};

		if(!row[field_capacity].empty())
			layer["FieldCapacity"] = J11Array{stof(row[field_capacity]) / 100.0, "vol% [0-1]"};

		if(!row[permanent_wilting_point].empty())
			layer["PermanentWiltingPoint"] = J11Array{stof(row[permanent_wilting_point]) / 100.0, "vol% [0-1]"};

		if(!row[saturation].empty())
			layer["PoreVolume"] = J11Array{stof(row[saturation]) / 100.0, "vol% [0-1]"};

		if(!row[initial_soil_moisture].empty())
			layer["SoilMoisturePercentFC"] = J11Array{stof(row[initial_soil_moisture]), "% [0-100]"};

		if(!row[soil_water_conductivity_coefficient].empty())
			layer["Lambda"] = stof(row[soil_water_conductivity_coefficient]);

		if(!row[soil_ammonium].empty())
			layer["SoilAmmonium"] = J11Array{stof(row[soil_ammonium]), "kg NH4-N m-3"};

		if(!row[soil_nitrate].empty())
			layer["SoilNitrate"] = J11Array{stof(row[soil_nitrate]), "kg NO3-N m-3"};

		if(!row[c_n].empty())
			layer["CN"] = stof(row[c_n]);

		if(!row[layer_description].empty())
			layer["description"] = row[layer_description];

		if(!row[is_in_groundwater].empty())
			layer["is_in_groundwater"] = stoi(row[is_in_groundwater]) == 1;

		if(!row[is_impenetrable].empty())
			layer["is_impenetrable"] = stoi(row[is_impenetrable]) == 1;

		//keyset = layer.keys()
		auto found = [&layer](string key){ return layer.find(key) != layer.end(); };
		bool layerIsOk =
			found("Thickness")
			&& (found("SoilOrganicCarbon")
					|| found("SoilOrganicMatter"))
			&& (found("SoilBulkDensity")
					|| found("SoilRawDensity"))
			&& (found("KA5TextureClass")
					|| (found("Sand") && found("Clay"))
					|| (found("PermanentWiltingPoint")
							&& found("FieldCapacity")
							&& found("PoreVolume")
							&& found("Lambda")));

		if(layerIsOk)
			layers.push_back(layer);
		else
			debug() << "Layer: " << Json(layer).dump() << " is incomplete. Skipping it!";
	}
	
	return layers;
}

json11::Json Soil::jsonSoilParameters(const string& abstractDbSchema,
																		int profileId)
{
	return jsonSoilParameters(DBPtr(newConnection(abstractDbSchema)),
														profileId);
}

SoilPMsPtr Soil::soilParameters(DBPtr con,
																int profileId)
{
	Json j = jsonSoilParameters(con, profileId);
	auto p = createSoilPMs(j.array_items());

	if(p.second.failure())
	{
		cerr << "Error while reading soil parameters for profileId: " << profileId << "! Errors: " << endl;
		for(auto e : p.second.errors)
			cerr << e << endl;
	}

	return p.first;
}

//------------------------------------------------------------------------------

SoilPMsPtr Soil::soilParameters(const string& abstractDbSchema,
																int profileId)
{
	return soilParameters(DBPtr(newConnection(abstractDbSchema)),
												profileId);
}

//------------------------------------------------------------------------------
