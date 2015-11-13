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

#include <map>
#include <sstream>
#include <iostream>
#include <fstream>
#include <cmath>
#include <utility>
#include <mutex>

#include "db/abstract-db-connections.h"
#include "tools/helper.h"
#include "tools/algorithms.h"
#include "conversion.h"
#include "soil.h"
#include "tools/debug.h"
#include "constants.h"
#include "tools/json11-helper.h"

using namespace Db;
using namespace std;
using namespace Tools;
using namespace Soil;

//----------------------------------------------------------------------------------

SoilParameters::SoilParameters(json11::Json j)
{
  merge(j);
}

void SoilParameters::merge(json11::Json j)
{
  set_double_value(vs_SoilSandContent, j, "Sand");
  set_double_value(vs_SoilClayContent, j, "Clay");
  set_double_value(vs_SoilpH, j, "pH");
  set_double_value(vs_SoilStoneContent, j, "Sceleton");
  set_double_value(vs_Lambda, j, "Lambda");
  set_double_value(vs_FieldCapacity, j, "FieldCapacity");
  set_double_value(vs_Saturation, j, "PoreVolume");
  set_double_value(vs_PermanentWiltingPoint, j, "PermanentWiltingPoint");
  set_string_value(vs_SoilTexture, j, "KA5TextureClass");
  set_double_value(vs_SoilAmmonium, j, "SoilAmmonium");
  set_double_value(vs_SoilNitrate, j, "SoilNitrate");
  set_double_value(vs_Soil_CN_Ratio, j, "CN");
  set_double_value(vs_SoilMoisturePercentFC, j, "SoilMoisturePercentFC");
  set_double_value(_vs_SoilRawDensity, j, "SoilRawDensity");
  set_double_value(_vs_SoilBulkDensity, j, "SoilBulkDensity");
  set_double_value(_vs_SoilOrganicCarbon, j, "SoilOrganicCarbon");
  set_double_value(_vs_SoilOrganicMatter, j, "SoilOrganicMatter");

  auto res = vs_SoilTexture == ""
             ? fcSatPwpFromVanGenuchten(vs_SoilSandContent,
                                        vs_SoilClayContent,
                                        vs_SoilStoneContent,
                                        vs_SoilBulkDensity(),
                                        vs_SoilOrganicCarbon())
             : fcSatPwpFromKA5textureClass(vs_SoilTexture,
                                           vs_SoilStoneContent,
                                           vs_SoilRawDensity(),
                                           vs_SoilOrganicMatter());
  if(vs_FieldCapacity < 0)
    vs_FieldCapacity = res.fc;
  if(vs_Saturation < 0)
    vs_Saturation = res.sat;
  if(vs_PermanentWiltingPoint < 0)
    vs_PermanentWiltingPoint = res.pwp;

  if(vs_Lambda < 0)
    vs_Lambda = sandAndClay2lambda(vs_SoilSandContent, vs_SoilClayContent);
}

json11::Json SoilParameters::to_json() const
{
  return J11Object {
    {"type", "SoilParameters"},
    {"Sand", vs_SoilSandContent},
    {"Clay", vs_SoilClayContent},
    {"pH", vs_SoilpH},
    {"Sceleton", vs_SoilStoneContent},
    {"Lambda", vs_Lambda},
    {"FieldCapacity", vs_FieldCapacity},
    {"PoreVolume", vs_Saturation},
    {"PermanentWiltingPoint", vs_PermanentWiltingPoint},
    {"KA5TextureClass", vs_SoilTexture},
    {"SoilAmmonium", vs_SoilAmmonium},
    {"SoilNitrate", vs_SoilNitrate},
    {"CN", vs_Soil_CN_Ratio},
    {"SoilRawDensity", _vs_SoilRawDensity},
    {"SoilBulkDensity", _vs_SoilBulkDensity},
    {"SoilOrganicCarbon", _vs_SoilOrganicCarbon},
    {"SoilOrganicMatter", _vs_SoilOrganicMatter},
    {"SoilMoisturePercentFC", vs_SoilMoisturePercentFC}};
}

//----------------------------------------------------------------------------------

void CapillaryRiseRates::addRate(std::string bodart, int distance, double value)
{
  //        std::cout << "Add cap rate: " << bodart.c_str() << "\tdist: " << distance << "\tvalue: " << value << std::endl;
  //cap_rates_map.insert(std::pair<std::string,std::map<int,double> >(bodart,std::pair<int,double>(distance,value)));
  cap_rates_map[bodart][distance] = value;
}

/**
   * Returns capillary rise rate for given soil type and distance to ground water.
   */
