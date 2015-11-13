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

#include <fstream>
#include <sstream>
#include <cstddef>
#include <cstdio>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <cmath>
#include <cstdlib>
#include <cassert>
#include <set>
#include <mutex>
#include <limits>

#include "grid-manager.h"
#include "tools/algorithms.h"
#include "db/abstract-db-connections.h"
#include "tools/read-ini.h"
#include "tools/helper.h"
#include "grid+.h"

using namespace Grids;
using namespace std;
using namespace Tools;

VirtualGrid::~VirtualGrid()
{
  for_each(_availableGrids.begin(), _availableGrids.end(), [](GridP* g){ delete g; });
}

vector<const GridP*> VirtualGrid::availableGrids()
{
	vector<const GridP*> ags(_availableGrids.size());
	std::transform(_availableGrids.begin(), _availableGrids.end(),
                 ags.begin(), [](GridP* g){return g;});
	return ags;
}

string VirtualGrid::toShortDescription() const
{
	ostringstream s;
	s << "Nutzer-Region [" << cols() << "x" << rows() << "], ZG:" << cellSize();
	return s.str();
}

bool VirtualGrid::areGridDatasetsAvailable(const list<string>& gdsns) //const
{
	set<string> given;
	std::transform(gdsns.begin(), gdsns.end(), inserter(given, given.end()),
                 [](const string& s){return s;});

	set<string> all;
  const vector<const GridP*>& ags = availableGrids();
  std::transform(ags.begin(), ags.end(), inserter(all, all.end()),
                 [](const GridP* g){ return g->datasetName(); });

	return std::includes(all.begin(), all.end(), given.begin(), given.end());
}

map<string, const GridP*>
    VirtualGrid::gridsForDatasetNames(const list<string>& gdsns) //const
{
	set<string> given;
	std::transform(gdsns.begin(), gdsns.end(), inserter(given, given.end()),
                 [](const string& s){return s;});

	map<string, const GridP*> m;
  const vector<const GridP*>& ags = availableGrids();
  for(const GridP* ag : ags)
  {
    const string& s = ag->datasetName();
    if(given.find(s) != given.end())
      m.insert(make_pair(s, ag));
	}

	return m;
}

const LatLngPolygonsMatrix& VirtualGrid::latLngCellPolygons()
{
  if(_cellPolygons.isEmpty() && _cols > 0 && _rows > 0)
  {
		//_cellPolygons.resize((_cols-1)*(_rows-1));

		vector<RectCoord> rccs((_cols+1) * (_rows+1));
		//paint the grid cells
    for(unsigned int j = 0; j < _rows+1; j++)
    {
      for(unsigned int i = 0; i < _cols+1; i++)
      {
				double left = _rect.tl.r + (double(i) * _cellSize);
				double top = _rect.tl.h - (double(j) * _cellSize);
        rccs[(j*(_cols+1))+i] = RectCoord(coordinateSystem(), left, top);
				//cout << "(" << left << "," << top << ") ";
			}
		}
		//cout << endl;
		const vector<LatLngCoord>& llcs = RC2latLng(rccs);

		_cellPolygons.resize(_rows, _cols);
    for(unsigned int j = 0; j < _rows; j++)
    {
      for(unsigned int i = 0; i < _cols; i++)
      {
				_cellPolygons[j][i] =
				(LatLngPolygon(llcs.at((j*(_cols+1))+i),
				               llcs.at((j*(_cols+1))+i+1),
				               llcs.at(((j+1)*(_cols+1))+i+1),
				               llcs.at(((j+1)*(_cols+1))+i)));
			}
		}

	}

	return _cellPolygons;
}

//------------------------------------------------------------------------------

map<string, double> VirtualGrid2::dataAt(size_t row, size_t col) const
{
  map<string, double> res;

  for(Dsn2GridPPtr::value_type p : _dsn2grid)
  {
    res[p.first] = p.second->dataAt(row, col);
  }

  return res;
}

map<string, double> VirtualGrid2::dataAt(const RectCoord& rcc) const
{
  map<string, double> res;

  for(Dsn2GridPPtr::value_type p : _dsn2grid)
  {
    res[p.first] = p.second->dataAt(rcc);
  }

  return res;
}

vector<const GridP*> VirtualGrid2::availableGrids()
{
  vector<const GridP*> ags(_dsn2grid.size());
  std::transform(_dsn2grid.begin(), _dsn2grid.end(),
                 ags.begin(), [](pair<string, GridPPtr> p) { return p.second.get(); });
  return ags;
}

string VirtualGrid2::toShortDescription() const
{
  ostringstream s;
  s << "Nutzer-Region [" << cols() << "x" << rows() << "], ZG:" << cellSize();
  return s.str();
}

bool VirtualGrid2::areGridDatasetsAvailable(const list<string>& gdsns) //const
{
  set<string> given;
  std::transform(gdsns.begin(), gdsns.end(), inserter(given, given.end()),
                 [](string s){return s;});

  set<string> all;
  const vector<const GridP*>& ags = availableGrids();
  std::transform(ags.begin(), ags.end(), inserter(all, all.end()),
                 [](const GridP* g){return g->datasetName();});

  return std::includes(all.begin(), all.end(), given.begin(), given.end());
}

map<string, const GridP*>
VirtualGrid2::gridsForDatasetNames(const list<string>& gdsns) //const
{
  set<string> given;
  std::transform(gdsns.begin(), gdsns.end(), inserter(given, given.end()),
                 [](string s){return s;});

  map<string, const GridP*> m;
  const vector<const GridP*>& ags = availableGrids();
  for(const GridP* ag : ags)
  {
    const string& s = ag->datasetName();
    if(given.find(s) != given.end())
      m.insert(make_pair(s, ag));
  }

  return m;
}

const LatLngPolygonsMatrix& VirtualGrid2::latLngCellPolygons()
{
  if(_cellPolygons.isEmpty() && _cols > 0 && _rows > 0)
  {
    //_cellPolygons.resize((_cols-1)*(_rows-1));

    vector<RectCoord> rccs((_cols+1) * (_rows+1));
    //paint the grid cells
    for(unsigned int j = 0; j < _rows+1; j++)
    {
      for(unsigned int i = 0; i < _cols+1; i++)
      {
        double left = _rect.tl.r + (double(i) * _cellSize);
        double top = _rect.tl.h - (double(j) * _cellSize);
        rccs[(j*(_cols+1))+i] = RectCoord(coordinateSystem(), left, top);
        //cout << "(" << left << "," << top << ") ";
      }
    }
    //cout << endl;
    const vector<LatLngCoord>& llcs = RC2latLng(rccs);

    _cellPolygons.resize(_rows, _cols);
    for(unsigned int j = 0; j < _rows; j++)
    {
      for(unsigned int i = 0; i < _cols; i++)
      {
        _cellPolygons[j][i] =
        (LatLngPolygon(llcs.at((j*(_cols+1))+i),
                       llcs.at((j*(_cols+1))+i+1),
                       llcs.at(((j+1)*(_cols+1))+i+1),
                       llcs.at(((j+1)*(_cols+1))+i)));
      }
    }

  }

  return _cellPolygons;
}


//------------------------------------------------------------------------------

NoVirtualGrid::NoVirtualGrid(CoordinateSystem cs,
                             const std::vector<GridProxyPtr>* gps,
                             const Grids::RCRect& rect,
                             double cellSize, size_t rows,
                             size_t cols,
                             int noDataValue)
	: VirtualGrid(cs, rect, cellSize, rows, cols, noDataValue),
		_gps(gps)
{
}

vector<const GridP*> NoVirtualGrid::availableGrids()
{
  if(_availableGrids.empty())
  {
    for(GridProxyPtr gp : *_gps)
    {
			_availableGrids.push_back(gp->copyOfFullGrid());
		}
	}
	return VirtualGrid::availableGrids();//_availableGrids;
}

