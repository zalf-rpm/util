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

#include <iostream>

#include "db/abstract-db-connections.h"

using namespace std;
using namespace Tools;
using namespace Db;

IniParameterMap& Db::dbConnectionParameters(const std::string& pathToIniFile)
{
	static IniParameterMap ipm(pathToIniFile);
	return ipm;
}

DBConData Db::dbConData(const IniParameterMap& dbParams,
												const string& abstractSchema)
{
	string dbSection = dbParams.value("abstract-schema", abstractSchema);

	DBConData d;
	//if not empty is a sqlite db and we're done
	d.filename = dbParams.value(dbSection, "filename");
	//else threat it as mysql db with all the connection parameters
	if(d.filename.empty())
	{
		d.host = dbParams.value(dbSection, "host");
		d.port = (unsigned int)dbParams.valueAsInt(dbSection, "port", 0);
		d.user = dbParams.value(dbSection, "user");
		d.pwd = dbParams.value(dbSection, "pwd");
		d.schema = dbParams.value(dbSection + ".schema", abstractSchema);
		d.maxNoOfConnections = dbParams.valueAsInt(dbSection, "maxNoOfConnections", 1);
	}

	return d;
}

Db::DB* Db::newConnection(const IniParameterMap& dbParams,
													const std::string& abstractSchema)
{
	DBConData cd = dbConData(dbParams, abstractSchema);
	if(!cd.isValid()){
		cout << "Couldn't establish new connection for abstract schema: "
		<< abstractSchema << " via retrieved connection settings: "
		<< cd.toString() << " !" << endl;
		exit(1);
	}

	return newConnection(cd);
}

bool Db::attachDB(DB* con, string attachAbstractSchema, string alias)
{
	DBConData cd = dbConData(attachAbstractSchema);
	if(cd.isSqliteDB())
		return con->attachDB(cd.filename, alias);

	return false;
}
