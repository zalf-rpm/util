#include <iostream>

#include "util/abstract-db-connections.h"

using namespace std;
using namespace Util;
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