double CapillaryRiseRates::getRate(std::string bodart, int distance) const
{
  typedef std::map<int, double> T_BodArtMap;
  //        std::cout << "Get capillary rise rate: " << bodart.c_str() << "\t" << distance << std::endl;
  T_BodArtMap map = getMap(bodart);
  if (map.size() <= 0 )
  {
    std::cout << "Error. No capillary rise rates in data structure available.\n" << std::endl;
    exit(-1);
  }

  T_BodArtMap::iterator it = map.find(distance);
  if (it != map.end())
    return it->second;

  return 0.0;
}

std::map<int,double> CapillaryRiseRates::getMap(std::string bodart) const
{
  typedef std::map<int, double> T_BodArtMap;
  typedef std::map<std::string, T_BodArtMap> T_CapRatesMap;

  T_CapRatesMap::const_iterator it2 = cap_rates_map.find(bodart);
  if (it2 != cap_rates_map.end())
    return it2->second;

  T_BodArtMap tmp;
  return tmp;
}

const CapillaryRiseRates& Soil::readCapillaryRiseRates()
{
  static mutex lockable;
  static bool initialized = false;
  static CapillaryRiseRates cap_rates;

  if (!initialized)
  {
    lock_guard<mutex> lock(lockable);

    if(!initialized)
    {

      static const string query =
          "select soil_type, distance, capillary_rate "
          "from capillary_rise_rate";

      // read capillary rise rates from database
      DB *con = newConnection("ka5-soil-data");
      con->select(query.c_str());

      DBRow row;
      while (!(row = con->getRow()).empty())
      {
        string soil_type = row[0];
        int distance = satoi(row[1]);
        double rate = satof(row[2]);
        cap_rates.addRate(soil_type, distance, rate);
      }

      delete con;

      initialized = true;
    }
  }

  return cap_rates;
}

//------------------------------------------------------------------------------

bool SoilParameters::isValid()
{
  bool is_valid = true;

  if (vs_FieldCapacity < 0) {
      debug() << "SoilParameters::Error: No field capacity defined in database for " << vs_SoilTexture << " , RawDensity: "<< _vs_SoilRawDensity << endl;
      is_valid = false;
  }
  if (vs_Saturation < 0) {
      debug() << "SoilParameters::Error: No saturation defined in database for " << vs_SoilTexture << " , RawDensity: " << _vs_SoilRawDensity << endl;
      is_valid = false;
  }
  if (vs_PermanentWiltingPoint < 0) {
      debug() << "SoilParameters::Error: No saturation defined in database for " << vs_SoilTexture << " , RawDensity: " << _vs_SoilRawDensity << endl;
      is_valid = false;
  }

  if (vs_SoilSandContent<0) {
      debug() << "SoilParameters::Error: Invalid soil sand content: "<< vs_SoilSandContent << endl;
      is_valid = false;
  }

  if (vs_SoilClayContent<0) {
      debug() << "SoilParameters::Error: Invalid soil clay content: "<< vs_SoilClayContent << endl;
      is_valid = false;
  }

  if (vs_SoilpH<0) {
      debug() << "SoilParameters::Error: Invalid soil ph value: "<< vs_SoilpH << endl;
      is_valid = false;
  }

  if (vs_SoilStoneContent<0) {
      debug() << "SoilParameters::Error: Invalid soil stone content: "<< vs_SoilStoneContent << endl;
      is_valid = false;
  }

  if (vs_Saturation<0) {
      debug() << "SoilParameters::Error: Invalid value for saturation: "<< vs_Saturation << endl;
      is_valid = false;
  }

  if (vs_PermanentWiltingPoint<0) {
      debug() << "SoilParameters::Error: Invalid value for permanent wilting point: "<< vs_PermanentWiltingPoint << endl;
      is_valid = false;
  }
  /*
  if (_vs_SoilRawDensity<0) {
      cout << "SoilParameters::Error: Invalid soil raw density: "<< _vs_SoilRawDensity << endl;
      is_valid = false;
  }
  */
  return is_valid;
}

/**
 * @brief Returns raw density of soil
 * @return raw density of soil
 */
double SoilParameters::vs_SoilRawDensity() const
{
  // conversion from g cm-3 in kg m-3
  return _vs_SoilRawDensity * 1000.0;
}

/**
* @brief Getter for soil bulk density.
* @return bulk density
*/
double SoilParameters::vs_SoilBulkDensity() const
{
  if (_vs_SoilRawDensity < 0)
    return _vs_SoilBulkDensity;

  return (_vs_SoilRawDensity + (0.009*100.0*vs_SoilClayContent))*1000.0;
}

