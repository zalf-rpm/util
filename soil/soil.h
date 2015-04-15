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

#ifndef SOIL_H_
#define SOIL_H_

#include <memory>

#include <string>
#include <vector>
#include <map>
#include <functional>
#include <iostream>

namespace Soil
{
  /**
   * @author Claas Nendel, Michael Berg
   */
  struct SoilParameters
  {
    SoilParameters();

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

    // members
    double vs_SoilSandContent;
    double vs_SoilClayContent;
    double vs_SoilpH;
    double vs_SoilStoneContent;
    double vs_Lambda;
    double vs_FieldCapacity;
    double vs_Saturation;
    double vs_PermanentWiltingPoint;
    std::string vs_SoilTexture;
    double vs_SoilAmmonium;
    double vs_SoilNitrate;

  private:
    double _vs_SoilRawDensity;
    double _vs_SoilBulkDensity;
    double _vs_SoilOrganicCarbon;
    double _vs_SoilOrganicMatter;
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
    int size() const { return cap_rates_map.size(); }

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
