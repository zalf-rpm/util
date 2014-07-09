#ifndef __INTERPOL_H_
#define __INTERPOL_H_


// Wettreg station definition

#include<vector>
#include<map>
#include <iterator>
#include <iostream>
#include <fstream>
#include <string>
#include "db.h"
#include <math.h>
#include <float.h>
#include <stdlib.h>
#include <grid/grid.h>
#include <vector>
#include <map>
#include <iterator>
#include <iostream>
#include <math.h>
#include <float.h>
#include <stdlib.h>


using namespace std;

class weather{
 public:
  weather(int,int,int,int);  // dat_id, SZ={0,11,22,...,99}, begin, end
  ~weather();
  double get_TX(int); // yearday 
  double get_TM(int); // yearday 
  double get_TN(int); // yearday 
  double get_RR(int); // yearday 
  double get_SD(int); // yearday 
  double get_FF(int); // yearday 
  double get_RF(int); // yearday 
  double get_GS(int); // yearday 
  double dist(double,double);  // distance to station hw,rw
  double nn;  // nn above sealevel
  double rw;  // coordinate r GK5
  double hw;  // coordinate h GK5
  double residium; // for interpolation
 private:
  int dat_id;
  vector<double> TX;  // Tmax
  vector<double> TM;  // Tmean
  vector<double> TN;  // Tmin
  vector<double> RR;  // presipitation
  vector<double> SD;  // sun
  vector<double> FF;  // wind
  vector<double> RF;  // humiditiy
  vector<double> GS;  // global radiation
  int yearday;
  int SL;             // station bearing
  int klim;           // 1 if climate station | 0 if precipitation
};

class interpolation{
 public:
  interpolation() : initialized(false) {}
  interpolation(grid*,grid*);  // dem,voronoi as grid
  ~interpolation();
  void add_stat(int,int,int,int); // id,sz,begin,end
  double get_TX(int,double,double,double); // yearday, hw,rw,nn 
  double get_TM(int,double,double,double); // yearday, hw,rw,nn 
  double get_TN(int,double,double,double); // yearday, hw,rw,nn 
  double get_RR(int,double,double,double); // yearday, hw,rw,nn 
  double get_GS(int,double,double,double); // yearday, hw,rw,nn 
  double get_SD(int,double,double,double); // yearday, hw,rw,nn 
  double get_FF(int,double,double,double); // yearday, hw,rw,nn
  double get_RF(int,double,double,double); // yearday, hw,rw,nn
  void map_TM(int);
  void map_TX(int);
  void map_TN(int);
  void map_FF(int);
  void map_SD(int);
  void map_GS(int);
  void map_RR(int);
  grid* igrid;
  bool initialized;
 private:
  vector<weather*> stv;
  grid* dgm;
  grid* voronoi;

};

#endif
