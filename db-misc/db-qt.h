#ifndef DBQT_H_
#define DBQT_H_

#include <QVector>
#include <QStringList>
#include <QList>
#include <QString>

#include "db.h"

namespace Db {

	typedef QVector<QVector<double> > QVVDouble;
	typedef QList<QVector<QVector<double> > > QLVVDouble;

	class DBQt : public DB {
	public:
		DBQt(const QString& host, const QString& user, const QString& pwd,
		     const QString& database)
		: DB(host.toStdString(), user.toStdString(),
		     pwd.toStdString(), database.toStdString()) {}

		/**
		 * return a list of columns of rows [columns][rows]
		 */
		QVVDouble makeRequest(const QString& query, int columns = 2);

		bool qtExecute(const QString& sqlStatement);

		/**
		 * split/group a list of columns by the values in the given columns
		 * thus, return a list of separate lists of columns
		 */
		QLVVDouble groupBy(const QVVDouble& cols, int byColumn);

		/**
		 * returns a list of columns of rows [columns][rows]
		 */
		QVector<QStringList> makeGenericRequestCols(const QString& query,
		                                            int columns);

		/**
		 * return a list of rows of columns [rows][columns]
		 */
		QVector<QStringList> makeGenericRequestRows(const QString& query,
		                                            int columns);

	private:
	};

	//----------------------------------------------------------------------------
	//----------------------------------------------------------------------------
	//----------------------------------------------------------------------------

	DBQt& newQtConnection(const DBConData& connectionData,
	                      bool userTakesOwnership);

	class DBQtConnectionHandler : public DBConnectionHandler {
		public:
			DBQtConnectionHandler() : DBConnectionHandler() {}
			~DBQtConnectionHandler();

			DBQt& getNewQtConnection(const DBConData& connectionData,
			                         bool userTakesOwnership);

		private: //state
			QList<DBQt*> _qtConnections;
		};

}

#endif