string NoVirtualGrid::toShortDescription() const
{
	ostringstream s;
	s << Tools::capitalize(_name) << " ZG:" << cellSize();
	//s << "Standard-Region [" << cols() << "x" << rows() << "], ZG:" << cellSize();
	return s.str();
}

vector<VirtualGrid::Data> NoVirtualGrid::dataAt(const RectCoord& rcc) const
{
	int ix = int(std::floor((rcc.r - _rect.tl.r) / _cellSize));
	if(ix == int(_cols)) ix--;
	int iy = _rows - int(std::floor((rcc.h - _rect.br.h) / _cellSize)) - 1;
	if(iy < 0) iy = 0;

	return dataAt(iy, ix);
}


//------------------------------------------------------------------------------

NoVirtualGrid2::NoVirtualGrid2(CoordinateSystem cs,
                             const std::vector<GridProxyPtr>* gps,
                             const Grids::RCRect& rect,
                             double cellSize, unsigned int rows,
                             unsigned int cols,
                             int noDataValue)
  : VirtualGrid2(cs, rect, cellSize, rows, cols, map<string, GridPPtr>(), noDataValue),
    _gps(gps)
{
}

vector<const GridP*> NoVirtualGrid2::availableGrids()
{
  if(_dsn2grid.empty())
  {
    for(GridProxyPtr gp : *_gps)
    {
      _dsn2grid[gp->datasetName] = GridPPtr(gp->copyOfFullGrid());
    }
  }
  return VirtualGrid2::availableGrids();//_availableGrids;
}

string NoVirtualGrid2::toShortDescription() const
{
  ostringstream s;
  s << Tools::capitalize(_name) << " ZG:" << cellSize();
  //s << "Standard-Region [" << cols() << "x" << rows() << "], ZG:" << cellSize();
  return s.str();
}

//------------------------------------------------------------------------------

RealVirtualGrid::RealVirtualGrid(CoordinateSystem cs, const RCRect& rect,
																 double cellSize,
                                 unsigned int rows, unsigned int cols,
																 const VVGP& proxies, int noDataValue)
	: VirtualGrid(cs, rect, cellSize, rows, cols, noDataValue),
		_data(rows, cols),
		_usedGridProxies(proxies.begin(), proxies.end())
{
}

RealVirtualGrid::~RealVirtualGrid()
{
	for_each(_usedGridProxies.begin(), _usedGridProxies.end(),
           [](vector<GridProxyPtr>* gs){ delete gs; });
}

vector<RealVirtualGrid::Data>
RealVirtualGrid::dataAt(unsigned int row, unsigned int col) const
{
	const vector<Data>& v = _data.valueAt(row, col);
	return v.empty() ? vector<Data>(1, Data()) : v;
}

vector<const GridP*> RealVirtualGrid::availableGrids()
{
  if(_availableGrids.empty())
  {
		map<string, GridP*> dsn2g;

    for(unsigned int i = 0; i < rows(); i++)
    {
      for(unsigned int j = 0; j < cols(); j++)
      {
        for(Data d : dataAt(i, j))
        {
          if(!d.isNoData())
          {
            for(GridProxyPtr gp : *d.gridProxies())
            {
              //target grid
              GridP* tg = valueD(dsn2g, gp->datasetName,
                                 static_cast<GridP*>(NULL));
              if(!tg)
              {
								RectCoord bl = rcRect().toTlTrBrBlVector().at(3);
                tg = new GridP(gp->datasetName, rows(), cols(), cellSize(),
															 bl.r, bl.h, noDataValue(),
															 gp->coordinateSystem);
                dsn2g[gp->datasetName] = tg;
								_availableGrids.push_back(tg);
              }

              tg->setDataAt(i, j, gp->gridPtr()->dataAt(d.row(), d.col()));
						}
					}
				}
			}
		}
	}

	return VirtualGrid::availableGrids();
}

//------------------------------------------------------------------------------

GridManager::GridManager(Env env)
: _env(env)
{
	init("");

	readRegionalizedData();
}

GridManager::~GridManager(){}

void GridManager::init(const Path& userSubPath)
{
	//cout << "entering GridManager::init(" << userSubPath << ")" << endl;

	DIR* dp;
	struct dirent* ep;

	string pathToGrids = _env.asciiGridsPath +
		(userSubPath.empty() ? "" : "/" + userSubPath);
	
	dp = opendir(pathToGrids.c_str());
	if(!dp)
		return;

	//bool gridsInDir = false;
  while((ep = readdir(dp)))
  {
		string dname(ep->d_name);

		//if there are any ascii grids in the top level folder run
		//process there too
		//if(dname.size() > 4 && dname.rfind(".asc") == dname.length() - 4){
		//	gridsInDir = true;
		//} else
		if(dname[0] != '.' && (dname.size() <= 4 ||
													 dname.rfind(".asc") != dname.length() - 4))
		{
			//filter out files starting with . and all files ending in .asc
			//so we basically should get folders
			string folderName(ep->d_name);
			string pathToFolder =
				(userSubPath.empty() ? string("") : userSubPath + "/") + folderName;
			//cout << "diving down into " << pathToFolder << endl;
			init(pathToFolder);
		}
	}

	//cout << "reading grids in " << pathToGrids << endl;
	readGrid2HdfMappingFile(userSubPath);
	checkAndUpdateHdfStore(userSubPath);

	//cout << "leaving GridManager::init(" << userSubPath << ")" << endl;
}

/*
VirtualGrid* GridManager::
createVirtualGrid(const Quadruple<LatLngCoord>& llrect, double cellSize,
									const Path& userSubPath, CoordinateSystem tcs)
{
	const vector<RectCoord>& rcs =
			latLng2RC(asTlTrBrBl<vector<LatLngCoord> >(llrect), tcs);
//	cout << "llrect after conversion to rcs" << endl;
//	for_each(rcs.begin(), rcs.end(), [](RectCoord rc){ cout << rc.toString() << endl; });
	return createVirtualGrid(Quadruple<RectCoord>(rcs), cellSize, userSubPath);
}
*/

//VirtualGrid* GridManager::createVirtualGrid(const Quadruple<RectCoord>& rcpoly,
//                                            double cellSize,
//                                            const Path& userSubPath)
//{
//  Path2GPS::const_iterator ci = _gmdMap.find(userSubPath);
//  if(ci != _gmdMap.end())
//    return createVirtualGrid(ci->second, rcpoly, cellSize);
//  return NULL;
//}

VirtualGrid2* GridManager::createVirtualGrid(const Quadruple<LatLngCoord>& llrect,
                                            const Path& userSubPath)
{
	Path2GPS::const_iterator ci = _gmdMap.find(userSubPath);
	if(ci != _gmdMap.end())
    return createVirtualGrid(ci->second, llrect);//, cellSize);
	return NULL;
}

