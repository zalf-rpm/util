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

#ifndef SOIL_SOIL_H_
#define SOIL_SOIL_H_

#include <memory>

#include <string>
#include <vector>
#include <map>
#include <functional>
#include <iostream>

#include "json11/json11.hpp"

namespace Soil
{
  /**
   * @author Claas Nendel, Michael Berg
   */
  struct SoilParameters
  {
    SoilParameters(){}

    SoilParameters(json11::Json j)
      : vs_SoilSandContent(j["SoilSandContent"].number_value()),
        vs_SoilClayContent(j["SoilClayContent"].number_value()),
        vs_SoilpH(j["SoilpH"].number_value()),
        vs_SoilStoneContent(j["SoilStoneContent"].number_value()),
        vs_Lambda(j["Lambda"].number_value()),
        vs_FieldCapacity(j["FieldCapacity"].number_value()),
        vs_Saturation(j["Saturation"].number_value()),
        vs_PermanentWiltingPoint(j["PermanentWiltingPoint"].number_value()),
        vs_SoilTexture(j["SoilTexture"].string_value()),
        vs_SoilAmmonium(j["SoilAmmonium"].number_value()),
        vs_SoilNitrate(j["SoilNitrate"].number_value()),
        _vs_SoilRawDensity(j["SoilRawDensity"].number_value()),
        _vs_SoilBulkDensity(j["SoilBulkDensity"].number_value()),
        _vs_SoilOrganicCarbon(j["SoilOrganicCarbon"].number_value()),
        _vs_SoilOrganicMatter(j["SoilOrganicMatter"].number_value())
    {}

    double vs_SoilRawDensity() const;
    void set_vs_SoilRawDensity(double srd);

    double vs_SoilBulkDensity() const;
    void set_vs_SoilBulkDensity(double sbd);

    double vs_SoilOrganicCarbon() const;
    void set_vs_SoilOrganicCarbon(double soc);

    double vs_SoilOrganicMatter() const;
    void set_vs_SoilOrganicMatter(double som);

    double vs_SoilSiltContent() const;

    std::string toString() const;

    double texture2lambda(double sand, double clay);

    bool isValid();

    json11::Json json() const
    {
      return json11::Json::object {
        {"type", "SoilParameters"},
        {"SoilSandContent", vs_SoilSandContent},
        {"SoilClayContent", vs_SoilClayContent},
        {"SoilpH", vs_SoilpH},
        {"SoilStoneContent", vs_SoilStoneContent},
        {"Lambda", vs_Lambda},
        {"FieldCapacity", vs_FieldCapacity},
        {"Saturation", vs_Saturation},
        {"PermanentWiltingPoint", vs_PermanentWiltingPoint},
        {"SoilTexture", vs_SoilTexture},
        {"SoilAmmonium", vs_SoilAmmonium},
        {"SoilNitrate", vs_SoilNitrate},
        {"SoilRawDensity", _vs_SoilRawDensity},
        {"SoilBulkDensity", _vs_SoilBulkDensity},
        {"SoilOrganicCarbon", _vs_SoilOrganicCarbon},
        {"SoilOrganicMatter", _vs_SoilOrganicMatter}};
    }

    json11::Json to_json() const { return json(); }

    // members
    double vs_SoilSandContent{0.4};
    double vs_SoilClayContent{0.05};
    double vs_SoilpH{6.9};
    double vs_SoilStoneContent{0.0};
    double vs_Lambda{0.0};
    double vs_FieldCapacity{0.0};
    double vs_Saturation{0.0};
    double vs_PermanentWiltingPoint{0.0};
    std::string vs_SoilTexture;
    double vs_SoilAmmonium{-1.0};
    double vs_SoilNitrate{-1.0};

  private:
    double _vs_SoilRawDensity{-1.0};
    double _vs_SoilBulkDensity{-1.0};
    double _vs_SoilOrganicCarbon{-1.0};
    double _vs_SoilOrganicMatter{-1.0};
  };

  /**
   * Data structure that holds information about capillary rise rates.
   */
  class CapillaryRiseRates
  {
  public:
    CapillaryRiseRates() {}
    ~CapillaryRiseRates() {}

    //Adds a capillary rise rate to data structure.
    void addRate(std::string bodart, int distance, double value);

    //Returns capillary rise rate for given soil type and distance to ground water.
    double getRate(std::string bodart, int distance) const;

    std::map<int,double> getMap(std::string bodart) const;

    //Returns number of elements of internal map data structure.
    size_t size() const { return cap_rates_map.size(); }

  private:
    std::map<std::string, std::map<int, double>> cap_rates_map;
  };

  const CapillaryRiseRates& readCapillaryRiseRates();

  typedef std::vector<SoilParameters> SoilPMs;
  typedef std::shared_ptr<SoilPMs> SoilPMsPtr;

  const SoilPMs* soilParameters(const std::string& abstractDbSchema,
                                int profileId,
                                int layerThicknessCm,
                                int maxDepthCm,
                                bool loadSingleParameter = false);

  std::string soilProfileId2KA5Layers(const std::string& abstractDbSchema,
                                      int soilProfileId);

  const SoilPMs* soilParametersFromHermesFile(int soilId,
                                              const std::string& pathToFile,
                                              int layerThicknessCm,
                                              int maxDepthCm,
                                              double soil_ph = -1.0,
                                              double drainage_coeff=-1.0);

  struct RPSCDRes
  {
    RPSCDRes() : sat(0), fc(0), pwp(0), initialized(false) {}
    RPSCDRes(bool initialized) : sat(0), fc(0), pwp(0), initialized(initialized) {}
    double sat, fc, pwp;
    bool initialized;
  };
  RPSCDRes readPrincipalSoilCharacteristicData(std::string soilType,
                                               double rawDensity);
  RPSCDRes readSoilCharacteristicModifier(std::string soilType,
                                          double organicMatter);

  void soilCharacteristicsKA5(SoilParameters&);
}

#endif
