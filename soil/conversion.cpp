/* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/*
Authors: 
Claas Nendel <claas.nendel@zalf.de>
Xenia Specka <xenia.specka@zalf.de>
Michael Berg <michael.berg@zalf.de>

Maintainers: 
Currently maintained by the authors.

This file is part of the util library used by models created at the Institute of
Landscape Systems Analysis at the ZALF.
Copyright (C) Leibniz Centre for Agricultural Landscape Research (ZALF)
*/

#include "conversion.h"
#include  <iostream>

using namespace Soil;
using namespace std;
using namespace Tools;

EResult<double> Soil::humusClass2corg(int humusClass)
{
	switch (humusClass)
	{
	case 0:
		return 0.0;
	case 1:
		return 0.5 / 1.72;
	case 2:
		return 1.5 / 1.72;
	case 3:
		return 3.0 / 1.72;
	case 4:
		return 6.0 / 1.72;
	case 5:
		return 11.5 / 2.0;
	case 6:
		return 17.5 / 2.0;
	case 7:
		return 30.0 / 2.0;
	}
	return{0.0, string("Soil::humusClass2corg: Unknown humus class: " + to_string(humusClass) + "!")};
}

EResult<double> Soil::bulkDensityClass2rawDensity(int bulkDensityClass, double clay)
{
	EResult<double> res;
	double x = 0.0;

	switch(bulkDensityClass)
	{
	case 1: x = 1.3; break;
	case 2: x = 1.5; break;
	case 3: x = 1.7; break;
	case 4: x = 1.9; break;
	case 5: x = 2.1; break;
	default:
		res.errors.push_back(string("Soil::bulkDensityClass2rawDensity: Unknown bulk density class: " + to_string(bulkDensityClass) + "!"));
	}
	
	res.result = (x - (0.9 * clay)) * 1000.0; //* 1000 = conversion from g cm-3 -> kg m-3
	return res;
}

double Soil::sandAndClay2lambda(double sand, double clay)
{
	double lambda = (2.0 * (sand * sand * 0.575)) + (clay * 0.1) + ((1.0 - sand - clay) * 0.35);
  // lambda = 1.0; //! @todo <b>Claas:</b> Temporary override until we have solved the problem with low water percolation loam soils
	return lambda;
}

EResult<string> Soil::sandAndClay2KA5texture(double sand, double clay)
{
	EResult<string> res;
	double silt = 1.0 - sand - clay;
	string& soilTexture = res.result;

	if(silt < 0.1 && clay < 0.05)
		soilTexture = "Ss";
	else if(silt < 0.25 && clay < 0.05)
		soilTexture = "Su2";
	else if(silt < 0.25 && clay < 0.08)
		soilTexture = "Sl2";
	else if(silt < 0.40 && clay < 0.08)
		soilTexture = "Su3";
	else if(silt < 0.50 && clay < 0.08)
		soilTexture = "Su4";
	else if(silt < 0.8 && clay < 0.08)
		soilTexture = "Us";
	else if(silt >= 0.8 && clay < 0.08)
		soilTexture = "Uu";
	else if(silt < 0.1 && clay < 0.17)
		soilTexture = "St2";
	else if(silt < 0.4 && clay < 0.12)
		soilTexture = "Sl3";
	else if(silt < 0.4 && clay < 0.17)
		soilTexture = "Sl4";
	else if(silt < 0.5 && clay < 0.17)
		soilTexture = "Slu";
	else if(silt < 0.65 && clay < 0.17)
		soilTexture = "Uls";
	else if(silt >= 0.65 && clay < 0.12)
		soilTexture = "Ut2";
	else if(silt >= 0.65 && clay < 0.17)
		soilTexture = "Ut3";
	else if(silt < 0.15 && clay < 0.25)
		soilTexture = "St3";
	else if(silt < 0.30 && clay < 0.25)
		soilTexture = "Ls4";
	else if(silt < 0.40 && clay < 0.25)
		soilTexture = "Ls3";
	else if(silt < 0.50 && clay < 0.25)
		soilTexture = "Ls2";
	else if(silt < 0.65 && clay < 0.30)
		soilTexture = "Lu";
	else if(silt >= 0.65 && clay < 0.25)
		soilTexture = "Ut4";
	else if(silt < 0.15 && clay < 0.35)
		soilTexture = "Ts4";
	else if(silt < 0.30 && clay < 0.45)
		soilTexture = "Lts";
	else if(silt < 0.50 && clay < 0.35)
		soilTexture = "Lt2";
	else if(silt < 0.65 && clay < 0.45)
		soilTexture = "Tu3";
	else if(silt >= 0.65 && clay >= 0.25)
		soilTexture = "Tu4";
	else if(silt < 0.15 && clay < 0.45)
		soilTexture = "Ts3";
	else if(silt < 0.50 && clay < 0.45)
		soilTexture = "Lt3";
	else if(silt < 0.15 && clay < 0.65)
		soilTexture = "Ts2";
	else if(silt < 0.30 && clay < 0.65)
		soilTexture = "Tl";
	else if(silt >= 0.30 && clay < 0.65)
		soilTexture = "Tu2";
	else if(clay >= 0.65)
		soilTexture = "Tt";
	else
	{
		soilTexture = "";
		res.errors.push_back(string("Soil::sandAndClay2KA5texture: Undefined combination of sand (" + to_string(sand) + ") and clay (" + to_string(clay) + ")!"));
	}

	return res;
}

