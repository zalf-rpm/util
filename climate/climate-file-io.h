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
#include <vector>
#include <istream>

#include "tools/json11-helper.h"
#include "climate-common.h"
#include "common/dll-exports.h"


namespace Climate
{
	//inline std::vector<ACD> defaultHeader()
	//{
	//	return{deDate, tmin, tavg, tmax, precip, globrad, relhumid, wind};
	//}

	struct CSVViaHeaderOptions : public Tools::Json11Serializable
	{
		CSVViaHeaderOptions();

		CSVViaHeaderOptions(json11::Json object);

		virtual Tools::Errors merge(json11::Json j);

		virtual json11::Json to_json() const;

		std::string separator;
		Tools::Date startDate;
		Tools::Date endDate;
		int noOfHeaderLines{1};
		double latitude{0};
		std::map<std::string, std::string> headerName2ACDName;
		std::map<std::string, std::pair<std::string, double>> convert;
		std::map<Climate::ACD, std::function<double(double)>> convertFn;
	};
	Climate::DataAccessor
		readClimateDataFromCSVInputStreamViaHeaders(std::istream& inputStream,
																								CSVViaHeaderOptions options = CSVViaHeaderOptions(),
																								bool strictDateChecking = true);

	Climate::DataAccessor
		readClimateDataFromCSVFileViaHeaders(std::string pathToFile,
		                                     CSVViaHeaderOptions options = CSVViaHeaderOptions());

	Climate::DataAccessor
		readClimateDataFromCSVFilesViaHeaders(std::vector<std::string> pathsToFiles,
																					CSVViaHeaderOptions options = CSVViaHeaderOptions());

	Climate::DataAccessor
		readClimateDataFromCSVStringViaHeaders(std::string csvString,
																					 CSVViaHeaderOptions options = CSVViaHeaderOptions());

	Climate::DataAccessor
		readClimateDataFromCSVInputStream(std::istream& inputStream,
																			std::vector<ACD> header,
																			CSVViaHeaderOptions options,
																			bool strictDateChecking = true);
}

extern "C" DLL_API char* Climate_readClimateDataFromCSVStringViaHeaders(const char* csvString, const char* options);
extern "C" DLL_API void Climate_freeCString(char* str);

#endif