/*
VirtualGrid*
GridManager::createVirtualGrid(const GMD2GPS& gmd2gridProxies,
															 const Quadruple<RectCoord>& rcpoly,
															 double cellSize)
{
	CoordinateSystem usedCS = rcpoly.tl.coordinateSystem;

	//get bounding rect of rc polygon
	RectCoord tl(usedCS, min(rcpoly.tl.r, rcpoly.bl.r), max(rcpoly.tl.h, rcpoly.tr.h));
	RectCoord br(usedCS, max(rcpoly.tr.r, rcpoly.br.r), min(rcpoly.bl.h, rcpoly.br.h));
	RCRect boundingRect(tl, br);

	vector<GridMetaData> gmds;
	//filter all GridMetaData with correct cellsize and intersecting rect
  for(GMD2GPS::value_type p, gmd2gridProxies)
	{
		//cout << "checking gmd: " << it->first.toString() << endl;
		if(p.first.cellsize == cellSize
			&& p.first.rcRect().intersects(boundingRect))
			gmds.push_back(p.first);
	}

	//cout << "the filtered gmds" << endl;
	//for_each(gmds.begin(), gmds.end(),
	//         cout << boost::lambda::bind(&GridMetaData::toString, _1) << '\n');

	if(gmds.empty())
		return NULL;

	//take the first region of the filtered ones and use this one as
	//base for expanding the bounding rect to full cell-size bounds
	const GridMetaData& firstGmd = gmds.front();
	//cout << "choosen gmd: " << firstGmd.toString() << endl;

	//first find position in choosen grid metadata
	//might in a grid if top left corner of bounding rect is inside
	//the gmd or 0,0 (aka the top left of gmd) if the top left corner of
	//the bounding rect is outside the gmd
	const RCRect& intersectedRect = firstGmd.rcRect().intersected(boundingRect);
	//cout << "intersectedRect: " << intersectedRect.toString() << endl;
	const RectCoord& delta1 = intersectedRect.tl - firstGmd.topLeftCorner();
	int indexR = int(std::floor(delta1.r / cellSize));
	int indexH = int(std::floor(abs(delta1.h / cellSize)));
	//cout << "delta1: " << delta1.toString() << " indexR: " << indexR
	//<< " indexH: " << indexH << endl;
	//rc position into choosen grid-class
	RectCoord firstGmdTl(usedCS,
											 firstGmd.topLeftCorner().r + (indexR * cellSize),
											 firstGmd.topLeftCorner().h - (indexH * cellSize));
	//cout << "grid-class top left: " << firstGmdTl.toString() << endl;

	//now expand the top left corner of the bounding rect to the full
	//outer (hypethetical) grid-cell bound
	//this might be either the same as the previous calculated corner
	//of a grid cell in the gmd (or gmd's 0,0 is case of an exact match)
	//or the bounds have to be extended if the top left corner of bounding rect
	//(of the users selection) was originally outside of the gmd
	RectCoord delta2 = boundingRect.tl - firstGmdTl;
	int nocsToTlr = delta1.r > 0 ? 0 : int(std::ceil(abs(delta2.r / cellSize))); //no of cells
	int nocsToTlh = delta1.h < 0 ? 0 : int(std::ceil(abs(delta2.h / cellSize)));
	//cout << "delta2: " << delta2.toString() << " nocsToTlr: " << nocsToTlr
	//<< " nocsToTlh: " << nocsToTlh << endl;
	//extended tl
	RectCoord etl(usedCS,
								firstGmdTl.r - (nocsToTlr * cellSize),
								firstGmdTl.h + (nocsToTlh * cellSize));
	//cout << "extended top left: " << etl.toString() << endl;

	//adjust br to multiple of cellSize
	RectCoord delta3 = br - etl;
	int nocsR = int(std::ceil(abs(delta3.r / cellSize))); //no of cells
  if(nocsR == 0)
    nocsR++; //the selection is choosing at least one cell
	int nocsH = int(std::ceil(abs(delta3.h / cellSize)));
  if(nocsH == 0)
    nocsH++;
	//cout << "delta3: " << delta3.toString() << " nocsR: " << nocsR
	//<< " nocsH: " << nocsH << endl;
	//the extended final bounding rect
	RectCoord ebr(usedCS,
								etl.r + (nocsR * cellSize),
								etl.h - (nocsH * cellSize));
	RCRect extendedBoundingRect(etl, ebr);
	//cout << "extended bounding rect: " << extendedBoundingRect.toString() << endl;

	//for efficiency reasons every data element just references a vector
	//of gridproxies with available grids from a given region (same gridmetadata)
	//but these have to be unique, thus a number of vectors is created below
	//which will be stored in the according virtual grid which can
	//be referenced directly in the data elements
	map<GridMetaData, GridProxies*> gmd2gps;
	//get the unique set of all dataset names to be used below
	set<string> uniqueDatasetNames;
  for(const GridMetaData& gmd, gmds)
  {
		GridProxies* gps = new GridProxies;
		GMD2GPS::const_iterator ci = gmd2gridProxies.find(gmd);
		if(ci == gmd2gridProxies.end())
			continue;
		const GridProxies& agps = ci->second;
    for(GridProxyPtr agp, agps)
    {
      string s = agp->datasetName;
      if(uniqueDatasetNames.find(s) == uniqueDatasetNames.end())
      {
				uniqueDatasetNames.insert(s);
        gps->push_back(agp);
			}
		}

		gmd2gps.insert(make_pair(gmd, gps));

		//cout << "copied these proxies: " << endl;
		//for_each(gps->begin(), gps->end(),
		//         cout << boost::lambda::bind(&GridProxy::toString, _1) << "\n");
	}
	//for_each(uniqueDatasetNames.begin(), uniqueDatasetNames.end(),
	//         cout << _1 << '\n');
	vector<GridProxies*> gpss;
	transform(gmd2gps.begin(), gmd2gps.end(), back_inserter(gpss),
						[](pair<const GridMetaData, GridProxies*> v){ return v.second; }
						//boost::lambda::bind<GridProxies*>(&pair<const GridMetaData, GridProxies*>::second,
															 //_1)
																	 );

	RealVirtualGrid* vg = new RealVirtualGrid(usedCS, extendedBoundingRect,
																						cellSize,
	                                          nocsH, nocsR, gpss);

	//now we have to iterate through the virtual grid and fill its cells
	//either with no data values or with references to the potential grids
	//and the correct indices into it
  for(unsigned int i = 0; i < vg->rows(); i++)
  {
    for(unsigned int k = 0; k < vg->cols(); k++)
    {
			vector<GridMetaData>::iterator it = gmds.begin();// - 1;
			const RectCoord& c = vg->rcCoordAt(i, k);

			auto f = [&](const GridMetaData& gmd){ return gmd.rcRect().contains(c, true); };

			while((it = find_if(it, gmds.end(), f)) != gmds.end())
			{
				//else it vg is preinitialized with no data values
				const GridMetaData& gmd = *it;

				//this is just temporary as it won't work if a virtual grid
				//consists of data from more than one region, but better than
				//nothing until the whole virtual grid thing will be redone
				if(gmd.regionName == "uecker" ||
					 gmd.regionName == "sachsen" ||
					 gmd.regionName == "brazil-sinop" ||
					 gmd.regionName == "brazil-campo-verde")
					vg->setCustomId(gmd.regionName);

				pair<Row, Col> rc = rowColInGrid(gmd, c);
				//cout << "using gmd: " << gmd.toString() << endl;
				//cout << "( " << i << "," << k << ") indexR (col): " << rc.col
				//<< " indexH (row): " << rc.row << endl;

				//cout << "adding data at: " << i << "/" << k << endl;
        vg->addDataAt(i, k, VirtualGrid::Data(gmd2gps[gmd],
                                              rc.first, rc.second));

				it++;
			}
		}
	}

	return vg;
}
*/

