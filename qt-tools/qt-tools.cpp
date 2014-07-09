#include <QtCore>
#include <cstdlib>

#include "qt-tools.h"

using namespace std;
using namespace Tools;

QColor Tools::desaturate(QColor c, double percentSaturation)
{
  double s = c.saturationF();
  QColor dc(c);
  dc.setHsvF(c.hueF(), s * percentSaturation / 100.0, c.valueF());
  return dc;
}

QString Tools::getEnvironmentVariable(const QString& name)
{
	char* cvar = getenv(name.toStdString().c_str());
	if(cvar)
		return QString::fromLatin1(cvar);
	else
		return QString();
}

QRectF Tools::fitRect1IntoRect2(const QRectF& r1, const QRectF& r2,
                                QPair<int, int> r2ScaleInPercent)
{
	QRectF r2s = r2;
	r2s.setWidth(r2.width() * double(r2ScaleInPercent.first) / 100);
	r2s.setHeight(r2.height() * double(r2ScaleInPercent.second) / 100);

	double a1 = r1.width() / r1.height(); //aspect ratio rectangle 1
	double a2 = r2s.width() / r2s.height(); //aspect ratio rectangle 2

  double hr = 0, wr = 0, xr = 0, yr = 0;
  if(a1 <= a2)
  {
		hr = r2s.height();
		wr = hr * a1;
		xr = r2s.x() + (r2s.width() - wr) / 2;
		yr = r2s.y();
  }
  else
  {
		wr = r2s.width();
		hr = wr / a1;
		yr = r2s.y() + (r2s.height() - hr) / 2;
		xr = r2s.x();
	}

	return QRectF(xr, yr, wr, hr);
}

QPair<double, double>
    Tools::scaleFactorForFitRect1IntoRect2(const QRectF& r1,
                                           const QRectF& r2,
                                           QPair<int, int> r2ScaleInPercent)
{
	QRectF scaledRect = fitRect1IntoRect2(r1, r2, r2ScaleInPercent);
	return QPair<double, double>(scaledRect.width() / r1.width(),
                               scaledRect.height() / r1.height());
}


QSizeF Tools::factorsToScaleRect1IntoRect2(QRectF r1, QRectF r2,
                                           QPair<int, int> r2ScaleInPercent)
{
  QRectF r2s = r2;
  r2s.setWidth(r2.width() * double(r2ScaleInPercent.first) / 100);
  r2s.setHeight(r2.height() * double(r2ScaleInPercent.second) / 100);

  return QSizeF(r2s.width() / r1.width(), r2s.height() / r1.height());
}
