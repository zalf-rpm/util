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

#ifndef GRID_TYPES_H_
#define GRID_TYPES_H_

#include "boost/shared_ptr.hpp"

#include <map>
#include <string>
#include <vector>

namespace Grids
{
	struct GridProxy;

	typedef boost::shared_ptr<GridProxy> GridProxyPtr;

	//! year 2 gridproxy -> to lazily load the grids
	typedef std::map<int, GridProxyPtr> Year2ProxyMap;

	typedef std::map<std::string, Year2ProxyMap> Scenario2RegValues;

	typedef std::map<std::string, Scenario2RegValues> Simulation2RegValues;

	//! regionalized data 2 the proxy map
	typedef std::map<std::string, Simulation2RegValues> RegDataMap;

	//! region 2 the regionalized data for that region
	typedef std::map<std::string, RegDataMap> Region2RegData;

	//! list of the ids of a group member
	typedef std::vector<std::string> GroupMemberIds;

	//! map from group name to the pair
	typedef std::map<std::string, GroupMemberIds> Groups2Members;

	struct RegData
	{
		RegData() : value(NULL) { }
		const Simulation2RegValues* value;
		std::string dataId;
		std::string dataName;
	};

	struct RegGroupOfData
	{
		std::vector<RegData> values;
		std::string groupId;
		std::string groupName;
	};
}

#endif
