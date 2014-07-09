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