/**
 * @brief Returns soil organic carbon.
 * @return soil organic carbon
 */
double SoilParameters::vs_SoilOrganicCarbon() const
{
  if (_vs_SoilOrganicMatter < 0)
    return _vs_SoilOrganicCarbon;

  return _vs_SoilOrganicMatter * OrganicConstants::po_SOM_to_C;
}

/**
 * @brief Getter for soil organic matter.
 * @return Soil organic matter
 */
double SoilParameters::vs_SoilOrganicMatter() const
{
  if (_vs_SoilOrganicCarbon < 0)
    return _vs_SoilOrganicMatter;
  return _vs_SoilOrganicCarbon / OrganicConstants::po_SOM_to_C;
}

/**
 * @brief Returns lambda from soil texture
 *
 * @param lambda
 *
 * @return
 */
double SoilParameters::sandAndClay2lambda(double sand, double clay)
{
  return ::sandAndClay2lambda(sand, clay);
}

//------------------------------------------------------------------------------

const SoilPMsPtr Soil::soilParameters(const string& abstractDbSchema,
                                      int profileId,
                                      int layerThicknessCm,
                                      int maxDepthCm,
                                      bool loadSingleParameter)
{
  int maxNoOfLayers = int(double(maxDepthCm)/double(layerThicknessCm));

  static mutex lockable;

  typedef map<int, SoilPMsPtr> Map;
  typedef map<string, Map> Map2;
  static bool initialized = false;
  static Map2 spss2;

  //yet unloaded schema
  if(initialized && spss2.find(abstractDbSchema) == spss2.end())
    initialized = false;

  if(!initialized)
  {
    lock_guard<mutex> lock(lockable);

    if (!initialized)
    {
      DBPtr con(newConnection(abstractDbSchema));
      DBRow row;

      ostringstream s;
      s << "select id, count(id) "
           "from soil_profiles "
           "group by id";
      con->select(s.str().c_str());

      map<int, int> id2layerCount;
      while (!(row = con->getRow()).empty())
        id2layerCount[satoi(row[0])] = satoi(row[1]);
      con->freeResultSet();

      set<int> skip;

      ostringstream s2;
      s2 << "select id, layer_depth_cm, soil_organic_carbon_percent, soil_raw_density_t_per_m3, "
            "sand_content_percent, clay_content_percent, ph_value, soil_type "
            "from soil_profiles ";
      if(loadSingleParameter)
        s2 << "where id = " << profileId << " ";
      s2 << "order by id, layer_depth_cm";

      Map& spss = spss2[abstractDbSchema];

      con->select(s2.str().c_str());
      int currenth = 0;
      while(!(row = con->getRow()).empty())
      {
        int id = satoi(row[0]);

        //Skip elements which are incomplete
        if(skip.find(id) != skip.end())
          continue;

        SoilPMsPtr sps = spss[id];
        if(!sps)
        {
          sps = spss[id] = SoilPMsPtr(new SoilPMs);
          currenth = 0;
        }

        int hcount = id2layerCount[id];
        currenth++;

        int ho = sps->size()*layerThicknessCm;
        int hu = !row[1].empty() ? satoi(row[1]) : maxDepthCm;
        int hsize = max(0, hu - ho);
        int subhcount = Tools::roundRT<int>(double(hsize)/double(layerThicknessCm), 0);
        if(currenth == hcount && (int(sps->size()) + subhcount) < maxNoOfLayers)
          subhcount += maxNoOfLayers - sps->size() - subhcount;

        SoilParameters p;
        p.set_vs_SoilOrganicCarbon(satof(row[2]) / 100.);
        p.set_vs_SoilRawDensity(satof(row[3]));
        p.vs_SoilSandContent = satof(row[4]) / 100.0;
        p.vs_SoilClayContent = satof(row[5]) / 100.0;
        if(!row[6].empty())
          p.vs_SoilpH = satof(row[6]);
        if(row[7].empty())
          p.vs_SoilTexture = sandAndClay2KA5texture(p.vs_SoilSandContent, p.vs_SoilClayContent);
        else
          p.vs_SoilTexture = row[7];
        p.vs_SoilStoneContent = 0.0;
        p.vs_Lambda = sandAndClay2lambda(p.vs_SoilSandContent, p.vs_SoilClayContent);

        // initialization of saturation, field capacity and perm. wilting point
        soilCharacteristicsKA5(p);
        if(!p.isValid())
        {
          skip.insert(id);
          cout << "Error in soil parameters. Skipping profileId: " << id << endl;
          spss.erase(id);
          continue;
        }

        for(int i = 0; i < subhcount; i++)
          sps->push_back(p);
      }

      initialized = true;
    }
  }

  static SoilPMsPtr nothing = make_shared<SoilPMs>();
  auto ci2 = spss2.find(abstractDbSchema);
  if(ci2 != spss2.end())
  {
    Map& spss = ci2->second;
    Map::const_iterator ci = spss.find(profileId);
    return ci != spss.end() ? ci->second : nothing;
  }

  return nothing;
}

