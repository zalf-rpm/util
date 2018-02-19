/* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/*
Authors: 
Michael Berg-Mohnicke <michael.berg-mohnicke@zalf.de>

Maintainers: 
Currently maintained by the authors.

This file is part of the util library used by models created at the Institute of
Landscape Systems Analysis at the ZALF.
Copyright (C) Leibniz Centre for Agricultural Landscape Research (ZALF)
*/

package util.soil;

using MonicaIO.EResultTools;
using StringTools;

typedef EResult<T> = {
  ?result: T,
  ?errors: Array<String>,
}

class EResultTools<T> {
  static public function success(r: EResult<T>) return r.result != null && r.errors != null && r.errors.length == 0;
  static public function failure(r: EResult<T>) return !r.success();
}

class Conversion 
{
  public static function humusClass2corg(humusClass : Int) : EResult<Float>
  {
    var corg = switch humusClass { 
      case 0: 0.0;
      case 1: 0.5 / 1.72;
      case 2: 1.5 / 1.72;
      case 3: 3.0 / 1.72;
      case 4: 6.0 / 1.72;
      case 5: 11.5 / 2.0;
      case 6: 17.5 / 2.0;
      case 7: 30.0 / 2.0;
      default: -1;
    };
    return if(corg < 0) {result: 0.0, errors: 'humusClass2corg: Unknown humus class: $humusClass !'} else {result: corg, errors: []};
  }

  public static function bulkDensityClass2rawDensity(bulkDensityClass : Int, clay : Float) : EResult<Float>
  {
    //EResult<double> res;
    var x = switch bulkDensityClass {
      case 1: 1.3;
      case 2: 1.5;
      case 3: 1.7;
      case 4: 1.9;
      case 5: 2.1;
      default -1;
    };

    return 
      if(x < 0) 
        {result: 0, errors: 'bulkDensityClass2rawDensity: Unknown bulk density class: $bulkDensityClass !'}
      else
        //* 1000 = conversion from g cm-3 -> kg m-3
        {result: (x - (0.9 * clay)) * 1000.0, errors: []}; 
  }

  public static function sandAndClay2lambda(sand : Float, clay : Float) : Float
  {
    var lambda = (2.0 * (sand * sand * 0.575)) + (clay * 0.1) + ((1.0 - sand - clay) * 0.35);
    // lambda = 1.0; //! @todo <b>Claas:</b> Temporary override until we have solved the problem with low water percolation loam soils
    return lambda;
  }

  public static function sandAndClay2KA5texture(sand : Float, clay : Float) : String
  {
    var silt = 1.0 - sand - clay;
    var soilTexture = "";

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

    return soilTexture;
  }

  public static function KA5texture2sand(soilType : String) : EResult<Float>
  {
    var x = -1;
    
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
    else if(soilType == "Lts")
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


    return 
      if(x < 0)
        {result: 0, errors: 'KA5texture2sand Unknown soil type: $soilType !'}
      else
        {result: x, errors: []};
  }

  public static function KA5texture2clay(soilType : String) : EResult<Float>
  {
    var x = -1;
    
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

    return 
      if(x < 0)
        {result: 0, errors: 'KA5texture2clay: Unknown soil type: $soilType !'}
      else
        {result: x, errors: []};
  }
}