EResult<double> Soil::KA5texture2sand(string soilType)
{
	EResult<double> res;
	double& x = res.result;
  
  if(soilType == "fS")
    x = 0.84;
  else if(soilType == "fSms")
    x = 0.86;
  else if(soilType == "fSgs")
    x = 0.88;
  else if(soilType == "gS")
    x = 0.93;
  else if(soilType == "mSgs")
    x = 0.96;
  else if(soilType == "mSfs")
    x = 0.93;
  else if(soilType == "mS")
    x = 0.96;
  else if(soilType == "Ss")
    x = 0.93;
  else if(soilType == "Sl2")
    x = 0.76;
  else if(soilType == "Sl3")
    x = 0.65;
  else if(soilType == "Sl4")
    x = 0.60;
  else if(soilType == "Slu")
    x = 0.43;
  else if(soilType == "St2")
    x = 0.84;
  else if(soilType == "St3")
    x = 0.71;
  else if(soilType == "Su2")
    x = 0.80;
  else if(soilType == "Su3")
    x = 0.63;
  else if(soilType == "Su4")
    x = 0.56;
  else if(soilType == "Ls2")
    x = 0.34;
  else if(soilType == "Ls3")
    x = 0.44;
  else if(soilType == "Ls4")
    x = 0.56;
  else if(soilType == "Lt2")
    x = 0.30;
  else if(soilType == "Lt3")
    x = 0.20;
  else if(soilType == "LtS")
    x = 0.42;
  else if(soilType == "Lu")
    x = 0.19;
  else if(soilType == "Uu")
    x = 0.10;
  else if(soilType == "Uls")
    x = 0.30;
  else if(soilType == "Us")
    x = 0.31;
  else if(soilType == "Ut2")
    x = 0.13;
  else if(soilType == "Ut3")
    x = 0.11;
  else if(soilType == "Ut4")
    x = 0.09;
  else if(soilType == "Utl")
    x = 0.19;
  else if(soilType == "Tt")
    x = 0.17;
  else if(soilType == "Tl")
    x = 0.17;
  else if(soilType == "Tu2")
    x = 0.12;
  else if(soilType == "Tu3")
    x = 0.10;
  else if(soilType == "Ts3")
    x = 0.52;
  else if(soilType == "Ts2")
    x = 0.37;
  else if(soilType == "Ts4")
    x = 0.62;
  else if(soilType == "Tu4")
    x = 0.05;
  else if(soilType == "L")
    x = 0.35;
  else if(soilType == "S")
    x = 0.93;
  else if(soilType == "U")
    x = 0.10;
  else if(soilType == "T")
    x = 0.17;
  else if(soilType == "HZ1")
    x = 0.30;
  else if(soilType == "HZ2")
    x = 0.30;
  else if(soilType == "HZ3")
    x = 0.30;
  else if(soilType == "Hh")
    x = 0.15;
  else if(soilType == "Hn")
    x = 0.15;
	else
	{
		x = 0.66;
		res.errors.push_back(string("Soil::KA5texture2sand Unknown soil type: " + soilType + "!"));
	}

  return res;
}


