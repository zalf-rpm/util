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

#include <iostream>

#include "db/abstract-db-connections.h"
#include "tools/helper.h"

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
	d.abstractSchemaName = abstractSchema;
	
	//if not empty is a sqlite db
	d.filename = dbParams.value(dbSection, "filename");
	// threat it as mysql db with all the connection parameters
	if(d.filename.empty())
	{
		d.host = dbParams.value(dbSection, "host");
		d.port = (unsigned int)dbParams.valueAsInt(dbSection, "port", 0);
		d.user = dbParams.value(dbSection, "user");
		d.pwd = dbParams.value(dbSection, "pwd");

    //get intial schema, first from abstract schema section, then from data-db-name and then from db relative schema section
    d.schema = dbParams.value(dbSection + "." + abstractSchema, "schema");
    if(d.schema.empty())
      d.schema = dbParams.value(dbSection + "." + abstractSchema, "data-db-name");
    if(d.schema.empty())
      d.schema = dbParams.value(dbSection + ".schema", abstractSchema);

		d.maxNoOfConnections = dbParams.valueAsInt(dbSection, "maxNoOfConnections", 1);
	}
	else
	{
		if(!isAbsolutePath(d.filename))
		{
			auto p = splitPathToFile(dbParams.pathToIniFile());
			if(!p.first.empty())
				d.filename = fixSystemSeparator(p.first + "/" + d.filename);
		}
	}

	return d;
}

Db::DB* Db::newConnection(const IniParameterMap& dbParams,
													const std::string& abstractSchema)
{
	DBConData cd = dbConData(dbParams, abstractSchema);
	if(!cd.isValid())
	{
		cout << "Couldn't establish new connection for abstract schema: "
			<< abstractSchema << " via retrieved connection settings: "
			<< cd.toString() << " !" << endl;
		return nullptr;
	}

	return newConnection(cd);
}

bool Db::attachDB(DB* con, string attachAbstractSchema, string alias)
{
	DBConData cd = dbConData(attachAbstractSchema);
	if(cd.isSqliteDB())
	{
		return con->attachDB(cd.filename, alias);
	}

	return false;
}
