/**
Authors: 
Michael Berg <michael.berg@zalf.de>

Maintainers: 
Currently maintained by the authors.

This file is part of the util library used by models created at the Institute of 
Landscape Systems Analysis at the ZALF.
Copyright (C) 2007-2013, Leibniz Centre for Agricultural Landscape Research (ZALF)

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <cstdio>
#include <ctime>
#include <cstdio>
#include <cmath>
#include <cstdlib>
#include <numeric>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <cassert>

#include "algorithms.h"
#include "tools/use-stl-algo-boost-lambda.h"
#include "tools/date.h"
#include "tools/helper.h"

using namespace std;
using namespace Tools;

vector<string> Tools::splitString(string s, string splitElements)
{
	vector<string> v;
	v.push_back("");
	for(auto cit = s.begin(); cit != s.end(); cit++)
	{
		if(splitElements.find(*cit) == string::npos)
			v.back().append(1, *cit);
		else if(v.back().size() > 0)
			v.push_back("");
	}

	return v;
}

int Tools::createRandomNumber()
{
	static time_t lastTime = -1;
	int id = 0; //reserved
	while(id == 0)
	{
		time_t t = time(NULL);
		if(lastTime != t)
			srand(t);
		id = rand();
		lastTime = t;
	}
	return id;
}

int Tools::createRandomNumber(int max)
{
	static time_t lastTime = -1;
	int id = 0; //reserved
	while(id == 0)
	{
		time_t t = time(NULL);
		if(lastTime != t)
			srand(t);
		id = rand() % max;
		lastTime = t;
	}
	return id;
}

string Tools::trim(const string& s, const string& whitespaces)
{
  string str(s);
  size_t found;
  found = str.find_last_not_of(whitespaces);
  if (found != string::npos)
    str.erase(found + 1);
  found = str.find_first_not_of(whitespaces);
  if (found != string::npos)
    str.erase(0, found);
  return str;
}

namespace 
{
  void de_capitalizeInPlace(string& s, int (*f)(int) = &toupper)
  {
    if(s.begin() == s.end())
      return;

    for(string::iterator si = s.begin()+1; si != s.end(); si++)
    {
      if(si-1 == s.begin())
        *(si-1) = f(*(si-1));

      switch(*(si-1))
      {
      case '-': case '(': case ')': case ',': case '/': case '_':
        *si = f(*si);
      }
    }
  }

}

void Tools::capitalizeInPlace(string& s)
{
  de_capitalizeInPlace(s);
}

string Tools::capitalize(const string& s)
{
  string res = s;
  capitalizeInPlace(res);
  return res;
}

void Tools::decapitalizeInPlace(string& s)
{
  de_capitalizeInPlace(s, &tolower);
}

string Tools::decapitalize(const string& s)
{
  string res = s;
  decapitalizeInPlace(res);
  return res;
}


double Tools::sunshine2globalRadiation(int yd, double sonn, double lat,
                                      bool asMJpm2pd)
{
  double pi=4.0*atan(1.0);
  double dec=-23.4*cos(2*pi*(yd+10)/365);
  double sinld=sin(dec*pi/180)*sin(lat*pi/180);
  double cosld=cos(dec*pi/180)*cos(lat*pi/180);
  double dl=12*(pi+2*asin(sinld/cosld))/pi;
  double dle=12*(pi+2*asin((-sin(8*pi/180)+sinld)/cosld))/pi;
  double rdn=3600*(sinld*dl+24/pi*cosld*sqrt(1.0-(sinld/cosld)*(sinld/cosld)));
  double drc=1300*rdn*exp(-0.14/(rdn/(dl*3600)));
  double dro=0.2*drc;
  double dtga=sonn/dle*drc+(1-sonn/dle)*dro;
  double t = dtga/10000.0;
  //convert J/cm²/d to MJ/m²/d
  //1cm²=1/(100*100)m², 1J = 1/1000000MJ
  //-> (t * 100.0 * 100.0) / 1000000.0 -> t / 100
  return asMJpm2pd ? t/100.0 : t;
}

double Tools::cloudAmount2globalRadiation(int doy,
                                         double nn, //[1/8]
                                         double lat, //[°]
                                         double hh, //[m]
                                         bool asMJpm2pd)
{
  static const double pi = 4.0 * atan(1.0);
  static const double s0 = 1367; //Wm-2
  static const double a = 0.50572;
  static const double b = 607995;
  static const double c = 1.6364;
  //static const double enc = 0;
  static const double als =	0.2;
  static const double albc = 0.5;
  static const double albh = 0.2;
  static const double alph = 0;
  static const double azh = 0;
  static const double swf = cos(alph / 2.0) * cos(alph / 2.0);

  //Datum	27.06.2009
  double enc = nn / 8.0;
  double phi = lat * pi / 180.0;
  double theta0	= 2.0 * pi * doy / 365.0;
  double xx	= pi * (0.9856 * doy - 2.72) / 180.0;
  double rrq = 1.0 + 0.0344 * cos(xx);
  double i0	= s0 * rrq;
  double delta = 0.006918 - 0.3999912 * cos(theta0) +
    0.070257 * sin(theta0) - 0.006758 * cos(2 * theta0) +
    0.000907 * sin(2 * theta0) - 0.002697 * cos(3 * theta0) +
    0.00148 * sin(theta0 * 3);
  double pp0 = exp(-hh / 8434.5);

  double rgsum = 0; //[0,5 Wh m-2] pro d
  //	cout << "rgsum: 0 ";
  for(int hs = 1; hs <= 48; hs++)
  {
    double t = 24.0 * (double(hs) - 1.0) / 48.0;
    //double zeit = t;
    double th = pi * (t - 12.0) / 12.0;
    double ctheta = sin(delta) * sin(phi) + cos(delta) * cos(phi) * cos(th);
    double theta = acos(ctheta);
    //double hsonne = pi / 2.0 - theta;
    double hdeg = 90.0 - (theta * 180.0 / pi);
    double gfaktor = cos(alph) * ctheta +
      sin(alph) * (cos(azh) *
      (tan(phi) * ctheta - sin(delta) / cos(phi)) +
      sin(azh) * cos(delta) * sin(th));
    double m = 1.0 / (ctheta + a * pow(hdeg + b, -c));
    double tl = 3.9 - 1.4 * cos(2 * pi * (doy - 15.0) / 365.0);
    double dr0 = hdeg > 0.5 ? 1.0 / (0.9 * m + 9.4) : 0.0408 + hdeg * 0.0028;
    double id0 = ctheta < 0 ? 0 : i0 * exp(-tl * dr0 * m * pp0);
    double id = ctheta < 0 ? 0 : (1.0 - enc) * id0;
    double rg0 = 0.9 * i0 * ctheta * exp(-0.027 * pp0 * tl / ctheta);
    //double rg = ctheta < 0 ? 0 : rg0 * (1.0 - 0.72 * pow(enc, 3.2));
    //double sh = ctheta < 0 ? 0 : id * ctheta;
    double d0 = rg0 - id0 * ctheta;
    double d1 = rg0 * 0.2;
    //double d = ctheta < 0 ? 0 : (1.0 - enc) * d0 + enc * d1;
    //double swf = cos(alph / 2.0) * cos(alph / 2.0);
    double tau = ctheta < 0 || fuzzyIsNull(id) ? 0 : id0 / id;
    double ds0 = gfaktor < 0 ? d0 * (1.0 - tau) * swf
      : d0 * (tau * gfaktor / ctheta + (1.0 - tau) * swf);
    double ds1 = d1 * swf;
    double ds = ctheta < 0 ? 0 : (1.0 - enc) * ds0 + ds1 * enc;
    double sd = gfaktor * id;
    //double rch = enc * albc * albh * (sh + d);
    //double qh = sh + d + rch;
    double rcs = enc * albc * als * (sd + ds);
    double qs = sd + ds + rcs;
    double rh = albh * qs;
    double rss = rh * (1.0 - swf);
    double rges = qs + rss;
    //double rs = als * (qs * rss);
    rgsum += rges < 0 ? 0 : rges;
    //		cout << rgsum << " ";
  }
  //	cout << endl;

  double rg_nn	= rgsum * 0.18;

  //	cout << "doy: " << doy << " nn: " << nn << " lat: " << lat << " hh: " << hh << " rgsum: " << rgsum << " rg_nn: " << rg_nn << endl;

  //convert J/cm²/d to MJ/m²/d
  //1cm²=1/(100*100)m², 1J = 1/1000000MJ
  //-> (t * 100.0 * 100.0) / 1000000.0 -> t / 100
  return asMJpm2pd ? rg_nn/100.0 : rg_nn;
}

HistogramData Tools::histogramDataByStepSize(const vector<double>& ys,
                                             double step,
                                             int normalizeCount)
{
  HistogramData res;
  if(ys.empty()) 
    return res;

  pair<double, double> mima = minMax(ys);
  //cout << "rmin: " << mima.first << " rmax: " << mima.second << endl;

  //round minY and maxY to/including full stepSize
  double minY = mima.first;
  double t = minY / step;
  minY = std::floor(t) * step;
  double shift = -1*minY;
  double maxY = mima.second;
  t = maxY / step;
  maxY = std::ceil(t) * step;

  int n = int(std::ceil((maxY - minY) / step));
  //cout << "minY: " << minY << " shift: " << shift << " maxY: " << maxY
  //<< " step: " << step << " n: " << n << endl;

  //if all values are the same
  if(n == 0)
  {
    minY = minY - (step / 2);
    maxY = maxY + (step / 2);
    n = 1;
  }

  res.xs.resize(n);
  for(int i = 0; i < n; i++)
    res.xs[i] = minY + (i * step + (step / 2));
  //cout << "xs: " << xs << endl;

  res.borders.resize(n+1);
  for(int i = 0; i < n+1; i++)
    res.borders[i] = minY + (i * step);
  //cout << "borders: " << borders << endl;

  res.classes.resize(n+1);
  for(vector<double>::const_iterator ci = ys.begin(); ci != ys.end(); ci++)
  {
    //cout << "int(ceil((" << shift << " + " << *ci << ") /" << step << "))=" <<
    //	(int(ceil((shift + *ci)/step))) << endl;
    res.classes[int(std::ceil((shift + *ci)/step))]++;
  }
  //cout << "classes: " << classes << endl;
  res.classes[1] += res.classes.at(0);
  res.classes.erase(res.classes.begin()); //lowest border actually belongs to class above (0 value)
  //cout << "classes: " << classes << endl;

  if(normalizeCount > 1)
    for(int i = 0, size = res.classes.size(); i < size; i++)
      res.classes[i] /= normalizeCount;

  return res;
}


HistogramData Tools::histogramDataByNoOfClasses(const vector<double>& ys, int n,
                                               int normalizeCount)
{
  HistogramData res;
  if(ys.empty()) return res;

  pair<double, double> mima = minMax(ys);
  double minY = mima.first;
  double shift = -1*minY;
  double maxY = mima.second;
  double step = (maxY - minY) / n;
  //qDebug() << "minY: " << minY << " shift: " << shift << " maxY: " << maxY << " step: " << step;

  res.xs.resize(n);
  for(int i = 0; i < n; i++)
    res.xs[i] = minY + (i * step + (step / 2));
  //cout << "xs: " << xs << endl;

  res.borders.resize(n+1);
  for(int i = 0; i < n+1; i++)
    res.borders[i] = minY + (i * step);
  //cout << "borders: " << borders << endl;

  res.classes.resize(n+1);
  for(vector<double>::const_iterator ci = ys.begin(); ci != ys.end(); ci++)
  {
    //cout << "int(ceil((" << shift << " + " << *ci << ") /" << step << "))=" <<
    //	(int(ceil((shift + *ci)/step))) << endl;
    res.classes[int(std::ceil((shift + *ci)/step))]++;
  }

  //lowest border actually belongs to class above (0 value)
  res.classes[1] += res.classes.at(0);
  res.classes.erase(res.classes.begin());
  //cout << "classes: " << classes << endl;

  if(normalizeCount > 1)
    for(int i = 0, size = res.classes.size(); i < size; i++)
      res.classes[i] /= normalizeCount;

  return res;
}

//------------------------------------------------------------------------------

BoxPlotInfo::BoxPlotInfo() 
  : median(0.0), Q25(0.0), Q75(0.0), min(0.0), max(0.0),
  minInnerFence(0.0), maxInnerFence(0.0){}

BoxPlotInfo::BoxPlotInfo(double m, double q25, double q75, double min, double max)
  : median(m), Q25(q25), Q75(q75), min(min), max(max),
  minInnerFence(0), maxInnerFence(0){}

double BoxPlotInfo::lowerInnerFence(int roundToDigits) const
{
  return Tools::round(Q25 - 1.5 * IQ(), roundToDigits);
}

//! upper inner fence value
double BoxPlotInfo::upperInnerFence(int roundToDigits) const
{
  return Tools::round(Q75 + 1.5 * IQ(), roundToDigits);
}

//! lower outer fence value
double BoxPlotInfo::lowerOuterFence(int roundToDigits) const
{
  return Tools::round(Q25 - 3.0 * IQ(), roundToDigits);
}

//! upper outer fence value
double BoxPlotInfo::upperOuterFence(int roundToDigits) const
{
  return Tools::round(Q75 + 3.0 * IQ(), roundToDigits);
}

string BoxPlotInfo::toString() const 
{
  ostringstream s;
  s << "\nextreme lower outliers: [ ";
  for_each(extremeLowerOutliers.begin(), extremeLowerOutliers.end(),
    s << _1 << " ");
  s << "]\nmild lower outliers: [ ";
  for_each(mildLowerOutliers.begin(), mildLowerOutliers.end(),
    s << _1 << " ");
  s << "]" << endl;

  s << "min(" << min << ") << "
    << "lof(" << lowerOuterFence() << ") -- "
    << "lif(" << lowerInnerFence() << ")|"
    << minInnerFence << "| -- "
    << "[" << Q25 << "| " << median << " |" << Q75 << "] -- |"
    << maxInnerFence << "|"
    << "(" << upperInnerFence() << ")uif -- "
    << "(" << upperOuterFence() << ")uof"
    << " >> (" << max << ")max" << endl;

  s << "mild upper outliers: [ ";
  for_each(mildUpperOutliers.begin(), mildUpperOutliers.end(),
    s << _1 << " ");
  s << "]\nextreme upper outliers: [ ";
  for_each(extremeUpperOutliers.begin(), extremeUpperOutliers.end(),
    s << _1 << " ");
  s << "]" << endl;
  return s.str();
}


BoxPlotInfo Tools::boxPlotAnalysis(const std::vector<double>& data,
                                   bool orderedData, int roundToDigits)
{
  if(data.empty())
    return BoxPlotInfo();

  vector<double> _odata;
  if(!orderedData)
  {
    _odata.insert(_odata.begin(), data.begin(), data.end());
    sort(_odata.begin(), _odata.end());
  }
  const vector<double>& odata = orderedData ? data : _odata;

  BoxPlotInfo bpi(median(odata, roundToDigits),
                  quartile(0.25, odata, roundToDigits),
                  quartile(0.75, odata, roundToDigits),
                  odata.front(), odata.back());

  //get extreme lower outliers
  remove_copy_if(odata.begin(), odata.end(),
                 inserter(bpi.extremeLowerOutliers, bpi.extremeLowerOutliers.end()),
                 [&](double elo){ return elo >= bpi.lowerOuterFence(); });//_1 >= bpi.lowerOuterFence());
  //keep just the 10 smallest elements, delete the rest
  if(bpi.extremeLowerOutliers.size() > 10)
  {
    int count = 0;
    set<double>::iterator i = bpi.extremeLowerOutliers.begin();
    while(i != bpi.extremeLowerOutliers.end() && count++ < 10)
      i++;
    bpi.extremeLowerOutliers.erase(i, bpi.extremeLowerOutliers.end());
  }

  //get mild lower outliers
  remove_copy_if(odata.begin(), odata.end(),
                 inserter(bpi.mildLowerOutliers, bpi.mildLowerOutliers.end()),
                 [&](double lof){ return lof < bpi.lowerOuterFence() || lof >= bpi.lowerInnerFence(); });
  //_1 < bpi.lowerOuterFence() || _1 >= bpi.lowerInnerFence());
  //keep just the 10 smallest elements, delete the rest
  if(bpi.mildLowerOutliers.size() > 10)
  {
    int count = 0;
    set<double>::iterator i = bpi.mildLowerOutliers.begin();
    while(i != bpi.mildLowerOutliers.end() && count++ < 10)
      i++;
    bpi.mildLowerOutliers.erase(i, bpi.mildLowerOutliers.end());
  }

  //get mild upper outliers
  remove_copy_if(odata.begin(), odata.end(),
                 inserter(bpi.mildUpperOutliers, bpi.mildUpperOutliers.end()),
                 [&](double uif){ return uif <= bpi.upperInnerFence() || uif > bpi.upperOuterFence(); });
  //_1 <= bpi.upperInnerFence() || _1 > bpi.upperOuterFence());
  //keep just the 10 largest elements, delete the rest
  if(bpi.mildUpperOutliers.size() > 10)
  {
    int count = 0;
    set<double>::iterator i = bpi.mildUpperOutliers.end();
    while(i != bpi.mildUpperOutliers.begin() && count++ < 10)
      i--;
    bpi.mildUpperOutliers.erase(bpi.mildUpperOutliers.begin(), i);
  }

  //get extreme upper outliers
  remove_copy_if(odata.begin(), odata.end(),
                 inserter(bpi.extremeUpperOutliers, bpi.extremeUpperOutliers.end()),
                 [&](double euo){ return euo <= bpi.upperOuterFence(); });
  //_1 <= bpi.upperOuterFence());
  //keep just the 10 largest elements, delete the rest
  if(bpi.extremeUpperOutliers.size() > 10)
  {
    int count = 0;
    set<double>::iterator i = bpi.extremeUpperOutliers.end();
    while(i != bpi.extremeUpperOutliers.begin() && count++ < 10)
      i--;
    bpi.extremeUpperOutliers.erase(bpi.extremeUpperOutliers.begin(), i);
  }

  //get smallest value above lower inner fence
  bpi.minInnerFence = bpi.min;
  vector<double>::const_iterator
      ci = find_if(odata.begin(), odata.end(), [&](double lif){ return lif > bpi.lowerInnerFence(); });//_1 > bpi.lowerInnerFence());
  if(ci != odata.end())
    bpi.minInnerFence = *ci;

  //get largest value below upper inner fence
  bpi.maxInnerFence = bpi.max;
  ci = find(odata.begin(), odata.end(), bpi.Q75); //start at Q75
  vector<double>::const_iterator cilast = odata.end();
  while(ci != odata.end())
  {
    cilast = ci++;
    ci = find_if(ci, odata.end(), [&](double uif){ return uif < bpi.upperInnerFence(); });//_1 < bpi.upperInnerFence());
  }
  if(cilast != odata.end())
    bpi.maxInnerFence = *cilast;

  return bpi;
}

/*
double Tools::median(const std::vector<double>& odata){
int size = odata.size();
return isEven(size)
? (odata.at((size / 2) - 1) + odata.at((size / 2) + 1 - 1)) / 2.0
: odata.at(int(size / 2.0) + 1 - 1);
}
*/

