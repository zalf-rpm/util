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
