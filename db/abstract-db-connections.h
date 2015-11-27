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

#ifndef ABSTRACT_DB_CONNECTIONS_H_
#define ABSTRACT_DB_CONNECTIONS_H_

#include <string>

#include "db.h"
#include "tools/read-ini.h"

namespace Db
{
	/*!
	 * returns the db connections parameter map
	 * - upon the first call the user can change where to find the ini-file
	 * - subsequent calls with a parameter won't change the initial map anymore
	 */
	Tools::IniParameterMap& dbConnectionParameters
		(const std::string& initialPathToIniFile = "db-connections.ini");

	//! connection data for schema from given parameter map
	DBConData dbConData(const Tools::IniParameterMap& dbParams,
	                    const std::string& abstractSchema);

	//! connection data via default parameter map
	inline DBConData dbConData(const std::string& abstractSchema)
	{
		return dbConData(dbConnectionParameters(), abstractSchema);
	}

	//! new connection from given parameters
	DB* newConnection(const Tools::IniParameterMap& dbParameters,
										const std::string& abstractSchema);

	//! new connection from default parameters
	inline DB* newConnection(const std::string& abstractSchema)
	{
		return newConnection(dbConnectionParameters(), abstractSchema);
	}

	bool attachDB(DB* connection, 
								std::string attachAbstractSchema,
								std::string alias);
}

#endif