//------------------------------------------------------------------------------

string Soil::soilProfileId2KA5Layers(const string& abstractDbSchema,
                                     int soilProfileId)
{
  static mutex lockable;

  typedef map<int, string> Map;
  typedef map<string, Map> Map2;
  static bool initialized = false;
  static Map2 m;

  //yet unloaded schema
  if(initialized && m.find(abstractDbSchema) == m.end())
    initialized = false;

  if (!initialized)
  {
    lock_guard<mutex> lock(lockable);

    if (!initialized)
    {
      DBPtr con(newConnection(abstractDbSchema));
      con->setCharacterSet("utf8");
      DBRow row;

      Map& m2 = m[abstractDbSchema];

      con->select("SELECT id, soil_type "
                  "from soil_profiles "
                  "order by id, layer_depth_cm");
      while (!(row = con->getRow()).empty())
      {
        string pre = m2[satoi(row[0])].empty() ? "" : "|";
        m2[satoi(row[0])].append(pre).append(row[1]);
      }

      initialized = true;
    }
  }

  auto ci2 = m.find(abstractDbSchema);
  if(ci2 != m.end())
  {
    Map& m2 = ci2->second;
    Map::const_iterator ci = m2.find(soilProfileId);
    return ci != m2.end() ? ci->second : "Soil profile not found!";
  }

  return "Soil profile database not found!";
}

//------------------------------------------------------------------------------

const SoilPMsPtr Soil::soilParametersFromHermesFile(int soilId,
                                                  const string& pathToFile,
                                                  int layerThicknessCm,
                                                  int maxDepthCm,
                                                  double soil_ph,
                                                  double drainage_coeff)
{
  debug() << pathToFile.c_str() << endl;
  int maxNoOfLayers = int(double(maxDepthCm) / double(layerThicknessCm));

  static mutex lockable;

  typedef map<int, SoilPMsPtr> Map;
  static bool initialized = false;
  static Map spss;
  if (!initialized)
  {
    lock_guard<mutex> lock(lockable);

    if (!initialized)
    {
      ifstream ifs(pathToFile.c_str(), ios::binary);
      string s;

      //skip first line(s)
      getline(ifs, s);

      int currenth = 1;
      while (getline(ifs, s))
      {
        //cout << "s: " << s << endl;
        if (trim(s) == "end")
          break;

        //BdID Corg Bart UKT LD Stn C/N C/S Hy Wmx AzHo
        int ti;
        string ba, ts;
        int id, hu, ld, stone, cn, hcount;
        double corg, wmax;
        istringstream ss(s);
        ss >> id >> corg >> ba >> hu >> ld >> stone >> cn >> ts
          >> ti >> wmax >> hcount;

        //double vs_SoilSpecificMaxRootingDepth = wmax / 10.0; //[dm] --> [m]

        hu *= 10;
        //Reset horizont count to start new soil definition
        if (hcount > 0)
          currenth = 1;

        Map::iterator spsi = spss.find(soilId);
        SoilPMsPtr sps;
        if (spsi == spss.end()) {
          spss.insert(make_pair(soilId, sps = SoilPMsPtr(new SoilPMs)));
        } else {
          sps = spsi->second;
        }

        int ho = sps->size()*layerThicknessCm;
        int hsize = max(0, hu - ho);
        int subhcount = int(Tools::round(double(hsize) / double(layerThicknessCm)));
        if (currenth == hcount && (int(sps->size()) + subhcount) < maxNoOfLayers)
          subhcount += maxNoOfLayers - sps->size() - subhcount;

        if ((ba != "Ss") && (ba != "Sl2") && (ba != "Sl3") && (ba != "Sl4") &&
          (ba != "Slu") && (ba != "St2") && (ba != "St3") && (ba != "Su2") &&
          (ba != "Su3") && (ba != "Su4") && (ba != "Ls2") && (ba != "Ls3") &&
          (ba != "Ls4") && (ba != "Lt2") && (ba != "Lt3") && (ba != "Lts") &&
          (ba != "Lu") && (ba != "Uu") && (ba != "Uls") && (ba != "Us") &&
          (ba != "Ut2") && (ba != "Ut3") && (ba != "Ut4") && (ba != "Tt") &&
          (ba != "Tl") && (ba != "Tu2") && (ba != "Tu3") && (ba != "Tu4") &&
          (ba != "Ts2") && (ba != "Ts3") && (ba != "Ts4") && (ba != "fS")  &&
          (ba != "fS") && (ba != "fSms") && (ba != "fSgs") && (ba != "mS") &&
          (ba != "mSfs") && (ba != "mSgs") && (ba != "gS") && (ba != "Hh") &&
          (ba != "Hn")) {
            cerr << "No valid texture class defined: " << ba.c_str() << endl;
            exit(1);
        }

        SoilParameters p;
        p.set_vs_SoilOrganicCarbon(corg / 100.0); //[kg C 100kg] --> [kg C kg-1]
        p.set_vs_SoilRawDensity(ld_eff2trd(ld, KA5texture2clay(ba)));
        p.vs_SoilSandContent = KA5texture2sand(ba);
        p.vs_SoilClayContent = KA5texture2clay(ba);
        p.vs_SoilStoneContent = stone / 100.0;
        p.vs_Lambda = sandAndClay2lambda(p.vs_SoilSandContent, p.vs_SoilClayContent);
        p.vs_SoilTexture = ba;

        if (soil_ph != -1.0)
          p.vs_SoilpH = soil_ph;

        if (drainage_coeff != -1.0)
          p.vs_Lambda = drainage_coeff;

        // initialization of saturation, field capacity and perm. wilting point
        soilCharacteristicsKA5(p);

        bool valid_soil_params = p.isValid();
        if (!valid_soil_params) {
          cout << "Error in soil parameters. Aborting now simulation";
          exit(-1);
        }

        for (int i = 0; i < subhcount; i++)
          sps->push_back(p);
        currenth++;
      }

      initialized = true;

    }
  }

  static SoilPMsPtr nothing = make_shared<SoilPMs>();
  auto ci = spss.find(soilId);
  return ci != spss.end() ? ci->second : nothing;
}