VirtualGrid2*
GridManager::createVirtualGrid(const GMD2GPS& gmd2gridProxies,
                               const Quadruple<Tools::LatLngCoord>& llrect)
{
  //map<CoordinateSystem, RCRect> cs2boundingRect;
  typedef map<CoordinateSystem, vector<GridMetaData>> CS2GMDS;
  CS2GMDS cs2gmds;
  int minCellSize = 100000000;
  typedef map<CoordinateSystem, int> CS2NO;
  CS2NO cs2noOfGPS;
  //filter all GridMetaData with correct cellsize and intersecting rect
  for(GMD2GPS::value_type p : gmd2gridProxies)
  {
    auto cs = p.first.coordinateSystem;
//    auto ci = cs2boundingRect.find(cs);
//    RCRect boundingRect;
//    if(ci == cs2boundingRect.end())
//    {
      //first convert latlng to rect coordinates of gmd
      const vector<RectCoord>& rcs = latLng2RC(asTlTrBrBl<vector<LatLngCoord> >(llrect), cs);
      auto rcpoly = Quadruple<RectCoord>(rcs);

      //create and store a bounding rect in this rect coordinate system
      RectCoord tl(cs, min(rcpoly.tl.r, rcpoly.bl.r), max(rcpoly.tl.h, rcpoly.tr.h));
      RectCoord br(cs, max(rcpoly.tr.r, rcpoly.br.r), min(rcpoly.bl.h, rcpoly.br.h));
      RCRect boundingRect = RCRect(tl, br);
//      cs2boundingRect[cs] = boundingRect;
//    }
//    else
//      boundingRect = ci->second;

    //cout << "checking gmd: " << it->first.toString() << endl;
    if(p.first.rcRect().intersects(boundingRect))
    {
      cs2noOfGPS[cs] += p.second.size();
      minCellSize = min(p.first.cellsize, minCellSize);
      cs2gmds[cs].push_back(p.first);
    }
  }

  //find most often used coordinate system in the selected region
  //we assume that this coordinate system can safely be used in this region,
  //else there wouldn't be that many grids there
  CoordinateSystem usedCS = CoordinateSystem();
  int noOfGPS = 0;
  for(CS2NO::value_type p : cs2noOfGPS)
  {
    if(p.second > noOfGPS)
    {
      noOfGPS = p.second;
      usedCS = p.first;
    }
  }

  //if we didn't find any coordinate system we leave
  if(usedCS == CoordinateSystem())
    return NULL;

  //determine the dimensions of the bounding rect if the top left corner is exactly
  //the user choosen one and the bottom right corner is the one determined to include the user choosen one
  //given the determined cellsize
  const vector<RectCoord>& rcs = latLng2RC(asTlTrBrBl<vector<LatLngCoord> >(llrect), usedCS);
  auto rcpoly = Quadruple<RectCoord>(rcs);
  int noOfRows = int(std::ceil((rcpoly.tl.h - rcpoly.br.h)/minCellSize));
  int noOfCols = int(std::ceil((rcpoly.br.r - rcpoly.tl.r)/minCellSize));
  RectCoord tl(usedCS, min(rcpoly.tl.r, rcpoly.bl.r), max(rcpoly.tl.h, rcpoly.tr.h));
  RectCoord br(usedCS, tl.r + noOfCols*minCellSize, tl.h - noOfRows*minCellSize);

  map<string, GridPPtr> dsn2grid;
  set<string> uniqueDatasetNames;
  for(CS2GMDS::value_type p : cs2gmds)
  {
    for(const GridMetaData& gmd : p.second)
    {
      GMD2GPS::const_iterator ci = gmd2gridProxies.find(gmd);
      if(ci == gmd2gridProxies.end())
        continue;
      const GridProxies& agps = ci->second;
      for(GridProxyPtr agp : agps)
      {
//        cout << "dsn: |" << agp->datasetName << "|" << endl;
        uniqueDatasetNames.insert(agp->datasetName);
      }
    }
  }

  //create all the grids needed
  for(string dsn : uniqueDatasetNames)
  {
    dsn2grid[dsn] = GridPPtr(new GridP(dsn, noOfRows, noOfCols, minCellSize, tl.r, br.h, -9999, usedCS));
  }

  if(dsn2grid.empty())
    return NULL;

  string regionName;

  GridPPtr someGrid = dsn2grid.begin()->second;

  /*
  for(int r = 0, rs = someGrid->rows(); r < rs; r++)
  {
    for(int c = 0, cs = someGrid->cols(); c < cs; c++)
    {
      RectCoord cellCenter = someGrid->rcCoordAtCenter(r, c);
      double sgCellSize = someGrid->cellSize();

      LatLngCoord llcc = RC2latLng(cellCenter);

      for(CS2GMDS::value_type p, cs2gmds)
      {
        CoordinateSystem cs = p.first;
        RectCoord rccc = latLng2RC(llcc, cs);

        for(const GridMetaData& gmd, p.second)
        {
//          cout << "gmd: " << gmd.toCanonicalString() << " regionName: " << regionName << endl;
          regionName = gmd.regionName;
//          if(gmd.regionName == "uecker" ||
//             gmd.regionName == "sachsen" ||
//             gmd.regionName == "uelzen" ||
//             gmd.regionName == "brazil-sinop" ||
//             gmd.regionName == "brazil-campo-verde")
//            regionName = gmd.regionName;

          GMD2GPS::const_iterator ci = gmd2gridProxies.find(gmd);
          if(ci == gmd2gridProxies.end())
            continue;
          const GridProxies& agps = ci->second;
          if(agps.empty())
            continue;

          GridP* g = agps.front()->gridPtr();
          double gCellSize = g->cellSize();

          auto pos = g->rc2rowCol(rccc);

          //if any value lies farther outside than a cellwidth of the target grid, it can't intersect the closest
          //source cell thus we associate a no data value with the target grid value
          bool setNoDataValue =
              (pos.row < 0 && abs(pos.row*gCellSize) > sgCellSize) ||
              (pos.col < 0 && abs(pos.col*gCellSize) > sgCellSize) ||
              (pos.row > 0 && pos.isRowOutside && abs((pos.row - g->rows())*gCellSize > sgCellSize)) ||
              (pos.col > 0 && pos.isColOutside && abs((pos.col - g->cols())*gCellSize > sgCellSize));

          //if the position is in the cell outside, just associate the target grid value with the border value of
          //the source grid
          if(!setNoDataValue)
          {
            if(pos.row < 0)
              pos.row = 0;
            else if(pos.row > 0 && pos.isRowOutside)
              pos.row = g->rows() - 1;
            if(pos.col < 0)
              pos.col = 0;
            else if(pos.col > 0 && pos.isColOutside)
              pos.col = g->cols() - 1;
          }

          for(GridProxyPtr agp, agps)
          {
            GridPPtr tg = dsn2grid[agp->datasetName];
            if(setNoDataValue)
              tg->setNoDataValueAt(r,c);
            else
              tg->setDataAt(r, c, agp->gridPtr()->dataAt(pos.row, pos.col));
          }
        }
      }
    }
  }
  /*/

  size_t minRow = numeric_limits<size_t>::max();
  size_t maxRow = numeric_limits<size_t>::min();
  size_t minCol = numeric_limits<size_t>::max();
  size_t maxCol = numeric_limits<size_t>::min();

  //*
  for(CS2GMDS::value_type p : cs2gmds)
  {
    for(const GridMetaData& gmd : p.second)
    {
      //          cout << "gmd: " << gmd.toCanonicalString() << " regionName: " << regionName << endl;
      regionName = gmd.regionName;
      //          if(gmd.regionName == "uecker" ||
      //             gmd.regionName == "sachsen" ||
      //             gmd.regionName == "uelzen" ||
      //             gmd.regionName == "brazil-sinop" ||
      //             gmd.regionName == "brazil-campo-verde")
      //            regionName = gmd.regionName;

      GMD2GPS::const_iterator ci = gmd2gridProxies.find(gmd);
      if(ci == gmd2gridProxies.end())
        continue;
      const GridProxies& agps = ci->second;
      if(agps.empty())
        continue;

      GridP* g = agps.front()->gridPtr();
      double gCellSize = g->cellSize();

      for(size_t r = 0, rows = someGrid->rows(); r < rows; r++)
      {
        for(size_t c = 0, cols = someGrid->cols(); c < cols; c++)
        {
          RectCoord cellCenter = someGrid->rcCoordAtCenter(r, c);
          double sgCellSize = someGrid->cellSize();

          LatLngCoord llcc = RC2latLng(cellCenter);

          CoordinateSystem cs = p.first;
          RectCoord rccc = latLng2RC(llcc, cs);

          auto pos = g->rc2rowCol(rccc);

          //if any value lies farther outside than a cellwidth of the target grid, it can't intersect the closest
          //source cell thus we associate a no data value with the target grid value
          bool setNoDataValue =
              (pos.row < 0 && abs(pos.row*gCellSize) > sgCellSize) ||
              (pos.col < 0 && abs(pos.col*gCellSize) > sgCellSize) ||
              (pos.row > 0 && pos.isRowOutside && abs((pos.row - g->rows())*gCellSize > sgCellSize)) ||
              (pos.col > 0 && pos.isColOutside && abs((pos.col - g->cols())*gCellSize > sgCellSize));

          //if the position is in the cell outside, just associate the target grid value with the border value of
          //the source grid
          if(!setNoDataValue)
          {
            if(pos.row < 0)
              pos.row = 0;
            else if(pos.row > 0 && pos.isRowOutside)
              pos.row = g->rows() - 1;
            if(pos.col < 0)
              pos.col = 0;
            else if(pos.col > 0 && pos.isColOutside)
              pos.col = g->cols() - 1;
          }

          for(GridProxyPtr agp : agps)
          {
            GridPPtr tg = dsn2grid[agp->datasetName];
            if(setNoDataValue)
              tg->setNoDataValueAt(r,c);
            else
              tg->setDataAt(r, c, agp->gridPtr()->dataAt(pos.row, pos.col));

            minRow = min(minRow, pos.row);
            maxRow = max(maxRow, pos.row);
            minCol = min(minCol, pos.col);
            maxCol = max(maxCol, pos.col);
          }
        }
      }
    }
  }
  //*/

  cout << "regionName: " << regionName << " noOfRows: " << noOfRows << " noOfCols: " << noOfCols
       << " source [from, to] row: [" << minRow << ", " << maxRow << "] "
       << " source [from, to] col: [" << minCol << ", " << maxCol << "]" << endl;

  auto vg = new VirtualGrid2(usedCS, RCRect(tl, br), minCellSize, noOfRows, noOfCols, dsn2grid);
  vg->setRegionName(regionName);

  return vg;
}