double Tools::quartile(double xth, const vector<double>& odata, int roundToDigits)
{
  switch(odata.size())
  {
  case 0: return 0;
  case 1: return odata.at(0);
  case 2: return xth < 0.5 ? odata.at(0) : odata.at(1);
  default: ;
  }

  double i;
  double frac = modf(xth * odata.size(), &i);
  i-=1; //0-indexed vector position
  if(i < 0 || i >= odata.size())
    i=0;
  double ip1 = i+1;
  if(ip1 >= odata.size())
    ip1 = odata.size()-1;

  return Tools::round(odata.at(int(i)) + frac*(odata.at(int(ip1)) - odata.at(int(i))), roundToDigits);
}

//------------------------------------------------------------------------------

std::pair<double, int> Tools::decomposeIntoSci(double value)
{
  if(abs(value) < 0.000001)
    return make_pair(0.0, 0);

  double man = value;
  int exp = 0;

  if(abs(man) > 0.1)
  {
    while(abs(man/10) > 1)
      man/=10, exp++;

    if(abs(man) > 1)
      man/=10, exp++;
  } 
  else 
  {
    while(abs(man*10) < 0.1)
      man*=10, exp--;

    if(abs(man) < 0.1)
      man*=10, exp--;
  }

  //cout << "decomposeIntoSci(" << value << ") = "
  //<< man << "*10^" << exp << " = "
  //<< (man * pow(10, double(exp))) << endl;

  return make_pair(man, exp);
}

