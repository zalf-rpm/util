/**
Authors: 
Michael Berg <michael.berg@zalf.de>

Maintainers: 
Currently maintained by the authors.

This file is part of the util library used by models created at the Institute of 
Landscape Systems Analysis at the ZALF.
<one line to give the program's name and a brief idea of what it does.>
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

#ifndef READINI_H_
#define READINI_H_

#include <string>
#include <map>
#include <cstdlib>
#include <memory>

#include "date.h"

namespace Util
{
	typedef std::map<std::string, std::string> Names2Values;
	typedef std::map<std::string, Names2Values> Sections2NVMaps;
	/*!
	 * don't use this class polymorphically, because
	 * the std:: containers don't implement virtual destructors
	 * and if used like that there might be memory leaks
	 * (in this case the two member variables won't be deleted)
	 */
	class IniParameterMap : public Sections2NVMaps
	{
	public:
		IniParameterMap() {}
		//! Create and init map from given values file.
		IniParameterMap(const std::string& pathToIniFile);

		//! Return plain std::string value.
		std::string value(const std::string& section, const std::string& key,
		                  const std::string& def = std::string()) const;

    //! return a whole section
    const Names2Values& values(const std::string& section) const;

		//! Same as value(const std::string&), but for interface consistency.
		std::string valueAsString(const std::string& section,
		                          const std::string& key,
															const std::string& def = std::string()) const
		{
			return value(section, key, def);
		}

		//! Return value as integer.
		int valueAsInt(const std::string& section, const std::string& key,
		               int def = 0) const;

		//! Return value as double.
		double valueAsDouble(const std::string& section, const std::string& key,
		                     double def = 0.0) const;

		//! Return value as bool.
		bool valueAsBool(const std::string& section, const std::string& key,
		                 bool def = false) const;

    Date valueAsRelativeDate(const std::string& section, const std::string& key,
                             Date def = Date()) const;

    Date valueAsDate(const std::string& section, const std::string& key,
                     Date def = Date()) const;

		//! Return path to values file used.
		std::string pathToIniFile() const { return _pathToIniFile; }

	private: //methods
		//! Worker method, to parse the values file.
		void parseIniFile();

		//! Helper method to parse the values file and insert pairs into map.
		void parseAndInsert(const std::string& line);

		//! Helper method to trim parsed results and cut whitespaces.
		//std::string trim(const std::string& s,
		//                 const std::string& whitespaces = " \t\f\v\n\r");

	private: //state
		std::string _pathToIniFile; //!< the path to the values file
		std::string _currentSection;
	};

}

#endif