VirtualGrid2* GridManager::virtualGridForGridMetaData(const GridMetaData& gmd,
                                        const Path& userSubPath)
{
	Path2GPS::const_iterator ci = _gmdMap.find(userSubPath);
  if(ci != _gmdMap.end())
  {
		GMD2GPS::const_iterator ci2 = ci->second.find(gmd);
    if(ci2 != ci->second.end())
    {
      NoVirtualGrid2* nvg =
          new NoVirtualGrid2(gmd.coordinateSystem,
														&(ci2->second),
														gmd.rcRect(), gmd.cellsize,
                            gmd.nrows, gmd.ncols);
			//cout << "gmd.regionName: " << gmd.regionName << endl;
			nvg->setName(gmd.regionName);
      nvg->setRegionName(gmd.regionName);
			return nvg;
		}
	}

	return NULL;
}

VirtualGrid2* GridManager::virtualGridForRegionName(const string& regionName,
																									 const Path& userSubPath) {
  for(const GridMetaData& gmd : regionGmds(userSubPath)) {
		if(gmd.regionName == regionName)
			return virtualGridForGridMetaData(gmd, userSubPath);
	}
	return NULL;
}

GridPPtr GridManager::gridFor(const string& regionName,
                              const string& datasetName,
                              const Path& userSubPath, int cellSize,
                              GridMetaData subgridMetaData)
{
  static mutex lockable;
  lock_guard<mutex> lock(lockable);

  for(const GridMetaData& gmd : regionGmds(userSubPath))
  {
    if(gmd.regionName == regionName && gmd.cellsize == cellSize)
    {
			Path2GPS::const_iterator ci = _gmdMap.find(userSubPath);
      if(ci != _gmdMap.end())
      {
				GMD2GPS::const_iterator ci2 = ci->second.find(gmd);
        if(ci2 != ci->second.end())
        {
					const GridProxies& gps = ci2->second;
          for(GridProxyPtr gp : gps)
          {
						if(gp->datasetName == datasetName)
              return createSubgrid(gp->gridPPtr(), subgridMetaData);
					}
				}
			}
			return GridPPtr();
		}
	}

	return GridPPtr();
}

GridPPtr GridManager::createSubgrid(GridPPtr g, GridMetaData subgridMetaData,
                                    bool alwaysClone)
{
	GridPPtr res = g;
	GridMetaData gmd(g.get());
  if(subgridMetaData.isValid() && gmd != subgridMetaData)
  {
		RCRect r = gmd.rcRect().intersected(subgridMetaData.rcRect());
    auto tlRowCol = g->rc2rowCol(r.tl);
    auto brRowCol = g->rc2rowCol(r.br);
    int top = tlRowCol.row;
    int left = tlRowCol.col;
    int rows = brRowCol.row - tlRowCol.row;
    int cols = brRowCol.col - tlRowCol.col;
		res = GridPPtr(g->subGridClone(top, left, rows, cols));
  }
  else
  {
		if(alwaysClone)
			res = GridPPtr(g->clone());
	}
	return res;
}

vector<GridPPtr> GridManager::gridsFor(const string& regionName,
                                       set<string> datasetNames,
																			 const Path& userSubPath,
																			 int cellSize,
                                       GridMetaData subgridMetaData)
{
  static mutex lockable;
  lock_guard<mutex> lock(lockable);

	vector<GridPPtr> res;
  for(const GridMetaData& gmd : regionGmds(userSubPath))
  {
    if(gmd.regionName == regionName && gmd.cellsize == cellSize)
    {
			Path2GPS::const_iterator ci = _gmdMap.find(userSubPath);
      if(ci != _gmdMap.end())
      {
				GMD2GPS::const_iterator ci2 = ci->second.find(gmd);
        if(ci2 != ci->second.end())
        {
					const GridProxies& gps = ci2->second;
          for(GridProxyPtr gp : gps)
          {
            //in case of empty dataset names we interpret this as return all grids
            if(datasetNames.find(gp->datasetName) != datasetNames.end() ||
               datasetNames.empty())
              res.push_back(createSubgrid(gp->gridPPtr(), subgridMetaData));
					}
				}
			}
			break;
		}
	}

	return res;
}

GridMetaData GridManager::gridMetaDataForRegionName(const string& regionName,
                                                    const Path& userSubPath)
{
  for(const GridMetaData& gmd : regionGmds(userSubPath))
  {
		if(gmd.regionName == regionName)
			return gmd;
	}
	return GridMetaData();
}

//! assume that if availabe the internal structure has already build been build up
void GridManager::checkAndUpdateHdfStore(const Path& userSubPath)
{
//	cout << "entering GridManager::checkAndUpdateHdfStore(" << userSubPath << ")" << endl;

	GFN2GP& gridFn2grid = _gridPathMap[userSubPath];

	//don't check if this file exists
	string pathToHdfs =
			_env.hdfsStorePath + (userSubPath.empty() ? "" : "/" + userSubPath);
  if(ifstream((pathToHdfs + "/" + _env.noCheckFileName).c_str()))
  {
		cout << "don't check file found" << endl;
		//cout << "leaving GridManager::checkAndUpdateHdfStore(" << userSubPath << ")" << endl;
		return;
	}

	DIR* dp;
	struct dirent* ep;

	string pathToAsciiGrids = 
		_env.asciiGridsPath + (userSubPath.empty() ? "" : "/" + userSubPath);
	dp = opendir(pathToAsciiGrids.c_str());
  if(!dp)
  {
		cout << "couldn't open " << pathToAsciiGrids << endl;
//		cout << "leaving GridManager::checkAndUpdateHdfStore(" << userSubPath << ")" << endl;
		return;
	}

	//after reading the directory contains grids in the mapping file
	//but not anymore in the system
	GridProxies leftOverGrids;
	transform(gridFn2grid.begin(), gridFn2grid.end(), back_inserter(leftOverGrids),
						[](const GFN2GP::value_type& p){ return p.second; });
//						std::bind<GridProxyPtr>(&GFN2GP::value_type::second, std::placeholders::_1));

//	cout << "left over grids before: (" << endl;
//	for_each(leftOverGrids.begin(), leftOverGrids.end(),
//					 [](GridProxyPtr gp){ cout << endl << gp->toString(); });
//	cout << (leftOverGrids.empty() ? ")" : "\n)");

	while((ep = readdir(dp)))
	{
		//filter out files starting with . and all files have to end with .asc
		string dname(ep->d_name);
    if(dname.size() > 4 && dname[0] != '.' &&
       dname.rfind(".asc") == dname.length()-4)
    {
			string gridFileName(ep->d_name);
			string pathToGridFile = pathToAsciiGrids + "/" + gridFileName;
			
			time_t t = modificationTime(pathToGridFile.c_str());

//			cout << "gfn: " << gridFileName << " -> "
//			<< extractMetadataFromGrid(pathToGridFile).toCanonicalString() << endl;

//			cout << "gfn: " << gridFileName << " t: " << ctime(&t)
//			<< " (" << ((long int)t) << ")" << endl;

			GFN2GP::const_iterator it = gridFn2grid.find(gridFileName);
      if(it != gridFn2grid.end())
      {
				//remove the found grid from the leftover list
				leftOverGrids.erase(find(leftOverGrids.begin(), leftOverGrids.end(),
																 it->second));

				//no new grid, but has been update - visible by GridProxy::state = eChanged
				if(it->second->modificationTime < t)
				{
					it->second->updateModificationTime(t);
					cout << "grid has updated mod time" << endl;
				}
      }
      else
				addNewGridProxy(userSubPath, gridFileName, t);
		}
	}

//	cout << "left over grids after: " << endl;
//	cout << "--------------------------" << endl;
//	for_each(leftOverGrids.begin(), leftOverGrids.end(),
//					 [](GridProxyPtr gp){ cout << endl << gp->toString(); });
//	cout << "--------------------------" << endl;

	closedir(dp);

  updateHdfStore(userSubPath, leftOverGrids);

//	cout << "leaving GridManager::checkAndUpdateHdfStore(" << userSubPath << ")" << endl;
}