//------------------------------------------------------------------------------

RPSCDRes Soil::readPrincipalSoilCharacteristicData(string soilType, double rawDensity)
{
  static mutex lockable;
  typedef map<int, RPSCDRes> M1;
  typedef map<string, M1> M2;
  static M2 m;
  static bool initialized = false;
  if(!initialized)
  {
    lock_guard<mutex> lock(lockable);

    if(!initialized)
    {
      DBPtr con(newConnection("ka5-soil-data"));

      string query("select soil_type, soil_raw_density*10, "
                   "air_capacity, field_capacity, n_field_capacity "
                   "from soil_characteristic_data "
                   "where air_capacity != 0 and field_capacity != 0 and n_field_capacity != 0 "
                   "order by soil_type, soil_raw_density");
      con->select(query.c_str());

      debug() << endl << query.c_str() << endl;
      DBRow row;
      while(!(row = con->getRow()).empty())
      {
        double ac = satof(row[2]);
        double fc = satof(row[3]);
        double nfc = satof(row[4]);

        RPSCDRes r(true);
        r.sat = ac + fc;
        r.fc = fc;
        r.pwp = fc - nfc;

        m[row[0]][satoi(row[1])] = r;
      }

      initialized = true;
    }
  }

  auto ci = m.find(soilType);
  if(ci != m.end())
  {
    int rd10 = int(rawDensity*10);
    int delta = rd10 < 15 ? 2 : -2;

    M1::const_iterator ci2;
    //if we didn't find values for a given raw density, e.g. 1.1 (= 11)
    //we try to find the closest next one (up (1.1) or down (1.9))
    while((ci2 = ci->second.find(rd10)) == ci->second.end() &&
          (11 <= rd10 && rd10 <= 19))
      rd10 += delta;

    return ci2 != ci->second.end() ? ci2->second : RPSCDRes();
  }

  return RPSCDRes();
}

//------------------------------------------------------------------------------

RPSCDRes Soil::readSoilCharacteristicModifier(string soilType, double organicMatter)
{
  static mutex lockable;
  typedef map<int, RPSCDRes> M1;
  typedef map<string, M1> M2;
  static M2 m;
  static bool initialized = false;
  if(!initialized)
  {
    lock_guard<mutex> lock(lockable);

    if(!initialized)
    {
      DBPtr con(newConnection("ka5-soil-data"));

      string query("select soil_type, organic_matter*10, "
                   "air_capacity, field_capacity, n_field_capacity "
                   "from soil_aggregation_values "
                   "order by soil_type, organic_matter");
      con->select(query.c_str());

      debug() << endl << query.c_str() << endl;
      DBRow row;
      while(!(row = con->getRow()).empty())
      {
        double ac = satof(row[2]);
        double fc = satof(row[3]);
        double nfc = satof(row[4]);

        RPSCDRes r(true);
        r.sat = ac + fc;
        r.fc = fc;
        r.pwp = fc - nfc;

        m[row[0]][satoi(row[1])] = r;
      }

      initialized = true;
    }
  }

  auto ci = m.find(soilType);
  if(ci != m.end())
  {
    auto ci2 = ci->second.find(int(organicMatter*10));
    return ci2 != ci->second.end() ? ci2->second : RPSCDRes();
  }

  return RPSCDRes();
}

