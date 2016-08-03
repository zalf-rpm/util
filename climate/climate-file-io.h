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

#ifndef CLIMATE_FILE_IO_H_
#define CLIMATE_FILE_IO_H_

#include <string>
#include <istream>

#include "climate-common.h"

namespace Climate
{
	inline std::vector<ACD> defaultHeader()
	{
		return{deDate, tmin, tavg, tmax, precip, globrad, relhumid, wind};
	}

	struct CSVViaHeaderOptions
	{
		CSVViaHeaderOptions() : separator(",") {}
		std::string separator;
		Tools::Date startDate;
		Tools::Date endDate;
		size_t noOfHeaderLines{2};
		std::map<std::string, std::string> headerName2ACDName;
	};
	Climate::DataAccessor
		readClimateDataFromCSVInputStreamViaHeaders(std::istream& inputStream,
																								CSVViaHeaderOptions options = CSVViaHeaderOptions());

	Climate::DataAccessor
		readClimateDataFromCSVFileViaHeaders(std::string pathToFile,
		                                     CSVViaHeaderOptions options = CSVViaHeaderOptions());

	Climate::DataAccessor
		readClimateDataFromCSVStringViaHeaders(std::string csvString,
																					 CSVViaHeaderOptions options = CSVViaHeaderOptions());

	Climate::DataAccessor
		readClimateDataFromCSVInputStream(std::istream& inputStream,
																			std::string separator = ",",
																			std::vector<ACD> header = std::vector<ACD>(),
																			Tools::Date startDate = Tools::Date(),
																			Tools::Date endDate = Tools::Date(),
																			size_t noOfHeaderLines = 2);

	Climate::DataAccessor
		readClimateDataFromCSVFile(std::string pathToFile,
															 std::string separator = ",",
															 std::vector<ACD> header = std::vector<ACD>(),
															 Tools::Date startDate = Tools::Date(),
															 Tools::Date endDate = Tools::Date(),
															 size_t noOfHeaderLines = 2);

	Climate::DataAccessor
		readClimateDataFromCSVString(std::string csvString,
																 std::string separator = ",",
																 std::vector<ACD> header = std::vector<ACD>(),
																 Tools::Date startDate = Tools::Date(),
																 Tools::Date endDate = Tools::Date(),
																 size_t noOfHeaderLines = 2);
}

#endif