pair<string, GridMetaData>
GridManager::addNewGridProxy(const Path& userSubPath,
                             const string& gridFileName,
                             time_t modTime,
                             const std::string& pathToGrid,
														 CoordinateSystem cs)
{
//	cout << "entering GridManager::addNewGridProxy(" << userSubPath
//		<< ", " << gridFileName << ", " << modTime << ", " << pathToGrid << ")" << endl;

	string ptg = pathToGrid.empty()
		? _env.asciiGridsPath + (userSubPath.empty() ? "" : "/" + userSubPath)
		: pathToGrid;
	string pathToGridFile = ptg + "/" + gridFileName;
//	cout << "userSubPath: " << userSubPath
//		<< " pathToGrid: " << ptg
//		<< " pathToGridFile: " << pathToGridFile << endl;
  CoordinateSystem fileNameCS = extractCoordinateSystem(gridFileName);
  CoordinateSystem cs2 = cs;
  if(!cs2.isValid() && fileNameCS.isValid())
    cs2 = fileNameCS;

  GridMetaData gmd = extractMetadataFromGrid(pathToGridFile, cs2);
	gmd.regionName = extractRegionName(gridFileName);
//  if(gmd.regionName.substr(0, 6) == "brazil")
//		gmd.coordinateSystem = UTM21S_EPSG32721;
	string dsn = extractDatasetName(gridFileName);

	//	cout << "gmd: " << gmd.toString() << endl;
	if(gmd.isValid())
  {
		GridProxyPtr gp = GridProxyPtr(new GridProxy(gmd.coordinateSystem,
																								 dsn, gridFileName, ptg, modTime));

		//cout << "gp: " << gp->toString() << endl;
		//if this is a completely new gmd, then there will be no hdf-name in the map
		_gmdMap[userSubPath][gmd].push_back(gp);
	}

//	cout << "leaving GridManager::addNewGridProxy(" << userSubPath
//		<< ", " << gridFileName << ", " << modTime << ", " << pathToGrid << ")" << endl;

	return make_pair(dsn, gmd);
}

//! GridMetaData will contain no hdf name, as this is unknown to a grid
GridMetaData GridManager::
extractMetadataFromGrid(const string& pathToGridFile, CoordinateSystem cs) const
{
	ifstream fin(pathToGridFile.c_str());
	GridMetaData gmd(cs);
  if(fin)
  {
		string temp;

    double value;
    fin >> temp >> value;
    gmd.ncols = int(value);
    fin >> temp >> value;
    gmd.nrows = int(value);
    fin >> temp >> value;
    gmd.xllcorner = int(value);
    fin >> temp >> value;
    gmd.yllcorner = int(value);
    fin >> temp >> value;
    gmd.cellsize = int(value);
    fin >> temp >> value;
    gmd.nodata = int(value);
	}

	return gmd;
}

int& GridManager::hdfIdCount(const Path& userSubPath)
{
	map<Path, int>::iterator i = _userSubPathToHdfIdCount.find(userSubPath);

  if(i == _userSubPathToHdfIdCount.end())
  {
		_userSubPathToHdfIdCount[userSubPath] = 0;
		i = _userSubPathToHdfIdCount.find(userSubPath);
	}

	return i->second;
}
 
void GridManager::updateHdfStore(const Path& userSubPath,
                                 const GridProxies& leftOverGrids)
{
//	cout << "entering GridManager::updateHdfStore(" << userSubPath
//			 << ", leftOverGrids)" << endl;

	string pathToHdfs = _env.hdfsStorePath +
		(userSubPath.empty() ? "" : "/" + userSubPath);
	string pathToGrids = _env.asciiGridsPath +
		(userSubPath.empty() ? "" : "/" + userSubPath);

	//go through all (potential) hdfs (aka common grid-metadata)
	//and create or update the grids
	bool somethingChanged = false;
  for(GMD2GPS::value_type& p : _gmdMap[userSubPath])
  {
		GridProxies toBeDeletedFromHDF;
		GridProxies& gps = p.second;
//		cout << "region in userSubPath: " << userSubPath << " with gmd: " << p.first.toString() << endl;
		GridProxies onlyAppends;
		bool update = false; //is a grid new or has changed ?
		bool onlyAppend = true; //are all the grids new ?
		string hdfFileName; //we have to get the name from one of the proxies
    for(GridProxyPtr gp : gps)
    {
			ostringstream userInfo;
			userInfo << "path: (" << userSubPath << ") " << gp->toString() << " -> ";

			GridProxies::const_iterator fgpi = find(leftOverGrids.begin(),
			                                        leftOverGrids.end(), gp);
      //not a grid to remove
      if(fgpi == leftOverGrids.end())
      {
        switch(gp->state)
        {
					case GridProxy::eChanged:
						somethingChanged = true;
						update = true;
						onlyAppend = false;
						hdfFileName = gp->hdfFileName;
						//the existing grid (set to load from hdf) has to reload
						//after deleting the source hdf from the ascii grid
						gp->resetToLoadFromAscii(pathToGrids);
						cout << userInfo.str() << "has changed" << endl;
						break;
					case GridProxy::eNew:
						somethingChanged = true;
						update = true;
						onlyAppends.push_back(gp);
						cout << userInfo.str() << "is new" << endl;
						break;
					case GridProxy::eNormal:
						hdfFileName = gp->hdfFileName;
//						cout << userInfo.str() << "is normal" << endl;
				}
      }
      else
      {
				hdfFileName = gp->hdfFileName;
				toBeDeletedFromHDF.push_back(gp);
				update = true;
				onlyAppend = false;
				somethingChanged = true;
				cout << userInfo.str() << "to be deleted" << endl;
			}
		}

//		cout << "removing found left over elements from the hdfs.ini file" << endl;
//		cout << "---------------------------------" << endl;
    for(GridProxyPtr gp : toBeDeletedFromHDF)
		{
//			cout << "removing leftover gp from hdfs.ini: " << gp->toString() << endl;
			gps.erase(find(gps.begin(), gps.end(), gp));
		}
//		cout << "------------------------------------" << endl;

    if(update)
    {
			GridProxies toBeDeletedSaveProblem;
      if(onlyAppend)
      {
				if(onlyAppends.size() == gps.size()){ //all are new
					//reach for the underlying grid of every proxy in order to load it
					//and be able to store it anew in the hdf-file
					//for_each(gps.begin(), gps.end(), [](GridProxyPtr gp){ gp->gridPtr(); });
          for(GridProxyPtr gp : gps)
					{
					  gp->gridPtr();
					}

					ostringstream s; s << ++hdfIdCount(userSubPath) << ".h5";
					//try to delete the new file first, so we
					//don't write into an old one accidentally, should be safe as
					//the hdfs dir shouldn't be touched anyway and modified by hand !!
					remove((pathToHdfs + "/" + s.str()).c_str());
					toBeDeletedSaveProblem = appendToHdf(userSubPath, gps, s.str());
        }
        else
					toBeDeletedSaveProblem = appendToHdf(userSubPath, onlyAppends, hdfFileName); //append only the new ones
      }
      else
      {
				//reach for the underlying grid of every proxy in order to load it
				//and be able to store it anew in the hdf-file
				//for_each(gps.begin(), gps.end(), [](GridProxyPtr gp){ gp->gridPtr(); });
        for(GridProxyPtr gp : gps)
				{
				  gp->gridPtr();
				}

				if(remove((pathToHdfs + "/" + hdfFileName).c_str()) != 0)
				{
					ostringstream s; s << ++hdfIdCount(userSubPath) << ".h5";
					cout << "Error deleting hdf-file: " << hdfFileName
							 << " before creating new one with with changed grids."
									" Creating one with name: " << s.str() << endl;
					hdfFileName = s.str();
				}
				toBeDeletedSaveProblem = appendToHdf(userSubPath, gps, hdfFileName); //all have to be written again
			}

//			cout << "elements which where supposed to be in the hdf:" << endl;
//			cout << "------------------------" << endl;
//			for_each(gps.begin(), gps.end(),
//							 [](GridProxyPtr gp){ cout << endl << gp->toString(); });
//			cout << "-----------------------------" << endl;

//			cout << "removing to be elements which couldn't be saved" << endl;
//			cout << "--------------------------" << endl;
      for(GridProxyPtr gp : toBeDeletedSaveProblem)
			{
//				cout << "removing unsaved gp: " << gp->toString() << endl;
				gps.erase(find(gps.begin(), gps.end(), gp));
			}
//			cout << "--------------------------" << endl;

//			cout << "remaining elements in the hdf: " << endl;
//			cout << "--------------------------" << endl;
//			for_each(gps.begin(), gps.end(),
//							 [](GridProxyPtr gp){ cout << endl << gp->toString(); });
//			cout << "--------------------------" << endl;
		}
	}

	if(somethingChanged)
		writeGrid2HdfMappingFile(userSubPath);

//	cout << "leaving GridManager::updateHdfStore(" << userSubPath
//			 << ", leftOverGrids)" << endl;
}