//------------------------------------------------------------------------------

void Soil::soilCharacteristicsKA5(SoilParameters& sp)
{
  auto res = fcSatPwpFromKA5textureClass(sp.vs_SoilTexture,
                                         sp.vs_SoilStoneContent,
                                         sp.vs_SoilRawDensity(),
                                         sp.vs_SoilOrganicMatter());
  sp.vs_FieldCapacity = res.fc;
  sp.vs_Saturation = res.sat;
  sp.vs_PermanentWiltingPoint = res.pwp;
}

FcSatPwp Soil::fcSatPwpFromKA5textureClass(std::string texture,
                                           double stoneContent,
                                           double soilRawDensity,
                                           double soilOrganicMatter)
{
  debug() << "soilCharacteristicsKA5" << endl;

  FcSatPwp res;

  if (texture != "")
  {
    double srd = soilRawDensity/1000.0; // [kg m-3] -> [g cm-3]
    double som = soilOrganicMatter*100.0; // [kg kg-1] -> [%]

    // ***************************************************************************
    // *** The following boundaries are extracted from:                        ***
    // *** Wessolek, G., M. Kaupenjohann, M. Renger (2009) Bodenphysikalische  ***
    // *** Kennwerte und Berechnungsverfahren für die Praxis. Bodenökologie    ***
    // *** und Bodengenese 40, Selbstverlag Technische Universität Berlin      ***
    // *** (Tab. 4).                                                           ***
    // ***************************************************************************

    double srd_lowerBound = 0.0;
    double srd_upperBound = 0.0;
    if (srd < 1.1)
    {
      srd_lowerBound = 1.1;
      srd_upperBound = 1.1;
    }
    else if ((srd >= 1.1) && (srd < 1.3))
    {
      srd_lowerBound = 1.1;
      srd_upperBound = 1.3;
    }
    else if ((srd >= 1.3) && (srd < 1.5))
    {
      srd_lowerBound = 1.3;
      srd_upperBound = 1.5;
    }
    else if ((srd >= 1.5) && (srd < 1.7))
    {
      srd_lowerBound = 1.5;
      srd_upperBound = 1.7;
    }
    else if ((srd >= 1.7) && (srd < 1.9))
    {
      srd_lowerBound = 1.7;
      srd_upperBound = 1.9;
    }
    else if (srd >= 1.9)
    {
      srd_lowerBound = 1.9;
      srd_upperBound = 1.9;
    }

    // special treatment for "torf" soils
    if (texture=="Hh" || texture=="Hn") {
        srd_lowerBound = -1;
        srd_upperBound = -1;
    }

    // Boundaries for linear interpolation
    auto lbRes = readPrincipalSoilCharacteristicData(texture, srd_lowerBound);
    double sat_lowerBound = lbRes.sat;
    double fc_lowerBound = lbRes.fc;
    double pwp_lowerBound = lbRes.pwp;

    auto ubRes = readPrincipalSoilCharacteristicData(texture, srd_upperBound);
    double sat_upperBound = ubRes.sat;
    double fc_upperBound = ubRes.fc;
    double pwp_upperBound = ubRes.pwp;

    if(lbRes.initialized && ubRes.initialized)
    {
      //    cout << "Soil Raw Density:\t" << vs_SoilRawDensity << endl;
      //    cout << "Saturation:\t\t" << vs_SaturationLowerBoundary << "\t" << vs_SaturationUpperBoundary << endl;
      //    cout << "Field Capacity:\t" << vs_FieldCapacityLowerBoundary << "\t" << vs_FieldCapacityUpperBoundary << endl;
      //    cout << "PermanentWP:\t" << vs_PermanentWiltingPointLowerBoundary << "\t" << vs_PermanentWiltingPointUpperBoundary << endl;
      //    cout << "Soil Organic Matter:\t" << vs_SoilOrganicMatter << endl;

      // ***************************************************************************
      // *** The following boundaries are extracted from:                        ***
      // *** Wessolek, G., M. Kaupenjohann, M. Renger (2009) Bodenphysikalische  ***
      // *** Kennwerte und Berechnungsverfahren für die Praxis. Bodenökologie    ***
      // *** und Bodengenese 40, Selbstverlag Technische Universität Berlin      ***
      // *** (Tab. 5).                                                           ***
      // ***************************************************************************

      double som_lowerBound = 0.0;
      double som_upperBound = 0.0;

      if(som >= 0.0 && som < 1.0)
      {
        som_lowerBound = 0.0;
        som_upperBound = 0.0;
      }
      else if(som >= 1.0 && som < 1.5)
      {
        som_lowerBound = 0.0;
        som_upperBound = 1.5;
      }
      else if(som >= 1.5 && som < 3.0)
      {
        som_lowerBound = 1.5;
        som_upperBound = 3.0;
      }
      else if(som >= 3.0 && som < 6.0)
      {
        som_lowerBound = 3.0;
        som_upperBound = 6.0;
      }
      else if(som >= 6.0 && som < 11.5)
      {
        som_lowerBound = 6.0;
        som_upperBound = 11.5;
      }
      else if(som >= 11.5)
      {
        som_lowerBound = 11.5;
        som_upperBound = 11.5;
      }

      // special treatment for "torf" soils
      if (texture=="Hh" || texture=="Hn")
      {
        som_lowerBound = 0.0;
        som_upperBound = 0.0;
      }

      // Boundaries for linear interpolation
      double fc_mod_lowerBound = 0.0;
      double sat_mod_lowerBound = 0.0;
      double pwp_mod_lowerBound = 0.0;
      // modifier values are given only for organic matter > 1.0% (class h2)
      if (som_lowerBound != 0.0)
      {
        auto lbRes = readSoilCharacteristicModifier(texture, som_lowerBound);
        sat_mod_lowerBound = lbRes.sat;
        fc_mod_lowerBound = lbRes.fc;
        pwp_mod_lowerBound = lbRes.pwp;
      }

      double fc_mod_upperBound = 0.0;
      double sat_mod_upperBound = 0.0;
      double pwp_mod_upperBound = 0.0;
      if (som_upperBound != 0.0)
      {
        auto ubRes = readSoilCharacteristicModifier(texture, som_upperBound);
        sat_mod_upperBound = ubRes.sat;
        fc_mod_upperBound = ubRes.fc;
        pwp_mod_upperBound = ubRes.pwp;
      }

//			cout << "Saturation-Modifier:\t" << sat_mod_lowerBound << "\t" << sat_mod_upperBound << endl;
//			cout << "Field capacity-Modifier:\t" << fc_mod_lowerBound << "\t" << fc_mod_upperBound << endl;
//			cout << "PWP-Modifier:\t" << pwp_mod_lowerBound << "\t" << pwp_mod_upperBound << endl;

      // Linear interpolation
      double fc_unmod = fc_lowerBound;
      if (fc_upperBound < 0.5 && fc_lowerBound >= 1.0)
        fc_unmod = fc_lowerBound;
      else if(fc_lowerBound < 0.5 && fc_upperBound >= 1.0)
        fc_unmod = fc_upperBound;
      else if(srd_upperBound != srd_lowerBound)
        fc_unmod = (srd - srd_lowerBound)/
                   (srd_upperBound - srd_lowerBound)*
                   (fc_upperBound - fc_lowerBound) + fc_lowerBound;

      double sat_unmod = sat_lowerBound;
      if(sat_upperBound < 0.5 && sat_lowerBound >= 1.0)
        sat_unmod = sat_lowerBound;
      else if(sat_lowerBound < 0.5 && sat_upperBound >= 1.0)
        sat_unmod = sat_upperBound;
      else if(srd_upperBound != srd_lowerBound)
        sat_unmod = (srd - srd_lowerBound)/
                    (srd_upperBound - srd_lowerBound)*
                    (sat_upperBound - sat_lowerBound) + sat_lowerBound;

      double pwp_unmod = pwp_lowerBound;
      if(pwp_upperBound < 0.5 && pwp_lowerBound >= 1.0)
        pwp_unmod = pwp_lowerBound;
      else if(pwp_lowerBound < 0.5 && pwp_upperBound >= 1.0)
        pwp_unmod = pwp_upperBound;
      else if(srd_upperBound != srd_lowerBound)
        pwp_unmod = (srd - srd_lowerBound)/
                    (srd_upperBound - srd_lowerBound)*
                    (pwp_upperBound - pwp_lowerBound) + pwp_lowerBound;

      //in this case upper and lower boundary are equal, so doesn't matter.
      double fc_mod = fc_mod_lowerBound;
      double sat_mod = sat_mod_lowerBound;
      double pwp_mod = pwp_mod_lowerBound;
      if(som_upperBound != som_lowerBound)
      {
        fc_mod = (som - som_lowerBound)/
                 (som_upperBound - som_lowerBound)*
                 (fc_mod_upperBound - fc_mod_lowerBound) + fc_mod_lowerBound;

        sat_mod = (som - som_lowerBound)/
                  (som_upperBound - som_lowerBound)*
                  (sat_mod_upperBound - sat_mod_lowerBound) + sat_mod_lowerBound;

        pwp_mod = (som - som_lowerBound)/
                  (som_upperBound - som_lowerBound)*
                  (pwp_mod_upperBound - pwp_mod_lowerBound) + pwp_mod_lowerBound;
      }

      // Modifying the principal values by organic matter
      res.fc = (fc_unmod + fc_mod)/100.0; // [m3 m-3]
      res.sat = (sat_unmod + sat_mod)/100.0; // [m3 m-3]
      res.pwp = (pwp_unmod + pwp_mod)/100.0; // [m3 m-3]

      // Modifying the principal values by stone content
      res.fc *= (1.0 - stoneContent);
      res.sat *= (1.0 - stoneContent);
      res.pwp *= (1.0 - stoneContent);
    }
  }

  debug() << "SoilTexture:\t\t\t" << texture << endl;
  debug() << "Saturation:\t\t\t" << res.sat << endl;
  debug() << "FieldCapacity:\t\t" << res.fc << endl;
  debug() << "PermanentWiltingPoint:\t" << res.pwp << endl << endl;

  return res;
}