EResult<double> Soil::KA5texture2clay(string soilType)
{
	EResult<double> res;
	double& x = res.result;
	
  if(soilType == "fS")
    x = 0.02;
  else if(soilType == "fSms")
    x = 0.02;
  else if(soilType == "fSgs")
    x = 0.02;
  else if(soilType == "gS")
    x = 0.02;
  else if(soilType == "mSgs")
    x = 0.02;
  else if(soilType == "mSfs")
    x = 0.02;
  else if(soilType == "mS")
    x = 0.02;
  else if(soilType == "Ss")
    x = 0.02;
  else if(soilType == "Sl2")
    x = 0.06;
  else if(soilType == "Sl3")
    x = 0.10;
  else if(soilType == "Sl4")
    x = 0.14;
  else if(soilType == "Slu")
    x = 0.12;
  else if(soilType == "St2")
    x = 0.11;
  else if(soilType == "St3")
    x = 0.21;
  else if(soilType == "Su2")
    x = 0.02;
  else if(soilType == "Su3")
    x = 0.04;
  else if(soilType == "Su4")
    x = 0.04;
  else if(soilType == "Ls2")
    x = 0.21;
  else if(soilType == "Ls3")
    x = 0.21;
  else if(soilType == "Ls4")
    x = 0.21;
  else if(soilType == "Lt2")
    x = 0.30;
  else if(soilType == "Lt3")
    x = 0.40;
  else if(soilType == "Lts")
    x = 0.35;
  else if(soilType == "Lu")
    x = 0.23;
  else if(soilType == "Uu")
    x = 0.04;
  else if(soilType == "Uls")
    x = 0.12;
  else if(soilType == "Us")
    x = 0.04;
  else if(soilType == "Ut2")
    x = 0.10;
  else if(soilType == "Ut3")
    x = 0.14;
  else if(soilType == "Ut4")
    x = 0.21;
  else if(soilType == "Utl")
    x = 0.23;
  else if(soilType == "Tt")
    x = 0.82;
  else if(soilType == "Tl")
    x = 0.55;
  else if(soilType == "Tu2")
    x = 0.55;
  else if(soilType == "Tu3")
    x = 0.37;
  else if(soilType == "Ts3")
    x = 0.40;
  else if(soilType == "Ts2")
    x = 0.55;
  else if(soilType == "Ts4")
    x = 0.30;
  else if(soilType == "Tu4")
    x = 0.30;
  else if(soilType == "L")
    x = 0.31;
  else if(soilType == "S")
    x = 0.02;
  else if(soilType == "U")
    x = 0.04;
  else if(soilType == "T")
    x = 0.82;
  else if(soilType == "HZ1")
    x = 0.15;
  else if(soilType == "HZ2")
    x = 0.15;
  else if(soilType == "HZ3")
    x = 0.15;
  else if(soilType == "Hh")
    x = 0.1;
  else if(soilType == "Hn")
    x = 0.1;
	else
	{
		x = 0.0;
		res.errors.push_back(string("Soil::KA5texture2clay: Unknown soil type: " + soilType + "!"));
	}

  return res;
}