GridManager::GridProxies
GridManager::appendToHdf(const Path& userSubPath,
												 const GridProxies& proxies,
												 const FileName& newHdfFileName)
{
	//cout << "entering GridManager::appendToHdf(" << userSubPath << ", "
	//	<< "gridProxies, " << newHdfFileName << ")" << endl;

	GridProxies toBeDeletedSaveProblem;

	cout << "userSubPath: " << userSubPath << " appending to hdf:(if new) " << newHdfFileName << endl;
  for(GridProxyPtr gp : proxies)
  {
		//string datasetName = extractDatasetName(gp->fileName);
		//string regionName = extractRegionName(gp->fileName);
		//gp->g = new GridP(datasetName, GridP::ASCII,
		//                  _gridsPath + "/" + gp->fileName);

    bool success = gp->gridPtr()->writeHdf
        (_env.hdfsStorePath + (userSubPath.empty() ? "" : "/" + userSubPath) +
         "/" + newHdfFileName,
         gp->datasetName,
         extractRegionName(gp->fileName),
         coordinateSystemToShortString(gp->coordinateSystem),
         gp->modificationTime);

		if(!newHdfFileName.empty())
		{
			gp->hdfFileName = newHdfFileName;
			gp->pathToHdf =
				_env.hdfsStorePath + (userSubPath.empty() ? "" : "/" + userSubPath);
		}

		if(success)
			gp->state = GridProxy::eNormal;
    else
    {
			cout << "Error writing grid " << gp->fileName << " to hdf-file "
					 << gp->hdfFileName << ", because datasetname "
					 << gp->datasetName << " exists already. "
					 << " Grid couldn't be appended to hdf file." << endl;
			toBeDeletedSaveProblem.push_back(gp);
		}

    //delete grid (smartpointer shouldn't be referenced from somewhere else)
		//to free space while converting grids to hdf
    gp->reset();
	}

	//cout << "leaving GridManager::appendToHdf(" << userSubPath << ", "
	//	<< "GridProxies, " << newHdfFileName << ")" << endl;
	
	return toBeDeletedSaveProblem;
}

time_t GridManager::modificationTime(const char* fileName)
{
	struct stat attrib;	// create a file attribute structure
	stat(fileName, &attrib);	// get the attributes of the dir-entry
	return attrib.st_mtime;
}

void GridManager::readRegionalizedData()
{
	//read the whole file
	IniParameterMap ipm(_env.regionalizationIniFilePath);

  const Names2Values& groups = ipm.values("groups");
  for(Names2Values::value_type g : groups)
  {
    const string& group = g.first;
//    cout << "group: " << group << endl;
    const string& section = g.second;
    const Names2Values& elements = ipm.values(section);
//    cout << "section: " << section << endl;
    for(Names2Values::value_type e : elements)
    {
      const string& element = e.first;
//      cout << "element: " << element << endl;
      _groups2members[group].push_back(element);
    }
  }

	string pattern = ipm.value("general", "pattern");
	//cout << "pattern: " << pattern << endl;
  for(IniParameterMap::const_iterator ci = ipm.begin(); ci != ipm.end(); ci++)
  {
		string hdfFileName = ci->first;
		//cout << "substr: " << hdfFileName.substr(hdfFileName.length()-2) << endl;
		if(hdfFileName.substr(hdfFileName.length()-3) != ".h5") continue;

		const Names2Values& ns2vs = ci->second;
		Names2Values::const_iterator ci2;

    string data = (ci2 = ns2vs.find("data")) == ns2vs.end()
                  ? "" : ci2->second;
    string region = (ci2 = ns2vs.find("region")) == ns2vs.end()
                    ? "" : ci2->second;
		string datasetNamesPattern =
        (ci2 = ns2vs.find("dataset-names-pattern")) == ns2vs.end()
        ? "" : ci2->second;
    string fromTo = (ci2 = ns2vs.find(pattern)) == ns2vs.end()
                    ? "" : ci2->second; //1961-2050
    string sim = (ci2 = ns2vs.find("simulation")) == ns2vs.end()
                 ? "" : ci2->second;
    string scen = (ci2 = ns2vs.find("scenario")) == ns2vs.end()
                  ? "" : ci2->second;

//    cout << "data: " << data << " | region: " << region
//    << 	" | datasetNamesPattern: " << datasetNamesPattern
//    << " | fromTo: " << fromTo << " | sim: " << sim
//    << " | scen: " << scen << endl;

    size_t s = fromTo.find_first_of("-");
		int from = atoi(trim(fromTo.substr(0, s)).c_str()); //1961
		int to = atoi(trim(fromTo.substr(s+1)).c_str()); //2050

    string path = _env.regionalizationHdfsPath;
    path.append("/").append(sim).append("/").append(scen).append("/");
//    cout << "path: " << path << endl;

    size_t pi = datasetNamesPattern.find_first_of(pattern);
    for(int year = from; year <= to; year++)
    {
			ostringstream oss;
			oss << datasetNamesPattern.substr(0, pi) << year <<
			datasetNamesPattern.substr(pi+pattern.length());
			string datasetName = oss.str();

//      cout << "datasetName: " << datasetName << endl;
      GridProxyPtr gp = GridProxyPtr(new GridProxy(shortStringToCoordinateSystem("gk5"),
																									 datasetName, "", path,
																									 hdfFileName, 0));

      _region2regData[region][data][sim][scen][year] = gp;
		}
	}
}

