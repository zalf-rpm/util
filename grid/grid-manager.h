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

#ifndef GRIDMANAGER_H_
#define GRIDMANAGER_H_

#include <vector>
#include <string>
#include <map>
#include <ctime>
#include <utility>

#include "grid+.h"
#include "tools/coord-trans.h"
#include "types.h"
#include "tools/datastructures.h"

namespace Grids
{
	typedef Tools::Quadruple<Tools::LatLngCoord> LatLngPolygon;
	typedef Tools::StdMatrix<LatLngPolygon> LatLngPolygonsMatrix;

	//! abstract class being the interface to the user
	class VirtualGrid
	{
	public:
		struct Data
		{
			Data() : _isNoData(true), _gps(NULL), _row(0), _col(0) {}
			Data(const std::vector<GridProxyPtr>* grids,
					unsigned int row, unsigned int col)
			: _isNoData(false), _gps(grids), _row(row), _col(col) {}
			bool isNoData() const { return _isNoData; }
			const std::vector<GridProxyPtr>* gridProxies() const { return _gps; }
			unsigned int row() const { return _row; }
			unsigned int col() const { return _col; }
		private:
			bool _isNoData;
			const std::vector<GridProxyPtr>* _gps;
			unsigned int _row;
			unsigned int _col;
		};

	public:
		VirtualGrid(Tools::CoordinateSystem cs,
								const Grids::RCRect& rect, double cellSize,
								unsigned int rows, unsigned int cols, int noDataValue = -9999)
			: _noDataValue(noDataValue),
				_rect(rect),
				_cellSize(cellSize),
				_rows(rows),
				_cols(cols),
				_coordinateSystem(cs)
		{}

		virtual ~VirtualGrid();

		//! get RC coord at the given cell position (corner)
		Tools::RectCoord rcCoordAt(unsigned int row, unsigned int col) const
		{
			return Tools::RectCoord(_coordinateSystem,
															_rect.tl.r+(col*_cellSize),
			                       _rect.tl.h-(row*_cellSize));
		}

		//! get data at given grid cell
		/*!
		 * - could potentially be data from overlapping grids
		 * -> that's why it's a vector, even though currently not used
		 * @param row
		 * @param col
		 * @return
		 */
		virtual std::vector<Data>
		dataAt(unsigned int row, unsigned int col) const = 0;

		virtual std::vector<Data>
		dataAt(const Tools::RectCoord& rcc) const = 0;

		//! number of rows of virtual grid
		unsigned int rows() const { return _rows; }

		//! number of columns of the virtual grid
		unsigned int cols() const { return _cols; }

		//! used value for representing "no data" for the virtual grid
		int noDataValue() const { return _noDataValue; }

		//! cell size
		double cellSize() const { return _cellSize; }

		//! rc rectangle represented by the virtual grid
		RCRect rcRect() const { return _rect; }

		//! bounding rect polygon for the whole virtual grid
		LatLngPolygon latLngBoundingPolygon() const
		{
			return Tools::RC2latLng(_rect.toTlTrBrBlVector());
		}

		//! get a polygon matrix for all the cells
		const LatLngPolygonsMatrix& latLngCellPolygons();

		//! should the cell resolution be used
		bool useCellResolution() const { return (_rows * _cols) < (15 * 15); }

		//! virtual grid keeps ownership of GridP's
		virtual std::vector<const GridP*> availableGrids();

		//! a short description for the virtual grid
		virtual std::string toShortDescription() const;

		//! are the datasets in the given list available for this virtual grid
    bool areGridDatasetsAvailable(const std::list<std::string>& gdsns);// const;

		//! return grids for the given datasetname list
		/*!
		 * keeps ownership of the returned grids
		 * @param datasetNames
		 * @return map from dataset names to the according grid
		 */
		virtual std::map<std::string, const GridP*>
    gridsForDatasetNames(const std::list<std::string>& datasetNames);// const;

		GridPPtr emptyGridPtr() const { return GridPPtr(emptyGrid()); }

		GridP* emptyGrid() const
		{
			return new GridP("template", rows(), cols(), cellSize(),
											 rcRect().tl.r, rcRect().br.h, noDataValue(),
											 coordinateSystem());
		}

		std::string name() const
		{
			return _name.empty() ? toShortDescription() : _name;
		}

		void setName(std::string name){ _name = name; }

		std::string customId() const { return _customId; }

		void setCustomId(std::string cid) { _customId = cid; }

		virtual bool isNoVirtualGrid() const { return false; }

		Tools::CoordinateSystem coordinateSystem() const { return _coordinateSystem; }

