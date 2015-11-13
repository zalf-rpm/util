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

#ifndef QTVALUESFILELOADER_H_
#define QTVALUESFILELOADER_H_

#include <QString>
#include <QMap>

namespace Models {
	
	//! Data for a model, parsed from a values file.
	/*!
	 * Is actually a Map which is after being initialized with
	 * a values file, being filed with name/mappings.
	 * 
	 * The map is just a QString->QString map, thus the user
	 * has to cast/parse to the right type or use one of the 
	 * convenience methods give.
	 */
	class QtValuesFileParameterMap : public QMap<QString, QString> {
	public:
		//! Create and init map from given values file.
		QtValuesFileParameterMap(const QString& pathToValuesFile);
		
		//! return value as QString, just for interface consistency purposes
		QString valueAsString(const QString& key) const {
			return value(key);
		}
		
		//! Return value as integer.
		int valueAsInt(const QString& key) const {
			return value(key).toInt();
		}
		
		//! Return value as double.
		double valueAsDouble(const QString& key) const {
			return value(key).toDouble();
		}
			
		//! Return path to values file used. 
		QString pathToValuesFile() const {
			return _pathToValuesFile;
		}
			
	private: //methods
		//! Worker method, to parse the values file.
		void parseValuesFile();
		
	private: //state
		QString _pathToValuesFile; //!< the path to the values file
	};
	
	
}

#endif 
