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

#ifndef __DB_H__
#define __DB_H__

#ifndef NO_MYSQL
#ifdef WIN32
#ifdef WINSOCK2
#include <WinSock2.h>
#else
#include <WinSock.h>
#endif //WINSOCK2
#include "mysql.h"
#else
#include <mysql/mysql.h>
#endif
#endif

#include <list>
#include <vector>
#include <string>
#include <memory>

#include "tools/date.h"

#ifndef NO_SQLITE
#include "sqlite3.h"
#endif

namespace Db
{
	typedef std::vector<std::string> DBRow;

	class DB
	{
	public:
    virtual ~DB() {}
		
		virtual bool exec(const char* sqlStatement) = 0;
		virtual bool exec(const std::string& sqlStatement) { return exec(sqlStatement.c_str()); }

		virtual bool select(const char* selectStatement){ return exec(selectStatement); }
		virtual bool select(const std::string& selectStatement){ return exec(selectStatement.c_str()); }

		virtual bool query(const char* sqlStatement){ return exec(sqlStatement); }
		virtual bool query(const std::string& sqlStatement){ return exec(sqlStatement.c_str()); }

		virtual bool insert(const char* insertStatement){ return exec(insertStatement); }
		virtual bool insert(const std::string& insertStatement){ return insert(insertStatement.c_str()); }

		virtual bool update(const char* updateStatement){ return exec(updateStatement); }
		virtual bool update(const std::string& updateStatement){ return update(updateStatement.c_str()); }

		virtual bool del(const char* deleteStatement){ return exec(deleteStatement); }
		virtual bool del(const std::string& deleteStatement){ return del(deleteStatement.c_str()); }

		virtual size_t getNumberOfFields() = 0;
		//MYSQL_FIELD* getFields();
		//MYSQL_FIELD* getNextField();
		virtual size_t getNumberOfRows() = 0;
		virtual DBRow getRow() = 0;
		virtual void freeResultSet() = 0;

		virtual bool isConnected() = 0; //{ return _isConnected; }

		virtual int insertId() = 0;

		virtual std::string errorMsg() { return ""; }

		virtual std::string now() const = 0;

		virtual std::string toDBDate(Tools::Date date) const = 0;

		virtual void setCharacterSet(const char* charsetName) = 0;
		virtual void setCharacterSet(const std::string charsetName) { setCharacterSet(charsetName.c_str()); }

		virtual DB* clone() const = 0; // { return new DB(_host, _user, _pwd, _schema, _port); }

		//! if supported by database, attach another db to the current one
		virtual bool attachDB(std::string /*pathToDB*/, std::string /*alias*/){ return false; }

    std::string abstractSchemaName() const { return _abstractSchemaName; }
    void setAbstractSchemaName(const std::string& asn){ _abstractSchemaName = asn; }
  private:
    std::string _abstractSchemaName;
	};

#ifndef NO_MYSQL
	class MysqlDB : public DB
	{
	public:
		MysqlDB(const std::string& host, const std::string& user, const std::string& pwd,
						const std::string& schema, unsigned int port = 0);
		virtual ~MysqlDB();
		virtual bool exec(const char* query);
		virtual size_t getNumberOfFields();
		MYSQL_FIELD* getFields();
		MYSQL_FIELD* getNextField();
		virtual size_t getNumberOfRows();
		virtual DBRow getRow();
		MYSQL_ROW getMysqlRow();
		virtual void freeResultSet();

		virtual bool isConnected(){ return _isConnected; }

		virtual int insertId();

		virtual std::string now() const { return "now()"; }

		virtual std::string toDBDate(Tools::Date date) const;

		virtual void setCharacterSet(const char* charsetName);

		virtual MysqlDB* clone() const { return new MysqlDB(_host, _user, _pwd, _schema, _port); }

	private: //methods
		void lazyInit();
		void init();

	private:
		std::string _host;
		std::string _user;
		std::string _pwd;
		std::string _schema;
		unsigned int _port{0};

		MYSQL* _connection{nullptr};
		MYSQL_RES* _resultSet{nullptr};
		bool _isConnected{false};
		long _id{-1};
		bool _initialized{false};
	};
#endif

	//------------------------------------------------------------------------

#ifndef NO_SQLITE
	class SqliteDB : public DB
	{
	public:
		SqliteDB(const std::string& filename);
		virtual ~SqliteDB();
		virtual bool exec(const char* sqlStatement);
		virtual bool insert(const char* insertStatement){ return inUpDel(insertStatement); }
		virtual bool update(const char* updateStatement){ return inUpDel(updateStatement); }
		virtual bool del(const char* deleteStatement){ return inUpDel(deleteStatement); }

		virtual size_t getNumberOfFields();
		virtual size_t getNumberOfRows();
		virtual DBRow getRow();

		virtual void freeResultSet();

		virtual bool isConnected(){ return _isConnected; }

		virtual int insertId();

		virtual std::string errorMsg();

		virtual std::string now() const { return "datetime('now')"; }

		virtual std::string toDBDate(Tools::Date date) const;

		virtual void setCharacterSet(const char* charsetName);

		virtual DB* clone() const { return new SqliteDB(_filename); }


		virtual bool attachDB(std::string pathToDB, std::string alias);

	private: //methods
		void lazyInit();
		void init();

		void addNeededSQLFunctions();

		bool inUpDel(const char* inUpDelStatement);

	private:
		std::string _filename;
		sqlite3* _db{nullptr};
		sqlite3_stmt* _ppStmt{nullptr};
		std::string _query;
		bool _isConnected{false};
		long _id{-1};
		bool _initialized{false};
		size_t _currentRowNo{0};
		size_t _noOfRows{0};
	};
#endif

	//----------------------------------------------------------------------------

	struct DBConData
	{
		DBConData() : maxNoOfConnections(0) {}
		//! construct a Mysql connection
		DBConData(const std::string& host, const std::string& user,
							const std::string& pwd, const std::string& schema,
							unsigned int port = 0)
			: host(host), user(user), pwd(pwd), schema(schema),
				maxNoOfConnections(1), port(port){}

		//! construct a SqliteDB connection
		DBConData(const std::string& filename) : filename(filename) {}
		std::string host;
		std::string user;
		std::string pwd;
		std::string schema;
		std::string filename;
    std::string abstractSchemaName;

		int maxNoOfConnections;
		unsigned int port;
		bool isValid() const
		{
			return !filename.empty() || (!host.empty() && !user.empty() && !pwd.empty() && !schema.empty());
		}
		bool isSqliteDB() const { return isValid() && !filename.empty(); }
		bool isMysqlDB() const { return !isSqliteDB(); }

		std::string toString() const;
	};

	//----------------------------------------------------------------------------

  typedef std::shared_ptr<DB> DBPtr;

	//! user takes ownership
	DB* newConnection(const DBConData& connectionData);

#ifndef NO_MYSQL
  typedef std::shared_ptr<MysqlDB> MysqlDBPtr;
	inline MysqlDB* toMysqlDB(DB* db){ return dynamic_cast<MysqlDB*>(db); }
#endif

}

#endif