	protected:
		int _noDataValue;
		RCRect _rect;
		double _cellSize;
		unsigned int _rows;
		unsigned int _cols;
		std::vector<GridP*> _availableGrids; //!< vector of Grids

		LatLngPolygonsMatrix _cellPolygons;

		std::string _name;
		std::string _customId;
		Tools::CoordinateSystem _coordinateSystem;
	};

  typedef std::map<std::string, GridPPtr> Dsn2GridPPtr;

  class VirtualGrid2
  {
  public:
    VirtualGrid2(Tools::CoordinateSystem cs,
                 const Grids::RCRect& rect, double cellSize,
                 unsigned int rows, unsigned int cols,
                 std::map<std::string, GridPPtr> dsn2grid = std::map<std::string, GridPPtr>(),
                 int noDataValue = -9999)
      : _noDataValue(noDataValue),
        _rect(rect),
        _cellSize(cellSize),
        _rows(rows),
        _cols(cols),
        _dsn2grid(dsn2grid),
        _coordinateSystem(cs)
    {}

    virtual ~VirtualGrid2() {}

    //! get RC coord at the given cell position (corner)
    Tools::RectCoord rcCoordAt(unsigned int row, unsigned int col) const
    {
      return Tools::RectCoord(_coordinateSystem,
                              _rect.tl.r+(col*_cellSize),
                             _rect.tl.h-(row*_cellSize));
    }

    //! get data at given grid cell
    /*!
     * - could potentially be data from overlapping grids
     * -> that's why it's a vector, even though currently not used
     * @param row
     * @param col
     * @return
     */
    virtual std::map<std::string, double>
    dataAt(unsigned int row, unsigned int col) const;

    virtual std::map<std::string, double>
    dataAt(const Tools::RectCoord& rcc) const;

    //! number of rows of virtual grid
    unsigned int rows() const { return _rows; }

    //! number of columns of the virtual grid
    unsigned int cols() const { return _cols; }

    //! used value for representing "no data" for the virtual grid
    int noDataValue() const { return _noDataValue; }

    //! cell size
    double cellSize() const { return _cellSize; }

    //! rc rectangle represented by the virtual grid
    RCRect rcRect() const { return _rect; }

    //! bounding rect polygon for the whole virtual grid
    LatLngPolygon latLngBoundingPolygon() const
    {
      return Tools::RC2latLng(_rect.toTlTrBrBlVector());
    }

    //! get a polygon matrix for all the cells
    const LatLngPolygonsMatrix& latLngCellPolygons();

    //! should the cell resolution be used
    bool useCellResolution() const { return (_rows * _cols) < (15 * 15); }

    //! virtual grid keeps ownership of GridP's
    virtual std::vector<const GridP*> availableGrids();

    //! a short description for the virtual grid
    virtual std::string toShortDescription() const;

    //! are the datasets in the given list available for this virtual grid
    bool areGridDatasetsAvailable(const std::list<std::string>& gdsns);// const;

    //! return grids for the given datasetname list
    /*!
     * keeps ownership of the returned grids
     * @param datasetNames
     * @return map from dataset names to the according grid
     */
    virtual std::map<std::string, const GridP*>
    gridsForDatasetNames(const std::list<std::string>& datasetNames);// const;

    GridPPtr emptyGridPtr() const { return GridPPtr(emptyGrid()); }

    GridP* emptyGrid() const
    {
      return new GridP("template", rows(), cols(), cellSize(),
                       rcRect().tl.r, rcRect().br.h, noDataValue(),
                       coordinateSystem());
    }

    std::string name() const
    {
      return _name.empty() ? toShortDescription() : _name;
    }

    void setName(std::string name){ _name = name; }

    std::string customId() const { return _customId; }

    void setCustomId(std::string cid) { _customId = cid; }

    virtual bool isNoVirtualGrid() const { return false; }

    Tools::CoordinateSystem coordinateSystem() const { return _coordinateSystem; }

  protected:
    int _noDataValue;
    RCRect _rect;
    double _cellSize;
    unsigned int _rows;
    unsigned int _cols;

    Dsn2GridPPtr _dsn2grid;

    LatLngPolygonsMatrix _cellPolygons;

    std::string _name;
    std::string _customId;
    Tools::CoordinateSystem _coordinateSystem;
  };


	//----------------------------------------------------------------------------

	//! actually just a normal grid, but with a VirtualGrid face
	class NoVirtualGrid : public VirtualGrid
	{
	public:
		NoVirtualGrid(Tools::CoordinateSystem cs,
									const std::vector<GridProxyPtr>* gps,
									const Grids::RCRect& rect,
									double cellSize, unsigned int rows, unsigned int cols,
									int noDataValue = -9999);

