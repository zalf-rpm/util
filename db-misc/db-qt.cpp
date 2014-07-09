#include <QtCore>

#include "db-qt.h"

using namespace Db;

QVVDouble DBQt::makeRequest(const QString& query, int columns){
  QVVDouble r(columns);

  select(query.toStdString().c_str());

	MYSQL_ROW row;
  while((row = getRow()))
  	for(int k = 0; k < columns; k++)
  		r[k].append(strtod(row[k], NULL));

  return r;
}

bool DBQt::qtExecute(const QString& sqlStatement){
	return query(sqlStatement.toStdString().c_str());
}

QLVVDouble DBQt::groupBy(const QVVDouble& cols, int byColumn){
	QList<QVector<QVector<double> > > r;
	double oldValue = cols.at(byColumn).first();
	int oldIndex = 0;
	for(int i = 0; i < cols.at(byColumn).size(); i++){
		double value = cols.at(byColumn).at(i);
		if(value != oldValue){
			QVVDouble ncols; //new columns
			foreach(QVector<double> v, cols){
				ncols.append(v.mid(oldIndex, i - oldIndex));
			}
			r.append(ncols);
			oldValue = value;
			oldIndex = i;
		}

	}
	QVVDouble ncols; //new columns
	foreach(QVector<double> v, cols){
		ncols.append(v.mid(oldIndex));
	}
	r.append(ncols);
	return r;
}

QVector<QStringList> DBQt::makeGenericRequestCols(const QString& query,
                                                  int columns){
	QVector<QStringList> r(columns);

	select(query.toStdString().c_str());

	MYSQL_ROW row;
	while((row = getRow()))
		for(int k = 0; k < columns; k++)
			r[k].append(QString::fromUtf8(row[k]));

	return r;
}

QVector<QStringList> DBQt::makeGenericRequestRows(const QString& query,
                                                  int columns){
  select(query.toStdString().c_str());

  QVector<QStringList> rows;

  MYSQL_ROW row;
  while((row = getRow())){
  	QStringList cols;
  	for(int k = 0; k < columns; k++)
  		cols.append(QString::fromUtf8(row[k]));
  	rows.append(cols);
  }

  return rows;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

DBQtConnectionHandler& dbQtConnectionHandler(){
	static DBQtConnectionHandler h;
	return h;
}

DBQt& Db::newQtConnection(const DBConData& connectionData,
                          bool userTakesOwnership){
	return dbQtConnectionHandler().getNewQtConnection(connectionData,
	                                                  userTakesOwnership);
}

DBQtConnectionHandler::~DBQtConnectionHandler(){
	qDeleteAll(_qtConnections);
}

DBQt& DBQtConnectionHandler::getNewQtConnection(const DBConData& cd,
                                                bool userTakesOwnership){
	DBQt* db = new DBQt(cd.host.c_str(), cd.user.c_str(), cd.pwd.c_str(),
	                    cd.schema.c_str());

	if(userTakesOwnership)
		return *db;
	_qtConnections.push_back(db);
	return *db;
}

