/**
Authors:
Ralf Wieland <ralf.wieland@zalf.de>

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

// Grundlegende Grid-Funktionalitaet, wie Einlesen, Schreiben, als
// Bild ausgeben
// wird schrittweise um Funktionen zur Gridberechnung erweitert
// Programmierer: Dr. Ralf Wieland
// Version 0.1 21.8.2001 ascii, combine, fuzzy
// Version 0.2 19.3.2002 kohonen, cluster, hdf
// Version 0.3 5.1.2004 review and
// Version 0.4 29.6.2004 line
// Version 0.5 27.2.2006 power, fractal, svd
// Version 0.6 25.10. 2006 watershed
// Version 0.7 26.3.2007 wavelet
// Version 0.8 10.4.2007 hill climbing cluster algorithm
// Version 0.9 21.12.2007 kadane algortihm and some maintain
// Version 1.0 21.2.2008 new wavlet implementation

#ifndef _GRID_H_
#define _GRID_H_

#include <stdio.h>
#include <math.h>
#include <float.h>
#include <stdlib.h>
#include <time.h>
#include <string>
#include <list>
#include <iterator>
#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <cmath>
#include "hdf5.h"
//#include <fftw3.h>

using namespace std;

// class QWidget;
// class QPaintEvent;

// Anschlussklasse zum HDF-Modul:
// HDF ist ein effizientes Datenformat, das neben den Daten
// (in kompakter binaerer Form, auch gepackt) auch Metadaten,
// wie raeumliche Lage, nodata, gridsize, Autor, Modell, Datum
// speichern kann. Damit werden Zugriffe gegenueber dem ASCII-
// Format von 10s auf 0.5s fuer ein Grid 1000*1000 beschleunigt
// Ueber die Metadaten koennen dann auch Modellabhaenigkeiten
// eingefuehrt werden, so dass eine Berechnung nach "Bedarf"
// erfolgen kann

#define __HDF5
#ifdef __HDF5
class hdf5{
   public:
    hdf5();
    ~hdf5();
    int open_f(const char*);                          // hdf5_name
		int closeFile();
    int create_f(const char*);
    int open_d(const char*);                          // dataset_name

    static std::list<std::string> allDatasetNames(const char* fileName);

    // data read/write
    void write_i_feld(const char*,int*,int,int);      // name,feld[NX*NY],NX,NY
    void write_f_feld(const char*,float*,int,int);
    int* read_i_feld(const char*);
    float* read_f_feld(const char*);
    // attributes
    int write_s_attribute(const char*,const char*);         // attribute_name,data
    int write_f_attribute(const char*,float);
    int write_d_attribute(const char*,double);
    int write_i_attribute(const char*,int);
    int write_l_attribute(const char*,long);
    char* get_s_attribute(const char*);               // attribute_name
    float get_f_attribute(const char*);
    double get_d_attribute(const char*);
    int get_i_attribute(const char*);
    long get_l_attribute(const char*);
    int size;
    int* i1;
    float* f1;
protected:
    hid_t file, dataset;
};
#endif

// Hauptklasse der Bibliothek
// Sie erlaubt das Einlesen und Speichern von ASCII-Grids und
// selbiges auch fuer HDF-Files. Damit ist auch eine Konvertierung
// sichergestellt. Weiterhin lassen sich Grids als Graphiken
// im PNM-Format ausgeben (SW oder COLOR). Wesentliche Teile
// der Grid-Klasse dienen der Skalierung, Normierung und Kombination
// unterschiedlicher Grids. Die Kombination unterstuetzt die wichtigsten
// Verfahren, wie Addition, Multiplikation, Mittelwert und MIN,MAX.
// Diese Verfahren werden haeufig benoetigt und bilden eine
// Voraussetzen fuer anspruchsvollere Verfahren, wie Fuzzy oder
// Kohonen

struct ipoint{
    int x;
    int y;
};

struct part{
    int xa;
    int xb;
    int ya;
    int yb;
    double var;
};

class point;
class line;
class stack2i;
class tree;

void grid_save_to_R(char*,int);   // filename, number of bins

class grid{
 public:
    grid(int);              // Rastergroesse in m (10,50,100,500,1000)
    grid(int,int);          // Rand_grid of size nrows, ncols
    ~grid();

// read in points an put it in the grid
    int read_ascii(const char*);   // ASCII-Grid name
    int read_ascii_inv(const char*);   // ASCII-Grid name
    void write_ascii(char*);   // ASCII-Grid name
    void write_ascii_inv(char*);   // ASCII-Grid name y=nrows-y
    bool write_hdf(char*,char*,char*,char*);
    // hdf-File_name, datasetname, Autor, Modell
    int read_hdf(char*,char*);
    // hdf-File_name, datasetname
    void write_pnm(char*,int); // filename, (0,1) 0=bw 1=f
    void write_pnm_inv(char*,int); // filename, (0,1) 0=bw 1=f
    void write_pnm_b(char*,int); // filename, (0,1) 0=bw 1=f
    void write_pnm_inv_b(char*,int); // filename, (0,1) 0=bw 1=f
    grid* upscale(int);      // generiert ein 100m grid aus 500m
    grid* downscale(int);      // und umgekehrt
    grid* downscale_s(int);    // bildet die Summe, nicht den Mittelwert
    grid* zoom();            // zooming a grid with 2
    grid* shepard(int,int,int,float);    // shepard interpolation
    grid* select(int,int,int,int); // selectiert ein Teilfeld
    grid* select(float); // selects only values=float
    // lu,rl  (x,y)-linke obere Ecke (x,y)-rechte untere Ecke
    void stat();          // berechnet min,max,mean,std
    int* hist(int);     // Histogramm (MAX-MIN)/BINS
    void class_grid(float,float,float); // min max step
    void set_value(float); // set all values
    void exchange(float,float); // exchange value1 with value2
    void rand_value(); // set all values random
    void calc_pattern(float); // clculate a presens/absens statistic
    void add_value(float); // add value to grid
    void mul_value(float); // mul value to grid
    void cut(float,float,float);   // removes values not in [min..max]
    void cut_off(float,float,float); // removes values  in [min..max]
    void cut_fuzzy(float,float);   // removes values not in [min..max]
    void grid_log();       // calculates the brigg log
    void grid_fabs(); // sqrt(x*x)
    void grid_kadane(float); // selects the brightest region
    float get_xy(int,int);
    void set_xy(int,int, float);
    // variance analysis of a grid with moving window
    double variance(int step);
    // Fuer Dr. Thinh
    float moore(int,int,int,int);
    // Moore-Nachbarschaft von i,j mit Radius r, paramter
    void nachbarmatrix(grid*,int,float); // radius, thres
    void naehematrix(grid*,int,float);   // radius, thres
    void kompaktheit(grid*,int);         // radius
    void attraktivitaet(grid*,int,int,float,float,float);
    // im,jm (Mittelpunkt) alpha,sigma,gamma
    // some algorithms for erosionsmodelling
    void w_fill();         // fills sink cells in the grid
    grid* w_focalflow();   // calculates a possible in flowdirection
    grid* w_d8();     // calc flow from dir and elevation
    // e:1 se:2 s:4 sw:8 w:16 nw:32 n:64 ne:128
    grid* w_flowdirection(); // calculates the steepest decent of outflow
    // e:1 se:2 s:4 sw:8 w:16 nw:32 n:64 ne:128
    grid* grid_copy();    // grid 1:1 kopieren
    void norm_grid();     // grid in [0 1] range
    void norm_grid(float,float);  // grid in [0 1] range from min to max
    void norm_grid1();     // grid in [0 1] range +- 3*std as cut
    void norm_grid2();     // (x-xmean)/std
    void inv_grid();      // inverts a grid
    int  cov_grid(grid*); // calculates the variance and covariance
    int  combine_grid(grid*,int);
    int  combine_or(grid*);
    grid* cluster2value(grid*); //
    grid* combine_grid(float (*f)(float));
    grid* combine_grid(grid*,float (*f)(float,float));
    grid* combine_grid(grid*,grid*,float (*f)(float,float,float));
    grid* distance(float); // distance grid to a selected value
    grid* difference(grid*); // difference between two grids
    grid* sobol();         // some filter algorithm
    grid* edge();          // edge detection algoritm
    grid* laplace();       // laplacian filter
    //grid* power();         // power spectrum algorithm
    //grid* svd(int,int);    // singular value decompositon smin, smax
    //grid* filter(double,double); // band pass filter based on fourrier
    grid* n2();            // cuts a quadratic region with a=2^n
    //grid* wavelet_h(int,int);// haar wavelet decompositon smin, smax
    //grid* wavelet_d(int,int,int);// daubechies wavelet decompositon smin,smax,k
    //grid* wavelet_s(int,int,int);// spline wavelet decompositon smin,smax,k
    //grid* wavelet_wh(); // haar wavelet coefficients
    //grid* wavelet_wd(int); // daubecies wavelet coefficients
    //grid* wavelet_ws(int); // spline wavelet coefficients
    grid* fractal(int);    // fractal decomposition with number of iter
    //grid* convolution(int); // convoultion of a grid with a gaussian bell shape
    //grid* akf();           // auto corelation function of a grid
    grid* rotate(int);   // rotation of a grid with angle ß
    grid* visuability(point*);
    grid* flood_fill(grid*,int,int,float);
    grid* tree_grid(float); // tree mosaic algorithm
// flood fill algorithm wit stack
    grid* delta_diff(grid *g1); // fabs(gx-g1)/gx
    point *sample(int number, int flag); // generates a sample
    // altes Grid=combine_grid(grid,selector,flag)
    int ncols,nrows;        // Zeilen,Spalten
    float min,max;          // Min und Maximum der Feldelemente
    float gridmax, gridmin, gridmean, gridstd; // Ergebnisse stat
    int count_data, count_nodata;
    float obv,ebv,sigmabv;
    float variance1,variance2,covariance;
    int minx,miny,maxx,maxy; // Ergebnisse stat Koordinaten
    float **feld;           // Feld[nrows][ncols]
    int has_nodata;         // yes=1 no=0 unknown=-1
    int nodata;
    int rgr;                // Rastergroesse
    double xcorner;         // header Infos from ASCII-Grid
    double ycorner;
    float csize;
    hdf5 *hd;
    double *dfeld;
    bool variance_flag;
};

class stack2i{
 public:
    stack2i(int);
    ~stack2i();
    bool pop(int &x, int &y);
    bool push(int x, int y);
    void emptyStack();
 private:
    int *feldx,*feldy;
    int stackpointer;
    int stackSize;
};

struct valpair{
    int n;
    double *x;
    double *y;
};


class point{
 public:
    point();
    ~point();
    int read_point(char*);
    grid* p2g(grid *, double r); // convertes a point theme in a grid
    grid* p2g_shepard(grid *, double r, double mu);
    grid* p2g_voronoi(grid *);
    // convertes a point theme in a grid using shepard method
    int write_point(char*);
    void set_point(double,double,double);
    void clean_point();
    struct valpair* gamma();  // calculates a sorted list of variances
    double **feld;
    int length;
    int alength;
    double xmin,xmax,ymin,ymax,zmin,zmax;
    struct valpair *vpp;
};

class line{
 public:
    line();
    ~line();
    int read_line(char*);
    grid* l2g(grid *); // convertes a point theme in a grid
    vector<double>x;
    vector<double>y;
    vector<double>z;
    vector<int>flxname;
    vector<int>id;
};

grid* read_xyz(const char*,grid* g1);

// Die Klasse fuzzy entstand aus der Forderung unscharfe Verscheidnungen
// effizient und transparent durchfuehren zu koennen. Die nutzt die
// oben eingefuehrten grids und gibt die Ergebnisse ebenfalls als
// grids zurueck.

struct sensor{
    float a;
    float b;
    float c;
    int flag;
};

// Kohonennetze fungieren als intelligenter Klassifikator, der
// sich besonders zur Detektion von Uebergaengen innerhalb der
// Daten eignet. Kohonennetze klassifizieren Massendaten besonders
// fein, vernachlaessigen aber Randdaten.
// Auch diese Klasse arbeitet wieder mit grids und erzeugt neue
// grids.
// Weiterhin koennen die Klassenschwerrpunkte als File gesichert werden.

class kohonen{
 public:
    kohonen(int,int,int);    // number of inputs, x,y-size
    ~kohonen();
    grid* calc(grid*,long int);  // grid, number of iterations
    grid* calc(grid*,grid*,long int);
    grid* calc(grid*,grid*,grid*,long int);
    void save(char*);       // save a net with name
    void save_hasse(char*,int);       // save a net with name
    struct sensor **sf;     // field of sensors
    int rows,cols;          // size of field
    int x,y;                // location
    float find(float);       // set x,y of max match element
    float find(float,float);
    float find(float,float,float);
 protected:
    float h(int,int,float); // h=f(x,y,sigma);
    float eps(int,float);
 private:
    float K0;               // fade out of train
    int inputs;
    double **mask;          // mask for h
    int a,b;                // size of x and y of mask
};

// Dieser Clusteralgorithmus versucht alle Daten moeglichst gut
// abzudecken. Dazu wird von einer zufaelligen Anfangsverteilung
// der Klassenzentren ausgegangen, die dann schrittweise verfeinert
// wird. Auch hier besteht wieder die Moeglichkeit die Klassen-
// zentren in ein File zu speichern

class cluster{
 public:
    cluster(int,grid*,int);
    cluster(int,grid*,grid*,int);
    cluster(int,grid*,grid*,grid*,int);
    ~cluster();
    void save(char*); // write out the centres of clusters
    void save_hasse(char*,int);       // save a net with name
    grid* get_cluster();      // get a new grid with cluster_nr
    struct sensor *sf;
 private:
    int **feld;
    int nrows, ncols, nr;
    struct sensor *sf1;
    int nodata;
    int rgr;                // Rastergroesse
    double xcorner;         // header Infos from ASCII-Grid
    double ycorner;
    float csize;
    int inputs;
};

// Clusteralgorithm using hill climbing

class cluster_hill{
 public:
    cluster_hill(int);  // number of grid cells
    ~cluster_hill();
    void load_grid(grid*); // loads a new grid
    void calc(int nLoops);
    void save(char*); // write out the centers of cluster
    grid* get_cluster();
 private:
    int nClusters;
    int nGrids;
    double *Variances;
    int *nMembers;
    grid **grids;  // list of grids
    int **cFeld;    // cluster space feld[i][j]->cluster
    double **Centroids; // centroids[iCluster][iGrid];
};

int compare(const void*, const void*);

// Die Klasse region erlaubt die Berechnung einer Umgebung
// um ein grid. Zurueckgegeben wird ebenfalls ein Grid.

class region{
 public:
    region(int,int);             // generates one mask wit radius=R
    ~region();                   // and parameter CIRCLE, MOORE, MOORE1
    void calc_count(grid*,grid*,int); // counts if val==value
    void calc_sum(grid*,grid*);       // grid:=sum(grid & mask)
    void calc_mean(grid*,grid*);      // grid:=mean(grid & mask)
    void calc_median(grid*,grid*);    // grid:=mean(grid & mask)
    void calc_std(grid*,grid*);       // grid:=std(grid & mask)
    void calc_min(grid*,grid*);       // grid:=min(grid & mask)
    void calc_max(grid*,grid*);       // grid:=max(grid & mask)
    void calc_hist(grid*,grid*);      // grid:=hist(grid & mask)
    void calc_apen(grid*,grid*,int);  // grid:=apen(grid & maks), int=m
 private:
    int** maske;
    int radius, ncols, nrows;
    double apen(vector<double>,int,double); // apen(pattern,m,r)
};


// splatter technique
struct splatter_field{
    float x;
    float y;
    float z;
    float s;
};

extern int splatter(grid *,grid *,grid *,grid *,int,struct splatter_field*);

// special struct for rotate and lattice gas

struct vertex{
    double a;
    double b;
};

struct triple{
    double x;
    double y;
    double z;
};

// new tree algorithm for mosaic search

typedef map<int,int> CodeType;
typedef CodeType::value_type ValuePair;

class tree{
 public:
    tree(grid*);
    ~tree();
    grid* map_tree();
    void print_tree();
    void thresh(double);
    void deep_search(int,int,double);
    void print_assoc();
 private:
    grid* g1;
    long** lmatrix;
    CodeType code;
    CodeType decode;
    int n;   // n=number of rows and columns
    long total;
    vector<int> assoc;
};

// Die Klasse water ist als experimental zu betrachten und soll
// zu einem zuverlaessigen Modell fuer die Wassererosion
// fuehren. Sie wurde hier mit aufgenommen, da sie auch auf
// grids basiert.

// wesentliche Teile des Algortihmus sind an einen modifizierten
// D8-Algorithmus angelehnt. Ausgegangen wird von einer
// Moore-Umgebung. Ein Tropfen kann ein beliebiges Gitter treffen.
// Trifft ein Tropfen erstmalig auf einen Gitterpunkt, so erfolgt die
// Wegwahl zufaellig, wobei die Wahrscheinlichkeiten entsprechend
// der Steilheit der Abstiege geordnet sind. Trifft ein Tropfen
// auf einen schonmal getroffenen Gitterpunkt, so folgt er dem
// frueher berechneten Weg. Damit ergeben sich effektive Stromlinien
// die in einer flux-Berechnung zur Ermittlung des Abtrages nutzbar
// sind.

/* struct flux{ */
/*     float ol; */
/*     float o; */
/*     float or; */
/*     float l; */
/*     float r; */
/*     float ul; */
/*     float u; */
/*     float ur; */
/* }; */

/* struct level{ */
/*     int x; */
/*     int y; */
/*     float h; */
/* }; */

/* class water{ */
/*  public: */
/*     water(grid*);       // init water with high_field */
/*     ~water(); */
/*     void calc_dir(long);        // n times modified d8 algorithm */
/*     void calc_fluxs(long);      // simple flux without waterlevel  */
/*     grid* get_dir();            // dir in [0..7] ul, u, ur, l r lol lo lor  */
/*     grid* get_lock();           // drain of water */
/*     grid* get_tag();            // water in x,y */
/*     void read_dir(grid&);       // put dir_grin in intern dir */
/*     void read_tag(grid&); */
/*     void read_lock(grid&); */
/*     struct flux **flux1; */
/*     float **hw; */
/*     int ncols, nrows; */
/*  private: */
/*     int **dir, **tag, **lock; */
/*     grid* g1; */
/*     // return the min of moore-environment */
/* };  */
#endif
