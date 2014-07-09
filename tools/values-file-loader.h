#ifndef VALUESFILELOADER_H_
#define VALUESFILELOADER_H_

#include <string>
#include <map>
#include <cstdlib>
#include <iostream>

//#include "../import-export.h"

namespace Models {

	//! Data for a model, parsed from a values file.
	/*!
	 * Is actually a Map which is after being initialized with
	 * a values file, being filed with name/mappings.
	 *
	 * The map is just a std::string->std::string map, thus the user
	 * has to cast/parse to the right type or use one of the
	 * convenience methods give.
	 */
  class /*LC_DLL*/ ValuesFileParameterMap : public std::map<std::string, std::string> {
	public:
		//! Create and init map from given values file.
		ValuesFileParameterMap(const std::string& pathToValuesFile);

		template<typename T>
		T valueAs(const std::string& key, T def = makeDefault<T>()) const {
			std::map<std::string, std::string>::const_iterator ci = find(key);
			return ci == end() ? def : parseValue<T>(ci->second);
		}

		std::string value(const std::string& key,
		                  const std::string& def = std::string()) const {
			return valueAs(key, def);
		}

		//! Return path to values file used.
		std::string pathToValuesFile() const {
			return _pathToValuesFile;
		}

	private: //methods
		//! Worker method, to parse the values file.
		void parseValuesFile();

		//! Helper method to parse the values file and insert pairs into map.
		void parseAndInsert(const std::string& line);

		//! Helper method to trim parsed results and cut whitespaces.
		std::string trim(const std::string& s,
		                 const std::string& whitespaces = " \t\f\v\n\r");

		template<typename T>
		T makeDefault() const;

		template<typename T>
		T parseValue(const std::string& value) const;

	private: //state
		std::string _pathToValuesFile; //!< the path to the values file
	};

	template<>
	inline int ValuesFileParameterMap::makeDefault<int>() const { return 0; }

	template<>
	inline double ValuesFileParameterMap::makeDefault<double>() const {
		return 0.0;
	}

	template<>
	inline bool ValuesFileParameterMap::makeDefault<bool>() const {
		return false;
	}

	template<typename T>
	inline T ValuesFileParameterMap::makeDefault() const { return T(); }


	template<>
	inline int ValuesFileParameterMap::
	parseValue<int>(const std::string& value) const {
		return std::atoi(value.c_str());
	}

	template<>
	inline double ValuesFileParameterMap::
	parseValue<double>(const std::string& value) const {
		return std::atof(value.c_str());
	}

	template<>
	inline bool ValuesFileParameterMap::
	parseValue<bool>(const std::string& value) const {
		return value == "true";
	}

	template<>
	inline std::string ValuesFileParameterMap::
	parseValue<std::string>(const std::string& value) const {
		return value;
	}

}

#endif
