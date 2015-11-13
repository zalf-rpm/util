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

#include <QtCore>

#include "qt-values-file-loader.h"

using namespace Models;

QtValuesFileParameterMap
::QtValuesFileParameterMap(const QString& pathToValuesFile)
: _pathToValuesFile(pathToValuesFile) {
	parseValuesFile();
}

void QtValuesFileParameterMap::parseValuesFile(){
	QFile data(_pathToValuesFile);
	if(data.open(QFile::ReadOnly)) {
		QTextStream ts(&data);
		QRegExp rx("\\S+\\s*(\\w+)\\s*=\\s*(?:\"(.*)\"|(\\S*))\\s*//");
		while(!ts.atEnd()){
			QString line = ts.readLine();
			rx.indexIn(line);
			QString value = rx.capturedTexts().at(2);
			if(value.isEmpty())
				value = rx.capturedTexts().at(3);
			if(!value.isEmpty())
				insert(rx.capturedTexts().at(1), value);
		}
	}
}

