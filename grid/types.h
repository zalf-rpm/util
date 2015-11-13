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

#ifndef GRID_TYPES_H_
#define GRID_TYPES_H_

#include <map>
#include <string>
#include <vector>
#include <memory>

namespace Grids
{
	struct GridProxy;

  typedef std::shared_ptr<GridProxy> GridProxyPtr;

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
