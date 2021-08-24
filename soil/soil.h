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

#ifndef SOIL_SOIL_H_
#define SOIL_SOIL_H_

#include <memory>

#include <string>
#include <vector>
#include <map>
#include <functional>
#include <iostream>

#ifdef CAPNPROTO_SERIALIZATION_SUPPORT
#include "models/monica/monica_params.capnp.h"
#endif

#include "json11/json11.hpp"
#include "json11/json11-helper.h"

namespace Soil
{
	//! @author Claas Nendel, Michael Berg 
	struct SoilParameters : public Tools::Json11Serializable
	{
		SoilParameters() {}

		SoilParameters(json11::Json object);

#ifdef CAPNPROTO_SERIALIZATION_SUPPORT
		void serialize(mas::models::monica::SoilParameters::Builder builder) const;

		void deserialize(mas::models::monica::SoilParameters::Reader reader);
#endif

		virtual Tools::Errors merge(json11::Json j);

		virtual json11::Json to_json() const;

		//! Soil layer's silt content [kg kg-1] (Schluff)
		double vs_SoilSiltContent() const
		{
			return (1.0 - vs_SoilSandContent - vs_SoilClayContent);
		}

		double vs_SoilRawDensity() const;
		void set_vs_SoilRawDensity(double srd) { _vs_SoilRawDensity = srd; }

		double vs_SoilBulkDensity() const;
		void set_vs_SoilBulkDensity(double sbd) { _vs_SoilBulkDensity = sbd; }

		//! returns soc [% [0-1]]
		double vs_SoilOrganicCarbon() const; //!< Soil layer's organic carbon content [kg C kg-1]
		//! soc [% [0-1]]
		void set_vs_SoilOrganicCarbon(double soc) { _vs_SoilOrganicCarbon = soc; }

		//!< Soil layer's organic matter content [kg OM kg-1]
		double vs_SoilOrganicMatter() const;
		void set_vs_SoilOrganicMatter(double som) { _vs_SoilOrganicMatter = som; }

		double sandAndClay2lambda(double sand, double clay);

		bool isValid();

		// members
		double vs_SoilSandContent{-1.0}; //!< Soil layer's sand content [kg kg-1] //{0.4}
		double vs_SoilClayContent{-1.0}; //!< Soil layer's clay content [kg kg-1] (Ton) //{0.05}
		double vs_SoilpH{6.9}; //!< Soil pH value [] //{7.0}
		double vs_SoilStoneContent{0.0}; //!< Soil layer's stone content in soil [m3 m-3]
		double vs_Lambda{-1.0}; //!< Soil water conductivity coefficient [] //{0.5}
		double vs_FieldCapacity{-1.0}; //{0.21} //!< [m3 m-3]
		double vs_Saturation{-1.0}; //{0.43} //!< [m3 m-3]
		double vs_PermanentWiltingPoint{-1.0}; //{0.08} //!< [m3 m-3]
		std::string vs_SoilTexture;
		double vs_SoilAmmonium{0.0005}; //!< soil ammonium content [kg NH4-N m-3]
		double vs_SoilNitrate{0.005}; //!< soil nitrate content [kg NO3-N m-3]
		double vs_Soil_CN_Ratio{10.0};
		double vs_SoilMoisturePercentFC{100.0};

	private:
		double _vs_SoilRawDensity{-1.0}; //!< [kg m-3]
		double _vs_SoilBulkDensity{-1.0}; //!< [kg m-3]
		double _vs_SoilOrganicCarbon{-1.0}; //!< [kg kg-1]
		double _vs_SoilOrganicMatter{-1.0}; //!< [kg kg-1]
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
		void addRate(std::string bodart, size_t distance, double value);

		//Returns capillary rise rate for given soil type and distance to ground water.
		double getRate(std::string bodart, size_t distance) const;

		std::map<size_t, double> getMap(std::string bodart) const;

		//Returns number of elements of internal map data structure.
		size_t size() const { return cap_rates_map.size(); }

	private:
		std::map<std::string, std::map<size_t, double>> cap_rates_map;
	};

	const CapillaryRiseRates& readCapillaryRiseRates();

	typedef std::vector<SoilParameters> SoilPMs;
	typedef std::shared_ptr<SoilPMs> SoilPMsPtr;

	std::pair<SoilPMs, Tools::Errors> createSoilPMs(const Tools::J11Array& jsonSoilPMs);

	//! creates a concatenated string of the KA5 soil-textures making up the soil-profile with the given id
	//std::string soilProfileId2KA5Layers(const std::string& abstractDbSchema,
	//																		int soilProfileId);

	const SoilPMsPtr soilParametersFromHermesFile(int soilId,
																								const std::string& pathToFile,
																								int layerThicknessCm,
																								int maxDepthCm,
																								double soil_ph = -1.0,
																								double drainage_coeff = -1.0);

	struct RPSCDRes
	{
		RPSCDRes() {}
		RPSCDRes(bool initialized) : initialized(initialized) {}
		double sat{ 0.0 }; // [m3 m-3]
		double fc{ 0.0 };  // [m3 m-3]
		double pwp{ 0.0 }; // [m3 m-3]
		bool initialized{ false };
	};
	RPSCDRes readPrincipalSoilCharacteristicData(std::string KA5TextureClass,
																							 double rawDensity);
	RPSCDRes readSoilCharacteristicModifier(std::string KA5TextureClass,
																					double organicMatter);

	void soilCharacteristicsKA5(SoilParameters&);

	struct FcSatPwp
	{
		double fc{0.0};
		double sat{0.0};
		double pwp{0.0};
	};
	FcSatPwp fcSatPwpFromKA5textureClass(std::string KA5textureClass,
																			 double soilStoneContent,
																			 double soilRawDensity,
																			 double soilOrganicMatter);

	FcSatPwp fcSatPwpFromVanGenuchten(double sandContent,
																		double clayContent,
																		double stoneContent,
																		double soilBulkDensity,
																		double soilOrganicCarbon);
}

#endif