const RegDataMap*
GridManager::regionalizedData(const string& region) const
{
	Region2RegData::const_iterator ci = _region2regData.find(region);
	return ci == _region2regData.end() ? NULL : &(ci->second);
}

RegData GridManager::regionalizedData(const string& region,
                                      const string& dataId) const
{
	RegData res;
	res.dataId = dataId;

	IniParameterMap ipm(_env.regionalizationIniFilePath);
	res.dataName = ipm.value("name-mappings", dataId);

  if(const RegDataMap* rdm = regionalizedData(region))
  {
		RegDataMap::const_iterator ci = rdm->find(dataId);
		if(ci != rdm->end())
			res.value = &(ci->second);
	}

	return res;
}

RegGroupOfData
GridManager::regionalizedGroupOfData(const string& region,
                                     const string& groupId) const
{
	RegGroupOfData res;
	res.groupId = groupId;

	IniParameterMap ipm(_env.regionalizationIniFilePath);
	res.groupName = ipm.value("name-mappings", groupId);

	Groups2Members::const_iterator ci = _groups2members.find(groupId);
  if(ci != _groups2members.end())
  {
    for(const string& dataId : ci->second)
    {
			//for(GroupMemberIds::const_iterator ci2 = ci->second.begin();
			//		ci2 != ci->second.end(); ci2++) {
			//string dataId = *ci2;
			res.values.push_back(regionalizedData(region, dataId));
    }
	}

	return res;
}

void GridManager::readGrid2HdfMappingFile(const Path& userSubPath)
{
	//cout << "entering GridManager::readGrid2HdfMappingFile(" << userSubPath << ")" << endl;
	
	//read the whole file
	string pathToHdfs =
		_env.hdfsStorePath + (userSubPath.empty() ? "" : "/" + userSubPath);
	//cout << "pathToHdfs: " << pathToHdfs << endl;
	IniParameterMap ipm(pathToHdfs + "/" + _env.hdfsIniFileName);
	IniParameterMap::const_iterator ipmci = ipm.find("mappings");
  if(ipmci == ipm.end())
  {
		//cout << "no mappings file there" << endl;
		//cout << "leaving GridManager::readGrid2HdfMappingFile(" << userSubPath << ")" << endl;
		return;
	}
	set<string> usedHdfFilenames;
	const Names2Values& ns2vs = ipmci->second;
  for(Names2Values::const_iterator ci = ns2vs.begin(); ci != ns2vs.end(); ci++)
  {
		string gridFileName = ci->first;
		string hdfFileName = ci->second;
		usedHdfFilenames.insert(hdfFileName);
		hdfIdCount(userSubPath) = max(hdfIdCount(userSubPath), atoi(hdfFileName.c_str()));
		//cout << "current max hdfIdCount(" << userSubPath << "): " << hdfIdCount(userSubPath) << endl;

		string datasetName = extractDatasetName(gridFileName);
    pair<GridMetaData, time_t> p = readGridMetadataFromHdf(pathToHdfs + "/" + hdfFileName, datasetName);
		//couldn't read hdf, so just ignore it
		//if there are grids for the supposed to be there hdf, it gonna
		//get created anew from the ascii grids
		if(p.second < 0)
			continue;

		GridProxyPtr gp = GridProxyPtr(new GridProxy(p.first.coordinateSystem,
																								 datasetName, gridFileName,
																								 pathToHdfs, hdfFileName,
																								 p.second));

		//cout << "created: gmd: " << p.first.toString() << " gp: " << gp->toString() << endl;

		//fill the structure to be used later
		_gmdMap[userSubPath][p.first].push_back(gp);
		//fill map to find elements necessary to be updated or added
		_gridPathMap[userSubPath].insert(make_pair(gridFileName, gp));
	}

	//delete hdf files which aren't used anymore
	//this seams to be necessary as some (all) hdfs can't be deleted (sometimes)
	//under windows when the dss is running as some handle to the files
	//seams to exist, even though it shouldn't
	//this is just a workaround
	for(int i = 1, max = hdfIdCount(userSubPath); i <= max; i++)
	{
		ostringstream s; s << i << ".h5";
		if(usedHdfFilenames.find(s.str()) == usedHdfFilenames.end())
			if(remove((pathToHdfs + "/" + s.str()).c_str()) != 0)
				;//cout << "Couldn't delete obsolete hdf-file: " << s.str() << endl;
	}

	//cout << "leaving GridManager::readGrid2HdfMappingFile(" << userSubPath << ")" << endl;
}

string GridManager::extractDatasetName(const string& gfn) const
{
	return gfn.substr(0, gfn.find_first_of("_"));
}

string GridManager::extractRegionName(const string& gfn) const
{
  size_t start = gfn.find_first_of("_")+1;
	return gfn.substr(start, gfn.find_first_of("_", start) - start);
}

Tools::CoordinateSystem GridManager::extractCoordinateSystem(const string& gfn) const
{
  size_t start1 = gfn.find_first_of("_")+1;
  size_t start2 = gfn.find_first_of("_", start1)+1;
  size_t dotPos = gfn.find_first_of(".", start2);
  string csShortString = gfn.substr(start2, dotPos - start2);
  //cout << "csShortString: " << csShortString << endl;
  return shortStringToCoordinateSystem(csShortString, Tools::CoordinateSystem());
}

void GridManager::writeGrid2HdfMappingFile(const Path& userSubPath)
{
	//cout << "entering GridManager::writeGrid2HdfMappingFile(" << userSubPath << ")" << endl;
	string pathToHdfsIniFile = _env.hdfsStorePath +
		(userSubPath.empty() ? "" : "/" + userSubPath) + "/" + _env.hdfsIniFileName;
//	cout << "userSubPath: " << userSubPath << " pathToHdfsIniFile: " << pathToHdfsIniFile << endl;
	ofstream fout(pathToHdfsIniFile.c_str());
  if(!fout.fail())
  {
		fout <<
		";ascii-grid to hdf-file mappings" << endl <<
		"[mappings]" << endl;
		Path2GPS::const_iterator ci = _gmdMap.find(userSubPath);
    if(ci != _gmdMap.end())
    {
      for(GMD2GPS::value_type p : ci->second)
      {
        for(GridProxyPtr gp : p.second)
        {
					//cout << "gfn: " << gp->fileName
					//	<< " hfn: " << gp->hdfFileName << endl;
					fout << gp->fileName << " = " << gp->hdfFileName << endl;
				}
			}
		}
	}
	fout.close();
	//cout << "leaving GridManager::writeGrid2HdfMappingFile(" << userSubPath << ")" << endl;
}

vector<vector<LatLngCoord> > GridManager::regions(const Path& userSubPath) const
{
	vector<vector<LatLngCoord> > v;
	Path2GPS::const_iterator ci = _gmdMap.find(userSubPath);
  if(ci != _gmdMap.end())
  {
    for(GMD2GPS::value_type p : ci->second)
    {
			v.push_back(RC2latLng(p.first.rcRect().toTlTrBrBlVector()));
		}
	}
	return v;
}

vector<GridMetaData> GridManager::regionGmds(const Path& userSubPath) const
{
	vector<GridMetaData> v;
	Path2GPS::const_iterator ci = _gmdMap.find(userSubPath);
  if(ci != _gmdMap.end())
  {
    for(GMD2GPS::value_type p : ci->second)
    {
			//cout << "gmd: " << p.first.toString() << endl;
			v.push_back(p.first);
		}
	}
	return v;
}

//------------------------------------------------------------------------------

GridManager& Grids::gridManager(GridManager::Env env)
{
	static GridManager gm(env);
	return gm;
}
