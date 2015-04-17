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

#include <sstream>
#include <fstream>
#include <algorithm>
#include <functional>
#include <list>

#ifdef WIN32
#include "grid/dirent.h"
#include <direct.h>
#else
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#endif

#include "grid+.h"
#include "tools/algorithms.h"
#include "tools/helper.h"

using namespace Grids;
using namespace std;
using namespace Tools;
using namespace boost;

namespace
{
  list<string> splitPath(string path)
  {
    list<string> p;
    int prevIndex = path.at(0) == '/' ? 1 : 0;
    int index = path.find_first_of('/', prevIndex);
    while(index != int(string::npos))
    {
      p.push_back(path.substr(prevIndex, index-prevIndex));
      prevIndex = index + 1;
      index = path.find_first_of('/', prevIndex);
    }
    if(prevIndex < int(path.length()))
      p.push_back(path.substr(prevIndex));
    return p;
  }

  //! return true if successfully checked or created
  bool ensureDirExists(string pathToDir)
  {
    //cout << "pathToDir: " << pathToDir << endl;

    DIR* dp = NULL;
    dp = opendir(pathToDir.c_str());
    if(dp)
    {
      closedir(dp);
      return true;
    }

    list<string> subpaths = splitPath(pathToDir);

    string subpath = pathToDir.at(0) == '/' ? "/" : "";
    for(string sp : subpaths)
    {
      subpath += sp + "/";

      dp = opendir(subpath.c_str());
      if(dp)
        closedir(dp);
      else
      {
        int status = -1;
#ifdef WIN32
        status = _mkdir(subpath.c_str());
#else
        status = mkdir(subpath.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif
        if(status == -1)
          return false;
      }
    }
    return true;
  }
}

#ifndef NO_HDF5
grid* Grids::loadGrid(const string& gridName, const std::string& pathToHdf)
{
	//cout << "reading hdf-file via |the-path-to-the-hdf-file|: |"
	//<< pathToHdf() << "|";

	grid* g = new grid(100);
	g->read_hdf((char*)pathToHdf.c_str(), (char*)gridName.c_str());
	return g;
}
#endif

void Grids::printGridFields(const grid& g, bool onlyDataFields)
{
  for(int i = 0; i < g.nrows; i++)
  {
		cout << "row: " << i << endl;
    for(int k = 0; k < g.ncols; k++)
    {
			if(g.feld[i][k] != g.nodata || !onlyDataFields)
				cout << g.feld[i][k] << " ";
		}
		cout << endl;
	}
}

void Grids::setAllGridFieldsTo(grid* g, double newValue, bool keepNoData)
{
  for(int i = 0; i < g->nrows; i++)
    for(int k = 0; k < g->ncols; k++)
			if(g->feld[i][k] != g->nodata || !keepNoData)
				g->feld[i][k] = newValue;
}

//------------------------------------------------------------------------------

bool RCRect::contains(const RCRect& o) const
{
  return
      o.tl.r >= tl.r && o.tl.h <= tl.h &&
      o.br.r <= br.r && o.br.h >= br.h;
}

bool RCRect::intersects(const RCRect& other) const
{
  return
      max(tl.r, other.tl.r) <= min(br.r, other.br.r) &&
      min(tl.h, other.tl.h) >= max(br.h, other.br.h);
}

RCRect RCRect::intersected(const RCRect& other) const
{
	//cout << "intersecting: this: " << toString()
	//<< " other: " << other.toString() << endl;
	if (isNull() || other.isNull())
		return RCRect(tl.coordinateSystem);
	RCRect temp(tl.coordinateSystem);
	temp.tl.r = max(tl.r, other.tl.r);
	temp.tl.h = min(tl.h, other.tl.h);
	temp.br.r = min(br.r, other.br.r);
	temp.br.h = max(br.h, other.br.h);
	return temp.isEmpty() ? RCRect(tl.coordinateSystem) : temp;
}

string RCRect::toString() const
{
	ostringstream s;
	s << "tl: " << tl.toString() << " br: " << br.toString()
	<< " w: " << (br.r - tl.r) << " h: " << (tl.h - br.h);
	return s.str();
}

vector<RectCoord> RCRect::toTlTrBrBlVector() const
{
	vector<RectCoord> v(4);
	v[0]=tl;
	v[1]=RectCoord(br.coordinateSystem, br.r, tl.h);
	v[2]=br;
	v[3]=RectCoord(tl.coordinateSystem, tl.r, br.h);
	return v;
}

//------------------------------------------------------------------------------

GridMetaData::GridMetaData(CoordinateSystem cs)
	: ncols(-1),
		nrows(-1),
		coordinateSystem(cs)
{}

GridMetaData::GridMetaData(const grid* g, CoordinateSystem cs)
: ncols(g->ncols),
	nrows(g->nrows),
	nodata(g->nodata),
	xllcorner(int(g->xcorner)),
	yllcorner(int(g->ycorner)),
	cellsize(int(g->csize)),
	coordinateSystem(cs)
{}

GridMetaData::GridMetaData(const GridP* g)
: ncols(g->cols()),
	nrows(g->rows()),
	nodata(g->noDataValue()),
	xllcorner(int(g->gridPtr()->xcorner)),
	yllcorner(int(g->gridPtr()->ycorner)),
	cellsize(int(g->cellSize())),
	coordinateSystem(g->coordinateSystem())
{}

bool GridMetaData::operator==(const GridMetaData& other) const
{
	return ncols == other.ncols && nrows == other.nrows &&
      nodata == other.nodata && xllcorner == other.xllcorner
			&& yllcorner == other.yllcorner && cellsize == other.cellsize
			&& coordinateSystem == other.coordinateSystem;
}

string GridMetaData::toString() const
{
	ostringstream s;
	s.precision(7);
	s << "ncols: " << ncols << " nrows: " << nrows << " nodata: " << nodata
		<< " cellsize: " << cellsize
		<< fixed
		<< " llx: " << xllcorner << " lly: " << yllcorner
		<< " coordinateSystem: " << coordinateSystemToString(coordinateSystem);
	return s.str();
}

string GridMetaData::toShortDescription() const
{
	ostringstream s;
	s << "Grid [" << ncols << "x" << nrows << "], ZG:" << cellsize
		<< ", CS:" << coordinateSystemToShortString(coordinateSystem);
	return s.str();
}

string GridMetaData::toCanonicalString(const std::string& sep) const
{
	ostringstream s;
	s.precision(7);
	s << ncols << sep << nrows << sep << nodata << sep << cellsize << sep
		<< ios::fixed << xllcorner << sep << yllcorner
		<< sep << coordinateSystemToShortString(coordinateSystem);
	return s.str();
}

//------------------------------------------------------------------------------

#ifndef NO_HDF5
pair<GridMetaData, time_t>
Grids::readGridMetadataFromHdf(const string& hdfFileName,
                               const string& datasetName)
{
	GridMetaData gmd;

	hdf5 hd;
  if(hd.open_f(hdfFileName.c_str())!=0)
  {
    cerr << "error (readGridMetadataFromHdf): can not open hdf_file: "
         << hdfFileName << endl;
    return make_pair(gmd, -1);
  }
  if(hd.open_d(datasetName.c_str())!=0)
  {
    cerr << "error (readGridMetadataFromHdf): can not open dataset: "
         << hdfFileName << endl;
    return make_pair(gmd, -1);
  }

  gmd.ncols = hd.get_i_attribute("ncols");
  gmd.nrows = hd.get_i_attribute("nrows");
  gmd.nodata = hd.get_i_attribute("nodata");
  gmd.xllcorner = hd.get_d_attribute("xllcorner");
  gmd.yllcorner = hd.get_d_attribute("yllcorner");
  gmd.cellsize = hd.get_f_attribute("cell-size");
  char* regionName = hd.get_s_attribute("region-name");
  gmd.regionName = regionName;
  free(regionName);
  char* cs = hd.get_s_attribute("coordinate-system");
  gmd.coordinateSystem = shortStringToCoordinateSystem(cs);
  free(cs);
//	if(gmd.regionName.substr(0, 6) == "brazil")
//		gmd.coordinateSystem = UTM21S_EPSG32721;
  time_t time = hd.get_l_attribute("time");
  return make_pair(gmd, time);
}
#endif

//------------------------------------------------------------------------------

string SubData::toString() const
{
	ostringstream s;
	s << "(row/col): (" << row << "/" << col << ") "
      << " (rows/cols): " << rows << "/" << cols << ")";
	return s.str();
}

RCRect Grids::extendedBoundingRect(const GridMetaData& gmd,
																	 const Quadruple<RectCoord>& rcpoly,
																	 double cellSize)
{
	//get bounding rect of rc polygon
	RectCoord tl(gmd.coordinateSystem, min(rcpoly.tl.r, rcpoly.bl.r), max(rcpoly.tl.h, rcpoly.tr.h));
	RectCoord br(gmd.coordinateSystem, max(rcpoly.tr.r, rcpoly.br.r), min(rcpoly.bl.h, rcpoly.br.h));
	RCRect boundingRect(tl, br);

	//first find position in choosen grid metadata
	//might in a grid if top left corner of bounding rect is inside
	//the gmd or 0,0 (aka the top left of gmd) if the top left corner of
	//the bounding rect is outside the gmd
	const RCRect& intersectedRect = gmd.rcRect().intersected(boundingRect);
	//cout << "intersectedRect: " << intersectedRect.toString() << endl;
	const RectCoord& delta1 = intersectedRect.tl - gmd.topLeftCorner();
	int indexR = int(std::floor(delta1.r / cellSize));
	int indexH = int(std::floor(abs(delta1.h / cellSize)));
	//cout << "delta1: " << delta1.toString() << " indexR: " << indexR
	//<< " indexH: " << indexH << endl;
	//rc position into choosen grid-class
	RectCoord gmdTl(gmd.coordinateSystem,
									gmd.topLeftCorner().r + (indexR * cellSize),
									gmd.topLeftCorner().h - (indexH * cellSize));
	//cout << "grid-class top left: " << firstGmdTl.toString() << endl;

	//now expand the top left corner of the bounding rect to the full
	//outer (hypethetical) grid-cell bound
	//this might be either the same as the previous calculated corner
	//of a grid cell in the gmd (or gmd's 0,0 is case of an exact match)
	//or the bounds have to be extended if the top left corner of bounding rect
	//(of the users selection) was originally outside of the gmd
	RectCoord delta2 = boundingRect.tl - gmdTl;
	int nocsToTlr = delta1.r > 0 ? 0 : int(std::ceil(abs(delta2.r / cellSize))); //no of cells
	int nocsToTlh = delta1.h < 0 ? 0 : int(std::ceil(abs(delta2.h / cellSize)));
	//cout << "delta2: " << delta2.toString() << " nocsToTlr: " << nocsToTlr
	//<< " nocsToTlh: " << nocsToTlh << endl;
	//extended tl
	RectCoord etl(gmd.coordinateSystem,
								gmdTl.r - (nocsToTlr * cellSize),
								gmdTl.h + (nocsToTlh * cellSize));
	//cout << "extended top left: " << etl.toString() << endl;

	//adjust br to multiple of cellSize
	RectCoord delta3 = br - etl;
	Cols nocsR = Cols(std::ceil(abs(delta3.r / cellSize))); //no of cells
	if(nocsR == 0) nocsR++; //the selection is choosing at least one cell
	Rows nocsH = Rows(std::ceil(abs(delta3.h / cellSize)));
	if(nocsH == 0) nocsH++;
	//cout << "delta3: " << delta3.toString() << " nocsR: " << nocsR
	//<< " nocsH: " << nocsH << endl;
	//the extended final bounding rect
	RectCoord ebr(gmd.coordinateSystem,
								etl.r + (nocsR * cellSize),
								etl.h - (nocsH * cellSize));

	return RCRect(etl, ebr);
}

pair<Row, Col> Grids::rowColInGrid(const GridMetaData& gmd, const RectCoord& c)
{
	const RectCoord& delta = c - gmd.topLeftCorner();
	Col indexR = Col(std::floor(abs(delta.r / gmd.cellsize)));
	if(int(indexR) == gmd.ncols) --indexR; //max border is in last cell included
	Row indexH = Row(std::floor(abs(delta.h / gmd.cellsize)));
	if(int(indexH) == gmd.nrows) --indexH;
	return make_pair(indexH, indexR);
}

//------------------------------------------------------------------------------

GridP::GridP(CoordinateSystem cs)
	: _coordinateSystem(cs) { }

//! new grid with given size and initialized to no data
GridP::GridP(const std::string& datasetName,
						 int nrows, int ncols,
						 float cellSize, double llx, double lly, float noDataValue,
						 CoordinateSystem cs)
	: _datasetName(datasetName),
		_coordinateSystem(cs)
{
	_grid = GridPtr(new grid(nrows, ncols));
  _grid->rgr = int(cellSize); // setze Rastergroesse
	setAllFieldsTo(noDataValue);
	_grid->has_nodata = 1;
	_grid->csize=cellSize;
	_grid->nodata=noDataValue;
	_grid->xcorner=llx;
	_grid->ycorner=lly;
}

GridP::GridP(GridMetaData gmd, const string& datasetName)
	: _datasetName(datasetName),
		_coordinateSystem(gmd.coordinateSystem)
{
  _grid = GridPtr(new grid(gmd.nrows, gmd.ncols));
  _grid->rgr = int(gmd.cellsize); // setze Rastergroesse
  setAllFieldsTo(gmd.nodata);
  _grid->has_nodata = 1;
  _grid->csize=gmd.cellsize;
  _grid->nodata=gmd.nodata;
  _grid->xcorner=gmd.xllcorner;
  _grid->ycorner=gmd.yllcorner;
}

GridP::GridP(const string& datasetName, FileType ft, const string& pathToFile,
						 CoordinateSystem cs)
	: _datasetName(datasetName),
		_coordinateSystem(cs)
{
  switch(ft)
  {
#ifndef NO_HDF5
  case HDF:
    _grid = GridPtr(new grid(100));
    readHdf(pathToFile.c_str(), datasetName.c_str());
    break;
#endif
  case ASCII:
    _grid = GridPtr(new grid(100));
    _grid->read_ascii((char*)pathToFile.c_str());
    break;
	}
}

GridP::GridP(grid* wrapThisGrid, CoordinateSystem cs)
	: _grid(GridPtr(wrapThisGrid)),
		_coordinateSystem(cs)
{}

//! copy constructor
GridP::GridP(const GridP& other)
  : _grid(GridPtr(other._grid->grid_copy())),
  _datasetName(other._datasetName),
  _descriptiveLabel(other._descriptiveLabel),
	_unit(other._unit),
	_coordinateSystem(other._coordinateSystem)
{
}

GridP::~GridP() { }

GridP::GridP(const grid& other, CoordinateSystem cs)
	: _grid(GridPtr(const_cast<grid*>(&other)->grid_copy())),
		_coordinateSystem(cs)
{ }

GridP::GridP(grid const *const other, CoordinateSystem cs)
	: _grid(GridPtr(const_cast<grid*>(other)->grid_copy())),
		_coordinateSystem(cs)
{ }

GridP& GridP::operator=(const GridP& other)
                       {
	_grid = GridPtr(other._grid->grid_copy());
	_datasetName = other._datasetName;
	_descriptiveLabel = other._descriptiveLabel;
  _unit = other._unit;
	_coordinateSystem = other._coordinateSystem;
	return *this;
}

GridP& GridP::operator=(const grid& other)
{
	_grid = GridPtr(const_cast<grid*>(&other)->grid_copy());
	return *this;
}

bool GridP::operator==(const GridP& other) const
{
	if(descriptiveLabel() != other.descriptiveLabel())
		return false;
	if(datasetName() != other.datasetName())
		return false;
  if(unit() != other.unit())
    return false;
	if(coordinateSystem() != other.coordinateSystem())
		return false;

  for(int i = 0; i < rows(); i++)
    for(int j = 0; j < cols(); j++)
			if(dataAt(i, j) != other.dataAt(i, j))
				return false;

	return true;
}

#ifndef NO_HDF5
int GridP::readHdf(const string& pathToHdfFile, const string& datasetName)
{
  hdf5* hd = new hdf5;
  if(hd->open_f(pathToHdfFile.c_str())!=0){
    cerr << "error (read_hdf): can not open hdf_file: " << pathToHdfFile << endl;
    return -1;
  }
  if(hd->open_d(datasetName.c_str())!=0){
    cerr << "error (read_hdf): can not open dataset: " << datasetName << endl;
    return -2;
  }
  if(_grid->feld!=(float**)NULL){
    for(int i=0; i<_grid->nrows; i++){
      delete[] _grid->feld[i];
    }
  }
  _grid->ncols=hd->get_i_attribute("ncols");
  _grid->nrows=hd->get_i_attribute("nrows");
  _grid->nodata=hd->get_i_attribute("nodata");
  _grid->xcorner=hd->get_d_attribute("xllcorner");
  _grid->ycorner=hd->get_d_attribute("yllcorner");
  _grid->csize=hd->get_f_attribute("cell-size");
  char* cs = hd->get_s_attribute("coordinate-system");
  _coordinateSystem = Tools::shortStringToCoordinateSystem(string(cs));
  free(cs);
  //cerr << ncols << " " << nrows << " " << csize << endl;
  hd->read_f_feld(datasetName.c_str()); // writes to f1 in grid
  int nrows = _grid->nrows;
  int ncols = _grid->ncols;
  _grid->feld = new float*[nrows];
  for(int i=0; i<nrows; i++){
    if((_grid->feld[i]=new float[ncols])==NULL){
      cerr << "error (read_hdf): no sufficient memory" << endl;
    }
  }
  // read_in
  for(int i=0; i<nrows; i++){
    for(int j=0; j<ncols; j++){
      _grid->feld[i][j]=hd->f1[i*ncols+j];
    }
  }
  delete hd;
  return 0;
}

bool GridP::writeHdf(const string& pathToHdfFile, const string& datasetName,
                     const string& regionName, const string& coordinateSystemShort, time_t t)
{
  //cout << "pathToHdfFile: " << pathToHdfFile << endl;
  if(!ensureDirExists(pathToHdfFile.substr(0, pathToHdfFile.find_last_of('/'))))
    return false;

	int ncols = _grid->ncols;
	int nrows = _grid->nrows;
	float** feld = _grid->feld;

  //cerr << "hdf " << fname << " " << datasetn << endl;
	float* f1 = new float[nrows * ncols];
  if(f1 == NULL)
  {
		cerr << "error (write_hdf): no space on device\n";
		exit(2);
	}
	for(int i = 0; i < nrows; i++)
		for(int j = 0; j < ncols; j++)
			f1[i * ncols + j] = feld[i][j];
	hdf5* hd = new hdf5;
  if(hd->open_f(pathToHdfFile.c_str()) != 0)
    hd->create_f(pathToHdfFile.c_str());
	bool success = false;
  if(hd->open_d(datasetName.c_str()) != 0)
  {
    hd->write_f_feld(datasetName.c_str(), f1, nrows, ncols);
    hd->write_s_attribute("coordinate-system", coordinateSystemShort.c_str());
    hd->write_s_attribute("region-name", regionName.c_str());
    hd->write_l_attribute("time", t);
    hd->write_d_attribute("xllcorner", _grid->xcorner);
    hd->write_d_attribute("yllcorner", _grid->ycorner);
    hd->write_f_attribute("cell-size", _grid->csize);
    hd->write_i_attribute("nodata", _grid->nodata);
    hd->write_i_attribute("ncols", ncols);
    hd->write_i_attribute("nrows", nrows);
		success = true;
	}
	delete[] f1;
	delete hd;
	return success;
}
#endif

//void GridP::writeAscii(const std::string& pathToAsciiFile)
//{
//	return _grid->write_ascii((char*)pathToAsciiFile.c_str());
//}

vector<double> GridP::allDataAsLinearVector() const
{
	vector<double> linear;
	for(int i = 0; i < rows(); i++)
		for(int j = 0; j < cols(); j++)
			if(isDataField(i, j))
				linear.push_back(dataAt(i, j));

	return linear;
}

HistogramData GridP::histogram(int noOfClasses)
{
	HistogramData res;

	vector<double> linear(rows()*cols());
	int k = -1;
	int nop = 0; // number of pixels
  for(int i = 0; i < rows(); i++)
  {
    for(int j = 0; j < cols(); j++)
    {
      if(isDataField(i, j))
      {
				linear[++k] = dataAt(i, j);
				nop++;
			}
		}
	}

	if(nop == 0) return res;

	pair<double, double> mima = Tools::minMax(linear);
	double diff = mima.second - mima.first;
	double stepSize = diff / double(noOfClasses);
	double ceiledStepSize = stepSize == 0 ? 1 : std::ceil(stepSize);

  res = histogramDataByStepSize(vector<double>(linear.begin(), linear.begin() + k + 1), ceiledStepSize);
  for(int i = 0, size = res.classes.size(); i < size; i++)
    //store percent of pixels in a certain class
    res.classes[i] = (res.classes[i] / nop) * 100;

	return res;
}

/*
multimap<double, double, greater<double> > 
	GridP::frequency(bool includeNoDataValues, int roundValueToDigits, int roundResultToDigits)
{
	typedef map<int, int> Map;
	Map m;
	int nops = 0; //number of pixels
  for(int i = 0; i < rows(); i++)
  {
    for(int j = 0; j < cols(); j++)
    {
      if(isDataField(i, j))
      {
				int v = int(dataAt(i, j)*pow(10, roundValueToDigits));
				if(m.find(v) == m.end())
					m[v] = 1;
				else
					m[v]++;
				nops++;
			}
		}
	}

	multimap<double, double, greater<double> > res;

	int allPixels = includeNoDataValues ? rows()*cols() : nops;
	if(includeNoDataValues)
	{
		double percentNoData = 
			Tools::round<double>(double(allPixels - nops)/double(allPixels)*100.0, roundResultToDigits);
		
		if(int(percentNoData) != 0)
			res.insert(make_pair(percentNoData, double(noDataValue()))); 
	}
	
  for(Map::value_type p : m)
  {
		double percent = double(p.second)/double(allPixels)*100.0;
		double rp = round<double>(percent, roundResultToDigits);
		if(int(percent != 0))
			res.insert(make_pair(rp, double(p.first)/pow(10, roundValueToDigits)));
	}

	return res;
}
*/

GridP* GridP::setAllFieldsTo(double newValue, bool keepNoData)
{
  for(int i = 0; i < rows(); i++)
    for(int k = 0; k < cols(); k++)
			if(!isNoDataField(i, k) || !keepNoData)
				setDataAt(i, k, newValue);
	return this;
}

GridP* GridP::setAllFieldsWithoutTo(float withoutValue, float toNewValue, bool includeNoData)
{
	for(int r = 0, rs = rows(); r < rs; r++)
		for(int c = 0, cs = cols(); c < cs; c++)
			if((includeNoData || isDataField(r, c)) && !fuzzyCompare(dataAt(r, c), withoutValue))
				setDataAt(r, c, toNewValue);
	return this;
}

GridP::Rc2RowColRes GridP::rc2rowCol(Tools::RectCoord rc) const
{
	grid& g = gridRef();
	int row = -1, col = -1;

  bool rowsInside = g.xcorner <= rc.r && rc.r <= (g.xcorner + cellSize()*cols());
  bool colsInside = g.ycorner <= rc.h && rc.h <= (g.ycorner + cellSize()*rows());

  col = int(std::floor((rc.r - g.xcorner)/cellSize()));
  if(col == cols())
    --col;
  row = rows() - int(std::ceil((rc.h - g.ycorner)/cellSize()));
  if(row == rows())
    --row;

  return Rc2RowColRes(row, col, !rowsInside, !colsInside);
}

float GridP::dataAt(RectCoord rcc) const
{
  auto p = rc2rowCol(rcc);
  return p.row < 0 || p.col < 0
    ? noDataValue() : dataAt(p.row, p.col);
}

GridP* GridP::setDataAt(Tools::RectCoord rcc, float value)
{
  auto p = rc2rowCol(rcc);
  if(p.row >= 0 && p.col >= 0)
    setDataAt(p.row, p.col, value);
	return this;
}

string GridP::toString() const
{
	ostringstream s;
  for(int i = 0; i < rows(); i++)
  {
    for(int k = 0; k < cols(); k++)
    {
			s << dataAt(i, k) << " ";
		}
		s << "\n";
	}
	return s.str();
}

RCRect GridP::rcRect() const
{
	return GridMetaData(this).rcRect();
}

RCRect GridP::cellRCRectAt(int row, int col) const
{
	grid& g = gridRef();
	RectCoord tl(coordinateSystem(),
							 g.xcorner + col*cellSize(),
							 g.ycorner + (rows() - row -1 + 1)*cellSize());
	RectCoord br(coordinateSystem(),
							 g.xcorner + (col + 1)*cellSize(),
							 g.ycorner + (rows() - row - 1)*cellSize());

	return RCRect(tl, br);
}

RectCoord GridP::rcCoordAt(int row, int col) const
{
	grid& g = gridRef();
	return RectCoord(coordinateSystem(),
									 g.xcorner + col*cellSize(),
									 g.ycorner + (rows() - row - 1)*cellSize());
}

RectCoord GridP::rcCoordAtCenter(int row, int col) const
{
	grid& g = gridRef();
	return RectCoord(coordinateSystem(),
									 g.xcorner + col*cellSize() + cellSize()/2.0,
									 g.ycorner + (rows() - row - 1)*cellSize() + cellSize()/2.0);
}

RectCoord GridP::lowerLeftCorner() const
{
	grid& g = gridRef();
	return RectCoord(coordinateSystem(), g.xcorner, g.ycorner);
}

RectCoord GridP::lowerLeftCenter() const
{
	return lowerLeftCorner() + cellSize()/2.0;
}

GridP* GridP::subGridClone(int top, int left, int nrows, int ncols) const
{
	GridP* subGrid = new GridP(datasetName(), nrows, ncols, cellSize(),
														 _grid->xcorner + left*cellSize(),
														 _grid->ycorner + (rows() - top)*cellSize(),
														 noDataValue(),
														 coordinateSystem());

  for(int i = top, j = 0; i < top + nrows; i++, j++)
  {
    for(int k = left, l = 0; k < left + ncols; k++, l++)
    {
			float data = dataAt(i, k);
			if(data == noDataValue())
				subGrid->_grid->has_nodata = 1;
			subGrid->setDataAt(j, l, data);
		}
	}

	return subGrid;
}

GridP* GridP::fillClone(double fillValue, bool keepNodata) const
{
	GridP* c = clone();
	c->setAllFieldsTo(fillValue, keepNodata);
	return c;
}

string GridP::descriptiveLabel() const
{
	return _descriptiveLabel.empty()
		? Tools::capitalize(datasetName()) + " [" + unit() + "]"
		: _descriptiveLabel;
}

std::pair<double, double> GridP::minMax() const
{
	if(rows() < 1 && cols() < 1)
		return make_pair(0.0, 0.0);

	double min = 0;
	for(int i = 0; i < rows(); i++)
  {
		for(int j = 0; j < cols(); j++)
    {
      if(isDataField(i, j))
      {
        min = dataAt(i, j);
        goto foundValidValue;
      }
    }
  }
  foundValidValue:
	double max = min;

  for(int i = 0; i < rows(); i++)
  {
    for(int j = 0; j < cols(); j++)
    {
      if(isDataField(i, j))
      {
				double v = dataAt(i, j);
				if(v < min) min = v;
				if(v > max) max = v;
			}
		}
	}

	return make_pair(min, max);
}

bool GridP::isCompatible(const GridP* other) const
{
	return cols() == other->cols() && rows() == other->rows();
}

/*
GridP* GridP::subtract(const GridP* other) const {
	if(!isCompatible(other)) return NULL;

	GridP* res = clone();
	for(int i = 0; i < rows(); i++){
		for(int j = 0; j < cols(); j++){
			if(isDataField(i, j) && other->isDataField(i, j))
				res->setDataAt(i, j, dataAt(i, j) - other->dataAt(i, j));
			else
				res->setDataAt(i, j, noDataValue());
		}
	}
	return res;
}
*/

double GridP::average() const
{
	double sum = 0;
	int count = 0;
  for(int r = 0, rs = rows(); r < rs; r++)
  {
    for(int c = 0, cs = cols(); c < cs; c++)
    {
      if(isDataField(r, c))
      {
				sum += dataAt(r, c);
				++count;
			}
		}
	}
	return sum / double(count);
}

GridP* GridP::transformInPlace(std::function<float(float)> transformFunction)
{
  for(int r = 0, rs = rows(); r < rs; r++)
    for(int c = 0, cs = cols(); c < cs; c++)
      if(isDataField(r, c))
        setDataAt(r, c, transformFunction(dataAt(r, c)));
	return this;
}

GridP* GridP::transformP(std::function<float(float)> transformFunction) const
{
	GridP* res = clone();
  res->transformInPlace(transformFunction);
	return res;
}

GridP* GridP::invert(float value)
{
  for(int r = 0, rs = rows(); r < rs; r++)
  {
    for(int c = 0, cs = cols(); c < cs; c++)
    {
      if(isNoDataField(r, c))
        setDataAt(r, c, value);
      else
        setNoDataValueAt(r, c);
    }
  }
	return this;
}

GridP* GridP::setFieldsTo(const GridP* other, bool keepNoData)
{
  if(!isCompatible(other))
		return this;

  for(int r = 0, rs = other->rows(); r < rs; r++)
  {
    for(int c = 0, cs = other->cols(); c < cs; c++)
    {
      if(isNoDataField(r, c) && keepNoData)
        continue;
      setDataAt(r, c, other->dataAt(r, c));
    }
  }

	return this;
}

GridP* GridP::maskOut(const GridP* maskGrid, float byValueInMaskGrid, bool keep)
{
  bool discard = !keep;

  if(!isCompatible(maskGrid))
		return this;

  for(int r = 0, rs = maskGrid->rows(); r < rs; r++)
  {
    for(int c = 0, cs = maskGrid->cols(); c < cs; c++)
    {
      bool found = fuzzyCompare(maskGrid->dataAt(r, c), byValueInMaskGrid);
      if((found && discard) || (keep && !found))
				setNoDataValueAt(r, c);
    }
  }

	return this;
}

GridP* GridP::maskTo(const GridP* maskGrid, float matchMaskValueTo,
									 float newValue, bool keepNoData)
{
	bool replaceNoData = !keepNoData;

	if(!isCompatible(maskGrid))
		return this;

	for(int r = 0, rs = maskGrid->rows(); r < rs; r++)
		for(int c = 0, cs = maskGrid->cols(); c < cs; c++)
			if((isDataField(r, c) || replaceNoData)
					&& fuzzyCompare(maskGrid->dataAt(r, c), matchMaskValueTo))
				setDataAt(r, c, newValue);

	return this;
}

GridP* GridP::adjustToP(GridMetaData gmd) const
{
  //the result
  GridP* res = new GridP(gmd, datasetName());

  //just return empty grid if they don't even intersect
	RCRect ir = rcRect().intersected(res->rcRect());
  if(ir.isEmpty())
    return res;

  GridPPtr transSelf;
  const GridP* self = this;

  //make both grids the same cellsize
  int cs = int(cellSize());
  int mcs = int(res->cellSize());
  if(cs > mcs)
  {
    if(cs % mcs == 0)
    {
			transSelf = GridPPtr(new GridP(gridPtr()->downscale(cs / mcs),
																		 coordinateSystem()));
      self = transSelf.get();
    }
    else
      return NULL;
  }
  else if(cs < mcs)
  {
    if(mcs % cs == 0)
    {
			transSelf = GridPPtr(new GridP(gridPtr()->upscale(mcs / cs),
																		 coordinateSystem()));
      self = transSelf.get();
    }
    else
      return NULL;
  }

  for(int h = ir.tl.h, hs = ir.br.h; h > hs; h -= mcs)
  {
    for(int r = ir.tl.r, rs = ir.br.r; r < rs; r += mcs)
    {
			RectCoord rcc(coordinateSystem(), r, h);
			res->setDataAt(rcc, dataAt(rcc));
    }
  }

  return res;
}

/*
int GridP::countDataFields() const
{
  //float ndv = noDataValue();
  function<int(int,float)> f =
      ret<int>(if_then_else_return(ll_static_cast<int>(_2) == noDataValue(),
                                   _1, _1 + 1));
  return foldF(0, f);
}
*/

vector<double> Grids::allDataAsLinearVector(const grid* g)
{
  vector<double> linear;
  for(int r = 0, rs = g->nrows; r < rs; r++)
    for(int c = 0, cs = g->ncols; c < cs; c++)
      if(g->feld[r][c] != g->nodata)
        linear.push_back(g->feld[r][c]);

  return linear;
}

//------------------------------------------------------------------------------

string GridProxy::toString() const
{
	ostringstream s;
	s << "gfn: (" << fileName << ") hfn: (" << hdfFileName << ") modT: ("
		<< ctime(&modificationTime) << ")";// << " (" << ((long int)modificationTime) << ")";
	return s.str();
}

GridP* GridProxy::gridPtr()
{
  if(!g)
  {
		Lock lock(this);
    if(!g)
    {
			if(!pathToHdf.empty())
				g = GridPPtr(new GridP(datasetName, GridP::HDF,
															 pathToHdf + "/" + hdfFileName,
															 coordinateSystem));
			else
				g = GridPPtr(new GridP(datasetName, GridP::ASCII,
															 pathToGrid + "/" + fileName,
															 coordinateSystem));
		}
	}
	return g.get();
}

GridPPtr GridProxy::gridPPtr()
{
	if(!g)
		gridPtr();
	
	return g;
}

void GridProxy::resetToLoadFromAscii(const string& ptg)
{
	pathToHdf = "";
	hdfFileName = "";
	pathToGrid = ptg;
}