int Tools::integerRound1stDigit(int value)
{
	auto v = div(value, 10);
	switch(v.rem)
	{
	case 0: case 1: case 2: case 3: case 4:
	case -1: case -2: case -3: case -4: return v.quot*10;
	case 5: case 6: case 7: case 8: case 9: return v.quot*10 + 10;
	case -5: case -6: case -7: case -8: case -9: return v.quot*10 - 10;
	}
}

int Tools::roundShiftedInt(double value, int roundToDigits)
{
	int shiftDigits = roundToDigits + 1;

	//round to full integers or digits after the decimal point
	if(roundToDigits >= 0) 
		return integerRound1stDigit(shiftDecimalPointRight<int>(value, shiftDigits)) / 10;

	//round to full 100s
	if(roundToDigits < -1) 
		return integerRound1stDigit(shiftDecimalPointLeft<int>(value, -shiftDigits)) / 10;

	//round to full 10s
	return integerRound1stDigit(value);
}

double Tools::floor(double value, int digits, bool trailingDigits)
{
  if(trailingDigits)
    return std::floor(value * std::pow(double(10), digits)) / std::pow(double(10), digits);
  return std::floor(value / std::pow(double(10), digits)) * std::pow(double(10), digits);
}

double Tools::ceil(double value, int digits, bool trailingDigits)
{
  if(trailingDigits)
    return std::ceil(value * std::pow(double(10), digits)) / std::pow(double(10), digits);
  return std::ceil(value / std::pow(double(10), digits)) * std::pow(double(10), digits);
}

