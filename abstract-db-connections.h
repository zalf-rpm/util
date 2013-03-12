/**
Authors: 
Michael Berg <michael.berg@zalf.de>

Maintainers: 
Currently maintained by the authors.

This file is part of the util library used by models created at the Institute of 
Landscape Systems Analysis at the ZALF.
<one line to give the program's name and a brief idea of what it does.>
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

#ifndef ABSTRACT_DB_CONNECTIONS_H_
#define ABSTRACT_DB_CONNECTIONS_H_

#include <string>

#include "db.h"
#include "read-ini.h"

namespace Db
{
	/*!
	 * returns the db connections parameter map
	 * - upon the first call the user can change where to find the ini-file
	 * - subsequent calls with a parameter won't change the initial map anymore
	 */
	Util::IniParameterMap& 
    dbConnectionParameters(const std::string& initialPathToIniFile = "db-connections.ini");

	//! connection data for schema from given parameter map
	DBConData dbConData(const Util::IniParameterMap& dbParams,
	                    const std::string& abstractSchema);

	//! connection data via default parameter map
	inline DBConData dbConData(const std::string& abstractSchema)
	{
		return dbConData(dbConnectionParameters(), abstractSchema);
	}

	//! new connection from given parameters
	DB* newConnection(const Util::IniParameterMap& dbParameters,
										const std::string& abstractSchema);

	//! new connection from default parameters
	inline DB* newConnection(const std::string& abstractSchema)
	{
		return newConnection(dbConnectionParameters(), abstractSchema);
	}

	bool attachDB(DB* connection, std::string attachAbstractSchema,
								std::string alias);
}

#endif