//------------------------------------------------------------------------------

FcSatPwp Soil::fcSatPwpFromVanGenuchten(double sandContent,
                                        double clayContent,
                                        double stoneContent,
                                        double soilBulkDensity,
                                        double soilOrganicCarbon)
{
  FcSatPwp res;

  //cout << "Permanent Wilting Point is calculated from van Genuchten parameters" << endl;
  res.pwp = (0.015 + 0.5*clayContent + 1.4*soilOrganicCarbon)*(1.0 - stoneContent);

  res.sat = (0.81 - 0.283*(soilBulkDensity / 1000.0) + 0.1*clayContent)*(1.0 - stoneContent);

  //    cout << "Field capacity is calculated from van Genuchten parameters" << endl;
  double thetaR = res.pwp;
  double thetaS = res.sat;

  double vanGenuchtenAlpha = exp(-2.486
                                 + 2.5*sandContent
                                 - 35.1*soilOrganicCarbon
                                 - 2.617*(soilBulkDensity / 1000.0)
                                 - 2.3*clayContent);

  double vanGenuchtenM = 1.0;

  double vanGenuchtenN = exp(0.053
                             - 0.9*sandContent
                             - 1.3*clayContent
                             + 1.5*(pow(sandContent, 2.0)));

  //***** Van Genuchten retention curve to calculate volumetric water content at
  //***** moisture equivalent (Field capacity definition KA5)

  double fieldCapacity_pF = 2.1;
  if((sandContent > 0.48) && (sandContent <= 0.9) && (clayContent <= 0.12))
    fieldCapacity_pF = 2.1 - (0.476 * (sandContent - 0.48));
  else if((sandContent > 0.9) && (clayContent <= 0.05))
    fieldCapacity_pF = 1.9;
  else if(clayContent > 0.45)
    fieldCapacity_pF = 2.5;
  else if((clayContent > 0.30) && (sandContent < 0.2))
    fieldCapacity_pF = 2.4;
  else if(clayContent > 0.35)
    fieldCapacity_pF = 2.3;
  else if((clayContent > 0.25) && (sandContent < 0.1))
    fieldCapacity_pF = 2.3;
  else if((clayContent > 0.17) && (sandContent > 0.68))
    fieldCapacity_pF = 2.2;
  else if((clayContent > 0.17) && (sandContent < 0.33))
    fieldCapacity_pF = 2.2;
  else if((clayContent > 0.08) && (sandContent < 0.27))
    fieldCapacity_pF = 2.2;
  else if((clayContent > 0.25) && (sandContent < 0.25))
    fieldCapacity_pF = 2.2;

  double matricHead = pow(10, fieldCapacity_pF);

  res.fc = (thetaR
            + ((thetaS - thetaR)
               / (pow(1.0 + pow(vanGenuchtenAlpha * matricHead,
                                vanGenuchtenN),
                      vanGenuchtenM))))
           * (1.0 - stoneContent);

  return res;
}