void Tools::testRoundFloorCeil()
{
  assert(round(1.5) == 2);
  assert(round(1.4) == 1);
  assert(round(2.) == 2);

  assert(round(1.55, 1) == 1.6);
  assert(round(1.54, 1) == 1.5);
  assert(round(1.5, 1) == 1.5);

  assert(roundShiftedInt(1.5555, 3) == 1556);
  assert(roundShiftedInt(1.5551, 3) == 1555);
  assert(roundShiftedInt(1.5559, 3) == 1556);

  assert(roundRT<int>(123., -1) == 120);
  assert(roundRT<int>(125., -1) == 130);
  assert(roundRT<int>(130., -1) == 130);

  assert(roundRT<int>(123456., -4) == 120000);
  assert(roundRT<int>(125456., -4) == 130000);
  assert(roundRT<int>(120456., -4) == 120000);

  assert(floor(1.5) == 1);
  assert(floor(1.4) == 1);
  assert(floor(2) == 2);

  assert(floor(1.55, 1) == 1.5);
  assert(floor(1.54, 1) == 1.5);
  assert(floor(1.5, 1) == 1.5);

  assert(floor(1.5555, 3) == 1.555);
  assert(floor(1.5551, 3) == 1.555);
  assert(floor(1.5559, 3) == 1.555);

  assert(floor(123, 1, false) == 120);
  assert(floor(125, 1, false) == 120);
  assert(floor(130, 1, false) == 130);

  assert(floor(123456, 4, false) == 120000);
  assert(floor(125456, 4, false) == 120000);
  assert(floor(120456, 4, false) == 120000);

  assert(ceil(1.5) == 2);
  assert(ceil(1.4) == 2);
  assert(ceil(2) == 2);

  assert(ceil(1.55, 1) == 1.6);
  assert(ceil(1.54, 1) == 1.6);
  assert(ceil(1.5, 1) == 1.5);

  assert(ceil(1.5555, 3) == 1.556);
  assert(ceil(1.5551, 3) == 1.556);
  assert(ceil(1.5559, 3) == 1.556);

  assert(ceil(123, 1, false) == 130);
  assert(ceil(125, 1, false) == 130);
  assert(ceil(130, 1, false) == 130);

  assert(ceil(123456, 4, false) == 130000);
  assert(ceil(125456, 4, false) == 130000);
  assert(ceil(120456, 4, false) == 130000);
}
