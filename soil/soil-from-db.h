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

#ifndef MONICA_SOIL_FROM_DB_H
#define MONICA_SOIL_FROM_DB_H

#include <string>

#include "soil.h"
#include "db/db.h"

namespace Soil
{
	Soil::SoilPMsPtr soilParameters(Db::DBPtr dbConnection,
	                                int profileId,
	                                int layerThicknessCm = 10,
	                                int maxDepthCm = 200);

	Soil::SoilPMsPtr soilParameters(const std::string& abstractDbSchema,
	                                int profileId,
	                                int layerThicknessCm = 10,
	                                int maxDepthCm = 200);
}

#endif //MONICA_SOIL_FROM_DB_H
