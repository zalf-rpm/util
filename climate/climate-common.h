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

#ifndef CLIMATE_COMMON_H_
#define CLIMATE_COMMON_H_

#include <vector>
#include <map>
#include <string>
#include <memory>

#include "json11/json11.hpp"

#include "tools/date.h"
#include "tools/json11-helper.h"
#include "tools/helper.h"

//! All climate data related classes.
namespace Climate
{
  /*!
	 * All abstract climate elements which are currently supported by the
	 * Climate classes. A user of the classes has to specify its requested
	 * climate elements in terms of these enumeration values. Usually
	 * with single elements or a list (vector) of elements.
	 *
	 * Attention: skip should always be the last element in this list as the index of skipped
	 * is used for the calculation of the ACDSize. 
	 */
  enum AvailableClimateData
  {
		day = 0, 
		month, 
		year, 
		tmin, 
		tavg, 
		tmax, 
		precip, 
		precipOrig, 
		globrad, 
		wind,
		sunhours, 
		cloudamount, 
		relhumid, 
		airpress, 
		vaporpress, 
		isoDate, 
		deDate, 
		co2,
		o3,
		et0,
		skip		
	};

	//! just a shortcut to the quite long name
	typedef AvailableClimateData ACD;

	inline std::map<std::string, ACD> name2acd()
	{
		return 
		{{"day", day}
		,{"month", month}
		,{"year", year}
		,{"tmin", tmin}
		,{"tavg", tavg}
		,{"tmax", tmax}
		,{"precip", precip}
		,{"precip-orig", precipOrig}
		,{"globrad", globrad}
		,{"wind", wind}
		,{"windspeed", wind}
		,{"sunhours", sunhours}
		,{"cloudamount", cloudamount}
		,{"relhumid", relhumid}
		,{"airpress", airpress}
		,{"vaporpress", vaporpress}
		,{"co2", co2}
		,{"o3", o3}
		,{"iso-date", isoDate}
		,{"de-date", deDate}
		,{"skip", skip}
		,{"et0", et0}
		};
	};



	/*!
	 * helper function to calculate the current size of the available climate
	 * data enumeration
	 * (this in instead of adding a dummy AvailableClimateDataSize element as
	 * last element -> to not pollute the enum)
	 * @return number of elements in the AvailableClimateData enumeration
	 */
	inline unsigned int availableClimateDataSize() { return int(skip) + 1; }

	//! just a shortcut for a list (vector) of climate data elements
	typedef std::vector<AvailableClimateData> ACDV;

	/*!
	 * helper function
	 * @param col availalbe climate data element
	 * @return column name of climate data element in CLM database
	 */
	std::string availableClimateData2CLMDBColName(AvailableClimateData col);

	/*!
	 * helper function
	 * @param col available climate data element
	 * @return column name of climate data element in Werex database
	 */
	std::string availableClimateData2WerexColName(AvailableClimateData col);

	/*!
	 * helper function
	 * @param col available climate data element
	 * @return column name of climate data element in WettReg database
	 */
	std::string availableClimateData2WettRegDBColName(AvailableClimateData col);

	std::pair<std::string, int>
	availableClimateData2CarbiocialDBColNameAndScaleFactor(AvailableClimateData col);

  std::string
  availableClimateData2UserSqliteDBColNameAndScaleFactor(AvailableClimateData col);

	/*!
	 * helper function
	 * @param col available climate data element
	 * @return column name of climate data element in Star database
	 */
  inline std::string availableClimateData2StarDBColName(AvailableClimateData col)
  {
		return availableClimateData2WettRegDBColName(col);
	}

	/*!
	 * helper function and just a different name for the WettReg mapping right now
	 * is here to have some generic name mapping, even though this might be just
	 * useable for display to the user
	 * @param col available climate data element
	 * @return column name of climate data element in default db
	 */
  inline std::string availableClimateData2DBColName(AvailableClimateData col)
  {
		return availableClimateData2WettRegDBColName(col);
	}

	std::string availableClimateData2Name(AvailableClimateData col);

	std::string availableClimateData2unit(AvailableClimateData col);

	//----------------------------------------------------------------------------

  struct YearRange
  {
    YearRange() : fromYear(0), toYear(0) {}
    YearRange(int f, int t) : fromYear(f), toYear(t) {}
    int fromYear, toYear;
    bool isValid() const { return fromYear > 0 && toYear > 0; }
  };

  YearRange snapToRaster(YearRange yr, int raster = 5);

  //----------------------------------------------------------------------------
  
  //! deep copied access to a range of climate data
  class DataAccessor : public Tools::Json11Serializable
  {
	public:
		DataAccessor();

		DataAccessor(const Tools::Date& startDate, 
								 const Tools::Date& endDate);

		DataAccessor(const DataAccessor& other);

		DataAccessor(json11::Json object);

		virtual Tools::Errors merge(json11::Json j);

		virtual json11::Json to_json() const;

		~DataAccessor(){}

    bool isValid() const { return noOfStepsPossible() > 0; }

    double dataForTimestep(AvailableClimateData acd, 
													 std::size_t stepNo,
													 double default_ = 0.0) const;

		Tools::Maybe<double> dataForTimestepM(AvailableClimateData acd,
																					std::size_t stepNo) const;

		std::map<Climate::ACD, double> allDataForStep(size_t stepNo,
																									double latitude) const;

		std::vector<double> dataAsVector(AvailableClimateData acd) const;

    DataAccessor cloneForRange(std::size_t fromStep,
                               std::size_t numberOfSteps) const;

    std::size_t noOfStepsPossible() const { return _numberOfSteps; }

		Tools::Date startDate() const { return _startDate; }

		Tools::Date endDate() const { return _endDate; }

    Tools::Date dateForStep(std::size_t stepNo) const { return _startDate + int(stepNo); }

    unsigned int julianDayForStep(std::size_t stepNo) const { return dateForStep(stepNo).julianDay(); }

		void addClimateData(AvailableClimateData acd,
		                    const std::vector<double>& data);

		void prependOrAppendClimateData(DataAccessor other,
																		bool replaceOverlappingData = true);

    void addOrReplaceClimateData(AvailableClimateData acd,
                                 const std::vector<double>& data);

    bool hasAvailableClimateData(AvailableClimateData acd) const
    {
      return _acd2dataIndex[acd] >= 0;
    }

	private: //state
		Tools::Date _startDate;
		Tools::Date _endDate;

    typedef std::vector<std::vector<double>> VVD;
    std::shared_ptr<VVD> _data;

		//! offsets to actual available climate data enum numbers
		std::vector<short> _acd2dataIndex;

    std::size_t _fromStep;
    std::size_t _numberOfSteps;
	};


	inline double potentialEvaporationTW(double globRad_Jpcm2,
																			 double tavg, double fk = 1)
	{
		return (globRad_Jpcm2 + 93 * fk)*(tavg + 22) / (150 * (tavg + 123));
	}

	inline double climaticWaterBalanceTW(double precip_mm, double globRad_Jpcm2,
																			 double tavg, double fk = 1)
	{
		return precip_mm - potentialEvaporationTW(globRad_Jpcm2, tavg, fk);
	}

}

#endif