    virtual std::vector<Data> dataAt(unsigned int row, unsigned int col) const
    {
      return std::vector<Data>(1, Data(_gps, row, col));
    }

    virtual std::vector<Data> dataAt(const Tools::RectCoord& rcc) const;

    virtual std::vector<const GridP*> availableGrids();

		virtual std::string toShortDescription() const;

		virtual bool isNoVirtualGrid() const { return true; }
	private:
		//! reference to gridmanager's vector holding the proxies
		const std::vector<GridProxyPtr>* _gps;
	};

  //----------------------------------------------------------------------------

  class NoVirtualGrid2 : public VirtualGrid2
  {
  public:
    NoVirtualGrid2(Tools::CoordinateSystem cs,
                  const std::vector<GridProxyPtr>* gps,
                  const Grids::RCRect& rect,
                  double cellSize, unsigned int rows, unsigned int cols,
                  int noDataValue = -9999);

    virtual std::vector<const GridP*> availableGrids();

    virtual std::string toShortDescription() const;

    virtual bool isNoVirtualGrid() const { return true; }
  private:
    //! reference to gridmanager's vector holding the proxies
    const std::vector<GridProxyPtr>* _gps;
  };

	//----------------------------------------------------------------------------

	//! a real virtual grid, possibly consisting of many
	class RealVirtualGrid : public VirtualGrid
	{
	public:
		typedef Tools::StdMatrix<std::vector<Data> > DataMatrix;
		typedef std::vector<std::vector<GridProxyPtr>*> VVGP;

	public:
		RealVirtualGrid(Tools::CoordinateSystem cs,
										const Grids::RCRect& rect, double cellSize,
										unsigned int rows, unsigned int cols,
										const VVGP& proxies, int noDataValue = -9999);

		~RealVirtualGrid();

		virtual std::vector<Data> dataAt(unsigned int row, unsigned int col) const;

		virtual std::vector<Data>
		dataAt(const Tools::RectCoord& /*rcc*/) const { return std::vector<Data>(); }

		void addDataAt(unsigned int row, unsigned int col, const Data& data)
		{
			_data[row][col].push_back(data);
		}

		virtual std::vector<const GridP*> availableGrids();

	private:
		DataMatrix _data;
		//! stores the proxy ptrs directly referenced in the Data elements
		std::vector<std::vector<GridProxyPtr>*> _usedGridProxies;
	};

	//----------------------------------------------------------------------------

	typedef std::string Path;

	//! manages all available grids in a system/the given paths
	class GridManager
	{
		typedef std::string FileName;

		typedef std::string PathToFile;

		typedef std::vector<GridProxyPtr> GridProxies;
		typedef std::map<GridMetaData, GridProxies> GMD2GPS;
		typedef std::map<Path, GMD2GPS> Path2GPS;
		typedef std::map<FileName, GridProxyPtr> GFN2GP;
		typedef std::map<Path, GFN2GP> Path2GP;


	public:
		struct Env {
			Env()
			: hdfsStorePath("hdfs"), hdfsIniFileName("hdfs.ini"),
			asciiGridsPath("grids"),
			noCheckFileName("DONT_CHECK_FOR_CHANGES"),
			regionalizationHdfsPath("regionalization-hdfs"),
			regionalizationIniFilePath("regionalization-hdfs/hdfs.ini") {}
			Env(const std::string& hsp, const std::string& hifn,
			    const std::string& agp, const std::string& ncfn,
			    const std::string& rhp, const std::string& rifp)
			: hdfsStorePath(hsp), hdfsIniFileName(hifn),
			asciiGridsPath(agp), noCheckFileName(ncfn),
			regionalizationHdfsPath(rhp), regionalizationIniFilePath(rifp) {}
			Path hdfsStorePath;
			FileName hdfsIniFileName;
			Path asciiGridsPath;
			FileName noCheckFileName;
			Path regionalizationHdfsPath;
			Path regionalizationIniFilePath;
		};

	public:
		GridManager(Env env);
		~GridManager();

    VirtualGrid2* createVirtualGrid(const Tools::Quadruple<Tools::LatLngCoord>& llrect,
                                    const Path& userSubPath = "general");

    /*
		VirtualGrid*
		createVirtualGrid(const Tools::Quadruple<Tools::LatLngCoord>& llrect,
											double cellSize, const Path& userSubPath = "general",
											Tools::CoordinateSystem targetCoordinateSystem = Tools::GK5_EPSG31469);
                      */
		
    VirtualGrid2* virtualGridForGridMetaData(const GridMetaData& gmd,
																						const Path& userSubPath = "general");

    VirtualGrid2* virtualGridForRegionName(const std::string& regionName,
																					const Path& userSubPath = "general");

		GridMetaData gridMetaDataForRegionName(const std::string& regionName,
																					 const Path& userSubPath = "general");

		GridPPtr gridFor(const std::string& regionName,
										 const std::string& datasetName,
										 const Path& userSubPath = "general",
										 int cellSize = 100,
										 GridMetaData subgridMetaData = GridMetaData());

		std::vector<GridPPtr> gridsFor(const std::string& regionName,
                                   std::set<std::string> datasetNames = std::set<std::string>(),
																	 const Path& userSubPath = "general",
																	 int cellSize = 100,
																	 GridMetaData subgridMetaData = GridMetaData());

		std::vector<std::vector<Tools::LatLngCoord> >
		regions(const Path& userSubPath = "general") const;
		
		std::vector<GridMetaData> regionGmds(const Path& userSubPath = "general") const;


		RegData regionalizedData(const std::string& region, const std::string& dataId) const;

		RegGroupOfData regionalizedGroupOfData(const std::string& region,
																					 const std::string& groupId) const;

		/*!
		* add new proxy to manager which loads the given gridfile
		* @param gridFileName
		* @param modificationTime ... optional the last modification time of the
		* grid
		* @param pathToGrid ... if grid is outside of standard grids dir
		* @return (datasetname, gridmetadata) created for grid
		*/
		std::pair<std::string, GridMetaData>
		addNewGridProxy(const Path& userSubPath,
										const std::string& gridFileName,
										time_t modificationTime = 0,
										const std::string& pathToGrid = std::string(),
                    Tools::CoordinateSystem cs = Tools::CoordinateSystem());

	private: //methods
		const RegDataMap* regionalizedData(const std::string& region) const;

		//! for given quadruple of coords create a virtual grid with given cellsize
		/*!
		 * - caller takes ownership of returned grid
		 * - the grid will be initialized with references to the potential
		 * grids available in the system and can be used to create
		 * particular virtual grids from the grid types available (eg. dgm, stt ...)
		 */
//		VirtualGrid* createVirtualGrid(const Tools::Quadruple<Tools::RectCoord>& rcrect,
//																	 double cellSize,
//																	 const Path& userSubPath = "general");



//		VirtualGrid* createVirtualGrid(const GMD2GPS& gmd2gridProxies,
//																	 const Tools::Quadruple<Tools::RectCoord>& rcrect,
//																	 double cellSize);

    VirtualGrid2* createVirtualGrid(const GMD2GPS& gmd2gridProxies,
                                   const Tools::Quadruple<Tools::LatLngCoord>& llrect);

		//! read all the regionalized data and create GridProxies for the maps
		void readRegionalizedData();

		//! read the mappings file and build up internal hdf store structure
		void readGrid2HdfMappingFile(const Path& userSubPath);

		//! write the mappingsfile
		void writeGrid2HdfMappingFile(const Path& userSubPath);

		//! get the modification time of the given file
		std::time_t modificationTime(const char* fileName);

		std::time_t modificationTime(const std::string& fileName)
		{
			return modificationTime(fileName.c_str());
		}

		//! filter out the dataset name from a grid file name
		std::string extractDatasetName(const std::string& gridFileName) const;

		//! filter the possible region name out of a grid file name
		std::string extractRegionName(const std::string& gfn) const;

    //! filter the possible coordinate system out of a grid file name
    Tools::CoordinateSystem extractCoordinateSystem(const std::string& gfn) const;

		//! extract metadata from a grid file
		GridMetaData extractMetadataFromGrid(const std::string& gridFileName,
                                         Tools::CoordinateSystem cs) const;// = Tools::GK5_EPSG31469) const;

		//! update the store by any changes to the grids available
		void updateHdfStore(const Path& userSubPath,
												const GridProxies& leftOverGrids);

		//! check if something changed in the store and update if necessary
		void checkAndUpdateHdfStore(const Path& userSubPath);

		//! append the given proxies to an existing hdf
		GridProxies appendToHdf(const Path& userSubPath,
														const GridProxies& proxies,
		                        const FileName& newHdfFileName);

		void init(const Path& userSubPath);

		int& hdfIdCount(const Path& userSubPath);

		GridPPtr createSubgrid(GridPPtr g, GridMetaData subgridMetaData,
													 bool alwaysClone = false);

	private: //state
		Env _env;

		Path2GPS _gmdMap;
		Path2GP _gridPathMap;

		Region2RegData _region2regData;

		Groups2Members _groups2members;

		std::map<Path, int> _userSubPathToHdfIdCount;
		int _hdfIdCount;
	};

	//----------------------------------------------------------------------------

	//! get gridmanager for initial env
	GridManager& gridManager(GridManager::Env initialEnv = GridManager::Env());
}

#endif
