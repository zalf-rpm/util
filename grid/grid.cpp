/**
Authors:
Ralf Wieland <ralf.wieland@zalf.de>
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

#include <cstdio>
#include <cstring>
//#include <gsl/gsl_linalg.h>
//#include <gsl/gsl_vector.h>
//#include <gsl/gsl_matrix.h>
//#include <gsl/gsl_wavelet2d.h>
//#include <gsl/gsl_sort.h>

#include "platform.h"
#include "grid.h"

using namespace std;
using namespace Grids;

#define MAX_X_KOHON 512
#define MAX_Y_KOHON 1024
#define RKOHON
#define SIGMA 2.5
#define FADE 2.0
#define KEND 0.0001
#define DEBUG
#define FLUX
#define RES 0.0001
#define TOL 1.0

#define max(x,y) (x<y ? y : x)
#define min(x,y) (x>y ? y : x)

#define DIRS 8     // 8 direction for waterflux
#define DELTA 0.0  // delta for gradient
#define ALPHA 0.1  // fill the drain const
#define BETA  0.5 // w+=sqrt(val)-Beta*w
#define EPS   0.0000001
#define T1    4   // const for zoom
#define T2    3

#define BW 0       // black and white
#define COLOR 1

#define MIN 0
#define MAX 1
#define AVG 2
#define ADD 3
#define MUL 4
#define DIV 5

#define CIRCLE 1  // Region is al circle
#define MOORE  2  // Region is a moore environmet
#define MOORE1 3  // Region is a moore environmet adapt to circle
#define MOORE2 4  // Moore region with a hole in the centre

#define YES 1
#define NO 0
#define UNKNOWN -1

#define SPLATTER_TRESH 0.5
#define THRESH 100 // Minima of class member
#define BINS 10

#define MAXSTACK 100000000  // max stacksize for flood fill

#define OD 0
#define N 1
#define NE 2
#define E 3
#define SE 4
#define S 5
#define SW 6
#define W 7
#define NW 8
#define ND -1

#define REAL(c) ((c)[0])
#define IMAG(c) ((c)[1])

#define HIST 20           // fractal

typedef map<int,int> mapType;
typedef mapType::value_type ValuePair;
typedef map<int,double> doubleMap;
typedef doubleMap::value_type doublePair;

int dcompare(const void* e1, const void* e2)
{
	double* v1=(double*)e1;
	double* v2=(double*)e2;
	return (*v1<*v1) ? -1 : (*v1>*v2) ? 1 : 0;
}


grid::grid(int rg)
{
	rgr = rg;       // setze Rastergroesse
	feld=(float**)NULL;
	has_nodata = UNKNOWN;
  nrows = 0;
  ncols = 0;
}

grid::grid(int rows, int cols)
{
	rgr = 1;       // setze Rastergroesse
	has_nodata = UNKNOWN;
	variance1=variance2=covariance=0.0;
	nrows=rows;
	ncols=cols;
	csize=1.0;
	nodata=-9999;
	xcorner=ycorner=0.0;
	feld= new float*[nrows];
	//   cerr << nrows << " " << ncols << " feld=" << feld << endl;
	if(feld==NULL)
		cerr << "no sufficient memory" << endl;
	for(int i=0; i<nrows; i++){
		if((feld[i]= new float[ncols])==NULL){
			cerr << "no sufficient memory" << endl;
		}
		//	cerr << "feld[" << i << "]=" << feld[i] << endl;
	}
	time_t now = time(NULL);
	srand(now);
	for(int i=0; i<rows; i++){
		for(int j=0; j<cols; j++){
			feld[i][j]=nodata;
		}
	}
}

grid::~grid()       // Freigabe inneres Feld
{
	if(feld!=(float**)NULL){
		for(int i=0; i<nrows; i++){
			delete [] feld[i];
		}
		delete [] feld;
	}
}

grid* Grids::read_xyz(const char* name,grid* g1)
{
	double x,y,z;
	double xmin,ymin,zmin, xmax,ymax,zmax,zmean;
	FILE *fp;
	xmin=ymin=zmin=FLT_MAX;
	xmax=ymax=zmax=-FLT_MAX;
	zmean=0.0;
	fp=fopen(name,"r");
	if(!fp){
		cerr << "errot (read_xyz): can not open inputfile: " << name << endl;
		return (grid*)0;
	}
	int length=0;
	// calculate length and min/max
	while(EOF!=fscanf(fp,"%lf %lf %lf",&x,&y,&z)){
		length++;
		if(x<xmin)xmin=x;
		if(y<ymin)ymin=y;
		if(z<zmin)zmin=z;
		if(x>xmax)xmax=x;
		if(y>ymax)ymax=y;
		if(z>zmax)zmax=z;
	}
  //cerr << length << endl;;
	double *xval,*yval,*zval;
	xval=new double[length];
	yval=new double[length];
	zval=new double[length];
	if(!xval || !yval || !zval){
		cerr << "error (read_xyz): no space on device\n";
		return (grid*)0;
	}
	int i=0;
	// read in values
	rewind(fp);
	while(EOF!=fscanf(fp,"%lf %lf %lf",&x,&y,&z)){
		xval[i]=x;
		yval[i]=1-y; // muss das immer so sein ???
		zval[i]=z;
		zmean+=z;
		i++;
	}
  //cerr << length << endl;
	fclose(fp);
	zmean/=length;
  //cerr << length << " " << zmean << endl;
  //cerr << xmin << " " << xmax << endl;
  //cerr << ymin << " " << ymax << endl;
  //cerr << zmin << " " << zmax << endl;
	// generate the result grid
	grid* gx=g1->grid_copy();
	// generate the  y-grid
	grid* gy=g1->grid_copy();
	// init x,y-grid
	for(int i=0; i<gx->nrows; i++){
		for(int j=0; j<gx->ncols; j++){
			if(int(gx->feld[i][j])!=gx->nodata){
				gx->feld[i][j]=0.0;
				gy->feld[i][j]=0.0;
			}
		}
	}
	grid* a1,*a2, *as;
	// gaussian interpolation
	a1=gx;
	a2=gy;
	float teiler=1.0/sqrt((double)g1->ncols*g1->nrows);
	for(int k=0; k<length; k++){
		for(int i=0; i<gx->nrows; i++){
			for(int j=0; j<gx->ncols; j++){
				if(int(a1->feld[i][j])!=a1->nodata){
					a1->feld[i][j]=exp(-((i-gx->nrows*yval[k])
							*(i-gx->nrows*yval[k])+
							(j-gx->ncols*xval[k])
							*(j-gx->ncols*xval[k]))*teiler)*
							zval[k];
					if((a2->feld[i][j])>(a1->feld[i][j]))
						a1->feld[i][j] = a2->feld[i][j];
				}
			}
		}
		as=a1;
		a1=a2;
		a2=as;
	}
	return a1;
}

int grid::read_ascii(const char* name)
{
	char buf[255];
	float val1;
	ifstream file1(name);
	if(!file1){
		cerr << "error (read_ascii): can not open inputfile: " << name << endl;
		file1.close();
		return(-1);
	}
	if(feld!=(float**)NULL){
		for(int i=0; i<nrows; i++){
			delete [] feld[i];
		}
	}
	file1 >> buf >> ncols;
	if(strncmp(buf,"ncols",5)!=0){
		cerr << "error (read_ascii): not an ASCII-Grid: " << name << endl;
		file1.close();
		return(-2);
	}
	file1 >> buf >> nrows;
	file1 >> buf >> xcorner;
	file1 >> buf >> ycorner;
	file1 >> buf >> csize;
	file1 >> buf >> nodata;
	// allocate memory
	if(nrows <= 0 || ncols <= 0) return -2;
	feld= new float*[nrows];
	for(int i=0; i<nrows; i++){
		if((feld[i]= new float[ncols])==NULL){
			cerr << "error (read_ascii): no sufficient memory" << endl;
		}
	}
	// read_in
	for(int i=0; i<nrows; i++){
		for(int j=0; j<ncols; j++){
			file1 >> val1;
			feld[i][j]=val1;
		}
	}
	file1.close();
	return 0;
}

int grid::read_ascii_inv(const char* name)
{
	char buf[255];
	float val1;
	ifstream file1(name);
	if(!file1){
		cerr << "errot (read_ascii): can not open inputfile: " << name << endl;
		return(-1);
	}
	if(feld!=(float**)NULL){
		for(int i=0; i<nrows; i++){
			delete [] feld[i];
		}
	}
	file1 >> buf >> ncols;
	if(strncmp(buf,"ncols",5)!=0){
		cerr << "error (read_ascii): not a ASCII-Grid: " << name << endl;
		return(-2);
	}
	file1 >> buf >> nrows;
	file1 >> buf >> xcorner;
	file1 >> buf >> ycorner;
	file1 >> buf >> csize;
	file1 >> buf >> nodata;
	// allocate memory
	if(nrows <= 0 || ncols <= 0) return -2;
	feld= new float*[nrows];
	for(int i=0; i<nrows; i++){
		if((feld[i]= new float[ncols])==NULL){
			cerr << "error (read_ascii): no sufficient memory" << endl;
		}
	}
	// read_in
	for(int i=nrows-1; i>=0; i--){
		for(int j=ncols-1; j>=0; j--){
			file1 >> val1;
			feld[i][j]=val1;
		}
	}
	file1.close();
	return 0;
}

grid* grid::grid_copy()
{
	grid* gx = new grid((int)csize);
	gx->ncols = ncols;
	gx->nrows = nrows;
	gx->xcorner = xcorner;
	gx->ycorner = ycorner;
	gx->csize = csize;
	gx->nodata = nodata;
	gx->feld = new float*[gx->nrows];
	for(int i=0; i<gx->nrows; i++){
		if((gx->feld[i]=new float[gx->ncols])==NULL){
			cerr << "error (grid_copy): no sufficient memory" << endl;
		}
	}
	for(int i=0; i<nrows; i++){
		for(int j=0; j<ncols; j++){
			gx->feld[i][j] = feld[i][j];
		}
	}
	return gx;
}

void grid::norm_grid()
{
	min=FLT_MAX;
	max=-FLT_MAX;
	for(int i=0; i<nrows; i++){
		for(int j=0; j<ncols; j++){
			if(int(feld[i][j])!=nodata){
				min = (feld[i][j]<min) ? feld[i][j] : min;
				max = (feld[i][j]>max) ? feld[i][j] : max;
			}
		}
	}
	if(max==min){
		min=0.0;
		max=0.0;
		for(int i=0; i<nrows; i++){
			for(int j=0; j<ncols; j++){
				if(int(feld[i][j])!=nodata){
					feld[i][j]=1.0;
				}
			}
		}
	}
	else{
		for(int i=0; i<nrows; i++){
			for(int j=0; j<ncols; j++){
				if(int(feld[i][j])!=nodata){
					feld[i][j] = (feld[i][j]-min)/(max-min);
				}
			}
		}
	}
}

void grid::norm_grid(float min1,float max1)
{
	min=min1;
	max=max1;
	for(int i=0; i<nrows; i++){
		for(int j=0; j<ncols; j++){
			if(int(feld[i][j])!=nodata){
				if(feld[i][j]>max)feld[i][j]=max;
				feld[i][j] = (feld[i][j]-min)/(max-min);
			}
		}
	}
}

void grid::norm_grid1()
{
	min=FLT_MAX;
	max=-FLT_MAX;
	int count1=0;
	gridstd=gridmean=0.0;
	float val;
	for(int i=0; i<nrows; i++){
		for(int j=0; j<ncols; j++){
			if(int(feld[i][j])!=nodata){
				min = (feld[i][j]<min) ? feld[i][j] : min;
				max = (feld[i][j]>max) ? feld[i][j] : max;
				gridmean+=feld[i][j];
				count1++;
			}
		}
	}
	gridmean/=count1;
	for(int i=0; i<nrows; i++){
		for(int j=0; j<ncols; j++){
			if(int(feld[i][j])!=nodata){
				gridstd+=(feld[i][j]-gridmean)*(feld[i][j]-gridmean);
			}
		}
	}
	gridstd /= (float)(count1-1.0);
	gridstd = sqrt(gridstd);
	if(max>gridmean+3*gridstd) max=gridmean+3*gridstd;
	if(min<gridmean-3*gridstd) min=gridmean-3*gridstd;
	for(int i=0; i<nrows; i++){
		for(int j=0; j<ncols; j++){
			if(int(feld[i][j])!=nodata){
				val=feld[i][j];
				if(val>=max)
					feld[i][j]=1.0;
				if(val<=min)
					feld[i][j]=0.0;
				if(val<max && val>min)
					feld[i][j] = (feld[i][j]-min)/(max-min);
			}
		}
	}
	fprintf(stderr,"norm1: mean=%f min=%f max=%f std=%f\n",
	        gridmean,min,max,gridstd);
}

void grid::norm_grid2()
{
	min=FLT_MAX;
	max=-FLT_MAX;
	int count1=0;
	gridstd=gridmean=0.0;
	//float val;
	for(int i=0; i<nrows; i++){
		for(int j=0; j<ncols; j++){
			if(int(feld[i][j])!=nodata){
				min = (feld[i][j]<min) ? feld[i][j] : min;
				max = (feld[i][j]>max) ? feld[i][j] : max;
				gridmean+=feld[i][j];
				count1++;
			}
		}
	}
	gridmean/=count1;
	for(int i=0; i<nrows; i++){
		for(int j=0; j<ncols; j++){
			if(int(feld[i][j])!=nodata){
				gridstd+=(feld[i][j]-gridmean)*(feld[i][j]-gridmean);
			}
		}
	}
	gridstd /= (float)(count1-1.0);
	gridstd = sqrt(gridstd);
	if(max>gridmean+3*gridstd) max=gridmean+3*gridstd;
	if(min<gridmean-3*gridstd) min=gridmean-3*gridstd;
	for(int i=0; i<nrows; i++){
		for(int j=0; j<ncols; j++){
			if(int(feld[i][j])!=nodata){
				if(gridstd==0)
					feld[i][j]=0.0;
				else
					feld[i][j]=(feld[i][j]-gridmean)/gridstd;
			}
		}
	}
	fprintf(stderr,"norm2: mean=%f min=%f max=%f std=%f\n",
	        gridmean,min,max,gridstd);
}

int grid::cov_grid(grid* g1)
{
	float m1,m2;
	unsigned int counter=0;
	if(nrows != g1->nrows || ncols != g1->ncols){
		fprintf(stderr,"error (cov): grids must be of same size\n");
		return 1;
	}
	m1=m2=0.0;
	for(int i=0; i<nrows; i++){
		for(int j=0; j<ncols; j++){
			if(feld[i][j]!=nodata && g1->feld[i][j]!=g1->nodata){
				counter++;
				m1+=feld[i][j];
				m2+=g1->feld[i][j];
			}
		}
	}
	m1/=counter;
	m2/=counter;
	variance1=variance2=covariance=0.0;
	for(int i=0; i<nrows; i++){
		for(int j=0; j<ncols; j++){
			if(feld[i][j]!=nodata && g1->feld[i][j]!=g1->nodata){
				variance1+=(feld[i][j]-m1)*(feld[i][j]-m1);
				variance2+=(g1->feld[i][j]-m2)*(g1->feld[i][j]-m2);
				covariance+=(feld[i][j]-m1)*(g1->feld[i][j]-m2);
			}
		}
	}
	counter--;
	variance1/=counter;
	variance2/=counter;
	covariance/=counter;
	return 0;
}

void grid::stat()
{
	gridmin=FLT_MAX;
	gridmax=-FLT_MAX;
	gridmean=0.0;
	gridstd=0.0;
	long int count=0;
	count_data=count_nodata=0;
	for(int i=0; i<nrows; i++){
		for(int j=0; j<ncols; j++){
			if(int(feld[i][j])!=nodata){
				count_data++;
				if(feld[i][j]<gridmin){
					gridmin=feld[i][j];
					minx=i;
					miny=j;
				}
				if(feld[i][j]>gridmax){
					gridmax=feld[i][j];
					maxx=i;
					maxy=j;
				}
				gridmean += feld[i][j];
				count++;
			}
			else count_nodata++;
		}
	}
	if(count_nodata>0) has_nodata=YES;
	gridmean /= count;
	for(int i=0; i<nrows; i++){
		for(int j=0; j<ncols; j++){
			if(int(feld[i][j])!=nodata){
				gridstd+=(feld[i][j]-gridmean)*(feld[i][j]-gridmean);
			}
		}
	}
	gridstd /= (count-1);
	if(gridstd>=0) gridstd = sqrt(gridstd);
	else gridstd = sqrt(-gridstd);
	max=gridmax;
	min=gridmin;
}

void grid::calc_pattern(float thresh)
{
	grid *evg=grid_copy();
	time_t now = time(NULL);
	stat();
	float th1=(thresh-gridmin)/(gridmax-gridmin);
	srand(now);
	obv=sigmabv=ebv=0.0;
	float meanbv;
	// set a grid random
	for(int i=0; i<nrows; i++){
		for(int j=0; j<ncols; j++){
			if(int(feld[i][j])==nodata)
				evg->feld[i][j]=nodata;
			else
				evg->feld[i][j]=(float)rand()/RAND_MAX;
		}
	}
	// calculates the obv
	for(int i=0; i<nrows; i++){
		for(int j=0; j<ncols; j++){
			if(i+1<nrows && int(feld[i+1][j])!=nodata
					&& feld[i+1][j]<=thresh && feld[i][j]>thresh)
				obv++;
			if(j+1<ncols && int(feld[i][j+1])!=nodata
					&& feld[i][j+1]<=thresh && feld[i][j]>thresh)
				obv++;
			if(j-1>=0 && int(feld[i][j-1])!=nodata
					&& feld[i][j-1]<=thresh && feld[i][j]>thresh)
				obv++;
			if(i-1>=0 && int(feld[i-1][j])!=nodata
					&& feld[i-1][j]<=thresh && feld[i][j]>thresh)
				obv++;
		}
	}
	// calculates the ebv
	for(int i=0; i<nrows; i++){
		for(int j=0; j<ncols; j++){
			if(i+1<nrows && int(feld[i+1][j])!=nodata
					&& evg->feld[i+1][j]<=0.5 && evg->feld[i][j]>0.5)
				ebv++;
			if(j+1<ncols && int(feld[i][j+1])!=nodata
					&& evg->feld[i][j+1]<=0.5 && evg->feld[i][j]>0.5)
				ebv++;
			if(j-1>=0 && int(feld[i][j-1])!=nodata
					&& evg->feld[i][j-1]<=0.5 && evg->feld[i][j]>0.5)
				ebv++;
			if(i-1>=0 && int(feld[i-1][j])!=nodata
					&& evg->feld[i-1][j]<=0.5 && evg->feld[i][j]>0.5)
				ebv++;
		}
	}
	float ssgv=0;
	float ebv1;
	meanbv=ebv/((nrows-1)*(ncols-1));
	// calculates the sigmabv
	for(int i=0; i<nrows; i++){
		for(int j=0; j<ncols; j++){
			if(i+1<nrows && int(feld[i+1][j])!=nodata
					&& evg->feld[i+1][j]<=th1 && evg->feld[i][j]>th1)
				ebv1++;
			if(j+1<ncols && int(feld[i][j+1])!=nodata
					&& evg->feld[i][j+1]<=th1 && evg->feld[i][j]>th1)
				ebv1++;
			if(j-1>=0 && int(feld[i][j-1])!=nodata
					&& evg->feld[i][j-1]<=th1 && evg->feld[i][j]>th1)
				ebv1++;
			if(i-1>=0 && int(feld[i-1][j])!=nodata
					&& evg->feld[i-1][j]<=th1 && evg->feld[i][j]>th1)
				ebv1++;
			ssgv+=(meanbv-ebv1)*(meanbv-ebv1);
			ebv1=0.0;
		}
	}
	sigmabv=ssgv;

}

void grid::inv_grid()
{
	stat();
	for(int i=0; i<nrows; i++){
		for(int j=0; j<ncols; j++){
			if(int(feld[i][j])!=nodata){
				feld[i][j]=gridmax-feld[i][j];
			}
		}
	}
}

int* grid::hist(int bins)
{
	int* erg=new int[bins+1];
	if(!erg){
		cerr << "error (hist): no space on device\n";
		return 0;
	}
	stat();
	float delta=(gridmax-gridmin);
	for(int i=0; i<=bins; i++) erg[i]=0;
	for(int i=0; i<nrows; i++){
		for(int j=0; j<ncols; j++){
			if(int(feld[i][j])!=nodata){
				erg[(int)((bins)*(feld[i][j]-gridmin)/delta)]++;
			}
		}
	}
	for(int i=0; i<=bins; i++)
		fprintf(stderr,"%-6d %f %f\n",erg[i],
		        gridmin+i*delta/bins,gridmin+(i+1)*delta/bins);
	return erg;
}

void grid::set_value(float val)
{
	for(int i=0; i<nrows; i++)
		for(int j=0; j<ncols; j++)
			if((int)feld[i][j]!=nodata)
				feld[i][j]=val;
}

void grid::exchange(float val1, float val2)
{
	for(int i=0; i<nrows; i++)
		for(int j=0; j<ncols; j++)
			if(feld[i][j]==val1)
				feld[i][j]=val2;
}

void grid::rand_value()
{
	time_t now = time(NULL);
	srand(now);
	for(int i=0; i<nrows; i++)
		for(int j=0; j<ncols; j++)
			feld[i][j]=(float)rand()/RAND_MAX;
}

void grid::add_value(float val)
{
	for(int i=0; i<nrows; i++)
		for(int j=0; j<ncols; j++)
			if(int(feld[i][j])!=nodata)
				feld[i][j]+=val;
}

void grid::mul_value(float val)
{
	for(int i=0; i<nrows; i++)
		for(int j=0; j<ncols; j++)
			if(int(feld[i][j])!=nodata)
				feld[i][j]*=val;
}

void grid::grid_fabs()
{
	for(int i=0; i<nrows; i++)
		for(int j=0; j<ncols; j++)
			if(int(feld[i][j])!=nodata)
				feld[i][j]=fabs(feld[i][j]);
}

void grid::cut(float val1, float val2, float val3)
{
	if(val3<0){
		for(int i=0; i<nrows; i++){
			for(int j=0; j<ncols; j++){
				if(int(feld[i][j])!=nodata){
					if(feld[i][j]>val2)feld[i][j]=-1.0;
					if(feld[i][j]<val1)feld[i][j]=-1.0;
				}
			}
		}
	}
	else{
		for(int i=0; i<nrows; i++){
			for(int j=0; j<ncols; j++){
				if(int(feld[i][j])!=nodata){
					if(feld[i][j]>=val1 && feld[i][j]<=val2)feld[i][j]=val3;
					else feld[i][j]=0.0;
				}
			}
		}
	}
}

void grid::cut_off(float val1, float val2, float val3)
{
	for(int i=0; i<nrows; i++){
		for(int j=0; j<ncols; j++){
			if(int(feld[i][j])!=nodata){
				if(feld[i][j]>=val1 && feld[i][j]<=val2)feld[i][j]=val3;
			}
		}
	}
}

void grid::cut_fuzzy(float val1, float val2)
{
	for(int i=0; i<nrows; i++){
		for(int j=0; j<ncols; j++){
			if(int(feld[i][j])!=nodata){
				if(feld[i][j]>val2)feld[i][j]=val2;
				if(feld[i][j]<val1)
					feld[i][j]=val1;
			}
		}
	}
}

void grid::grid_log()
{
	for(int i=0; i<nrows; i++){
		for(int j=0; j<ncols; j++){
			if(int(feld[i][j])!=nodata){
				if(feld[i][j]>0)
					feld[i][j]=log10(feld[i][j]);
			}
		}
	}
}


double grid::variance(int step)
{
	double dval=0.0;
	double dmean=0.0;
	int endi,endj;
	if(variance_flag==false){
		dfeld=new double[nrows*ncols];
		variance_flag=true;
	}
	if(dfeld==0){
		fprintf(stderr,"error in variance: no space=%d left on device\n",
		        (nrows*ncols));
		return -9999;
	}
	unsigned int length=(nrows-step)*(ncols-step);
	if(length<1){
		fprintf(stderr,"error in variance: length=%d too big\n",length);
		return -9999;
	}
	for(unsigned int i=0; i<length; i++)
		dfeld[i]=0.0;
	endi=nrows-step;
	endj=ncols-step;
	int step1=step+1;
	for(int i=0; i<endi; i++){
		for(int j=0; j<endj; j++){
			for(int k1=0; k1<step1; k1++){
				for(int k2=0; k2<step1; k2++){
					dfeld[i*(ncols-step)+j]+=feld[i+k1][j+k2];
				}
			}
			dfeld[i*(ncols-step)+j]/=((step1)*(step1));
		}
	}
	// calculate the variance
	for(unsigned int i=0; i<length; i++){
		dmean+=dfeld[i];
	}
	dmean/=length;
	for(unsigned int i=0; i<length; i++){
		if(dfeld[i]!=0)
			dval+=(dfeld[i]-dmean)*(dfeld[i]-dmean);
	}
	//    cerr << step << " " << length << " " << dmean << " "
	//	 << dval << endl;
	return (dval/(length-1));
}

grid* grid::sobol()
{
	grid* gx;
	gx=grid_copy();
	double  n,m;
	for(int i=1; i<nrows-1; i++){
		for(int j=1; j<ncols-1; j++){
			gx->feld[i][j]=gx->nodata;
			if(int(feld[i][j])!=nodata){
				if(int(feld[i-1][j-1])!=nodata &&
						int(feld[i][j-1])!=nodata &&
						int(feld[i+1][j-1])!=nodata &&
						int(feld[i-1][j])!=nodata &&
						int(feld[i][j])!=nodata &&
						int(feld[i+1][j])!=nodata &&
						int(feld[i-1][j+1])!=nodata &&
						int(feld[i][j+1])!=nodata &&
						int(feld[i+1][j+1])!=nodata){
					n= feld[i-1][j+1]+2*feld[i][j+1]+feld[i+1][j+1]
					                                           -feld[i-1][j-1]-2*feld[i][j-1]-feld[i+1][j-1];
					m=feld[i+1][j-1]+2*feld[i+1][j]+feld[i+1][j+1]+
							-feld[i-1][j-1]-2*feld[i-1][j]+feld[i-1][j+1];
					gx->feld[i][j]=(float)(fabs(n)+fabs(m));
				}
			}
		}
	}
	return gx;
}

grid* grid::laplace()
{
	grid* gx;
	gx=grid_copy();
	//double  n,m;
	for(int i=1; i<nrows-1; i++){
		for(int j=1; j<ncols-1; j++){
			gx->feld[i][j]=gx->nodata;
			if(int(feld[i][j])!=nodata){
				if(int(feld[i-1][j-1])!=nodata &&
						int(feld[i][j-1])!=nodata &&
						int(feld[i+1][j-1])!=nodata &&
						int(feld[i-1][j])!=nodata &&
						int(feld[i][j])!=nodata &&
						int(feld[i+1][j])!=nodata &&
						int(feld[i-1][j+1])!=nodata &&
						int(feld[i][j+1])!=nodata &&
						int(feld[i+1][j+1])!=nodata){
					gx->feld[i][j]=-feld[i-1][j-1]-feld[i-1][j]-
							feld[i-1][j+1]-feld[i][j-1]+8*feld[i][j]-
							feld[i][j+1]-feld[i+1][j-1]-
							feld[i+1][j]-feld[i+1][j+1];
				}
			}
		}
	}
	return gx;
}

grid* grid::edge()
{
	grid* gx;
	gx=grid_copy();
	//const float sq2=sqrt(2.0);
	for(int i=1; i<nrows-1; i++){
		for(int j=1; j<ncols-1; j++){
			gx->feld[i][j]=gx->nodata;
			if(int(feld[i][j])!=nodata){
				if(int(feld[i-1][j-1])!=nodata &&
						int(feld[i][j-1])!=nodata &&
						int(feld[i+1][j-1])!=nodata &&
						int(feld[i-1][j])!=nodata &&
						int(feld[i][j])!=nodata &&
						int(feld[i+1][j])!=nodata &&
						int(feld[i-1][j+1])!=nodata &&
						int(feld[i][j+1])!=nodata &&
						int(feld[i+1][j+1])!=nodata){
					gx->feld[i][j]=
							-2.0/8*feld[i-1][j-1]
							                 -2.0/8*feld[i][j-1]
							                                -2.0/8*feld[i+1][j-1]
							                                                 -2.0/8*feld[i-1][j]
							                                                                  +3.0*feld[i][j]
							                                                                               -2.0/8*feld[i+1][j]
							                                                                                                -2.0/8*feld[i-1][j+1]
							                                                                                                                 -2.0/8*feld[i][j+1]
							                                                                                                                                -2.0/8*feld[i+1][j+1];
				}
			}
		}
	}
	return gx;
}

/*
grid* grid::power()
{
	grid *gx= new grid(100);
	gx=grid_copy();
	for(int i1=0; i1<nrows; i1++)
		for(int j1=0; j1<ncols; j1++)
			gx->feld[i1][j1]=gx->nodata;
	int rows=nrows;
	int cols=ncols;
	double total=(double)nrows*(double)ncols;
	fftw_complex *in, *out;
	fftw_plan p;
	in = (fftw_complex *) fftw_malloc(sizeof(fftw_complex)*rows*cols);
	out = (fftw_complex *) fftw_malloc(sizeof(fftw_complex)*rows*cols);
	if(in==0 || out==0){
		cerr << "(error in fourierm:) no space on device\n";
		return 0;
	}
	p = fftw_plan_dft_2d(rows, cols, in,out,
	                     FFTW_FORWARD,FFTW_MEASURE);
	//int x,y;
	double val;//,sum=0.0;
	// fill the matrix A
	for(int i1=0; i1<nrows; i1++){
		for(int j1=0; j1<ncols; j1++){
			in[i1*cols+j1][0]=(double)(feld[i1][j1]);
			in[i1*cols+j1][1]=0.0;
		}
	}
	fftw_execute(p);
	int i2,j2;
	int nrow2=(int)(nrows/2);
	int ncol2=(int)(ncols/2);
	for(int i1=0; i1<nrows; i1++){
		for(int j1=0; j1<ncols; j1++){
			// val=sqrt(out[i1*cols+j1][0]*out[i1*cols+j1][0]+
			//	     out[i1*cols+j1][1]*out[i1*cols+j1][1]);
			val=(out[i1*cols+j1][0]*out[i1*cols+j1][0]+
					out[i1*cols+j1][1]*out[i1*cols+j1][1]);
			i1 <= nrow2 ? i2=nrow2-i1 : i2=nrows-(i1-nrow2);
			j1 <= ncol2 ? j2=ncol2-j1 : j2=ncols-(j1-ncol2);
			// gx->feld[i2][j2]=20*log10(val);
			gx->feld[i2][j2]=val/total;
		}
	}
	// clean up
	fftw_destroy_plan(p);
	fftw_free(in);
	fftw_free(out);
	// end
	return gx;
}
*/

/*
grid* grid::svd(int smin, int smax)
{
	if(smin < 0 || smax > nrows || smax > ncols) return 0;
	grid *gx= new grid(100);
	gx=grid_copy();
	int zeilen=nrows;
	int spalten=ncols;
	if(zeilen<spalten){
		zeilen=ncols;
		spalten=nrows;
	}
	gsl_matrix *A_=gsl_matrix_alloc(zeilen,spalten);
	gsl_matrix *V_=gsl_matrix_alloc(spalten,spalten);
	gsl_vector *S_=gsl_vector_alloc(spalten);
	gsl_vector *W_=gsl_vector_alloc(spalten);
	// fill the matrix A
	for(int i=0; i<nrows; i++)
		for(int j=0; j<ncols; j++)
			if(nrows<ncols)
				gsl_matrix_set(A_,j,i,feld[i][j]);
			else
				gsl_matrix_set(A_,i,j,feld[i][j]);
	// decomposition
	//int error=gsl_linalg_SV_decomp(A_,V_,S_,W_);
	gsl_vector_fprintf(stderr, S_, "%g");
	for(int i=0; i<spalten; i++)
		if(i>=smin && i<smax);
		else
			gsl_vector_set(S_,i,0.0);
	// make result matrix
	double **val=  (double **) new double*[zeilen];
	double zwsp=0.0;
	for(int i=0; i<zeilen; i++)
		val[i]= new double[spalten];
	for(int i=0; i<zeilen; i++)
		for(int j=0; j<spalten; j++)
			val[i][j]=gsl_matrix_get(A_,i,j)*gsl_vector_get(S_,j);
	for(int i=0; i<zeilen; i++){
		for(int j=0; j<spalten; j++){
			for(int k=0; k<spalten; k++)
				zwsp+=val[i][k]*gsl_matrix_get(V_,j,k);
			if(nrows<ncols)
				gx->feld[j][i]=zwsp;
			else
				gx->feld[i][j]=zwsp;
			zwsp=0.0;
		}
	}
	// clean up
	for(int i=0; i<zeilen; i++)
		delete [] val[i];
	delete [] val;
	gsl_vector_free (S_);
	gsl_vector_free (W_);
	gsl_matrix_free (A_);
	gsl_matrix_free (V_);
	// end
	return gx;
}
*/

grid* grid::n2()
{
	int a, dx,dy;
	nrows<ncols ? a=nrows : a=ncols;
	fprintf(stderr,"n2: nrows=%d ncols=%d a=%d\n",nrows,ncols,a);
	int k=2;
	while((k*=2) < a);
	k/=2; // make i smaller than a
	dx=(ncols-k)/2-1;
	if(dx<0)dx=0;
	dy=(nrows-k)/2-1;
	if(dy<0)dy=0;
	fprintf(stderr,"n2: k=%d dx=%d dy=%d\n",k,dx,dy);
	grid* gx=new grid((int)csize);
	gx->ncols=k;
	gx->nrows=k;
	gx->csize=csize;
	gx->nodata=nodata;
	gx->xcorner=xcorner+dx*csize;
	gx->ycorner=ycorner+(nrows-dy)*csize;
	// allocate memory
	gx->feld = new float*[k];
	for(int i=0; i<k; i++){
		if((gx->feld[i]=new float[k])==NULL){
			cerr << "error (grid::select): no sufficient memory" << endl;
		}
	}
	// fill the new grid
	for(int i=0; i<k; i++)
		for(int j=0; j<k; j++)
			gx->feld[i][j]=feld[i+dy][j+dx];
	return(gx);
}

/*
grid* grid::tree_grid(float val)
{
	tree* treep=new tree(this);
	treep->thresh(val);
	treep->print_assoc();
	grid* gx=treep->map_tree();
	delete treep;
	return gx;
}
*/

/*
grid* grid::filter(double f1, double f2)
{
	const double PI=3.1415926;
	// check if no nodata
	for(int i=0; i<nrows; i++)
		for(int j=0; j<ncols; j++)
			if((int)feld[i][j]==nodata){
				cerr << "error in filter: nodata values detected in g1\n";
				return 0;
			}
	double sum;
	// organize the workspace
	double *a1=new double[nrows];     // workspace low
	double *a2=new double[nrows];     // workspace high
	double *b1=new double[ncols];     // workspace low
	double *b2=new double[ncols];     // workspace high
	double *h1=new double[nrows];     // workspace final
	double *h2=new double[ncols];     // workspace final
	if(a1==0 || a2==0 || b1==0 || b2==0 || h1==0 || h2==0){
		cerr << "error in filter: no space left on device\n";
		return 0;
	}
	double fcx1=f1;
	double fcy1=f1;
	double fcx2=f2;
	double fcy2=f2;
	// set a1 and b1 zero
	for(int i=0; i<nrows; i++)
		a1[i]=0.0;
	for(int j=0; j<ncols; j++)
		b1[j]=0.0;
	fprintf(stderr,"fcx1=%f fcx2=%f fcy1=%f fcy2=%f\n",fcx1,fcx2,fcy1,fcy2);
	// fill a1
	if(fcy1!=0.0){
		for(int i=0; i<nrows; i++){
			if((i-nrows/2)==0)a1[i]=2.0*PI*fcy1;
			else a1[i]=sin(2.0*PI*fcy1*(i-nrows/2))/(i-nrows/2);
			a1[i]*=(0.42-0.5*cos(2.0*PI*i/nrows)+0.08*cos(4.0*PI*i/nrows));
		}
		// normalize a1
		sum=0.0;
		for(int i=0; i<nrows; i++)
			sum+=a1[i];
		for(int i=0; i<nrows; i++)
			a1[i]/=sum;
	}
	// b1 is the first low pass
	// fill b1
	if(fcx1!=0.0){
		for(int j=0; j<ncols; j++){
			if((j-ncols/2)==0)b1[j]=2.0*PI*fcx1;
			else b1[j]=sin(2.0*PI*fcx1*(j-ncols/2))/(j-ncols/2);
			b1[j]*=(0.42-0.5*cos(2.0*PI*j/ncols)+0.08*cos(4.0*PI*j/ncols));
		}
		// normalize b1
		sum=0.0;
		for(int j=0; j<ncols; j++)
			sum+=b1[j];
		for(int j=0; j<ncols; j++)
			b1[j]/=sum;
	}

	// calculate second low pass in x and y direction
	// fill a2
	for(int i=0; i<nrows; i++){
		if((i-nrows/2)==0)a2[i]=2.0*PI*fcy2;
		else a2[i]=sin(2.0*PI*fcy2*(i-nrows/2))/(i-nrows/2);
		a2[i]*=(0.42-0.5*cos(2.0*PI*i/nrows)+0.08*cos(4.0*PI*i/nrows));
	}
	sum=0.0;
	// normalize a2
	for(int i=0; i<nrows; i++)
		sum+=a2[i];
	for(int i=0; i<nrows; i++)
		a2[i]/=sum;
	// fill b2
	for(int j=0; j<ncols; j++){
		if((j-ncols/2)==0)b2[j]=2.0*PI*fcx2;
		else b2[j]=sin(2.0*PI*fcx2*(j-ncols/2))/(j-ncols/2);
		b2[j]*=(0.42-0.5*cos(2.0*PI*j/ncols)+0.08*cos(4.0*PI*j/ncols));
	}
	sum=0.0;
	// normalize b2
	for(int j=0; j<ncols; j++)
		sum+=b2[j];
	for(int j=0; j<ncols; j++)
		b2[j]/=sum;
	// change low pass filter (a2,b2) into high pass
	for(int i=0; i<nrows; i++)
		a2[i]*=-1.0;
	a2[nrows/2]+=1.0;
	for(int j=0; j<ncols; j++)
		b2[j]*=-1.0;
	b2[ncols/2]+=1.0;
	// generate the final kernel
	for(int i=0; i<nrows; i++)
		h1[i]=a1[i]+a2[i];
	for(int j=0; j<ncols; j++)
		h2[j]=b1[j]+b2[j];
	// change the band reject into a band pass
	for(int i=0; i<nrows; i++)
		h1[i]*=-1.0;
	h1[nrows/2]+=1.0;
	for(int j=0; j<ncols; j++)
		h2[j]*=-1.0;
	h2[ncols/2]+=1.0;
	//     cerr << "h1[nrows]" << endl;
	//     for(int i=0; i<nrows; i++)
	// 	cerr << h1[i] << endl;
	//     cerr << "h2[ncols]" << endl;
	//     for(int j=0; j<ncols; j++)
	// 	cerr << h2[j] << endl;
	// clean up
	delete [] a1;
	delete [] a2;
	delete [] b1;
	delete [] b2;
	// organize the fftw data
	fftw_complex *z1, *z2, *out1, *out2, *erg, *zwsp;
	fftw_plan p;
	z1 = (fftw_complex *) fftw_malloc(sizeof(fftw_complex)*
	                                  (nrows*ncols));
	z2 = (fftw_complex *) fftw_malloc(sizeof(fftw_complex)*
	                                  (nrows*ncols));
	out1 = (fftw_complex *) fftw_malloc(sizeof(fftw_complex)*
	                                    (nrows*ncols));
	out2 = (fftw_complex *) fftw_malloc(sizeof(fftw_complex)*
	                                    (nrows*ncols));
	erg = (fftw_complex *) fftw_malloc(sizeof(fftw_complex)*
	                                   (nrows*ncols));

	zwsp = (fftw_complex *) fftw_malloc(sizeof(fftw_complex)*
	                                    (nrows*ncols));

	if(z1==0 || z2==0 || out1==0 || out2==0 || erg==0 || zwsp==0){
		cerr << "(error in fourierm:) no space on device\n";
		exit(1);
	}
	// fill the complex inputs z1 and z2
	for(int i=0; i<nrows; i++){
		for(int j=0; j<ncols; j++){
			z1[i*ncols+j][0]=feld[i][j];
			z1[i*ncols+j][1]=0.0;
			z2[i*ncols+j][0]=h1[i]*h2[j];
			z2[i*ncols+j][1]=0.0;
		}
	}
	// fourrier transformation
	p = fftw_plan_dft_2d(nrows, ncols, z1,out1,
	                     FFTW_FORWARD,FFTW_ESTIMATE);
	fftw_execute(p);
	fftw_destroy_plan(p);
	p = fftw_plan_dft_2d(nrows, ncols, z2,out2,
	                     FFTW_FORWARD,FFTW_ESTIMATE);
	fftw_execute(p);
	fftw_destroy_plan(p);
	// convolution
	for(int i1=0; i1<nrows; i1++){
		for(int j1=0; j1<ncols; j1++){
			zwsp[i1*ncols+j1][0]=
					out1[i1*ncols+j1][0]*out2[i1*ncols+j1][0]+
					out1[i1*ncols+j1][1]*out2[i1*ncols+j1][1];
			zwsp[i1*ncols+j1][1]=
					out1[i1*ncols+j1][1]*out2[i1*ncols+j1][0]-
					out1[i1*ncols+j1][0]*out2[i1*ncols+j1][1];
		}
	}
	// clean up
	fftw_free(z1);
	fftw_free(z2);
	p = fftw_plan_dft_2d(nrows,ncols,zwsp,erg,
	                     FFTW_BACKWARD,FFTW_ESTIMATE);
	fftw_execute(p);
	fftw_destroy_plan(p);
	double Nx=(double)(nrows*ncols);
	int j2,i2;
	const int nrow2=(int)(nrows/2);
	const int ncol2=(int)(ncols/2);
	grid* gx = new grid((int)csize);
	gx=grid_copy();
	for(int i1=0; i1<nrows; i1++){
		for(int j1=0; j1<ncols; j1++){
			i1 <= nrow2 ? i2=nrow2-i1 : i2=nrows-(i1-nrow2);
			j1 <= ncol2 ? j2=ncol2-j1 : j2=ncols-(j1-ncol2);
			gx->feld[nrows-1-i2][ncols-1-j2]=
					erg[i1*(ncols)+j1][0]/Nx;
		}
	}
	// clean up
	delete [] h1;
	delete [] h2;
	fftw_free(out1);
	fftw_free(out2);
	fftw_free(zwsp);
	fftw_free(erg);
	return(gx);
}
*/

// determine the optimal tresh for denoising
//int compare (const void * a, const void * b)
//{
//	if( *(double*)a < *(double*)b ) return -1;
//	if( *(double*)a == *(double*)b ) return 0;
//	if( *(double*)a > *(double*)b ) return 1;
//}

/*
grid* grid::wavelet_d(int smin, int smax, int koeff)
{
	if(nrows!=ncols) return 0;
	if(smin < 0) return 0;
	cerr << "wavelet: " << smin << " " << smax << endl;
	grid *gx= new grid(100);
	gx=grid_copy();
	// construct a matrix(n,n) with n=2^k
	int maxn;
	(nrows>ncols) ? maxn=nrows : maxn=ncols;
	int val=2;
	while(val<maxn) val*=2;
	maxn=val;
	double *data=new double[maxn*maxn];
	cerr << "wavelet new matrix with n=" << maxn << " created" <<endl;
	if(data==0){
		cerr << " error in wavelet: no space left on device" << endl;
		return 0;
	}
	// optimal threshold parameter
	if(smax==-1){
		int nc1,nc2;
		double lambda,median,mad;
		double *sortcoeff,*mm;
		sortcoeff=new double[nrows];
		mm=new double[nrows];
		for(int i=0; i<nrows; i++) sortcoeff[i]=feld[i][0];
		qsort(sortcoeff, nrows, sizeof(double), dcompare);
		median=0.5*(sortcoeff[nrows/2]+sortcoeff[nrows/2+1]);
		for(int i=0; i<nrows; i++)mm[i]=fabs(feld[i][0]-median);
		qsort(mm, nrows, sizeof(double), dcompare);
		mad=0.5*(mm[nrows/2]+mm[nrows/2+1]);
		lambda=sqrt(2.0*log(static_cast<double>(nrows)))*sqrt(mad/0.6745);
		nc1=nrows-1;
		fprintf(stderr,"lambda=%lf ",lambda);
		while(mm[nc1--]>lambda);
		nc1=nrows-nc1;
		fprintf(stderr,"nc=%d ",nc1);
		for(int i=0; i<ncols; i++) sortcoeff[i]=feld[0][i];
		qsort(sortcoeff, ncols, sizeof(double), dcompare);
		median=0.5*(sortcoeff[ncols/2]+sortcoeff[ncols/2+1]);
		for(int i=0; i<ncols; i++)mm[i]=fabs(feld[0][i]-median);
		qsort(mm, ncols, sizeof(double), dcompare);
		mad=0.5*(mm[ncols/2]+mm[ncols/2+1]);
		lambda=sqrt(2.0*log(static_cast<double>(ncols)))*sqrt(mad/0.6745);
		nc2=nrows-1;
		fprintf(stderr,"lambda=%lf ",lambda);
		while(mm[nc2--]>lambda);
		nc2=nrows-nc1;
		fprintf(stderr,"nc=%d ",nc2);
		smax=nc1*nc2;
		fprintf(stderr,"smax=%d\n",smax);
		delete mm;
		delete sortcoeff;
	}

	// fill the matrix
	for(int i=0; i<maxn; i++)
		for(int j=0; j<maxn; j++)
			data[i*maxn+j]=0.0;
	for(int i=0; i<nrows; i++)
		for(int j=0; j<ncols; j++)
			data[i*ncols+j]=feld[i][j];
	// organize the wavelet space
	gsl_wavelet *w;
	gsl_wavelet_workspace *work;
	if(koeff <4 || koeff%2!=0)koeff=4;
	w = gsl_wavelet_alloc (gsl_wavelet_daubechies, koeff);
	work = gsl_wavelet_workspace_alloc(maxn);
	// make the transformation
	if(GSL_SUCCESS!=gsl_wavelet2d_transform_forward(
	                                                w, data, maxn, maxn, maxn, work)){
		cerr << "error in wavelet: gsl_wavelet2d_transform_forward" << endl;
		return 0;
	}
	// sorting
	double *abscoeff = new double[maxn*maxn];
	size_t *p = new size_t[smax];
	if(p==0 || abscoeff==0){
		cerr << " error in wavelet: no space left on device" << endl;
		return 0;
	}
	for (int i=0; i<maxn*maxn; i++)
	{
		abscoeff[i] = fabs(data[i]);
	}
	gsl_sort_largest_index (p, smax, abscoeff, 1, maxn*maxn);
	// gsl_sort_index (p, abscoeff, 1, maxn*maxn);
	if(smax>20){
		for(int i=smin; i<smin+20; i++)
			cerr << "p["<<i<<"]="<<p[i]<<" abscoeff["
			<<p[i]<<"]="<<abscoeff[p[i]]
			                       << " data["<<p[i]<<"]=" << data[p[i]]<<endl;
	}
	// select the important coeff
	int *dpi=new int[maxn*maxn];
	for (int i=0; i<maxn*maxn; i++)
		dpi[i]=0;
	for(int i=smin; i<smax; i++)
		dpi[p[i]]=1;
	for (int i=0; i<maxn*maxn; i++)
		if(dpi[i]==0) data[i]=0.0;
	delete [] dpi;
	//    // inverse transformation
	if(GSL_SUCCESS!=gsl_wavelet2d_transform_inverse(
	                                                w, data, maxn, maxn, maxn, work)){
		cerr << "error in wavelet: gsl_wavelet2d_transform_inverse" << endl;
		return 0;
	}

	// fill it back to the grid
	for(int i=0; i<nrows; i++)
		for(int j=0; j<ncols; j++)
			gx->feld[i][j]=data[i*ncols+j];

	//     // clean up
	gsl_wavelet_free(w);
	gsl_wavelet_workspace_free (work);
	delete [] data;
	delete [] abscoeff;
	delete [] p;
	// end
	return gx;
}
*/

/*
grid* grid::wavelet_h(int smin, int smax)
{
	if(nrows!=ncols) return 0;
	if(smin < 0) return 0;
	cerr << "wavelet: " << smin << " " << smax << endl;
	grid *gx= new grid(100);
	gx=grid_copy();
	// construct a matrix(n,n) with n=2^k
	int maxn;
	(nrows>ncols) ? maxn=nrows : maxn=ncols;
	int val=2;
	while(val<maxn) val*=2;
	maxn=val;
	double *data=new double[maxn*maxn];
	cerr << "wavelet new matrix with n=" << maxn << " created" <<endl;
	if(data==0){
		cerr << " error in wavelet: no space left on device" << endl;
		return 0;
	}
	// optimal threshold parameter
	if(smax==-1){
		int nc1,nc2;
		double lambda,median,mad;
		double *sortcoeff,*mm;
		sortcoeff=new double[nrows];
		mm=new double[nrows];
		for(int i=0; i<nrows; i++) sortcoeff[i]=feld[i][0];
		qsort(sortcoeff, nrows, sizeof(double), dcompare);
		median=0.5*(sortcoeff[nrows/2]+sortcoeff[nrows/2+1]);
		for(int i=0; i<nrows; i++)mm[i]=fabs(feld[i][0]-median);
		qsort(mm, nrows, sizeof(double), dcompare);
		mad=0.5*(mm[nrows/2]+mm[nrows/2+1]);
		lambda=sqrt(2.0*log(static_cast<double>(nrows)))*sqrt(mad/0.6745);
		nc1=nrows-1;
		fprintf(stderr,"lambda=%lf ",lambda);
		while(mm[nc1--]>lambda);
		nc1=nrows-nc1;
		fprintf(stderr,"nc=%d ",nc1);
		for(int i=0; i<ncols; i++) sortcoeff[i]=feld[0][i];
		qsort(sortcoeff, ncols, sizeof(double), dcompare);
		median=0.5*(sortcoeff[ncols/2]+sortcoeff[ncols/2+1]);
		for(int i=0; i<ncols; i++)mm[i]=fabs(feld[0][i]-median);
		qsort(mm, ncols, sizeof(double), dcompare);
		mad=0.5*(mm[ncols/2]+mm[ncols/2+1]);
		lambda=sqrt(2.0*log(static_cast<double>(ncols)))*sqrt(mad/0.6745);
		nc2=nrows-1;
		fprintf(stderr,"lambda=%lf ",lambda);
		while(mm[nc2--]>lambda);
		nc2=nrows-nc1;
		fprintf(stderr,"nc=%d ",nc2);
		smax=nc1*nc2;
		fprintf(stderr,"smax=%d\n",smax);
		delete mm;
		delete sortcoeff;
	}
	// fill the matrix
	for(int i=0; i<maxn; i++)
		for(int j=0; j<maxn; j++)
			data[i*maxn+j]=0.0;
	for(int i=0; i<nrows; i++)
		for(int j=0; j<ncols; j++)
			data[i*ncols+j]=feld[i][j];
	// organize the wavelet space
	gsl_wavelet *w;
	gsl_wavelet_workspace *work;
	w = gsl_wavelet_alloc (gsl_wavelet_haar, 2);
	work = gsl_wavelet_workspace_alloc(maxn);
	// make the transformation
	if(GSL_SUCCESS!=gsl_wavelet2d_transform_forward(
	                                                w, data, maxn, maxn, maxn, work)){
		cerr << "error in wavelet: gsl_wavelet2d_transform_forward" << endl;
		return 0;
	}
	// sorting
	double *abscoeff = new double[maxn*maxn];
	size_t *p = new size_t[smax];
	if(p==0 || abscoeff==0){
		cerr << " error in wavelet: no space left on device" << endl;
		return 0;
	}
	for (int i=0; i<maxn*maxn; i++)
	{
		abscoeff[i] = fabs(data[i]);
	}
	gsl_sort_largest_index (p, smax, abscoeff, 1, maxn*maxn);
	// gsl_sort_index (p, abscoeff, 1, maxn*maxn);
	if(smax>20){
		for(int i=smin; i<smin+20; i++)
			cerr << "p["<<i<<"]="<<p[i]<<" abscoeff["
			<<p[i]<<"]="<<abscoeff[p[i]]
			                       << " data["<<p[i]<<"]=" << data[p[i]]<<endl;
	}
	// select the important coeff
	int *dpi=new int[maxn*maxn];
	for (int i=0; i<maxn*maxn; i++)
		dpi[i]=0;
	for(int i=smin; i<smax; i++)
		dpi[p[i]]=1;
	for (int i=0; i<maxn*maxn; i++)
		if(dpi[i]==0) data[i]=0.0;
	delete [] dpi;
	//    // inverse transformation
	if(GSL_SUCCESS!=gsl_wavelet2d_transform_inverse(
	                                                w, data, maxn, maxn, maxn, work)){
		cerr << "error in wavelet: gsl_wavelet2d_transform_inverse" << endl;
		return 0;
	}

	// fill it back to the grid
	for(int i=0; i<nrows; i++)
		for(int j=0; j<ncols; j++)
			gx->feld[i][j]=data[i*ncols+j];

	//     // clean up
	gsl_wavelet_free(w);
	gsl_wavelet_workspace_free (work);
	delete [] data;
	delete [] abscoeff;
	delete [] p;
	// end
	return gx;
}
*/

/*
grid* grid::wavelet_s(int smin, int smax, int koeff)
{
	if(nrows!=ncols) return 0;
	if(smin < 0) return 0;
	cerr << "wavelet: " << smin << " " << smax << endl;
	grid *gx= new grid(100);
	gx=grid_copy();
	// construct a matrix(n,n) with n=2^k
	int maxn;
	(nrows>ncols) ? maxn=nrows : maxn=ncols;
	int val=2;
	while(val<maxn) val*=2;
	maxn=val;
	double *data=new double[maxn*maxn];
	cerr << "wavelet new matrix with n=" << maxn << " created" <<endl;
	if(data==0){
		cerr << " error in wavelet: no space left on device" << endl;
		return 0;
	}
	// optimal threshold parameter
	if(smax==-1){
		int nc1,nc2;
		double lambda,median,mad;
		double *sortcoeff,*mm;
		sortcoeff=new double[nrows];
		mm=new double[nrows];
		for(int i=0; i<nrows; i++) sortcoeff[i]=feld[i][0];
		qsort(sortcoeff, nrows, sizeof(double), dcompare);
		median=0.5*(sortcoeff[nrows/2]+sortcoeff[nrows/2+1]);
		for(int i=0; i<nrows; i++)mm[i]=fabs(feld[i][0]-median);
		qsort(mm, nrows, sizeof(double), dcompare);
		mad=0.5*(mm[nrows/2]+mm[nrows/2+1]);
		lambda=sqrt(2.0*log(static_cast<double>(nrows)))*sqrt(mad/0.6745);
		nc1=nrows-1;
		fprintf(stderr,"lambda=%lf ",lambda);
		while(mm[nc1--]>lambda);
		nc1=nrows-nc1;
		fprintf(stderr,"nc=%d ",nc1);
		for(int i=0; i<ncols; i++) sortcoeff[i]=feld[0][i];
		qsort(sortcoeff, ncols, sizeof(double), dcompare);
		median=0.5*(sortcoeff[ncols/2]+sortcoeff[ncols/2+1]);
		for(int i=0; i<ncols; i++)mm[i]=fabs(feld[0][i]-median);
		qsort(mm, ncols, sizeof(double), dcompare);
		mad=0.5*(mm[ncols/2]+mm[ncols/2+1]);
		lambda=sqrt(2.0*log(static_cast<double>(ncols)))*sqrt(mad/0.6745);
		nc2=nrows-1;
		fprintf(stderr,"lambda=%lf ",lambda);
		while(mm[nc2--]>lambda);
		nc2=nrows-nc1;
		fprintf(stderr,"nc=%d ",nc2);
		smax=nc1*nc2;
		fprintf(stderr,"smax=%d\n",smax);
		delete mm;
		delete sortcoeff;
	}
	// fill the matrix
	for(int i=0; i<maxn; i++)
		for(int j=0; j<maxn; j++)
			data[i*maxn+j]=0.0;
	for(int i=0; i<nrows; i++)
		for(int j=0; j<ncols; j++)
			data[i*ncols+j]=feld[i][j];
	// organize the wavelet space
	gsl_wavelet *w;
	gsl_wavelet_workspace *work;
	if(!(
			(koeff==103) ||
			(koeff==105) ||
			(koeff==202) ||
			(koeff==204) ||
			(koeff==206) ||
			(koeff==208) ||
			(koeff==301) ||
			(koeff==303) ||
			(koeff==305) ||
			(koeff==307) ||
			(koeff==309))) koeff=103;
	w = gsl_wavelet_alloc (gsl_wavelet_bspline, koeff);
	work = gsl_wavelet_workspace_alloc(maxn);
	// make the transformation
	if(GSL_SUCCESS!=gsl_wavelet2d_transform_forward(
	                                                w, data, maxn, maxn, maxn, work)){
		cerr << "error in wavelet: gsl_wavelet2d_transform_forward" << endl;
		return 0;
	}
	// sorting
	double *abscoeff = new double[maxn*maxn];
	size_t *p = new size_t[smax];
	if(p==0 || abscoeff==0){
		cerr << " error in wavelet: no space left on device" << endl;
		return 0;
	}
	for (int i=0; i<maxn*maxn; i++)
	{
		abscoeff[i] = fabs(data[i]);
	}
	gsl_sort_largest_index (p, smax, abscoeff, 1, maxn*maxn);
	// gsl_sort_index (p, abscoeff, 1, maxn*maxn);
	if(smax>20){
		for(int i=smin; i<smin+20; i++)
			cerr << "p["<<i<<"]="<<p[i]<<" abscoeff["
			<<p[i]<<"]="<<abscoeff[p[i]]
			                       << " data["<<p[i]<<"]=" << data[p[i]]<<endl;
	}
	// select the important coeff
	int *dpi=new int[maxn*maxn];
	for (int i=0; i<maxn*maxn; i++)
		dpi[i]=0;
	for(int i=smin; i<smax; i++)
		dpi[p[i]]=1;
	for (int i=0; i<maxn*maxn; i++)
		if(dpi[i]==0) data[i]=0.0;
	delete [] dpi;
	//    // inverse transformation
	if(GSL_SUCCESS!=gsl_wavelet2d_transform_inverse(
	                                                w, data, maxn, maxn, maxn, work)){
		cerr << "error in wavelet: gsl_wavelet2d_transform_inverse" << endl;
		return 0;
	}

	// fill it back to the grid
	for(int i=0; i<nrows; i++)
		for(int j=0; j<ncols; j++)
			gx->feld[i][j]=data[i*ncols+j];

	//     // clean up
	gsl_wavelet_free(w);
	gsl_wavelet_workspace_free (work);
	delete [] data;
	delete [] abscoeff;
	delete [] p;
	// end
	return gx;
}
*/

/*
grid* grid::wavelet_wh()
{
	grid *gx= new grid(100);
	gx=grid_copy();
	// construct a matrix(n,n) with n=2^k
	int maxn;
	(nrows>ncols) ? maxn=nrows : maxn=ncols;
	int val=2;
	while(val<maxn) val*=2;
	maxn=val;
	double *data=new double[maxn*maxn];
	cerr << "wavelet new matrix with n=" << maxn << " created" <<endl;
	if(data==0){
		cerr << " error in wavelet: no space left on device" << endl;
		return 0;
	}
	// fill the matrix
	for(int i=0; i<maxn; i++)
		for(int j=0; j<maxn; j++)
			data[i*maxn+j]=0.0;
	for(int i=0; i<nrows; i++)
		for(int j=0; j<ncols; j++)
			data[i*ncols+j]=feld[i][j];
	// organize the wavelet space
	gsl_wavelet *w;
	gsl_wavelet_workspace *work;
	w = gsl_wavelet_alloc (gsl_wavelet_haar, 2);
	work = gsl_wavelet_workspace_alloc(maxn);
	// make the transformation
	if(GSL_SUCCESS!=gsl_wavelet2d_transform_forward(
	                                                w, data, maxn, maxn, maxn, work)){
		cerr << "error in wavelet: gsl_wavelet2d_transform_forward" << endl;
		return 0;
	}
	// fill it back to the grid in centered way
	for(int i=0; i<nrows/2; i++){
		for(int j=0; j<ncols/2; j++){
			gx->feld[nrows/2-i][ncols/2-j]=data[i*ncols+j];
			gx->feld[nrows/2-i][ncols/2+j]=data[i*ncols+j];
			gx->feld[nrows/2+i][ncols/2-j]=data[i*ncols+j];
			gx->feld[nrows/2+i][ncols/2+j]=data[i*ncols+j];
		}
	}

	//     // clean up
	gsl_wavelet_free(w);
	gsl_wavelet_workspace_free (work);
	delete [] data;
	return gx;
}
*/

/*
grid* grid::wavelet_ws(int koeff)
{
	grid *gx= new grid(100);
	gx=grid_copy();
	// construct a matrix(n,n) with n=2^k
	int maxn;
	(nrows>ncols) ? maxn=nrows : maxn=ncols;
	int val=2;
	while(val<maxn) val*=2;
	maxn=val;
	double *data=new double[maxn*maxn];
	cerr << "wavelet new matrix with n=" << maxn << " created" <<endl;
	if(data==0){
		cerr << " error in wavelet: no space left on device" << endl;
		return 0;
	}
	// fill the matrix
	for(int i=0; i<maxn; i++)
		for(int j=0; j<maxn; j++)
			data[i*maxn+j]=0.0;
	for(int i=0; i<nrows; i++)
		for(int j=0; j<ncols; j++)
			data[i*ncols+j]=feld[i][j];
	// organize the wavelet space
	gsl_wavelet *w;
	gsl_wavelet_workspace *work;
	if(!(
			(koeff==103) ||
			(koeff==105) ||
			(koeff==202) ||
			(koeff==204) ||
			(koeff==206) ||
			(koeff==208) ||
			(koeff==301) ||
			(koeff==303) ||
			(koeff==305) ||
			(koeff==307) ||
			(koeff==309))) koeff=103;
	w = gsl_wavelet_alloc (gsl_wavelet_bspline, koeff);
	work = gsl_wavelet_workspace_alloc(maxn);
	// make the transformation
	if(GSL_SUCCESS!=gsl_wavelet2d_transform_forward(
	                                                w, data, maxn, maxn, maxn, work)){
		cerr << "error in wavelet: gsl_wavelet2d_transform_forward" << endl;
		return 0;
	}
	// fill it back to the grid in centered way
	for(int i=0; i<nrows/2; i++){
		for(int j=0; j<ncols/2; j++){
			gx->feld[nrows/2-i][ncols/2-j]=data[i*ncols+j];
			gx->feld[nrows/2-i][ncols/2+j]=data[i*ncols+j];
			gx->feld[nrows/2+i][ncols/2-j]=data[i*ncols+j];
			gx->feld[nrows/2+i][ncols/2+j]=data[i*ncols+j];
		}
	}

	// fill it back to the grid
	// for(int i=0; i<nrows; i++)
	//   for(int j=0; j<ncols; j++)
	//    gx->feld[i][j]=data[i*ncols+j];

	//     // clean up
	gsl_wavelet_free(w);
	gsl_wavelet_workspace_free (work);
	delete [] data;
	return gx;
}
*/

/*
grid* grid::wavelet_wd(int koeff)
{
	grid *gx= new grid(100);
	gx=grid_copy();
	// construct a matrix(n,n) with n=2^k
	int maxn;
	(nrows>ncols) ? maxn=nrows : maxn=ncols;
	int val=2;
	while(val<maxn) val*=2;
	maxn=val;
	double *data=new double[maxn*maxn];
	cerr << "wavelet new matrix with n=" << maxn << " created" <<endl;
	if(data==0){
		cerr << " error in wavelet: no space left on device" << endl;
		return 0;
	}
	// fill the matrix
	for(int i=0; i<maxn; i++)
		for(int j=0; j<maxn; j++)
			data[i*maxn+j]=0.0;
	for(int i=0; i<nrows; i++)
		for(int j=0; j<ncols; j++)
			data[i*ncols+j]=feld[i][j];
	// organize the wavelet space
	gsl_wavelet *w;
	gsl_wavelet_workspace *work;
	if(koeff<4 || koeff%2!=0)koeff=4;
	w = gsl_wavelet_alloc (gsl_wavelet_daubechies, koeff);
	work = gsl_wavelet_workspace_alloc(maxn);
	// make the transformation
	if(GSL_SUCCESS!=gsl_wavelet2d_transform_forward(
	                                                w, data, maxn, maxn, maxn, work)){
		cerr << "error in wavelet: gsl_wavelet2d_transform_forward" << endl;
		return 0;
	}
	// fill it back to the grid in centered way
	for(int i=0; i<nrows/2; i++){
		for(int j=0; j<ncols/2; j++){
			gx->feld[nrows/2-i][ncols/2-j]=data[i*ncols+j];
			gx->feld[nrows/2-i][ncols/2+j]=data[i*ncols+j];
			gx->feld[nrows/2+i][ncols/2-j]=data[i*ncols+j];
			gx->feld[nrows/2+i][ncols/2+j]=data[i*ncols+j];
		}
	}

	// fill it back to the grid
	// for(int i=0; i<nrows; i++)
	// for(int j=0; j<ncols; j++)
	//    gx->feld[i][j]=data[i*ncols+j];

	//     // clean up
	gsl_wavelet_free(w);
	gsl_wavelet_workspace_free (work);
	delete [] data;
	return gx;
}
*/

grid* grid::fractal(int iter)
{
	if(iter<2 || iter>100000) return 0;
	grid *gx= new grid(100);
	gx=grid_copy();
	list<struct part*> plist;
	list<struct part*>::iterator li;
	struct part *pp0, *pp1, *pp2;
	double variance=0.0;
	double mean=0.0;
	int count=0;
	struct part* part0=new struct part;
	if(part0==0){
		fprintf(stderr,"error in new struct part\n");
		return 0;
	}
	part0->xa=0;
	part0->xb=ncols;
	part0->ya=0;
	part0->yb=nrows;
	for(int i=0; i<nrows; i++){
		for(int j=0; j<ncols; j++){
			if((int)feld[i][j]!=nodata){
				mean+=feld[i][j];
				count++;
			}
		}
	}
	if(count==0){
		fprintf(stderr,"error count = %d\n",count);
		return 0;
	}
	mean/=count;
	for(int i=0; i<nrows; i++){
		for(int j=0; j<ncols; j++){
			if((int)feld[i][j]!=nodata){
				variance+=(feld[i][j]-mean)*(feld[i][j]-mean);
			}
		}
	}
	variance/=count;
	part0->var=variance;
	plist.push_front(part0);
	//int dx,dy;
	//double var0=variance;
	for(int k=0; k<iter; k++){
		// find maximum element in the list
		double mvar=0;
		for(li=plist.begin(); li!=plist.end(); ++li){
			if(mvar<(*li)->var){
				mvar=(*li)->var;
				pp0=(*li);
			}
		}
		if(pp0->xb-pp0->xa<=1 && pp0->yb-pp0->ya<=1) goto END;

		if(k%100==0) fprintf(stderr,"%d  %lf\n",k,pp0->var);
		// split the pp0 in pp1 and pp2
		pp1=new struct part;
		pp2=new struct part;
		if(pp1==0 || pp2==0){
			fprintf(stderr,"error in create pp1=%p or pp2=%p\n",pp1,pp2);
			exit(1);
		}
		if(pp0->xb-pp0->xa >= pp0->yb-pp0->ya){
			pp1->xa=pp0->xa;
			pp1->xb=(int)((pp0->xa+(pp0->xb-pp0->xa)/2));
			pp1->ya=pp0->ya;
			pp1->yb=pp0->yb;
			pp2->xa=(int)((pp0->xa+(pp0->xb-pp0->xa)/2));
			pp2->xb=pp0->xb;
			pp2->ya=pp0->ya;
			pp2->yb=pp0->yb;
		}
		else{
			pp1->xa=pp0->xa;
			pp1->xb=pp0->xb;
			pp1->ya=pp0->ya;
			pp1->yb=(int)((pp0->ya+(pp0->yb-pp0->ya)/2));
			pp2->xa=pp0->xa;
			pp2->xb=pp0->xb;
			pp2->ya=(int)((pp0->ya+(pp0->yb-pp0->ya)/2));
			pp2->yb=pp0->yb;
		}
		// calc variance fo pp1 and pp2;
		mean=variance=0.0;
		count=0;
		for(int i=pp1->ya; i<pp1->yb; i++){
			for(int j=pp1->xa; j<pp1->xb; j++){
				if((int)feld[i][j]!=nodata){
					mean+=feld[i][j];
					count++;
				}
			}
		}
		if(count==0){
			fprintf(stderr,"error pp1 count = %d\t %d %d %d %d\n",count,
			        pp1->xa, pp1->xb, pp1->ya, pp1->yb);
			delete pp1;
			delete pp2;
			goto END;
		}
		mean/=count;
		for(int i=pp1->ya; i<pp1->yb; i++){
			for(int j=pp1->xa; j<pp1->xb; j++){
				if((int)feld[i][j]!=nodata){
					variance+=(feld[i][j]-mean)*
							(feld[i][j]-mean);
				}
			}
		}
		variance/=count;
		pp1->var=variance;
		mean=variance=0.0;
		count=0;
		for(int i=pp2->ya; i<pp2->yb; i++){
			for(int j=pp2->xa; j<pp2->xb; j++){
				if((int)feld[i][j]!=nodata){
					mean+=feld[i][j];
					count++;
				}
			}
		}
		if(count==0){
			fprintf(stderr,"error pp2 count = %d\t %d %d %d %d\n",count,
			        pp2->xa,pp2->xb,pp2->ya,pp2->yb);
			return 0;
		}
		mean/=count;
		for(int i=pp2->ya; i<pp2->yb; i++){
			for(int j=pp2->xa; j<pp2->xb; j++){
				if((int)feld[i][j]!=nodata){
					variance+=(feld[i][j]-mean)*
							(feld[i][j]-mean);
				}
			}
		}
		variance/=count;
		pp2->var=variance;
		// add new parts to the list
		plist.remove(pp0);
		plist.push_front(pp1);
		plist.push_front(pp2);
		delete pp0;
	}
	// return of mean of parts
	END:
	// generate a histogram over the size
	double min_size, max_size, size;
	min_size=FLT_MAX;
	max_size=-FLT_MAX;
	for(li=plist.begin(); li!=plist.end(); ++li){
		size=((*li)->xb-(*li)->xa)*((*li)->yb-(*li)->ya);
		if(max_size<size)max_size=size;
		if(min_size>size)min_size=size;
	}
	cerr << "max_size: " << max_size << " min_size: " << min_size << endl;
	//double delta=max_size-min_size;
	vector<double> hist(HIST);
	int selector;
	for(li=plist.begin(); li!=plist.end(); ++li){
		size=((*li)->xb-(*li)->xa)*((*li)->yb-(*li)->ya);
		selector=(int)((double)HIST*(size-min_size)/(max_size-min_size));
		if(selector==HIST)selector--;
		++hist[selector];
		// cerr << size << " " << selector << endl;
	}
	for(int i=0; i<HIST; i++)
		cerr << (double)i/((double)HIST)*(max_size-min_size)+min_size
		<< "\t : \t" << hist[i] << endl;

	// make output data
	for(li=plist.begin(); li!=plist.end(); ++li){
		mean=0.0;
		count=0;
		for(int i=(*li)->ya; i<(*li)->yb; i++){
			for(int j=(*li)->xa; j<(*li)->xb; j++){
				if((int)feld[i][j]!=nodata){
					mean+=feld[i][j];
					count++;
				}
			}
		}
		if(count>0) mean/=count;
		for(int i=(*li)->ya; i<(*li)->yb; i++){
			for(int j=(*li)->xa; j<(*li)->xb; j++){
				if((int)feld[i][j]!=nodata){
					// size=((*li)->xb-(*li)->xa)*((*li)->yb-(*li)->ya);
					// if(size<20*min_size) feld[i][j]=nodata;
					// else
					gx->feld[i][j]=mean;
				}
			}
		}
	}
	// clean up
	for(li=plist.begin(); li!=plist.end(); ++li){
		delete (*li);
	}
	return gx;
}

/*
grid* grid::convolution(int R)
{
	if(R<2 || R>=nrows || R>=ncols){
		cerr << "error in convolution: R=" << R << " too big\n";
		return 0;
	}
	grid* gx=new grid(100);
	grid* gauss=new grid(100);
	fftw_complex *z1, *z2, *o1, *o2;
	fftw_plan p;
	z1 = (fftw_complex *) fftw_malloc(sizeof(fftw_complex)*
	                                  (nrows)*(ncols));
	z2 = (fftw_complex *) fftw_malloc(sizeof(fftw_complex)*
	                                  (nrows)*(ncols));
	o1 = (fftw_complex *) fftw_malloc(sizeof(fftw_complex)*
	                                  (nrows)*(ncols));
	o2 = (fftw_complex *) fftw_malloc(sizeof(fftw_complex)*
	                                  (nrows)*(ncols));
	if(z1==0 || z2==0 || o1==0 || o2==0){
		cerr << "error in convoultion: no space on device\n";
		return 0;
	}
	gx=grid_copy();    // a new grid as output
	gauss=grid_copy(); // a new grid for gaussian bell shape
	// generate the gaussian grid and make normalization
	int nrows2=nrows/2;
	int ncols2=ncols/2;
	double val;
	double sum=0.0;
	for(int i=0; i<nrows; i++){
		for(int j=0; j<ncols; j++){
			val=exp(-(double)((i-nrows2)*(i-nrows2)+(j-ncols2)*(j-ncols2))/(R*R));
			gauss->feld[i][j]=(float)val;
			sum+=val;
		}
	}
	for(int i=0; i<nrows; i++){
		for(int j=0; j<ncols; j++){
			gauss->feld[i][j]/=(float)sum;
		}
	}
	// fill the array z1 and z2 and make fft
	for(int i=0; i<nrows; i++){
		for(int j=0; j<ncols; j++){
			z1[i*ncols+j][0]=(double)(feld[i][j]);
			z2[i*ncols+j][0]=(double)(gauss->feld[i][j]);
			z1[i*ncols+j][1]=z2[i*(ncols)+j][1]=0.0;
		}
	}
	p = fftw_plan_dft_2d(nrows, ncols, z1,o1,
	                     FFTW_FORWARD,FFTW_ESTIMATE);
	fftw_execute(p);
	fftw_destroy_plan(p);
	p = fftw_plan_dft_2d(nrows, ncols, z2,o2,
	                     FFTW_FORWARD,FFTW_ESTIMATE);
	fftw_execute(p);
	fftw_destroy_plan(p);
	// calculate the akf
	double n=(double)(nrows*ncols);
	for(int i=0; i<nrows; i++){
		for(int j=0; j<ncols; j++){
			z1[i*ncols+j][0]=
					o1[i*ncols+j][0]*o2[i*ncols+j][0]+
					o1[i*ncols+j][1]*o2[i*ncols+j][1];
			z1[i*ncols+j][1]=
					o1[i*ncols+j][1]*o2[i*ncols+j][0]-
					o1[i*ncols+j][0]*o2[i*ncols+j][1];
		}
	}
	// backward fft and new order of result
	p = fftw_plan_dft_2d(nrows,ncols,z1,z2,
	                     FFTW_BACKWARD,FFTW_ESTIMATE);
	fftw_execute(p);
	fftw_destroy_plan(p);
	int j2,i2;
	for(int i=0; i<nrows; i++){
		for(int j=0; j<ncols; j++){
			i <= nrows2 ? i2=nrows2-i : i2=nrows-(i-nrows2);
			j <= ncols2 ? j2=ncols2-j : j2=ncols-(j-ncols2);
			gx->feld[nrows-1-i2][ncols-1-j2]=
					z2[i*ncols+j][0]/n;
		}
	}
	// clean up
	delete gauss;
	fftw_free(z1);
	fftw_free(z2);
	fftw_free(o1);
	fftw_free(o2);
	return gx;
}
*/

/*
grid* grid::akf()
{
	grid* gx=new grid(100);
	fftw_complex *z1, *z2, *o1;
	fftw_plan p;
	z1 = (fftw_complex *) fftw_malloc(sizeof(fftw_complex)*
	                                  (nrows)*(ncols));
	z2 = (fftw_complex *) fftw_malloc(sizeof(fftw_complex)*
	                                  (nrows)*(ncols));
	o1 = (fftw_complex *) fftw_malloc(sizeof(fftw_complex)*
	                                  (nrows)*(ncols));
	if(z1==0 || z2==0 || o1==0){
		cerr << "error in convoultion: no space on device\n";
		return 0;
	}
	gx=grid_copy();    // a new grid as output
	int nrows2=nrows/2;
	int ncols2=ncols/2;
	double sum=0.0;
	for(int i=0; i<nrows; i++)
		for(int j=0; j<ncols; j++)
			sum+=feld[i][j];
	// fill the array z1 and z2 and make fft
	for(int i=0; i<nrows; i++){
		for(int j=0; j<ncols; j++){
			z1[i*ncols+j][0]=(double)(feld[i][j]);
			z2[i*ncols+j][0]=z1[i*ncols+j][0];
			z1[i*ncols+j][1]=z2[i*(ncols)+j][1]=0.0;
		}
	}
	p = fftw_plan_dft_2d(nrows, ncols, z1,o1,
	                     FFTW_FORWARD,FFTW_ESTIMATE);
	fftw_execute(p);
	fftw_destroy_plan(p);
	// calculate the akf
	double n=(double)(nrows*ncols);
	// double omega=2.0*3.1415926/(sqrt(nrows*nrows+ncols*ncols));
	for(int i=0; i<nrows; i++){
		for(int j=0; j<ncols; j++){
			z1[i*ncols+j][0]=
					o1[i*ncols+j][0]*o1[i*ncols+j][0]+
					o1[i*ncols+j][1]*o1[i*ncols+j][1];
			z1[i*ncols+j][1]=
					o1[i*ncols+j][1]*o1[i*ncols+j][0]-
					o1[i*ncols+j][0]*o1[i*ncols+j][1];
			// z1[i*ncols+j][0]=-o1[i*ncols+j][0]*sqrt(i*i+j*j)*omega;
			// z1[i*ncols+j][1]=o1[i*ncols+j][1]*sqrt(i*i+j*j)*omega;
		}
	}
	// backward fft and new order of result
	p = fftw_plan_dft_2d(nrows,ncols,z1,z2,
	                     FFTW_BACKWARD,FFTW_ESTIMATE);
	fftw_execute(p);
	fftw_destroy_plan(p);
	int j2,i2;
	for(int i=0; i<nrows; i++){
		for(int j=0; j<ncols; j++){
			i <= nrows2 ? i2=nrows2-i : i2=nrows-(i-nrows2);
			j <= ncols2 ? j2=ncols2-j : j2=ncols-(j-ncols2);
			gx->feld[nrows-1-i2][ncols-1-j2]=
					z2[i*ncols+j][0]/(n*sum);
			// gx->feld[i][j]=
					// z2[i*ncols+j][0]/n;

		}
	}
	// clean up
	fftw_free(z1);
	fftw_free(z2);
	fftw_free(o1);
	return gx;
}
*/

// grid* grid::edge()
// {
//     grid* gx;
//     gx=grid_copy();
//     double  n,m;
//     const float sq2=sqrt(2.0);
//     for(int i=1; i<nrows-1; i++){
// 	for(int j=1; j<ncols-1; j++){
// 	    gx->feld[i][j]=gx->nodata;
// 	    if(int(feld[i][j])!=nodata){
// 		if(int(feld[i-1][j-1])!=nodata &&
// 		   int(feld[i][j-1])!=nodata &&
// 		   int(feld[i+1][j-1])!=nodata &&
// 		   int(feld[i-1][j])!=nodata &&
// 		   int(feld[i][j])!=nodata &&
// 		   int(feld[i+1][j])!=nodata &&
// 		   int(feld[i-1][j+1])!=nodata &&
// 		   int(feld[i][j+1])!=nodata &&
// 		   int(feld[i+1][j+1])!=nodata){
// 		    n= feld[i-1][j+1]+sq2*feld[i][j+1]+feld[i+1][j+1]
// 			-feld[i-1][j-1]-sq2*feld[i][j-1]-feld[i+1][j-1];
// 		    m=feld[i+1][j-1]+sq2*feld[i+1][j]+feld[i+1][j+1]-
// 			feld[i-1][j-1]-sq2*feld[i-1][j]+feld[i-1][j+1];
// 		    gx->feld[i][j]=(float)(fabs(n)+fabs(m));
// 		}
// 	    }
// 	}
//     }
//     return gx;
// }

grid* grid::rotate(int beta1)
{
	grid* gx=new grid(100);
	struct vertex a,b,c,d;
	const double deg=3.1415926/180.0;
	double xm,ym,beta;
	if(beta1==0){
		gx->grid_copy();
		return gx;
	}
	if(beta1==90 || beta1==-270){
		gx->nodata=nodata;
		gx->xcorner = ycorner;
		gx->ycorner = xcorner;
		gx->ncols=nrows;
		gx->nrows=ncols;
		gx->csize = csize;
		gx->feld=new float*[gx->nrows];
		for(int i=0; i<gx->nrows; i++){
			gx->feld[i]=new float[gx->ncols];
			if(gx->feld[i]==0){
				fprintf(stderr,"error in rotate: no space on device\n");
				return 0;
			}
		}
		for(int i=0; i<nrows; i++)
			for(int j=0; j<ncols; j++)
				gx->feld[j][i]=feld[nrows-i-1][j];
		return gx;
	}
	if(beta1==270 || beta1==-90){
		gx->nodata=nodata;
		gx->xcorner = ycorner;
		gx->ycorner = xcorner;
		gx->ncols=nrows;
		gx->nrows=ncols;
		gx->csize = csize;
		gx->feld=new float*[gx->nrows];
		for(int i=0; i<gx->nrows; i++){
			gx->feld[i]=new float[gx->ncols];
			if(gx->feld[i]==0){
				fprintf(stderr,"error in rotate: no space on device\n");
				return 0;
			}
		}
		for(int i=0; i<nrows; i++)
			for(int j=0; j<ncols; j++)
				gx->feld[j][i]=feld[i][ncols-j-1];
		return gx;
	}
	if(beta1==180){
		gx->nodata=nodata;
		gx->xcorner = ycorner;
		gx->ycorner = xcorner;
		gx->ncols=ncols;
		gx->nrows=nrows;
		gx->csize = csize;
		gx->feld=new float*[gx->nrows];
		for(int i=0; i<gx->nrows; i++){
			gx->feld[i]=new float[gx->ncols];
			if(gx->feld[i]==0){
				fprintf(stderr,"error in rotate: no space on device\n");
				return 0;
			}
		}
		for(int i=0; i<nrows; i++)
			for(int j=0; j<ncols; j++)
				gx->feld[i][j]=feld[nrows-i-1][j];
		return gx;
	}
	return 0;
	xm=ncols/2.0;
	ym=nrows/2.0;
	beta=beta1*deg;   // rotation in clock direction
	fprintf(stderr,"beta1=%d beta=%f\n",beta1,beta);
	double radius=ceil(sqrt(nrows/2.0*nrows/2.0+ncols/2.0*ncols/2.0));
	int n_nrows,n_ncols;
	n_ncols=(int)(2*radius);
	n_nrows=(int)(2*radius);
	// lower left corner
	a.a=(0-xm)*cos(beta)-(0-ym)*sin(beta)+radius;
	a.b=(0-xm)*sin(beta)+(0-ym)*cos(beta)+radius;
	// upper left corner
	b.a=(0-xm)*cos(beta)-(nrows-ym)*sin(beta)+radius;
	b.b=(0-xm)*sin(beta)+(nrows-ym)*cos(beta)+radius;
	// upper right corner
	c.a=(ncols-xm)*cos(beta)-(nrows-ym)*sin(beta)+radius;
	c.b=(ncols-xm)*sin(beta)+(nrows-ym)*cos(beta)+radius;
	// lower right corner
	d.a=(ncols-xm)*cos(beta)-(0-ym)*sin(beta)+radius;
	d.b=(ncols-xm)*sin(beta)+(0-ym)*cos(beta)+radius;
	fprintf(stderr,"P1 %f %f\nP2 %f %f\nP3 %f %f\nP4 %f %f\n",
	        a.a,a.b,b.a,b.b,c.a,c.b,d.a,d.b);
	fprintf(stderr,"nrows=%d ncols=%d\n",n_ncols,n_nrows);
	gx->feld= new float*[n_nrows];
	for(int i=0; i<n_nrows; i++){
		if((gx->feld[i]= new float[n_ncols])==NULL){
			cerr << "error (read_ascii): no sufficient memory" << endl;
		}
	}
	gx->nodata=nodata;
	gx->xcorner = xcorner;
	gx->ycorner = ycorner;
	gx->ncols=n_ncols;
	gx->nrows=n_nrows;
	gx->csize = csize;
	for(int i=0; i<n_nrows; i++){
		for(int j=0; j<n_ncols; j++){
			gx->feld[i][j]=nodata;
		}
	}

	xm=ncols/2.0;
	ym=nrows/2.0;
	double deltax,deltay;
	deltay=n_nrows/2.0;
	deltax=n_ncols/2.0;
	double cb,sb;
	cb=cos(beta);
	sb=sin(beta);
	fprintf(stderr,"cb=%f sb=%f xm=%f ym=%f\n",cb,sb,xm,ym);
	struct triple *tp=new struct triple[nrows*ncols];
	if(tp==0){
		fprintf(stderr,"error (rotate): no space left on device\n");
		return 0;
	}
	for(int i=0; i<nrows; i++){
		for(int j=0; j<ncols; j++){
			tp[i*ncols+j].x=(j-xm)*cb-(i-ym)*sb+deltax;
			tp[i*ncols+j].y=(j-xm)*sb+(i-ym)*cb+deltay;
			tp[i*ncols+j].z=feld[i][j];
		}
	}
	for(int i=0; i<n_nrows; i++){
		for(int j=0; j<n_ncols; j++){
			for(int k=0; k<nrows*ncols; k++){
				if(((tp[k].x-j)*(tp[k].x-j)+(tp[k].y-i)*(tp[k].y-i))<0.75){
					gx->feld[i][j]=tp[k].z;
					break;
				}
			}
		}
	}
	delete [] tp;
	return gx;
}

void grid::class_grid(float minx, float maxx, float step)
{
	int n;
	if(step<=0 || minx>=maxx) return;
	for(int i=0; i<nrows; i++){
		for(int j=0; j<ncols; j++){
			if(int(feld[i][j])!=nodata){
				if(feld[i][j]<=minx) feld[i][j]=minx;
				if(feld[i][j]>=maxx) feld[i][j]=maxx;
				if(feld[i][j]>minx && feld[i][j]<maxx){
					n=int((feld[i][j]-minx)/step);
					feld[i][j]=n*step+minx;
				}
			}
		}
	}
}

// cluster2value takes one value grid and allocates the mean of the
// values to the cluster x

grid* grid::cluster2value(grid* vgrid)
{
	grid *gx=grid_copy(); // output grid
	mapType clusterCounter;
	doubleMap Cluster;
	mapType Member;
	int nCluster=0;
	for(int i=0; i<nrows; i++){
		for(int j=0; j<ncols; j++){
			if((int)(feld[i][j])!=nodata &&
					(int)vgrid->feld[i][j]!=vgrid->nodata){
				if(clusterCounter.end()==
						clusterCounter.find((int)feld[i][j])){
					clusterCounter.insert(ValuePair((int)feld[i][j],1));
					nCluster++;
					if(Cluster.end()==Cluster.find((int)feld[i][j])){
						cerr << "new cluster: " << (int)feld[i][j]
						                                        << endl;
						Cluster.insert(doublePair((int)feld[i][j],
						                          vgrid->feld[i][j]));
					}
					else
						Cluster[(int)feld[i][j]]+=vgrid->feld[i][j];
					if(Member.end()==Member.find((int)feld[i][j]))
						Member.insert(ValuePair((int)feld[i][j],1));
					else
						Member[(int)feld[i][j]]+=1;
				}
			}
		}
	}
	for(int i=0; i<nrows; i++){
		for(int j=0; j<ncols; j++){
			if((int)(feld[i][j])!=nodata &&
					(int)vgrid->feld[i][j]!=vgrid->nodata){
				if(Member[(int)feld[i][j]]!=0)
					gx->feld[i][j]=Cluster[(int)feld[i][j]]/
					(double)Member[(int)feld[i][j]];
				else
					gx->feld[i][j]=gx->nodata;
			}
			else
				gx->feld[i][j]=gx->nodata;
		}
	}
	return gx;
}

// some erosion algorithms

void grid::w_fill()
{
	grid* gx=w_focalflow();
	float val;
	for(int i=0; i<nrows; i++){
		for(int j=0; j<ncols; j++){
			if(int(feld[i][j])!=nodata){
				if(gx->feld[i][j]==255){
					for(int k=0; k<8;k++){
						val=FLT_MAX;
						if(i+1<nrows && val>feld[i+1][j])
							val=feld[i+1][j];
						if(j+1<ncols && val>feld[i][j+1])
							val=feld[i][j+1];
						if(i-1>=0 && val>feld[i-1][j])
							val=feld[i-1][j];
						if(j-1>=0 && val>feld[i][j-1])
							val=feld[i][j-1];
						if(i+1<nrows && j+1<ncols && val>feld[i+1][j+1])
							val=feld[i+1][j+1];
						if(i+1<nrows && j-1>=0 && val>feld[i+1][j-1])
							val=feld[i+1][j-1];
						if(i-1>=0 && j+1<ncols && val>feld[i-1][j+1])
							val=feld[i-1][j+1];
						if(i-1>=0 && j-1>=0 && val>feld[i-1][j-1])
							val=feld[i-1][j-1];
					}
					feld[i][j]=val;
				}
			}
		}
	}
	delete gx;
}

grid* grid::w_focalflow()
{
	grid *gx=grid_copy();
	for(int i=0; i<nrows; i++){
		for(int j=0; j<ncols; j++){
			if(int(feld[i][j])!=nodata){
				gx->feld[i][j]=0;
				if(j+1<ncols && feld[i][j]<feld[i][j+1])
					gx->feld[i][j]+=1; // E
				if(j+1<ncols && i+1<nrows && feld[i][j]<feld[i+1][j+1])
					gx->feld[i][j]+=2; //SE
				if(i+1<nrows && feld[i][j]<feld[i+1][j])
					gx->feld[i][j]+=4; // S
				if(j-1>=0 && i+1<nrows && feld[i][j]<feld[i+1][j-1])
					gx->feld[i][j]+=8; // SW
				if(j-1>=0 && feld[i][j]<feld[i][j-1])
					gx->feld[i][j]+=16; // W
				if(j-1>=0 && i-1>=0 && feld[i][j]<feld[i-1][j-1])
					gx->feld[i][j]+=32; // NW
				if(i-1>=0 && feld[i][j]<feld[i-1][j])
					gx->feld[i][j]+=64; // N
				if(j+1<ncols && i-1>=0 && feld[i][j]<feld[i-1][j+1])
					gx->feld[i][j]+=128; // NE
			}
		}
	}
	return gx;
}


// simple function for stack manipulation
stack2i::stack2i( int l)
{
	if(l<0 || l> MAXSTACK) return;
	feldx=new int[l];
	feldy=new int[l];
	if(feldx==0 || feldy==0){
		fprintf(stderr,"error in stack: no space left on divice: %d\n",l);
		return;
	}
	stackpointer=0;
	stackSize=l;
}

stack2i::~stack2i()
{
	delete [] feldx;
	delete [] feldy;
}

bool stack2i::pop( int &x,  int &y)
{
	if(stackpointer>0){
		x=feldx[stackpointer];
		y=feldy[stackpointer];
		stackpointer--;
		return true;
	}
	return false;
}

bool stack2i::push( int x,  int y)
{
	if(stackpointer<stackSize-1){
		stackpointer++;
		feldx[stackpointer]=x;
		feldy[stackpointer]=y;
		return true;
	}
	fprintf(stderr,"stack error: %d x=%d y=%d\n",stackpointer,x,y);
	return false;
}

void stack2i::emptyStack()
{
	//int x,y;
	stackpointer=0;
	// while(pop(x,y));
}


grid* grid::flood_fill(grid* gx, int x, int y, float val)
{
	fprintf(stderr,"flood_fill: %d %d %f\n",x,y,val);
	if(x<0 || y<0 || x>=ncols || y>=nrows){
		fprintf(stderr,"error in flood_fill: x=%d y=%d out of range\n",x,y);
		return (grid*)0;
	}
	if(val<=0){
		fprintf(stderr,"error in flood_fill: val=%f must be >0\n",val);
		return (grid*)0;
	}
	// grid* gx=grid_copy();
	stack2i *s=new stack2i(MAXSTACK);
	if(s==0) return (grid*) 0;
	s->emptyStack();
	float ground=feld[y][x];
	if(!(s->push(x,y))) return (grid*) 0;
	while(s->pop(x,y)){
		gx->feld[y][x]=-1;
		if(x+1<ncols && gx->feld[y][x+1]!=-1 && feld[y][x+1]<ground+val){
			if(!(s->push)(x+1,y)) return (grid*)0;
		}
		if(x-1>=0 && gx->feld[y][x-1]!=-1 && feld[y][x-1]<ground+val){
			if(!(s->push(x-1,y))) return (grid*)0;
		}
		if(y+1<nrows && gx->feld[y+1][x]!=-1 && feld[y+1][x]<ground+val){
			if(!(s->push(x,y+1))) return (grid*)0;
		}
		if(y-1>=0 && gx->feld[y-1][x]!=-1 && feld[y-1][x]<ground+val){
			if(!(s->push(x,y-1))) return (grid*)0;
		}
		if(x+1<ncols && y+1<nrows && gx->feld[y+1][x+1]!=-1 &&
				feld[y+1][x+1]<ground+val){
			if(!(s->push(x+1,y+1))) return (grid*)0;
		}
		if(x-1>=0 && y+1<nrows && gx->feld[y+1][x-1]!=-1 &&
				feld[y+1][x-1]<ground+val){
			if(!(s->push(x-1,y+1))) return (grid*)0;
		}
		if(x+1<ncols && y-1>=0 && gx->feld[y-1][x+1]!=-1 &&
				feld[y-1][x+1]<ground+val){
			if(!(s->push(x+1,y-1))) return (grid*)0;
		}
		if(x-1>=0 && y-1>=0 && gx->feld[y-1][x-1]!=-1 &&
				feld[y-1][x-1]<ground+val){
			if(!(s->push(x-1,y-1))) return (grid*)0;
		}
	}
	delete s;
	return gx;
}


grid* grid::delta_diff(grid *g1)
{
	if(g1==0) return 0;
	if(g1->nrows != nrows || g1->ncols != ncols) return 0;
	grid *gr=new grid(100);
	gr=grid_copy();
	for(int i=0; i<nrows; i++){
		for(int j=0; j<ncols; j++){
			if(feld[i][j]==nodata ||
					g1->feld[i][j]==g1->nodata ||
					feld[i][j]==0)
				gr->feld[i][j]=gr->nodata;
			else
				gr->feld[i][j]=fabs(feld[i][j]-g1->feld[i][j])/feld[i][j];
		}
	}
	return gr;
}

// very early version please do not use!
grid* grid::w_flowdirection()
{   // usage after w_fill !!!
	float x;
	float val,maxval;
	//int xxx, yyy,a,b;
	//float delta;
	//    stack2i *s=new stack2i(1000000);
	//    if(s==0){
	//	fprintf(stderr,"error in w_flowdirection: no space left\n");
	//	return (grid*) 0;
	//    }
	grid *gx=grid_copy();
	for(int i=0; i<nrows; i++){
		for(int j=0; j<ncols; j++){
			if(int(feld[i][j])!=nodata){
				gx->feld[i][j]=0;
				maxval=0.0;
				x=0;
				if(j+1<ncols && feld[i][j]>feld[i][j+1]){
					val=feld[i][j]-feld[i][j+1];
					if(val>maxval){
						maxval=val;
						x=1;  // E
					}
				}
				if(j+1<ncols && i+1<nrows && feld[i][j]>feld[i+1][j+1]){
					val=feld[i][j]-feld[i+1][j+1];
					if(val>maxval){
						maxval=val;
						x=2; // SE
					}
				}
				if(i+1<nrows && feld[i][j]>feld[i+1][j]){
					val=feld[i][j]-feld[i+1][j];
					if(val>maxval){
						maxval=val;
						x=4; // S
					}
				}
				if(j-1>=0 && i+1<nrows && feld[i][j]>feld[i+1][j-1]){
					val=feld[i][j]-feld[i+1][j-1];
					if(val>maxval){
						maxval=val;
						x=8;  // SW
					}
				}
				if(j-1>=0 && feld[i][j]>feld[i][j-1]){
					val=feld[i][j]-feld[i][j-1];
					if(val>maxval){
						maxval=val;
						x=16; // W
					}
				}
				if(j-1>=0 && i-1>=0 && feld[i][j]>feld[i-1][j-1]){
					val=feld[i][j]-feld[i-1][j-1];
					if(val>maxval){
						maxval=val;
						x=32; // NW
					}
				}
				if(i-1>=0 && feld[i][j]>feld[i-1][j]){
					val=feld[i][j]-feld[i-1][j];
					if(val>maxval){
						maxval=val;
						x=64; // N
					}
				}
				if(j+1<ncols && i-1>=0 && feld[i][j]>feld[i-1][j+1]){
					val=feld[i][j]-feld[i-1][j+1];
					if(val>maxval){
						maxval=val;
						x=128; // NE
					}
				}
				gx->feld[i][j]=x;
			}
		}
	}
	// correction on the border
	for(int i=0; i<gx->nrows; i++){
		if(gx->feld[i][0]!=gx->nodata)
			gx->feld[i][0]=16; // W
		if(gx->feld[i][gx->ncols-1]!=gx->nodata)
			gx->feld[i][gx->ncols-1]=1; // E
	}
	for(int j=0; j<gx->ncols; j++){
		if(gx->feld[0][j]!=gx->nodata)
			gx->feld[0][j]=64; // N
		if(gx->feld[gx->nrows-1][j]!=gx->nodata)
			gx->feld[gx->nrows-1][j]=4; // S
	}
	// collect zeros;

	//    delete s;
	return gx;
}

// not usable
grid* grid::w_d8()
{
	//int row,col;
	//bool flag;
	unsigned int steps;
	steps=nrows*ncols;
	steps=10;
	grid* ele=grid_copy();  // make a copy of the grid
	grid* out=grid_copy();  // make a copy of the grid
	ele->w_fill();    // fill it
	grid* flowd=ele->w_flowdirection(); // calc directed flow

	delete ele;
	delete out;
	return flowd;
}


// algorithm for dr. thinh
float grid::moore(int a, int b, int r, int PARA)
{
	float res=0;
	float r2=r*r;
	if(PARA==MOORE){
		for(int i=-r; i<=r; i++){
			for(int j=-r; j<=r; j++){
				if(a+i>=0 && b+j>=0
						&& i+a<nrows  && j+b<ncols
						&& int(feld[i+a][j+b])!=nodata)
					res+=feld[i+a][j+b];
			}
		}
		return res;
	}
	if(PARA==CIRCLE){
		for(int i=-r; i<=r; i++){
			for(int j=-r; j<=r; j++){
				if(a+i>=0 && b+j>=0
						&& i+a<nrows  && j+b<ncols
						&& int(feld[i+a][j+b])!=nodata
						&& ((i-r)*(i-r)+(j-r)*(j-r))<=r2)
					res+=feld[i+a][j+b];
			}
		}
		return res;
	}

	return res;
}

void grid::nachbarmatrix(grid* g1, int r, float thres)
{
	for(int i=0; i<nrows; i++){
		for(int j=0; j<ncols; j++){
			if(this->moore(i,j,r,MOORE)>thres)g1->feld[i][j]=1.0;
			else g1->feld[i][j]=0.0;
		}
	}
}

void grid::naehematrix(grid* g1, int r, float thres)
{
	int k;
	for(int i=0; i<nrows; i++){
		for(int j=0; j<ncols; j++){
			g1->feld[i][j]=9999;
		}
	}
	for(int i=0; i<nrows; i++){
		for(int j=0; j<ncols; j++){
			k=1;
			while(k<=r){
				if(this->moore(i,j,k,MOORE)>thres){
					g1->feld[i][j]=k;
					break;
				}
				k++;
			}
		}
	}
}

void grid::kompaktheit(grid* g1, int r)
{
	float teiler=(2*r+1)*(2*r+1);
	for(int i=0; i<nrows; i++){
		for(int j=0; j<ncols; j++){
			g1->feld[i][j]=this->moore(i,j,r,MOORE)/teiler;
		}
	}
}

void grid::attraktivitaet(grid* g1, int im, int jm, float alpha,
                          float sigma, float gamma)
{
	for(int i=0; i<nrows; i++){
		for(int j=0; j<ncols; j++){
			if(int(feld[i][j])!=nodata)
				g1->feld[i][j]=1
				-exp(1/gamma*(
						-((i-im)*(i-im)+(j-jm)*(j-jm))/(sigma*sigma)))*
						exp(-(feld[i][j]*feld[i][j])/(alpha*alpha));
		}
	}
}

grid* grid::select(float val)
{
	grid* gx = grid_copy();
	for(int i=0; i<nrows; i++){
		for(int j=0; j<ncols; j++){
			if(int(feld[i][j])!=nodata){
				if(fabs((double)(feld[i][j]-val))<RES)
					gx->feld[i][j]=1;
				else gx->feld[i][j]=0;
			}
			else
				gx->feld[i][j]=nodata;
		}
	}
	return gx;
}

grid* grid::select(int a,int b,int c, int d)
{
	if(a<0 || b<0 || a>ncols || b>nrows){
		cerr << "error (grid::select): upper left corner: ";
		cerr << a << " " << b << endl;
		return 0;
	}
	if(c<0 || d<0 || c>ncols || d>nrows){
		cerr << "error (grid::select): lower right corner: ";
		cerr << a << " " << b << endl;
		return 0;
	}
	if(c<a || d<b){
		cerr << "error (grid::select): lower right corner: ";
		cerr << a << " " << b << endl;
		cerr << " upper right corner: ";
		cerr << c << " " << d << endl;
		return 0;
	}
	grid* gx = new grid((int)csize);
	gx->ncols = c-a;
	gx->nrows = d-b;
	gx->xcorner = xcorner+a*csize;
	gx->ycorner = ycorner+(nrows-d)*csize;
	gx->csize = csize;
	gx->nodata = nodata;
	gx->feld = new float*[gx->nrows];
	for(int i=0; i<gx->nrows; i++){
		if((gx->feld[i]=new float[gx->ncols])==NULL){
			cerr << "error (grid::select): no sufficient memory" << endl;
		}
	}
	for(int i=0; i<gx->nrows; i++){
		for(int j=0; j<gx->ncols; j++){
			gx->feld[i][j] = feld[i+b][j+a];
		}
	}
	return gx;
}

#ifndef NO_HDF5
bool grid::write_hdf(char* fname, char* datasetn, char* autor, char* modell) {
  //cerr << "hdf " << fname << " " << datasetn << endl;
	float* f1=new float[nrows*ncols];
	if (f1==NULL) {
		cerr << "error (write_hdf): no space on device\n";
		exit(2);
	}
	for (int i=0; i<nrows; i++)
		for (int j=0; j<ncols; j++)
			f1[i*ncols+j]=feld[i][j];
	hd=new hdf5;
	if (hd->open_f(fname)!=0)
		hd->create_f(fname);
	bool success = false;
	if (hd->open_d(datasetn)!=0){
		hd->write_f_feld(datasetn, f1, nrows, ncols);
		hd->write_s_attribute("Autor", autor);
		hd->write_s_attribute("Modell", modell);
		hd->write_l_attribute("time", time(NULL));
		hd->write_d_attribute("xcorner", xcorner);
		hd->write_d_attribute("ycorner", ycorner);
		hd->write_f_attribute("csize", csize);
		hd->write_i_attribute("nodata", nodata);
		hd->write_i_attribute("ncols", ncols);
		hd->write_i_attribute("nrows", nrows);
		success = true;
	}
	delete[] f1;
	delete hd;
	hd = NULL;
	return success;
}


// saves a hdf file to R
// if the size of the files <= bins then save the grids complete
// if the size of the files > bins  then save only a sample

static char __io__char[40][40]; /** field for max 40 grids from hdf */
static int __io__int__length=0;
/** helpfunktion to read the dataset names from hdf */
static herr_t file_info(hid_t /*loc_id*/, const char *name, void* /*opdata*/)
{
	strcpy(__io__char[__io__int__length++],name);
	return 0;
}


void Grids::grid_save_to_R(char* name, int bins)
{
	cerr << name << " " << bins << endl;
	hid_t    file;
	int i,j,k;
	int      idx;
	char buf[255];
	bool flag;
	time_t now = time(NULL);
	srand(now);
	sprintf(buf,"%s/modell/R/data/samt.txt",getOption("SAMTHOME"));
	FILE *fp=fopen(buf,"w");
	file = H5Fopen(name, H5F_ACC_RDWR, H5P_DEFAULT);
	idx = H5Giterate(file, "/", NULL, file_info, NULL);
	H5Fclose(file);
	for(i=0; i<__io__int__length-1; i++){
		fprintf(fp,"%s ",__io__char[i]);
	}
	fprintf(fp,"%s\n",__io__char[__io__int__length-1]);
	grid *gx[40];
	for(i=0; i<__io__int__length; i++){
		gx[i] = new grid(100);
	}
	for(i=0; i<__io__int__length; i++){
		gx[i]->read_hdf(name,__io__char[i]);
	}
	if(gx[0]->nrows*gx[0]->ncols < bins){
		for(i=0; i<gx[0]->nrows; i++){
			for(j=0; j<gx[0]->ncols; j++){
				flag=true;
				for(k=0; k<__io__int__length; k++)
					if(gx[k]->feld[i][j]==gx[k]->nodata) flag=false;
				if(flag==true){
					for(k=0; k<__io__int__length-1; k++){
						fprintf(fp,"%f ",(gx[k])->feld[i][j]);
					}
					fprintf(fp,"%f\n",(gx[__io__int__length-1])->feld[i][j]);
				}
			}
		}
	}
	else{ // select random values
		int count = 0;
		while(count<bins){
			flag=true;
			while(flag==true){
				i=rand()%gx[0]->nrows;
				j=rand()%gx[0]->ncols;
				flag=false;
				for(k=0; k<__io__int__length; k++)
					if(gx[k]->feld[i][j]==gx[k]->nodata) flag=true;
			}
			for(k=0; k<__io__int__length-1; k++){
				fprintf(fp,"%f ",(gx[k])->feld[i][j]);
			}
			fprintf(fp,"%f\n",(gx[__io__int__length-1])->feld[i][j]);
			count++;
		}
	}
	fclose(fp);
	for(i=0; i<__io__int__length; i++){
		delete gx[i];
	}
	__io__int__length=0;
}

int grid::read_hdf(char* fname, char* datasetn)
{
	hd=new(hdf5);
	if(hd->open_f(fname)!=0){
		cerr << "error (read_hdf): can not open hdf_file: " << fname << endl;
		return -1;
	}
	if(hd->open_d(datasetn)!=0){
		cerr << "error (read_hdf): can not open dataset: " << datasetn << endl;
		return -2;
	}
	if(feld!=(float**)NULL){
		for(int i=0; i<nrows; i++){
			delete [] feld[i];
		}
	}
	ncols=hd->get_i_attribute("ncols");
	nrows=hd->get_i_attribute("nrows");
	nodata=hd->get_i_attribute("nodata");
	xcorner=hd->get_d_attribute("xcorner");
	ycorner=hd->get_d_attribute("ycorner");
	csize=hd->get_f_attribute("csize");
  //cerr << ncols << " " << nrows << " " << csize << endl;
	hd->read_f_feld(datasetn); // writes to f1 in grid
	feld=new float*[nrows];
	for(int i=0; i<nrows; i++){
		if((feld[i]=new float[ncols])==NULL){
			cerr << "error (read_hdf): no sufficient memory" << endl;
		}
	}
	// read_in
	for(int i=0; i<nrows; i++){
		for(int j=0; j<ncols; j++){
			feld[i][j]=hd->f1[i*ncols+j];
		}
	}
	delete hd;
	return 0;
}
#endif //NO_HDF5

float grid::get_xy(int i, int j)
{
  if(i>=0 && i<nrows && j>=0 && j<ncols)
    return(feld[i][j]);
  else 
    return nodata;
}

void grid::set_xy(int i,int j, float value) 
{
  if(i>=0 && i<nrows && j>=0 && j<ncols)
    feld[i][j]=value;
}

int grid::combine_or(grid *g1)
{
	if(ncols != g1->ncols || nrows != g1->nrows){
		fprintf(stderr,"ncols=%d g1.ncols=%d or nrows=%d g1.nrows=%d are differnt\n",
		        ncols,g1->ncols,nrows,g1->nrows);
		return -1;
	}
	for(int i=0; i<nrows; i++){
		for(int j=0; j<ncols; j++){
			if(int(feld[i][j]) == nodata){
				feld[i][j]=g1->feld[i][j];
			}
		}
	}
	return 0;
}

int grid::combine_grid(grid* g1,int selector)
{
	if(ncols != g1->ncols || nrows != g1->nrows){
		fprintf(stderr,"ncols=%d g1.ncols=%d or nrows=%d g1.nrows=%d are differnt\n",ncols,g1->ncols,nrows,g1->nrows);
		return -1;
	}
	switch(selector){
	case MIN:
		for(int i=0; i<nrows; i++){
			for(int j=0; j<ncols; j++){
				if(int(feld[i][j]) == nodata ||
						int(g1->feld[i][j]) == g1->nodata)
					feld[i][j]=nodata;
				else
					feld[i][j]=
							g1->feld[i][j]>feld[i][j] ?
									feld[i][j] : g1->feld[i][j];
			}
		}
		break;
	case MAX:
		for(int i=0; i<nrows; i++){
			for(int j=0; j<ncols; j++){
				feld[i][j]=
						g1->feld[i][j]<feld[i][j] ? feld[i][j] : g1->feld[i][j];
			}
		}
		break;
	case AVG:
		for(int i=0; i<nrows; i++){
			for(int j=0; j<ncols; j++){
				if(int(feld[i][j]) == nodata
						|| int(g1->feld[i][j]) == g1->nodata)
					feld[i][j]=nodata;
				else
					feld[i][j]=
							(feld[i][j]+g1->feld[i][j])/2;
			}
		}
		break;
	case ADD:
		for(int i=0; i<nrows; i++){
			for(int j=0; j<ncols; j++){
				if(int(feld[i][j]) == nodata
						|| int(g1->feld[i][j])==nodata)
					feld[i][j] = nodata;
				else
					feld[i][j]=
							(feld[i][j]+g1->feld[i][j]);
			}
		}
		break;
	case MUL:
		for(int i=0; i<nrows; i++){
			for(int j=0; j<ncols; j++){
				if(int(feld[i][j]) == nodata
						|| int(g1->feld[i][j]) == g1->nodata)
					feld[i][j]=nodata;
				else
					feld[i][j]=
							(feld[i][j]*g1->feld[i][j]);
			}
		}
		break;
	case DIV:
		for(int i=0; i<nrows; i++){
			for(int j=0; j<ncols; j++){
				if(int(feld[i][j]) == nodata || g1->feld[i][j]==0
						|| int(g1->feld[i][j]) == g1->nodata)
					feld[i][j]=nodata;
				else
					feld[i][j]=
							(feld[i][j]/g1->feld[i][j]);
			}
		}
		break;
	}
	return 0;
}

grid* grid::combine_grid(float (*f)(float))
{
	grid* gx=grid_copy();
	for(int i=0; i<nrows; i++){
		for(int j=0; j<ncols; j++){
			if(int(feld[i][j])!=nodata){
				gx->feld[i][j]=f(feld[i][j]);
			}
			else gx->feld[i][j]=nodata;
		}
	}
	return gx;
}

grid* grid::combine_grid(grid *g1, float (*f)(float,float))
{
	grid* gx=grid_copy();
	for(int i=0; i<nrows; i++){
		for(int j=0; j<ncols; j++){
			if(int(feld[i][j])!=nodata &&
					int(g1->feld[i][j])!=g1->nodata){
				gx->feld[i][j]=f(feld[i][j],g1->feld[i][j]);
			}
			else gx->feld[i][j]=nodata;
		}
	}
	return gx;
}

grid* grid::combine_grid(grid *g1, grid *g2, float (*f)(float,float,float))
{
	grid* gx=grid_copy();
	for(int i=0; i<nrows; i++){
		for(int j=0; j<ncols; j++){
			if(int(feld[i][j])!=nodata &&
					int(g1->feld[i][j])!=g1->nodata &&
					int(g2->feld[i][j])!=g1->nodata){
				gx->feld[i][j]=f(feld[i][j],g1->feld[i][j],g2->feld[i][j]);
			}
			else gx->feld[i][j]=nodata;
		}
	}
	return gx;
}

grid* grid::zoom()
{
	grid* gxxx = new grid(rgr/2);
	gxxx->ncols = 2*ncols-1;
	gxxx->nrows = 2*nrows-1;
	gxxx->xcorner=xcorner;
	gxxx->ycorner=ycorner;
	gxxx->csize=csize/2;
	gxxx->nodata=nodata;
	// allocate memory
	gxxx->feld=new float*[gxxx->nrows];
	for(int i=0; i<gxxx->nrows; i++){
		if((gxxx->feld[i]=new float[gxxx->ncols])==NULL){
			cerr << "error (upscale): no sufficient memory" << endl;
		}
	}
	has_nodata=NO;
	for(int i=0; i<nrows; i++)
		for(int j=0; j<ncols; j++)
			if(feld[i][j]==nodata) has_nodata=YES;

	for(int i=0; i<gxxx->nrows; i++){
		for(int j=0; j<gxxx->ncols; j++){
			gxxx->feld[i][j]=nodata;
		}
	}

	if(has_nodata==NO){
		// put the values form the orignial in the new grid
		// other values set to nodata
		for(int i=0; i<nrows; i++){
			for(int j=0; j<ncols; j++){
				gxxx->feld[2*i][2*j]=255*feld[i][j];
			}
		}
		//first step of algorithm
		float a,b,c,d,x1,x2;
		for(int i=1; i<gxxx->nrows-1; i++){
			for(int j=1; j<gxxx->ncols-1; j++){
				// i is odd and j is odd and x==nodata
				if(i%2==1 && j%2==1 && gxxx->feld[i][j]==nodata){
					a=gxxx->feld[i-1][j-1];
					b=gxxx->feld[i-1][j+1];
					c=gxxx->feld[i+1][j-1];
					d=gxxx->feld[i+1][j+1];
					if(fabs(a-b)<T1 && fabs(a-c)<T1 && fabs(a-d)<T1 &&
							fabs(b-c)<T1 && fabs(b-d)<T1 && fabs(c-d)<T1){
						gxxx->feld[i][j]=(a+b+c+d)/4;
					}
					else{
						// edge in SW-NE direction
						if(fabs(a-d)>T2 && fabs(a-d)>fabs(b-c))
							gxxx->feld[i][j]=(b+c)/2;
						// edge in NW-SE direction
						if(fabs(b-c)>T2 && fabs(b-c)>fabs(a-d))
							gxxx->feld[i][j]=(a+b)/2;
						// edge in NS direction
						if(fabs(a-d)>T1 && fabs(b-c)>T1 && (a-d)*(b-c)>0){
							gxxx->feld[i-1][j]=(a+b)/2;
							gxxx->feld[i+1][j]=(c+d)/2;
						}
						// edge in EW direction
						if(fabs(a-d)>T1 && fabs(b-c)>T1 && (a-d)*(b-c)>0){
							gxxx->feld[i][j-1]=(a+c)/2;
							gxxx->feld[i][j+1]=(b+d)/2;
						}
					}
				}
			}
		}
		// step two
		for(int i=1; i<gxxx->nrows-1; i++){
			for(int j=1; j<gxxx->ncols-1; j++){
				if((i%2==1||j%2==1) && gxxx->feld[i][j]==nodata){
					x2=gxxx->feld[i][j-1]; // left
					x1=gxxx->feld[i][j+1]; // right
					a=gxxx->feld[i-1][j];  // up
					b=gxxx->feld[i+1][j];  // down
					if((x2==nodata || x1==nodata) && fabs(a-b)<T1)
						gxxx->feld[i][j]=(a+b)/2;
					if(x1!=nodata && x2!=nodata){
						if(fabs(a-b)>T2 && fabs(a-b)>fabs(x1-x2))
							gxxx->feld[i][j]=(x1+x2)/2;
						if(fabs(x1-x2)>T2 && fabs(x1-x2)>fabs(a-b))
							gxxx->feld[i][j]=(a+b)/2;
					}
				}
			}
		}
		int divisor=4;
		// step four: fill all nodata cells
		for(int i=0; i<gxxx->nrows; i++){
			for(int j=0; j<gxxx->ncols; j++){
				if(gxxx->feld[i][j]==nodata){
					a=b=c=d=-1.0;
					divisor=4;
					if(i-1>=0)
						a=gxxx->feld[i-1][j];
					if(a<0){a=0.0; divisor--;}
					if(i+1<gxxx->nrows)
						b=gxxx->feld[i+1][j];
					if(b<0){b=0;divisor--;}
					if(j-1>=0)
						c=gxxx->feld[i][j-1];
					if(c<0){c=0;divisor--;}
					if(j+1<gxxx->ncols)
						d=gxxx->feld[i][j+1];
					if(d<0){d=0;divisor--;}
					if(divisor!=0)
						gxxx->feld[i][j]=(a+b+c+d)/divisor;
				}
			}
		}
	}
	if(has_nodata==YES){
		for(int i=0; i<nrows; i++){
			for(int j=0; j<ncols; j++){
				if(2*i<gxxx->nrows && 2*j<gxxx->ncols &&
						gxxx->feld[2*i][2*j]==nodata)
					gxxx->feld[2*i][2*j]=feld[i][j];
				if(2*i<gxxx->nrows && 2*j+1<gxxx->ncols &&
						gxxx->feld[2*i][2*j+1]==nodata)
					gxxx->feld[2*i][2*j+1]=feld[i][j];
				if(2*i+1<gxxx->nrows && 2*j<gxxx->ncols &&
						gxxx->feld[2*i+1][2*j]==nodata)
					gxxx->feld[2*i+1][2*j]=feld[i][j];
				if(2*i+1<gxxx->nrows && 2*j+1<gxxx->ncols &&
						gxxx->feld[2*i+1][2*j+1]==nodata)
					gxxx->feld[2*i+1][2*j+1]=feld[i][j];
			}
		}
	}
	return gxxx;
}


grid* grid::upscale(int teiler)
{
	grid* gxxx = new grid(rgr/teiler);
	gxxx->ncols = teiler * ncols;
	gxxx->nrows = teiler * nrows;
	gxxx->xcorner=xcorner;
	gxxx->ycorner=ycorner;
	gxxx->csize=csize/teiler;
	gxxx->nodata=nodata;

	gxxx->feld= new float*[gxxx->nrows];
	for(int i=0; i<gxxx->nrows; i++){
		if((gxxx->feld[i]=new float[gxxx->ncols])==NULL){
			cerr << "error (upscale): no sufficient memory" << endl;
		}
	}
	for(int i=0; i<nrows; i++){
		for(int j=0; j<ncols; j++){
			for(int k=0; k<teiler; k++){
				for(int l=0; l<teiler; l++){
					gxxx->feld[teiler*i+k][teiler*j+l]=feld[i][j];
				}
			}
		}
	}
	return gxxx;
}

grid* grid::downscale(int multi)
{
	if(ncols%multi != 0 || nrows%multi !=0){
		cerr << "multi ist kein teiler der Spalten bzw. Reihen" << endl;
		cerr << " nrows= " << nrows << " multi=" << multi << endl;
		cerr << " ncols= " << ncols << " multi=" << multi << endl;
		return 0;
	}
	grid* gxxx = new grid(rgr*multi);
	gxxx->ncols = ncols/multi;
	gxxx->nrows = nrows/multi;
	gxxx->xcorner=xcorner;
	gxxx->ycorner=ycorner;
	gxxx->csize=csize*multi;
	gxxx->nodata=nodata;
	int flag=0;
	gxxx->feld=new float*[gxxx->nrows];
	for(int i=0; i<gxxx->nrows; i++){
		if((gxxx->feld[i]=new float[gxxx->ncols])==NULL){
			cerr << "error (downscale): no sufficient memory" << endl;
		}
	}
	for(int i=0; i<gxxx->nrows; i++){
		for(int j=0; j<gxxx->ncols; j++){
			gxxx->feld[i][j]=0.0;
			flag=0;
			for(int k=0; k<multi; k++){
				for(int l=0; l<multi; l++){
					if(int(feld[multi*i+k][multi*j+l])==nodata) flag++;
					else
						gxxx->feld[i][j] += feld[multi*i+k][multi*j+l];
				}
			}
			if(flag>(multi*multi/2)) gxxx->feld[i][j]=nodata;
			else gxxx->feld[i][j]/=(multi*multi-flag);
		}
	}
	return gxxx;
}

grid* grid::downscale_s(int multi)
{
	if(ncols%multi != 0 || nrows%multi !=0){
		cerr << "multi ist kein teiler der Spalten bzw. Reihen" << endl;
		cerr << " nrows= " << nrows << " multi=" << multi << endl;
		cerr << " ncols= " << ncols << " multi=" << multi << endl;
		exit(2);
	}
	grid* gxxx = new grid(rgr*multi);
	gxxx->ncols = ncols/multi;
	gxxx->nrows = nrows/multi;
	gxxx->xcorner=xcorner;
	gxxx->ycorner=ycorner;
	gxxx->csize=csize*multi;
	gxxx->nodata=nodata;
	int flag=0;
	gxxx->feld= new float*[gxxx->nrows];
	for(int i=0; i<gxxx->nrows; i++){
		if((gxxx->feld[i]=new float[gxxx->ncols])==NULL){
			cerr << "error (downscale_s): no sufficient memory" << endl;
		}
	}
	for(int i=0; i<gxxx->nrows; i++){
		for(int j=0; j<gxxx->ncols; j++){
			gxxx->feld[i][j]=0.0;
			flag=0;
			for(int k=0; k<multi; k++){
				for(int l=0; l<multi; l++){
					if(int(feld[multi*i+k][multi*j+l])==nodata) flag++;
					else
						gxxx->feld[i][j] += feld[multi*i+k][multi*j+l];
				}
			}
			if(flag>(multi*multi/2)) gxxx->feld[i][j]=nodata;
			else gxxx->feld[i][j]*=(float)(multi*multi-flag)/(multi*multi);
		}
	}
	return gxxx;
}


grid* grid::distance(float val)
{
	vector<ipoint*> ivec;
	vector<ipoint*>::iterator p;
	double dist;
	grid* gx=grid_copy();
	// init gx
	for(int i=0; i<nrows; i++){
		for(int j=0; j<ncols; j++){
			if(fabs(feld[i][j]-val)<RES)
				gx->feld[i][j]=0;
			else
				gx->feld[i][j]=FLT_MAX;
		}
	}
	for(int i=0; i<nrows; i++){
		for(int j=0; j<ncols; j++){
			if(fabs((double)gx->feld[i][j])<RES){
				ipoint* fp=new ipoint();
				fp->x=i;
				fp->y=j;
				ivec.push_back(fp);
			}
		}
	}
	for(int i=0; i<nrows; i++){
		for(int j=0; j<ncols; j++){
			if(fabs((double)gx->feld[i][j])<RES);
			else{
				for(p=ivec.begin(); p!=ivec.end(); p++){
					dist=sqrt((double)(((*p)->x-i)*((*p)->x-i)+
							((*p)->y-j)*((*p)->y-j)));
					if(gx->feld[i][j]>dist) gx->feld[i][j]=
							(float)dist;
					if(fabs((double)(gx->feld[i][j]-1)) < RES) break;
				}
			}
			if(feld[i][j]!=nodata)
				gx->feld[i][j]*=gx->csize;
			else
				gx->feld[i][j]=nodata;
		}
	}
	for(p=ivec.begin(); p!=ivec.end(); p++){
		delete (*p);
	}
	return gx;
}

grid* grid::visuability(point* pf)
{
	bool flag1=false;
	bool flag2;
	//bool flag3;
	double gamma=12742000.0;
	double dist1,dist2,dist,s1,s2,sa,h1,h2,m;//,smax;
	int x1, xw, y1, yw, deltax, deltay, e, xstep, ystep, length1;
	// init the output
	grid *v_grid=grid_copy();
	for(int i=0; i<nrows; i++){
		for(int j=0; j<ncols; j++){
			v_grid->feld[i][j]=-1.0;
		}
	}
	cerr << "visuability called: " << pf->length << endl;
	for(int k=0; k<pf->length; k++){
		cerr << pf->feld[k][0] << " " << pf->feld[k][1]
		                                             << " " << pf->feld[k][2] << endl;
	}

	for(int k=0; k<pf->length; k++){
		xw=(int)((pf->feld[k][0]-xcorner)/csize);
		yw=(nrows-1)-(int)((pf->feld[k][1]-ycorner)/csize);
		cerr << "xw: " << xw << " yw: " << yw << " high: "
		<< pf->feld[k][2]+feld[yw][xw] << endl;
		for(int i=0; i<nrows; i++){
			for(int j=0; j<ncols; j++){
				if(feld[i][j]!=nodata){ // nodata are not allowed
					dist=sqrt(double((xw-j)*(xw-j)+(yw-i)*(yw-i)))*csize;
					dist2=sqrt(2.0*6371000*(pf->feld[k][2]+feld[yw][xw]));
					dist1=sqrt(2.0*6371000*feld[i][j]);
					if(dist1+dist2>dist){
						// calc line beween Px and Pw
						x1=j;
						y1=i;
						deltax=xw-x1;
						deltay=yw-y1;
						ystep=1;
						xstep=1;
						flag2=false;
						if(deltax<0){
							deltax*=-1;xstep*=-1;
						}
						if(deltay<0){
							deltay*=-1;ystep*=-1;
						}
						if(deltay>deltax){
							int t;
							t=deltax;
							deltax=deltay;
							deltay=t;
							flag2=true;
						}
						length1=deltax-1;
						e=deltay;
						flag1=false;
						s1=(dist1*dist)/(dist1+dist2);
						s2=(dist2*dist)/(dist1+dist2);
						h1=(s1*s1)/gamma;
						h2=(s2*s2)/gamma;
						m=(((pf->feld[k][2]+feld[yw][xw]-h2)-feld[i][j]-h1))/dist;
						// 		    cerr << i << " " << j << " "
						// 			 << h1 << " " << h2 << " " << m << " " << dist << endl;
						for(int x=0; x<length1; x++){
							e+=deltay;
							if(flag2==false) x1+=xstep;
							else y1+=ystep;
							if(e>=deltax){
								e-=deltax;
								if(flag2==false) y1+=ystep;
								else x1+=xstep;
							}
							// distance between P0 and Px
							sa=sqrt(double(((j-x1)*(j-x1)+(i-y1)*(i-y1))))*csize;
							if(m*sa+feld[i][j]<feld[y1][x1]){
								flag1=true;
								break;
							}
						}
						if(flag1==false)
							if(v_grid->feld[i][j]<0 || v_grid->feld[i][j]>sa)
								v_grid->feld[i][j]=sa;
					}
				}
				else v_grid->feld[i][j]=nodata;
			}

		}
	}
	return v_grid;
}

void grid::write_ascii(char* name)
{
	FILE *fp1;
	fp1 = fopen(name,"w");

	fprintf(fp1,"ncols         %d\n",ncols);
	fprintf(fp1,"nrows         %d\n",nrows);
	fprintf(fp1,"xllcorner     %f\n",xcorner);
	fprintf(fp1,"yllcorner     %f\n",ycorner);
	fprintf(fp1,"cellsize      %5.1f\n",csize);
	fprintf(fp1,"NODATA_value  %d\n",nodata);
	// write out
	for(int i=0; i<nrows; i++){
		for(int j=0; j<ncols; j++){
			fprintf(fp1,"%4.3f ",feld[i][j]);
		}
		fprintf(fp1,"\n");
	}
	fclose(fp1);
}
void grid::write_ascii_inv(char* name)
{
	FILE *fp1;
	fp1 = fopen(name,"w");
	fprintf(fp1,"ncols         %d\n",ncols);
	fprintf(fp1,"nrows         %d\n",nrows);
	fprintf(fp1,"xllcorner     %f\n",xcorner);
	fprintf(fp1,"yllcorner     %f\n",ycorner);
	fprintf(fp1,"cellsize      %5.1f\n",csize);
	fprintf(fp1,"NODATA_value  %d\n",nodata);
	// write out
	for(int i=nrows-1; i>=0; i--){
		for(int j=0; j<ncols; j++){
			fprintf(fp1,"%4.3f ",feld[i][j]);
		}
		fprintf(fp1,"\n");
	}
	fclose(fp1);
}

void grid::write_pnm(char* name, int farbe)
{
	float wert;
	min=FLT_MAX;
	max=-FLT_MAX;
	for(int i=0; i<nrows; i++){
		for(int j=0; j<ncols; j++){
			wert = feld[i][j];
			if(wert!=nodata){
				if(min > wert) min=wert;
				if(max < wert) max=wert;
			}
		}
	}
	int i,j;
	int r,g,b;
	int value;
	ofstream file(name);
	if(!file){
		cerr << "error (write_pnm): can not open: " << name << endl;
		exit(2);
	}
	// sinvolle Startwerte fuer min max
	if(farbe == BW){
		cerr << "min: " << min << " max: " << max << endl;
		file << "P2" << endl;
		file << "# CREATOR: grid2pnm Version 0.1" << endl;
		file << ncols << " " << nrows << endl;
		file << "255" << endl;
		for(i=0; i<nrows; i++){
			for(j=0; j<ncols; j++){
				wert = feld[i][j];
				if(wert < 0)
					value = 255;
				else
					value = (int)(250*(wert-min)/(max-min));
				if(value<0) value=250;
				file << value << endl;
			}
		}
	}
	if(farbe == COLOR){
		cerr << "min: " << min << " max: " << max << endl;
		file << "P3" << endl;
		file << "# CREATOR: grid2pnm Version 0.1" << endl;
		file << ncols << " " << nrows << endl;
		file << "255" << endl;
		for(i=0; i<nrows; i++){
			for(j=0; j<ncols; j++){
				wert = feld[i][j];
				if(wert < 0) wert = -0.001;
				value = (int)(255*(wert-min)/(max-min));
				// Farbcodierung
				if(wert<=0){
					r=g=b=255;
				}
				else{
					if(value <=128){
						r=255;
						g=(int)(2*value);
						if(g>=255)g=255;
						b=100;
					}
					else{
						r=(int)(255-2*(value-128));
						if(r<0)r=0;
						if(r>255)r=255;
						g=255;
						b=100;
					}
				}
				file << r << " " << g << " " << b << endl;
			}
		}
		file.close();
	}
	if(farbe != COLOR && farbe != BW){
		cerr << "error (write_pnm): color must be in [BW,COLOR]! " << endl;
		exit(2);
	}
}

void grid::write_pnm_inv(char* name, int farbe)
{
	float wert;
	min=FLT_MAX;
	max=-FLT_MAX;
	for(int i=0; i<nrows; i++){
		for(int j=0; j<ncols; j++){
			wert = feld[i][j];
			if(wert < -999) wert = -0.001;
			if(min > wert) min=wert;
			if(max < wert) max=wert;
		}
	}
	int i,j;
	int r,g,b;
	int value;
	ofstream file(name);
	if(!file){
		cerr << "error (write_pnm): can not open: " << name << endl;
		exit(2);
	}
	// sinvolle Startwerte fuer min max
	if(farbe == BW){
		cerr << "min: " << min << " max: " << max << endl;
		file << "P2" << endl;
		file << "# CREATOR: grid2pnm Version 0.1" << endl;
		file << ncols << " " << nrows << endl;
		file << "255" << endl;
		for(i=0; i<nrows; i++){
			for(j=0; j<ncols; j++){
				wert = feld[i][j];
				if(wert < 0) wert = -0.001;
				value = 250-(int)(250*(wert-min)/(max-min));
				file << value << endl;
			}
		}
	}
	if(farbe == COLOR){
		cerr << "min: " << min << " max: " << max << endl;
		file << "P3" << endl;
		file << "# CREATOR: grid2pnm Version 0.1" << endl;
		file << ncols << " " << nrows << endl;
		file << "255" << endl;
		for(i=0; i<nrows; i++){
			for(j=0; j<ncols; j++){
				wert = feld[i][j];
				if(wert < 0) wert = -0.001;
				value = 255-(int)(255*(wert-min)/(max-min));
				// Farbcodierung
				if(wert<=0){
					r=g=b=255;
				}
				else{
					if(value <=128){
						r=255;
						g=(int)(2*value);
						if(g>=255)g=255;
						b=100;
					}
					else{
						r=(int)(255-2*(value-128));
						if(r<0)r=0;
						if(r>255)r=255;
						g=255;
						b=100;
					}
				}
				file << r << " " << g << " " << b << endl;
			}
		}
		file.close();
	}
	if(farbe != COLOR && farbe != BW){
		cerr << "error (write_pnm): color must be in [BW,COLOR]! " << endl;
		exit(2);
	}
}

void grid::write_pnm_b(char* name, int farbe)
{
	float wert;
	min=FLT_MAX;
	max=-FLT_MAX;
	for(int i=0; i<nrows; i++){
		for(int j=0; j<ncols; j++){
			wert = feld[i][j];
			if(wert < 0) wert = -0.001;
			if(min > wert) min=wert;
			if(max < wert) max=wert;
		}
	}
	int i,j;
	int r,g,b;
	int value;
	FILE *fp;
	fp=fopen(name,"w");
	// sinvolle Startwerte fuer min max
	if(farbe == BW){
		cerr << "min: " << min << " max: " << max << endl;
		fprintf(fp,"P5\n");
		fprintf(fp,"# CREATOR: grid2pnm Version 0.1\n");
		fprintf(fp,"%d %d\n",ncols,nrows);
		fprintf(fp,"255\n");
		for(i=0; i<nrows; i++){
			for(j=0; j<ncols; j++){
				wert = feld[i][j];
				if(wert < 0) wert = -0.001;
				value = (int)(250*(wert-min)/(max-min));
				fprintf(fp,"%c",(char)value);
			}
		}
	}
	if(farbe == COLOR){
		cerr << "min: " << min << " max: " << max << endl;
		fprintf(fp,"P6\n");
		fprintf(fp,"# CREATOR: grid2pnm Version 0.1\n");
		fprintf(fp,"%d %d\n",ncols,nrows);
		fprintf(fp,"255\n");
		for(i=0; i<nrows; i++){
			for(j=0; j<ncols; j++){
				wert = feld[i][j];
				if(wert < 0) wert = -0.001;
				value = (int)(255*(wert-min)/(max-min));
				// Farbcodierung
				if(wert<=0){
					r=g=b=255;
				}
				else{
					if(value <=128){
						r=255;
						g=(int)(2*value);
						if(g>=255)g=255;
						b=100;
					}
					else{
						r=(int)(255-2*(value-128));
						if(r<0)r=0;
						if(r>255)r=255;
						g=255;
						b=100;
					}
				}
				fprintf(fp,"%c%c%c",(char)r,(char)g,(char)b);
			}
		}
		fclose(fp);
	}
	if(farbe != COLOR && farbe != BW){
		cerr << "error (write_pnm): color must be in [BW,COLOR]! " << endl;
		exit(2);
	}
}

void grid::write_pnm_inv_b(char* name, int farbe)
{
	float wert;
	min=FLT_MAX;
	max=-FLT_MAX;
	for(int i=0; i<nrows; i++){
		for(int j=0; j<ncols; j++){
			wert = feld[i][j];
			if(wert < 0) wert = -0.001;
			if(min > wert) min=wert;
			if(max < wert) max=wert;
		}
	}
	int i,j;
	int r,g,b;
	int value;
	FILE *fp;
	fp=fopen(name,"w");
	// sinvolle Startwerte fuer min max
	if(farbe == BW){
		cerr << "min: " << min << " max: " << max << endl;
		fprintf(fp,"P5\n");
		fprintf(fp,"# CREATOR: grid2pnm Version 0.1\n");
		fprintf(fp,"%d %d\n",ncols,nrows);
		fprintf(fp,"255\n");
		for(i=0; i<nrows; i++){
			for(j=0; j<ncols; j++){
				wert = feld[i][j];
				if(wert < 0) wert = -0.001;
				value = 250-(int)(250*(wert-min)/(max-min));
				fprintf(fp,"%c",(char)value);
			}
		}
	}
	if(farbe == COLOR){
		cerr << "min: " << min << " max: " << max << endl;
		fprintf(fp,"P6\n");
		fprintf(fp,"# CREATOR: grid2pnm Version 0.1\n");
		fprintf(fp,"%d %d\n",ncols,nrows);
		fprintf(fp,"255\n");
		for(i=0; i<nrows; i++){
			for(j=0; j<ncols; j++){
				wert = feld[i][j];
				if(wert < 0) wert = -0.001;
				value = 255-(int)(255*(wert-min)/(max-min));
				// Farbcodierung
				if(wert<=0){
					r=g=b=255;
				}
				else{
					if(value <=128){
						r=255;
						g=(int)(2*value);
						if(g>=255)g=255;
						b=100;
					}
					else{
						r=(int)(255-2*(value-128));
						if(r<0)r=0;
						if(r>255)r=255;
						g=255;
						b=100;
					}
				}
				fprintf(fp,"%c%c%c",(char)r,(char)g,(char)b);
			}
		}
		fclose(fp);
	}
	if(farbe != COLOR && farbe != BW){
		cerr << "error (write_pnm): color must be in [BW,COLOR]! " << endl;
		exit(2);
	}
}

grid* grid::difference(grid* gx) // only for view !!!
{
	if(gx==0) return 0;
	if(gx->nrows != nrows || gx->ncols != ncols) return 0;
	grid *gout=grid_copy();
	for(int i=0; i<nrows; i++){
		for(int j=0; j<ncols; j++){
			if(feld[i][j]==nodata ||
					gx->feld[i][j]==gx->nodata)
				gout->feld[i][j]=nodata;
			else
				gout->feld[i][j]=feld[i][j]-gx->feld[i][j];
			// only for view
			if(gout->feld[i][j]==0)gout->feld[i][j]=nodata;
		}
	}
	return gout;
}

point* grid::sample(int number, int flag)
{
	if(number<1) return 0;
	point * pp=new point();
	if(pp==0){
		cerr << "error in sample: no space left on device: " << number << endl;
		return 0;
	}
	pp->length=pp->alength=number;
	pp->feld=(double**) new double[pp->alength];
	if(pp->feld==0){
		cerr << "error in sample: no space left on device: " << number << endl;
		return 0;
	}
	for(int i=0; i<pp->alength; i++){
		if((pp->feld[i]=(double*) new double[3])==NULL){
			cerr << "error (sample): no sufficient memory" << endl;
		}
	}
	time_t now = time(NULL);
	srand(now);
	double l=ycorner+csize*nrows;
	if(flag==0){  // random search
		for(int i=0; i<number; i++){ // multiple values are possible
			int sx=rand()%ncols;
			int sy=rand()%nrows;
			pp->feld[i][0]=xcorner+(sx+0.5)*csize;
			pp->feld[i][1]=l-(sy+.5)*csize;
			pp->feld[i][2]=feld[sy][sx];
		}
	}
	if(flag==1){ // histogram search
		stat();
		float delta=gridmax-gridmin;
		cerr << gridmin << " " << gridmax << " " << delta << endl;
		vector<int> iv(nrows*ncols); // vector for selected i
		vector<int> jv(nrows*ncols); // vector for selected j
		vector<int> isel(THRESH*BINS);
		vector<int> jsel(THRESH*BINS);
		int top=0;
		int tops=0;
		for(int k=0; k<100; k++){
			top=0;
			for(int i=0; i<nrows; i++){
				for(int j=0; j<ncols; j++){
					if((int)feld[i][j]!=nodata){
						if(feld[i][j]>=gridmin+delta*(float)k/(BINS-1.0) &&
								feld[i][j]<=gridmin+delta*(float)(k+1)/(BINS-1.0)){
							iv[top]=i;
							jv[top]=j;
							top++;
						}
					}
				}
			}
			// selection procedure
			cerr << k << " " << top << endl;
			if(top>0){
				for(int l=tops; l<THRESH+tops; l++){
					jsel[l]=jv[rand()%top];
					isel[l]=iv[rand()%top];
				}
				tops+=THRESH;
			}
		}
		// main random selection
		for(int i=0; i<number; i++){
			int j=rand()%tops;
			int sx=jsel[j];
			int sy=isel[j];
			pp->feld[i][0]=xcorner+(sx+0.5)*csize;
			pp->feld[i][1]=l-(sy+.5)*csize;
			pp->feld[i][2]=feld[sy][sx];
		}
	}
	return pp;
}

// simple point class
point::point()
{
	length=0;
	alength=0;
	feld=NULL;
}

point::~point()
{
	if(length==0) return;
	if(feld!=NULL) delete [] feld;
}

void point::set_point(double x, double y, double z)
{
	if(length==0){
		alength=50;
		feld=(double**) new double[alength];
		for(int i=0; i<alength; i++){
			if((feld[i]=(double*) new double[3])==NULL){
				cerr << "error (read_ascii): no sufficient memory" << endl;
			}
		}
	}
	if(length==alength){
		double **feld1;
		alength+=50;
		feld1=(double**) new double[alength];
		for(int i=0; i<alength; i++){
			if((feld1[i]=(double*) new double[3])==NULL){
				cerr << "error (read_ascii): no sufficient memory" << endl;
			}
		}
		for(int i=0; i<length; i++)
			for(int j=0; j<3; j++)
				feld1[i][j]=feld[i][j];
		delete [] feld;
		feld=feld1;
	}
	feld[length][0]=x;
	feld[length][1]=y;
	feld[length][2]=z;
	length++;
}

void point::clean_point()
{
	if(length==0) return;
	if(feld!=NULL) delete [] feld;
	length=0;
	alength=0;
}

int point::read_point(char *filename)
{
	double x,y,z;
	FILE *fp;
	if((fp=fopen(filename,"r"))==NULL){
		fprintf(stderr,"error (read_point): no such file: %s\n",filename);
		return -1;
	}
	xmin=ymin=zmin=FLT_MAX;
	xmax=ymax=zmax=-FLT_MAX;
	length=0;
	// calculate length and min/max
	while(EOF!=fscanf(fp,"%lf %lf %lf",&x,&y,&z)){
		fprintf(stderr,"%lf %lf %lf\n",x,y,z);
		length++;
		if(x<xmin)xmin=x;
		if(y<ymin)ymin=y;
		if(z<zmin)zmin=z;
		if(x>xmax)xmax=x;
		if(y>ymax)ymax=y;
		if(z>zmax)zmax=z;
	}

	rewind(fp);
	// organize memory
	alength=length;
	feld=(double**) new double*[length+1];
	for(int i=0; i<length; i++){
		if((feld[i]=new double[3])==NULL){
			cerr << "error (read_ascii): no sufficient memory" << endl;
			return -2;
		}
	}
	// read in
	int i=0;
	while(EOF!=fscanf(fp,"%lf %lf %lf",&x,&y,&z)){
		feld[i][0]=x;
		feld[i][1]=y;
		feld[i][2]=z;
		i++;
	}
	fclose(fp);
	fprintf(stderr,
	        "read_point length=%d xmin=%lf xmax=%lf ymin=%lf ymax=%lf zmin=%lf zmax=%lf\n"
	        ,length,xmin,xmax,ymin,ymax,zmin,zmax);
	return 0;
}

int point::write_point(char *filename)
{
	FILE *fp;
	fp=fopen(filename,"w");
	for(int i=0; i<length; i++){
		fprintf(fp,"%lf %lf %lf\n",feld[i][0],feld[i][1],feld[i][2]);
	}
	fclose(fp);
	return 0;
}

grid* point::p2g(grid* gx, double r)
{
	fprintf(stderr,"point to grid: length=%d radius=%lf\n",length, r);
	grid* nx=gx->grid_copy();
	unsigned count=0;
	for(int i=0; i<gx->nrows; i++)
		for(int j=0; j<gx->ncols; j++)
			nx->feld[i][j]=nx->nodata;
	// if(nx->feld[i][j]!=nx->nodata)
	for(int i=0; i<length; i++){
		if(feld[i][0]>0 && feld[i][1]>0){
			if(feld[i][0]>=gx->xcorner &&
					feld[i][0]<gx->xcorner+gx->csize*gx->ncols &&
					feld[i][1]>=gx->ycorner &&
					feld[i][1]<gx->ycorner+gx->csize*gx->nrows){
				if(gx->feld[(int)(gx->nrows-(feld[i][1]-gx->ycorner)/gx->csize)]
				            [(int)((feld[i][0]-gx->xcorner)/gx->csize)]==gx->nodata)
					nx->feld[(int)(gx->nrows-(feld[i][1]-gx->ycorner)/gx->csize)]
					         [(int)((feld[i][0]-gx->xcorner)/gx->csize)]=gx->nodata;
				else
					nx->feld
					[(int)(gx->nrows-(feld[i][1]-gx->ycorner)/gx->csize)]
					 [(int)((feld[i][0]-gx->xcorner)/gx->csize)]=(float)feld[i][2];
				if(feld[i][2]>0)
					count++;
			}
		}
	}
	fprintf(stderr,"p2g: %d points used\n",count);
	if(r<=0) return nx;
	grid* ny=nx->grid_copy();
	bool flag=false;
	double val;
	int step2=(int)(r/nx->csize);
	fprintf(stderr,"step2=%d\n",step2);
	for(int i=0; i<nx->nrows; i++){
		for(int j=0; j<nx->ncols; j++){
			flag=false;
			for(int k1=-step2; k1<step2; k1++){
				for(int k2=-step2; k2<step2; k2++){
					if(i+k1>=0 && j+k2>=0 &&
							i+k1<nx->nrows && j+k2<nx->ncols &&
							nx->feld[i+k1][j+k2]!=nx->nodata &&
							(k1*nx->csize)*(k1*nx->csize)+
							(k2*nx->csize)*(k2*nx->csize)<r*r){
						flag=true;
						val=nx->feld[i+k1][j+k2];
					}
				}
			}
			if(flag==false) ny->feld[i][j]=nx->nodata;
			else ny->feld[i][j]=val;
		}
	}
	delete nx;
	return ny;
}

grid* point::p2g_shepard(grid* gx,double R, double mu)
{
	double l,val;
	unsigned count=0;
	fprintf(stderr,"point to grid using shepard: R=%lf mu=%lf\n",R,mu);
	if(R>sqrt((gx->nrows*gx->csize)*(gx->nrows*gx->csize)+
	          (gx->ncols*gx->csize)*(gx->ncols*gx->csize))/2){
		fprintf(stderr,"Radius too big: %lf\n",R);
		return (grid*)0;
	}
	if(mu<2 || mu>6){
		fprintf(stderr,"mu=%lf must be in [2..6]\n",mu);
		return (grid*)0;
	}
	grid* nx=gx->grid_copy();
	for(int i=0; i<nx->nrows; i++)
		for(int j=0; j<nx->ncols; j++)
			if(nx->feld[i][j]!=nx->nodata)
				nx->feld[i][j]=0;
	for(int i=0; i<nx->nrows; i++){
		for(int j=0; j<nx->ncols; j++){
			double r=0;
			double rall=0;
			count=0;
			for(int k=0; k<length; k++){
				if(feld[k][0]>=nx->xcorner &&
						feld[k][0]<nx->xcorner+gx->csize*gx->ncols &&
						feld[k][1]>=nx->ycorner &&
						feld[k][1]<nx->ycorner+nx->csize*gx->nrows){
					count++;
					l=sqrt((nx->xcorner+nx->csize*j-feld[k][0])*
					       (nx->xcorner+nx->csize*j-feld[k][0])+
					       (nx->ycorner+nx->csize*i-feld[k][1])*
					       (nx->ycorner+nx->csize*i-feld[k][1]));
					if(l<R){
						l=1-l/R;
						val=pow(l,mu);
						rall+=val;
						r+=val*feld[k][2];}
				}
			}
			if(gx->feld[nx->nrows-1-i][j]!=gx->nodata && r!=0 && rall!=0)
				nx->feld[nx->nrows-1-i][j]=r/rall;
			else
				nx->feld[nx->nrows-1-i][j]=nx->nodata;
		}
	}
	fprintf(stderr,"p2g_shepard: %d points are used\n",count);
	return nx;
}


grid* point::p2g_voronoi(grid* gx)
{
	double l,val;
	unsigned count=0;
	grid* nx=gx->grid_copy();
	for(int i=0; i<nx->nrows; i++)
		for(int j=0; j<nx->ncols; j++)
			if(nx->feld[i][j]!=nx->nodata)
				nx->feld[i][j]=0;
	for(int i=0; i<nx->nrows; i++){
		for(int j=0; j<nx->ncols; j++){
			count=0;
			val=FLT_MAX;
			for(int k=0; k<length; k++){
				if(feld[k][0]>=nx->xcorner &&
						feld[k][0]<nx->xcorner+gx->csize*gx->ncols &&
						feld[k][1]>=nx->ycorner &&
						feld[k][1]<nx->ycorner+nx->csize*gx->nrows){
					l=sqrt((nx->xcorner+nx->csize*j-feld[k][0])*
					       (nx->xcorner+nx->csize*j-feld[k][0])+
					       (nx->ycorner+nx->csize*(nx->nrows-i-1)
					      		 -feld[k][1])*
					      		 (nx->ycorner+nx->csize*(nx->nrows-i-1)
					      				 -feld[k][1]));
					if(val>l){
						val=l;
						count=k;
					}
				}
			}
			if(gx->feld[i][j]!=gx->nodata)
				nx->feld[i][j]=feld[count][2];
			else
				nx->feld[i][j]=nx->nodata;
		}
	}
	fprintf(stderr,"p2g_voronoi: %d points are used\n",count);
	return nx;
}


struct valpair* point::gamma()
{

	if(length<5 || feld==NULL) return 0;
	vpp = new (struct valpair);
	unsigned int anz=((length-1)*length)/2; // number of pairs
	double* dist=new double[anz];
	double* diff=new double[anz];
	double* dist1=new double[anz];
	if(dist==NULL || diff==NULL || dist1==NULL){
		fprintf(stderr,"error in point::gamma now space left on device\n");
		return 0;
	}
	int k=0;
	// calc all distances
	for(int i=0; i<length-1; i++){
		for(int j=i+1; j<length; j++){
			dist[k]=sqrt((feld[i][0]-feld[j][0])*(feld[i][0]-feld[j][0])+
			             (feld[i][1]-feld[j][1])*(feld[i][1]-feld[j][1]));
			diff[k]=(feld[i][2]-feld[j][2])*(feld[i][2]-feld[j][2]);
			dist1[k]=dist[k];
			k++;
		}
	}
	qsort((void*)dist,(size_t)anz,sizeof(double),dcompare);
	int classes=(int)sqrt(double(k))+1;
	int elements=(int)(anz/classes);
	fprintf(stderr,"number of classes=%d elements pro class=%d\n",
	        classes,elements);
	vpp->n=classes;
	vpp->x=new double[classes];
	vpp->y=new double[classes];
	if(vpp->x==NULL || vpp->y==NULL){
		fprintf(stderr,"error in point::gamma now space left on device\n");
		return 0;
	}
	double mu;
	double z;
	//double d1;
	int counter;
	for(int i=0; i<classes; i++){
		mu=0.0;
		z=0.0;
		counter=0;
		for(int j=0; j<elements; j++){
			mu+=dist[i*elements+j];
			for(unsigned int k=0; k<anz; k++){
				if(fabs(dist1[k]-dist[i*elements+j])<EPS){
					z+=diff[k];
				}
			}
		}
		vpp->x[i]=mu/elements;
		vpp->y[i]=z/elements;
	}
	for(int i=0; i<classes; i++)
		fprintf(stderr,"%d: %lf %lf\n",i,vpp->x[i],vpp->y[i]);
	delete [] dist;
	delete [] dist1;
	delete [] diff;
	return vpp;
}


// a connection to the vtk splatter technique
int splatter(grid *lg1,grid *lg2,grid *lg3,grid *lg4,
             int length, struct splatter_field *f1)
{
	int a,b;//,c;
	if(lg1->ncols != lg2->ncols || lg1->ncols != lg3->ncols ||
			lg1->ncols != lg4->ncols ||
			lg1->nrows != lg2->nrows || lg1->nrows != lg3->nrows ||
			lg1->nrows != lg4->nrows){
		fprintf(stderr,"error (splatter): the grids are not of same size\n");
		return -1;
	}
	if(length<10 || length>100000){
		fprintf(stderr,"error (splatter): length not in [10..100000]\n");
		return -1;
	}
	time_t now = time(NULL);
	srand(now);
	grid* g1=lg1->grid_copy();
	grid* g2=lg2->grid_copy();
	grid* g3=lg3->grid_copy();
	grid* g4=lg4->grid_copy();
	g1->norm_grid();
	g2->norm_grid();
	g3->norm_grid();
	g4->norm_grid();
	for(int i=0; i<length; i++){
		int flag = 0;
		// select a random cell
		while(flag==0){
			a=rand()%g1->nrows;
			b=rand()%g1->ncols;
			if(g1->feld[a][b]!=g1->nodata &&
					g2->feld[a][b]!=g2->nodata &&
					g3->feld[a][b]!=g3->nodata &&
					g4->feld[a][b]!=g4->nodata
			) flag=1;
		}
		// fill in splatter_field
		f1[i].x=g1->feld[a][b];
		f1[i].y=g2->feld[a][b];
		f1[i].z=g3->feld[a][b];
		if(g4->feld[a][b]>SPLATTER_TRESH)
			f1[i].s=g4->feld[a][b];
		else f1[i].s=0.0;
	}
	delete g1;
	delete g2;
	delete g3;
	delete g4;
	return 0;
}



kohonen::kohonen(int input, int rows1, int cols1)
{
	if(rows1>MAX_X_KOHON || cols1>MAX_Y_KOHON){
		cerr << "error (Kohon): x1 or y1 too big!" << endl;
		return;
	}
	x=y=0;
	inputs=input;
	rows=rows1;
	cols=cols1;
	// generate sensor field
	sf = (struct sensor**) new struct sensor*[rows];
	if(sf==NULL){
		cerr << "kohonen: no space on divice" << endl;
		return;
	}
	for(int i=0; i<rows; i++){
		sf[i] = new struct sensor[cols];
		if(sf[i]==NULL){
			cerr << "kohonen: no space on divice" << endl;
			return;
		}
	}
	// init sensor field with random values between 0,1

	time_t now = time(NULL);
	srand(now);
	a=b=int(sqrt(10*SIGMA));
	if(a>cols1)a=cols1;
	if(b>rows1)b=rows1;
	mask=new double*[a];
	if(mask==NULL){
		cerr << "error in kohonen: no space left on divice" << a << endl;
		return;
	}
	for(int i=0; i<a; i++){
		mask[i]=new double[b];
		if(mask==NULL){
			cerr << "error in kohonen: no space left on divice" << b << endl;
			return;
		}
	}
	// fill the mask
	for(int i=0; i<a; i++){
		for(int j=0; j<b; j++){
			mask[i][j]=exp(-SIGMA*sqrt((double)(i*i)+(double)(j*j)));
			// cerr << mask[i][j] << " ";
		}
		// cerr << endl;
	}

}

kohonen::~kohonen()
{
	if(sf!=NULL){
		for(int i=0; i<rows; i++){
			if(sf[i]!=NULL) delete sf[i];
		}
		delete  sf;
	}
	for(int i=0; i<a; i++)
		delete [] mask[i];
	delete [] mask;
}

float kohonen::find(float val1)
{
	float min=FLT_MAX;
	for(int i=0; i<rows; i++){
		for(int j=0; j<cols; j++){
			if(min>(val1-sf[i][j].a)*(val1-sf[i][j].a)){
				min=(val1-sf[i][j].a)*(val1-sf[i][j].a);
				x=i;
				y=j;
			}
		}
	}
	return min;
}

float kohonen::find(float val1, float val2)
{
	float min=FLT_MAX;
	for(int i=0; i<rows; i++){
		for(int j=0; j<cols; j++){
			if(min>((val1-sf[i][j].a)*(val1-sf[i][j].a)+
					(val2-sf[i][j].b)*(val2-sf[i][j].b)))
			{
				min=(val1-sf[i][j].a)*(val1-sf[i][j].a)+
						(val2-sf[i][j].b)*(val2-sf[i][j].b);
				x=i;
				y=j;
			}
		}
	}
	return min;
}

float kohonen::find(float val1, float val2, float val3)
{
	float min=FLT_MAX;
	for(int i=0; i<rows; i++){
		for(int j=0; j<cols; j++){
			if(min>((val1-sf[i][j].a)*(val1-sf[i][j].a)+
					(val2-sf[i][j].b)*(val2-sf[i][j].b)+
					(val3-sf[i][j].c)*(val3-sf[i][j].c))){
				min=(val1-sf[i][j].a)*(val1-sf[i][j].a)+
						(val2-sf[i][j].b)*(val2-sf[i][j].b)+
						(val3-sf[i][j].c)*(val3-sf[i][j].c);
				x=i;
				y=j;
			}
		}
	}
	return min;
}

float kohonen::h(int a1, int b1, float /*sigma*/)
{
	//    float r2 = (x-a)*(x-a)+(y-b)*(y-b);
	//    return exp(-r2/(sigma));
	if(a1==x && b1==y) return 1.0;
	int dx,dy;
	dx=a1-x;
	dy=b1-y;
	if(dx<0)dx=-dx;
	if(dy<0)dy=-dy;
	if(dx>=a || dy>=b) return 0.0;
	return mask[dx][dy];
}

float kohonen::eps(int i, float val)
{
	return(val*exp(-i/K0));
}

grid* kohonen::calc(grid* lg1, long int iter)
{
	float val1;
	float fade;
	time_t now = time(NULL);
	srand(now);
	K0=iter/FADE;
	grid* g1=lg1->grid_copy();
	g1->norm_grid2();
	// random elements from the grid
#ifdef NKOHON
	int a,b;
	for(int i=0; i<rows; i++){
		for(int j=0; j<cols; j++){
			a=rand()%g1->nrows;
			b=rand()%g1->ncols;
			while(g1->feld[a][b]==g1->nodata){
				a=rand()%g1->nrows;
				b=rand()%g1->ncols;
			}
			sf[i][j].a=g1->feld[a][b];
			sf[i][j].b=g1->feld[a][b];
			sf[i][j].c=g1->feld[a][b];
		}
	}
#endif
#ifdef RKOHON
	for(int i=0; i<rows; i++){
		for(int j=0; j<cols; j++){
			sf[i][j].a=(float)rand()/RAND_MAX;
			sf[i][j].b=(float)rand()/RAND_MAX;
			sf[i][j].c=(float)rand()/RAND_MAX;
		}
	}
#endif

	for(long int k=0; k<iter; k++){
		fade=eps(k,1.0);
		if(k%50000==0)
			cerr << fade << " ";
		while((val1=g1->feld[rand()%(g1->nrows)][rand()%(g1->ncols)])==
				g1->nodata);
		find(val1);
		for(int i=0; i<rows; i++){
			for(int j=0; j<cols; j++){
				sf[i][j].a+=fade*h(i,j,SIGMA)*(val1-sf[i][j].a);
			}
		}
	}
	// output grid
	float min=FLT_MAX;
	float d;
	grid* ng=g1->grid_copy();
	for(int i=0; i<g1->nrows; i++){
		for(int j=0; j<g1->ncols; j++){
			if(g1->feld[i][j]!=g1->nodata){
				for(int k1=0; k1<rows; k1++){
					for(int k2=0; k2<cols; k2++){
						d=(sf[k1][k2].a-g1->feld[i][j])*
								(sf[k1][k2].a-g1->feld[i][j]);
						if(min>d){
							ng->feld[i][j]=rows*k1+k2+1;
							// ng->feld[i][j]=sf[k1][k2].a;
							min=d;
						}
					}
				}
				min=FLT_MAX;
			}
			else
				ng->feld[i][j]=-0.001;
		}
	}
	delete g1;
	return ng;
}

grid* kohonen::calc(grid* lg1, grid* lg2, long int iter)
{
	int vx,vy;
	float val1,val2;
	float fade;
	time_t now = time(NULL);
	srand(now);
	K0=iter/FADE;
	grid* g1=lg1->grid_copy();
	grid* g2=lg2->grid_copy();
	g1->norm_grid2();
	g2->norm_grid2();
	// random elements from the grid
#ifdef NKOHON
	int a,b;
	for(int i=0; i<rows; i++){
		for(int j=0; j<cols; j++){
			a=rand()%g1->nrows;
			b=rand()%g1->ncols;
			while(g1->feld[a][b]==g1->nodata ||
					g2->feld[a][b]==g1->nodata){
				a=rand()%g1->nrows;
				b=rand()%g1->ncols;
			}
			sf[i][j].a=g1->feld[a][b];
			sf[i][j].b=g2->feld[a][b];
			sf[i][j].c=g1->feld[a][b];
		}
	}
#endif
#ifdef RKOHON
	for(int i=0; i<rows; i++){
		for(int j=0; j<cols; j++){
			sf[i][j].a=(float)rand()/RAND_MAX;
			sf[i][j].b=(float)rand()/RAND_MAX;
			sf[i][j].c=(float)rand()/RAND_MAX;
		}
	}
#endif

	for(long int k=0; k<iter; k++){
		fade=eps(k,1.0);
		if(k%50000==0)
			cerr << fade << " ";
		do{
			vx=rand()%(g1->nrows);
			vy=rand()%(g1->ncols);
			val1=g1->feld[vx][vy];
			val2=g2->feld[vx][vy];
		}while(val1==g1->nodata || val2==g1->nodata);
		find(val1,val2);
		for(int i=0; i<rows; i++){
			for(int j=0; j<cols; j++){
				sf[i][j].a+=fade*h(i,j,SIGMA)*(val1-sf[i][j].a);
				sf[i][j].b+=fade*h(i,j,SIGMA)*(val2-sf[i][j].b);
			}
		}
	}
	float min=FLT_MAX;
	float d;
	grid* ng=g1->grid_copy();
	for(int i=0; i<g1->nrows; i++){
		for(int j=0; j<g1->ncols; j++){
			if(g1->feld[i][j]!=g1->nodata &&
					g2->feld[i][j]!=g2->nodata){
				for(int k1=0; k1<rows; k1++){
					for(int k2=0; k2<cols; k2++){
						d=(sf[k1][k2].a-g1->feld[i][j])
			    		*(sf[k1][k2].a-g1->feld[i][j])
			    		+(sf[k1][k2].b-g2->feld[i][j])
			    		*(sf[k1][k2].b-g2->feld[i][j]);
						if(min>d){
							ng->feld[i][j]=rows*k1+k2+1;
							min=d;
						}
					}
				}
				min=FLT_MAX;
			}
			else ng->feld[i][j]=-0.001;
		}
	}
	delete g1;
	delete g2;
	return ng;
}

grid* kohonen::calc(grid* lg1, grid* lg2, grid* lg3, long int iter)
{
	float val1,val2,val3;
	float fade;
	time_t now = time(NULL);
	int vx,vy;//,vz;
	srand(now);
	K0=iter/FADE;
	grid* g1=lg1->grid_copy();
	grid* g2=lg2->grid_copy();
	grid* g3=lg3->grid_copy();
	g1->norm_grid2();
	g2->norm_grid2();
	g3->norm_grid2();
	// random elements from the grid
#ifdef NKOHON
	int a,b;
	for(int i=0; i<rows; i++){
		for(int j=0; j<cols; j++){
			a=rand()%g1->nrows;
			b=rand()%g1->ncols;
			while(g1->feld[a][b]==g1->nodata
					|| g2->feld[a][b]==g2->nodata
					|| g3->feld[a][b]==g3->nodata){
				a=rand()%g1->nrows;
				b=rand()%g1->ncols;
			}
			sf[i][j].a=g1->feld[a][b];
			sf[i][j].b=g2->feld[a][b];
			sf[i][j].c=g3->feld[a][b];
		}
	}
#endif
#ifdef RKOHON
	for(int i=0; i<rows; i++){
		for(int j=0; j<cols; j++){
			sf[i][j].a=(float)rand()/RAND_MAX;
			sf[i][j].b=(float)rand()/RAND_MAX;
			sf[i][j].c=(float)rand()/RAND_MAX;
			cerr << sf[i][j].a << " " <<
			sf[i][j].b << " " <<
			sf[i][j].c << " " << endl;
		}
	}
#endif
	for(long int k=0; k<iter; k++){
		fade=eps(k,1.0);
		if(k%50000==0)
			cerr << fade << " ";
		do{
			vx=rand()%(g1->nrows);
			vy=rand()%(g1->ncols);
			val1=g1->feld[vx][vy];
			val2=g2->feld[vx][vy];
			val3=g3->feld[vx][vy];
		}
		while(val1==g1->nodata || val2==g2->nodata || val3==g3->nodata);
		find(val1,val2,val3);
		for(int i=0; i<rows; i++){
			for(int j=0; j<cols; j++){
				sf[i][j].a+=fade*h(i,j,SIGMA)*(val1-sf[i][j].a);
				sf[i][j].b+=fade*h(i,j,SIGMA)*(val2-sf[i][j].b);
				sf[i][j].c+=fade*h(i,j,SIGMA)*(val3-sf[i][j].c);
			}
		}
	}
	float min=FLT_MAX;
	float d;
	grid* ng=g1->grid_copy();
	for(int i=0; i<g1->nrows; i++){
		for(int j=0; j<g1->ncols; j++){
			if(g1->feld[i][j]!=g1->nodata &&
					g2->feld[i][j]!=g2->nodata &&
					g3->feld[i][j]!=g3->nodata){
				for(int k1=0; k1<rows; k1++){
					for(int k2=0; k2<cols; k2++){
						d=(sf[k1][k2].a-g1->feld[i][j])
			    		*(sf[k1][k2].a-g1->feld[i][j])
			    		+(sf[k1][k2].b-g2->feld[i][j])
			    		*(sf[k1][k2].b-g2->feld[i][j])
			    		+(sf[k1][k2].c-g3->feld[i][j])
			    		*(sf[k1][k2].c-g3->feld[i][j]);
						if(min>d){
							ng->feld[i][j]=rows*k1+k2+1;
							// ng->feld[i][j]=sf[k1][k2].c;
							min=d;
						}
					}
				}
				min=FLT_MAX;
			}
			else ng->feld[i][j]=-0.001;
		}
	}
	delete g1;
	delete g2;
	delete g3;
	return ng;
}

void kohonen::save(char* fname)
{
	ofstream file(fname);
	for(int i=0; i<rows; i++){
		for(int j=0; j<cols; j++){
			if(inputs==1)
				file << sf[i][j].a << endl;
			if(inputs==2)
				file << sf[i][j].a << " " << sf[i][j].b << endl;
			if(inputs==3)
				file << sf[i][j].a << " "
				<< sf[i][j].b << " "
				<< sf[i][j].c << endl;
		}
	}
	file.close();
}

void kohonen::save_hasse(char *fname,int flag)
{
	ofstream file(fname);
	float val1,val2,val3;
	for(int i=0; i<rows; i++){
		for(int j=0; j<cols; j++){
			val1=sf[i][j].a;
			val2=sf[i][j].b;
			val3=sf[i][j].c;
			if(flag&1)val1=1-val1;
			if(flag&2)val2=1-val2;
			if(flag&4)val3=1-val3;
			file << "situation: " << i*cols+j+1 << endl;
			if(inputs==1)
				file << "a " << val1 << endl;
			if(inputs==2){
				file << "a " << val1 << endl;
				file << "b " << val2 << endl;
			}
			if(inputs==3){
				file << "a " << val1 << endl;
				file << "b " << val2 << endl;
				file << "c " << val3 << endl;
			}
		}
	}
	file.close();
}


cluster::cluster(int nr1, grid* lg1,int /*flag*/)
{
	inputs=1;
	if(nr1<=1 || nr1>1000){
		cerr << "error (cluster): number of clusters must be between 2..1000"
		<< endl;
		exit(2);
	}
	nr = nr1;
	sf = new struct sensor[nr];
	sf1 = new struct sensor[nr];
	if(sf == NULL || sf1 == NULL){
		cerr << "error (cluster): no sufficient memory" << endl;
		exit(2);
	}
	grid* g1=lg1->grid_copy();
	nrows=g1->nrows;
	ncols=g1->ncols;
	xcorner = g1->xcorner;
	ycorner = g1->ycorner;
	csize = g1->csize;
	nodata = g1->nodata;
	g1->norm_grid2();
	g1->stat();
	feld=(int**) new int*[nrows];
	for(int i=0; i<nrows; i++){
		if((feld[i]=new int[ncols])==NULL){
			cerr << "error (cluster): no sufficient memory" << endl;
			exit(2);
		}
	}
	for(int i=0; i<nrows; i++)
		for(int j=0; j<ncols; j++)
			feld[i][j]=-1;
	// fill with linear numbers
	float delta=(g1->gridmax-g1->gridmin)/(nr-1);
	for(int i=0; i<nr; i++){
		sf[i].a=g1->gridmin+delta*i;
		sf1[i].a=sf[i].a;
		sf[i].b=0.0;
		sf1[i].b=sf[i].b;
		sf[i].c=0.0;
		sf1[i].c=sf[i].c;
		sf1[i].flag=sf[i].flag=0;
	}

	// loop of clustering
	float min=FLT_MAX;
	float d;
	do{
		for(int i=0; i<nrows; i++){
			for(int j=0; j<ncols; j++){
				if(g1->feld[i][j]!=g1->nodata){
					for(int k=0; k<nr; k++){
						d=(sf[k].a-g1->feld[i][j])*(sf[k].a-g1->feld[i][j]);
						if(min>d){
							feld[i][j]=k;
							min=d;
						}
					}
					min=FLT_MAX;
				}
			}
		}
		// recalculate the centers
		float xmin;//,ymin,zmin;
		int count;
		for(int k=0; k<nr; k++){
			xmin=0.0;
			count=0;
			for(int i=0; i<nrows; i++){
				for(int j=0; j<ncols; j++){
					if(g1->feld[i][j]!=g1->nodata){
						if(feld[i][j]==k){
							xmin+=g1->feld[i][j];
							count++;
						}
					}
				}
			}
			if(count>0) xmin/=count;
			if(count==0)sf[k].flag=1;
			sf[k].a=xmin;
		}
		// end of loop?
		d=0.0;
		for(int k=0; k<nr; k++){
			d+=(sf1[k].a-sf[k].a)*(sf1[k].a-sf[k].a);
			sf1[k].a=sf[k].a;
			sf1[k].flag=sf[k].flag;
		}
	}
	while(d>KEND);
	cerr << "cluster algorithm executed\n";
	delete g1;
}

cluster::cluster(int nr1, grid* lg1, grid* lg2,int flag)
{
	inputs=2;
	grid* g1=lg1->grid_copy();
	grid* g2=lg2->grid_copy();
	if(g1->nrows!=g2->nrows || g1->ncols!=g2->ncols){
		cerr << "error (cluster): the grids must be of same size" << endl;
		exit(2);
	}
	if(nr1<=1 || nr1>1000){
		cerr << "error (cluster): number of clusters must be between 2..1000"
		<< endl;
		exit(2);
	}
	nr = nr1;
	nrows=g1->nrows;
	ncols=g1->ncols;
	xcorner = g1->xcorner;
	ycorner = g1->ycorner;
	csize = g1->csize;
	nodata = g1->nodata;
	g1->norm_grid2();
	g2->norm_grid2();

	sf = new struct sensor[nr];
	sf1 = new struct sensor[nr];
	if(sf == NULL || sf1 == NULL){
		cerr << "error (cluster): no sufficient memory" << endl;
		exit(2);
	}
	feld=(int**) new int*[nrows];
	for(int i=0; i<nrows; i++){
		if((feld[i]=new int[ncols])==NULL){
			cerr << "error (cluster): no sufficient memory" << endl;
			exit(2);
		}
	}
	for(int i=0; i<nrows; i++)
		for(int j=0; j<ncols; j++)
			feld[i][j]=-1;

	// fill with random numbers
	time_t now = time(NULL);
	srand(now);
	int a,b; // init with random elements from the grid
	for(int i=0; i<nr; i++){
		a=rand()%nrows;
		b=rand()%ncols;
		while(g1->feld[a][b]==g1->nodata || g2->feld[a][b]==g2->nodata){
			a=rand()%nrows;
			b=rand()%ncols;
		}
		sf[i].a=g1->feld[a][b];
		sf1[i].a=sf[i].a;
		sf[i].b=g2->feld[a][b];
		sf1[i].b=sf[i].b;
		sf[i].c=(float)rand()/RAND_MAX;
		sf1[i].c=sf[i].c;
		sf1[i].flag=sf[i].flag=0;
	}
	for(int k=0; k<nr; k++){
		fprintf(stderr,"%d %f %f\n",k,sf[k].a,sf[k].b);
	}
	if(flag==0){
		// loop of clustering
		float min=FLT_MAX;
		float d;
		do{
			for(int i=0; i<nrows; i++){
				for(int j=0; j<ncols; j++){
					if(g1->feld[i][j]!=g1->nodata && g2->feld[i][j]!=
							g2->nodata){
						for(int k=0; k<nr; k++){
							d=(sf[k].a-g1->feld[i][j])*(sf[k].a-g1->feld[i][j])
						+(sf[k].b-g2->feld[i][j])*
						(sf[k].b-g2->feld[i][j]);
							if(min>d){
								feld[i][j]=k;
								min=d;
							}
						}
						min=FLT_MAX;
					}
				}
			}
			// recalculate the centers
			float xmin,ymin;//,zmin;
			int count;
			for(int k=0; k<nr; k++){
				xmin=0.0;
				ymin=0.0;
				count=0;
				for(int i=0; i<nrows; i++){
					for(int j=0; j<ncols; j++){
						if(g1->feld[i][j]!=g1->nodata &&
								g2->feld[i][j]!=g2->nodata){
							if(feld[i][j]==k){
								xmin+=g1->feld[i][j];
								ymin+=g2->feld[i][j];
								count++;
							}
						}
					}
				}
				if(count>0){
					xmin/=count;
					ymin/=count;
				}
				if(count==0)sf[k].flag=1;
				sf[k].a=xmin;
				sf[k].b=ymin;
			}
			// end of loop?
			d=0.0;
			for(int k=0; k<nr; k++){
				d+=(sf1[k].a-sf[k].a)*(sf1[k].a-sf[k].a)+
						(sf1[k].b-sf[k].b)*(sf1[k].b-sf[k].b);
				sf1[k].a=sf[k].a;
				sf1[k].b=sf[k].b;
				sf1[k].flag=sf[k].flag;
			}
		}
		while(d>KEND);
	}
	else{
		float min=FLT_MAX;
		int klass1,klass2;
		double di,dj,window,dist;
		double alpha,eps;
		alpha=0.1;
		eps=0.2;
		window=0.6;
		for(int l=0; l<10; l++){
			for(int i=0; i<nrows; i++){
				for(int j=0; j<ncols; j++){
					if(g1->feld[i][j]!=g1->nodata &&
							g2->feld[i][j]!=g2->nodata){
						klass1=klass2=0;
						for(int k=0; k<nr; k++){
							di=(sf[k].a-g1->feld[i][j])*
									(sf[k].a-g1->feld[i][j])
									+(sf[k].b-g2->feld[i][j])*
									(sf[k].b-g2->feld[i][j]);
							if(min>di){
								klass2=klass1;
								klass1=k;
								min=di;
							}
						}
						// adapt classes
						if(klass1!=klass2){
							di=(sf[klass1].a-g1->feld[i][j])*
									(sf[klass1].a-g1->feld[i][j])
									+(sf[klass1].b-g2->feld[i][j])*
									(sf[klass1].b-g2->feld[i][j]);
							dj=(sf[klass2].a-g1->feld[i][j])*
									(sf[klass2].a-g1->feld[i][j])
									+(sf[klass2].b-g2->feld[i][j])*
									(sf[klass2].b-g2->feld[i][j]);
							if(di>0.0 && dj>0.0){
								if(di/dj>dj/di) dist=dj/di;
								else dist=di/dj;
								if(dist>window){
									sf[klass1].a+=
											alpha*(g1->feld[i][j]-sf[klass1].a);
									sf[klass1].b+=
											alpha*(g2->feld[i][j]-sf[klass1].b);
									sf[klass2].a-=
											alpha*(g1->feld[i][j]-sf[klass2].a);
									sf[klass2].b-=
											alpha*(g2->feld[i][j]-sf[klass2].b);
								}
							}
							sf[klass1].a+=
									alpha*eps*(g1->feld[i][j]-sf[klass1].a);
							sf[klass1].b+=
									alpha*eps*(g2->feld[i][j]-sf[klass1].b);
							sf[klass2].a+=
									alpha*eps*(g1->feld[i][j]-sf[klass2].a);
							sf[klass2].b+=
									alpha*eps*(g2->feld[i][j]-sf[klass2].b);
						}
						else{
							sf[klass1].a+=
									alpha*eps*(g1->feld[i][j]-sf[klass1].a);
							sf[klass1].b+=
									alpha*eps*(g2->feld[i][j]-sf[klass1].b);
						}
						min=FLT_MAX;
					}
				}
			}
		}
		for(int i=0; i<nrows; i++){
			for(int j=0; j<ncols; j++){
				if(g1->feld[i][j]!=g1->nodata &&
						g2->feld[i][j]!=g2->nodata){
					min=FLT_MAX;
					for(int k=0; k<nr; k++){
						di=(sf[k].a-g1->feld[i][j])*(sf[k].a-g1->feld[i][j])
			    		+(sf[k].b-g2->feld[i][j])*(sf[k].b-g2->feld[i][j]);
						if(min>di){
							feld[i][j]=k;
							min=di;
						}
					}
				}
			}
		}

	}
	delete g1;
	delete g2;
}

cluster::cluster(int nr1, grid* lg1, grid* lg2, grid* lg3,int flag)
{
	inputs=3;
	grid* g1=lg1->grid_copy();
	grid* g2=lg2->grid_copy();
	grid* g3=lg3->grid_copy();

	if(g1->nrows!=g2->nrows || g1->ncols!=g2->ncols
			|| g3->nrows!=g2->nrows || g3->ncols!=g2->ncols){
		cerr << "cluster_error: the grids must be of same size" << endl;
		exit(2);
	}
	if(nr1<=1 || nr1>1000){
		cerr << "error (cluster): number of clusters must be between 2..1000"
		<< endl;
		exit(2);
	}
	nrows=g1->nrows;
	ncols=g1->ncols;
	xcorner = g1->xcorner;
	ycorner = g1->ycorner;
	csize = g1->csize;
	nodata = g1->nodata;
	g1->norm_grid2();
	g2->norm_grid2();
	g3->norm_grid2();

	nr = nr1;
	sf = new struct sensor[nr];
	sf1 = new struct sensor[nr];
	if(sf == NULL || sf1 == NULL){
		cerr << "error (cluster): no sufficient memory" << endl;
		exit(2);
	}
	feld=(int**) new int*[nrows];
	for(int i=0; i<nrows; i++){
		if((feld[i]=new int[ncols])==NULL){
			cerr << "error (cluster): no sufficient memory" << endl;
			exit(2);
		}
	}
	for(int i=0; i<nrows; i++)
		for(int j=0; j<ncols; j++)
			feld[i][j]=-1;
	// fill with random numbers
	time_t now = time(NULL);
	srand(now);
	int a,b; // init with random elements from the grid
	for(int i=0; i<nr; i++){
		a=rand()%nrows;
		b=rand()%ncols;
		while(g1->feld[a][b]==g1->nodata ||
				g2->feld[a][b]==g2->nodata ||
				g3->feld[a][b]==g3->nodata){
			a=rand()%nrows;
			b=rand()%ncols;
		}
		sf[i].a=g1->feld[a][b];
		sf1[i].a=sf[i].a;
		sf[i].b=g2->feld[a][b];
		sf1[i].b=sf[i].b;
		sf[i].c=g3->feld[a][b];
		sf1[i].c=sf[i].c;
		sf1[i].flag=sf[i].flag=0;
	}


	// loop of clustering
	if(flag==0){
		float min=FLT_MAX;
		float d;
		do{
			for(int i=0; i<nrows; i++){
				for(int j=0; j<ncols; j++){
					if(g1->feld[i][j]!=g1->nodata &&
							g2->feld[i][j]!=g2->nodata &&
							g3->feld[i][j]!=g3->nodata){
						for(int k=0; k<nr; k++){
							d=(sf[k].a-g1->feld[i][j])*
									(sf[k].a-g1->feld[i][j])
									+(sf[k].b-g2->feld[i][j])*
									(sf[k].b-g2->feld[i][j])
									+(sf[k].c-g3->feld[i][j])*
									(sf[k].c-g3->feld[i][j]);
							if(min>d){
								feld[i][j]=k;
								min=d;
							}
						}
						min=FLT_MAX;
					}
				}
			}
			// recalculate the centers
			float xmin,ymin,zmin;
			int count;
			for(int k=0; k<nr; k++){
				xmin=0.0;
				ymin=0.0;
				zmin=0.0;
				count=0;
				for(int i=0; i<nrows; i++){
					for(int j=0; j<ncols; j++){
						if(g1->feld[i][j]!=g1->nodata &&
								g2->feld[i][j]!=g2->nodata &&
								g3->feld[i][j]!=g3->nodata){
							if(feld[i][j]==k){
								xmin+=g1->feld[i][j];
								ymin+=g2->feld[i][j];
								zmin+=g3->feld[i][j];
								count++;
							}
						}
					}
				}
				if(count>0){
					xmin/=count;
					ymin/=count;
					zmin/=count;
				}
				else{xmin=ymin=zmin=0.0;}
				if(count==0)sf[k].flag=1;

				// fprintf(stderr,"%f %f %f\n",xmin,ymin,zmin);
				sf[k].a=xmin;
				sf[k].b=ymin;
				sf[k].c=zmin;
			}
			// end of loop?
			d=0.0;
			for(int k=0; k<nr; k++){
				d+= (sf1[k].a-sf[k].a)*(sf1[k].a-sf[k].a)+
						(sf1[k].b-sf[k].b)*(sf1[k].b-sf[k].b)+
						(sf1[k].c-sf[k].c)*(sf1[k].c-sf[k].c);
				sf1[k].a=sf[k].a;
				sf1[k].b=sf[k].b;
				sf1[k].c=sf[k].c;
				sf1[k].flag=sf1[k].flag;
			}
		}while(d>KEND);
	}
	// loop of clustering
	else{
		float min=FLT_MAX;
		int klass1,klass2;
		double di,dj,window,dist;
		double alpha,eps;
		alpha=0.1;
		eps=0.2;
		window=0.6;
		for(int l=0; l<10; l++){
			for(int i=0; i<nrows; i++){
				for(int j=0; j<ncols; j++){
					if(g1->feld[i][j]!=g1->nodata &&
							g2->feld[i][j]!=g2->nodata &&
							g3->feld[i][j]!=g3->nodata){
						klass1=klass2=0;
						for(int k=0; k<nr; k++){
							di=(sf[k].a-g1->feld[i][j])*
									(sf[k].a-g1->feld[i][j])
									+(sf[k].b-g2->feld[i][j])*
									(sf[k].b-g2->feld[i][j])
									+(sf[k].c-g3->feld[i][j])*
									(sf[k].c-g3->feld[i][j]);
							if(min>di){
								klass2=klass1;
								klass1=k;
								min=di;
							}
						}
						// adapt classes
						if(klass1!=klass2){
							di=(sf[klass1].a-g1->feld[i][j])*
									(sf[klass1].a-g1->feld[i][j])
									+(sf[klass1].b-g2->feld[i][j])*
									(sf[klass1].b-g2->feld[i][j])
									+(sf[klass1].c-g3->feld[i][j])*
									(sf[klass1].c-g3->feld[i][j]);
							dj=(sf[klass2].a-g1->feld[i][j])*
									(sf[klass2].a-g1->feld[i][j])
									+(sf[klass2].b-g2->feld[i][j])*
									(sf[klass2].b-g2->feld[i][j])
									+(sf[klass2].c-g3->feld[i][j])*
									(sf[klass2].c-g3->feld[i][j]);
							if(di>0.0 && dj>0.0){
								if(di/dj>dj/di) dist=dj/di;
								else dist=di/dj;
								if(dist>window){
									sf[klass1].a+=
											alpha*(g1->feld[i][j]-sf[klass1].a);
									sf[klass1].b+=
											alpha*(g2->feld[i][j]-sf[klass1].b);
									sf[klass1].c+=
											alpha*(g3->feld[i][j]-sf[klass1].c);
									sf[klass2].a-=
											alpha*(g1->feld[i][j]-sf[klass2].a);
									sf[klass2].b-=
											alpha*(g2->feld[i][j]-sf[klass2].b);
									sf[klass2].c-=
											alpha*(g3->feld[i][j]-sf[klass2].c);
								}
							}
							sf[klass1].a+=
									alpha*eps*(g1->feld[i][j]-sf[klass1].a);
							sf[klass1].b+=
									alpha*eps*(g2->feld[i][j]-sf[klass1].b);
							sf[klass1].c+=
									alpha*eps*(g3->feld[i][j]-sf[klass1].c);
							sf[klass2].a+=
									alpha*eps*(g1->feld[i][j]-sf[klass2].a);
							sf[klass2].b+=
									alpha*eps*(g2->feld[i][j]-sf[klass2].b);
							sf[klass2].c+=
									alpha*eps*(g3->feld[i][j]-sf[klass2].c);
						}
						else{
							sf[klass1].a+=
									alpha*eps*(g1->feld[i][j]-sf[klass1].a);
							sf[klass1].b+=
									alpha*eps*(g2->feld[i][j]-sf[klass1].b);
							sf[klass1].c+=
									alpha*eps*(g3->feld[i][j]-sf[klass1].c);
						}
						min=FLT_MAX;
					}
				}
			}
		}
		for(int i=0; i<nrows; i++){
			for(int j=0; j<ncols; j++){
				if(g1->feld[i][j]!=g1->nodata
						&& g2->feld[i][j]!=g2->nodata
						&& g3->feld[i][j]!=g3->nodata){
					min=FLT_MAX;
					for(int k=0; k<nr; k++){
						di=(sf[k].a-g1->feld[i][j])*(sf[k].a-g1->feld[i][j])
			    		+(sf[k].b-g2->feld[i][j])*(sf[k].b-g2->feld[i][j])
			    		+(sf[k].c-g3->feld[i][j])*(sf[k].c-g3->feld[i][j]);
						if(min>di){
							feld[i][j]=k;
							min=di;
						}
					}
				}
			}
		}
	}
	delete g1;
	delete g2;
	delete g3;
}

cluster::~cluster()
{
	if(sf1!=NULL) delete [] sf1;
	if(sf!=NULL)  delete [] sf;
	for(int i=0; i<nrows; i++){
		if(feld[i]!=NULL)
			delete [] feld[i];
	}
	delete [] feld;
}

void cluster::save(char *fname)
{
	ofstream file(fname);
	for(int i=0; i<nr; i++){
		if(inputs==1)
			file << sf1[i].a << endl;
		if(inputs==2)
			file << sf1[i].a << " " << sf1[i].b << endl;
		if(inputs==3)
			file << sf1[i].a << " "
			<< sf1[i].b << " "
			<< sf1[i].c << endl;
	}
	file.close();
}

void cluster::save_hasse(char *fname,int flag)
{
	ofstream file(fname);
	float val1,val2,val3;
	for(int i=0; i<nr; i++){
		file << "situation: " << i << endl;
		val1=sf1[i].a;
		val2=sf1[i].b;
		val3=sf1[i].c;
		if(flag&1)val1=1-val1;
		if(flag&2)val2=1-val2;
		if(flag&4)val3=1-val3;
		if(inputs==1)
			file << "a " << val1 << endl;
		if(inputs==2){
			file << "a " << val1 << endl;
			file << "b " << val2 << endl;
		}
		if(inputs==3){
			file << "a " << val1 << endl;
			file << "b " << val2 << endl;
			file << "c " << val3 << endl;
		}
	}
	file.close();
}

grid* cluster::get_cluster()
{
	grid* gx = new grid((int)csize);
	gx->ncols = ncols;
	gx->nrows = nrows;
	gx->xcorner = xcorner;
	gx->ycorner = ycorner;
	gx->csize = csize;
	gx->nodata = nodata;
	gx->feld = (float**) new float*[gx->nrows];
	for(int i=0; i<gx->nrows; i++){
		if((gx->feld[i]=new float[gx->ncols])==NULL){
			cerr << "error (get_cluster): no sufficient memory" << endl;
			exit(2);
		}
	}
	for(int i=0; i<nrows; i++){
		for(int j=0; j<ncols; j++){
			if(feld[i][j]<0) gx->feld[i][j]=-0.001;
			else
				gx->feld[i][j] = (float)feld[i][j];
		}
	}
	return gx;
}


cluster_hill::cluster_hill(int n)
{
	if(n<2 || n>1000){
		cerr << "error in cluster_hill: too few number of clusters: "
		<< n << endl;
		return;
	}
	nClusters=n;
	grids = new grid*[nClusters];
	if(grids==0){
		cerr << "error in cluster_hill: no space left on device\n";
		return;
	}
	nGrids=0;
	Variances=new double[nClusters];
	nMembers=new int[nClusters];
}

cluster_hill::~cluster_hill()
{
	if(cFeld!=0){
		for(int i=0; i<grids[0]->nrows; i++)
			delete [] cFeld[i];
		delete [] cFeld;
	}
	if(grids!=0)
		delete [] grids;
	if(Variances!=0)
		delete [] Variances;
	if(nMembers!=0)
		delete [] nMembers;
}

void cluster_hill::load_grid(grid *gx)
{
	if(gx==0) return;
	grids[nGrids++]=gx;
}

void cluster_hill::calc(int nLoops)
{
	double V;
	double delta=-0.001;
	int iCluster, jCluster, kCluster;
	// init the clusters with all grid cells
	// organize memory
	if(grids[0]==0){
		cerr << " error in cluster_hill calc: no grids loaded\n";
		return;
	}
	cFeld=new int*[grids[0]->nrows];
	if(cFeld==0){
		cerr << "error in cluster_hill calc: no space left on device\n";
		return;
	}
	for(int i=0; i<grids[0]->nrows; i++){
		cFeld[i]=new int[grids[0]->ncols];
		if(cFeld[i]==0){
			cerr << "error in cluster_hill calc: no space left on device\n";
			return;
		}
	}
	// memory for the centroids
	Centroids = new double*[nClusters];
	if(Centroids==0){
		cerr << "error in cluster_hill calc: no space left on device\n";
		return;
	}
	for(int i=0; i<nClusters; i++){
		Centroids[i] = new double[nGrids];
		if(Centroids[i]==0){
			cerr << "error in cluster_hill calc: no space left on device\n";
			return;
		}
	}
	for(iCluster=0; iCluster<nClusters; iCluster++)
	{
		Variances[iCluster]	= 0;
		nMembers [iCluster]	= 0;

		for(int iGrid=0; iGrid<nGrids; iGrid++)
		{
			Centroids[iCluster][iGrid]	= 0;
		}
	}
	// init cFeld, centroids, Variances and nMembers
	V = 0.0;
	double d;
	for(int i=0; i<grids[0]->nrows; i++){
		for(int j=0; j<grids[0]->ncols; j++){
			cFeld[i][j]=(i*(grids[0]->ncols)+j)%nClusters;
			for(int k=0; k<nGrids; k++){
				d=grids[k]->feld[i][j];
				if(d==grids[k]->nodata) d=grids[k]->gridmin+delta;
				Centroids[cFeld[i][j]][k]+=d;
				V += d*d;
			}
			Variances[cFeld[i][j]] += V;
			nMembers[cFeld[i][j]]++;
		}
	}
	// renorm Centoids
	double SP = 0.0;
	double SP_last=-1.0;
	for(iCluster=0; iCluster<nClusters; iCluster++)
	{
		d = nMembers[iCluster] != 0 ? 1.0 / (double)nMembers[iCluster] : 0;
		V = 0.0;
		for(int iGrid=0; iGrid<nGrids; iGrid++)
		{
			Centroids[iCluster][iGrid]	*= d;
			double e = Centroids[iCluster][iGrid];
			V += e * e;
		}
		Variances[iCluster] -= nMembers[iCluster] * V;
		SP += Variances[iCluster];
	}
	// main loop
	int noShift = 0;
	bool bcontinue=true;
	int n_iK, n_jK;
	double V1,V2,VMin;
	for(int l=0; l<nLoops && bcontinue; l++){
		if(noShift>=(grids[0]->nrows)*(grids[0]->ncols))
			bcontinue=false;
		else{
			for(int i=0; i<grids[0]->nrows; i++){
				for(int j=0; j<grids[0]->ncols; j++){
					if(nMembers[cFeld[i][j]] > 1 )
					{
						iCluster=cFeld[i][j];
						V  = 0.0;
						for(int iGrid=0; iGrid<nGrids; iGrid++)
						{
							d = grids[iGrid]->feld[i][j];
							if(d==grids[iGrid]->nodata)
								d=grids[iGrid]->gridmin+delta;
							d	= Centroids[iCluster][iGrid] - d;
							V	+= d * d;
						}

						n_iK	= nMembers[iCluster];
						V1	= V * n_iK / (n_iK - 1.0);
						VMin	= -1.0;

						//-----------------------------------------
						// select best matching cluster

						for(jCluster=0; jCluster<nClusters; jCluster++)
						{
							if( jCluster != iCluster )
							{
								V = 0.0;
								for(int iGrid=0; iGrid<nGrids; iGrid++)
								{
									d = grids[iGrid]->feld[i][j];
									if(d==grids[iGrid]->nodata)
										d=grids[iGrid]->gridmin+delta;
									d	= Centroids[jCluster][iGrid] - d;
									V	+= d * d;
								}

								n_jK	= nMembers[jCluster];
								V2	= V * n_jK / (n_jK + 1.0);

								if( VMin < 0 || V2 < VMin )
								{
									VMin	= V2;
									kCluster	= jCluster;
								}
							}
						}
						//-----------------------------------------
						// change cluster
						if( VMin >= 0 && VMin < V1 )
						{
							noShift = 0;
							Variances[iCluster]	-= V1;
							Variances[kCluster]	+= VMin;
							SP	= SP - V1 + VMin;
							V1	= 1.0 / (n_iK - 1.0);
							n_jK= nMembers[kCluster];
							V2	= 1.0 / (n_jK + 1.0);
							for(int iGrid=0; iGrid<nGrids; iGrid++)
							{
								d = grids[iGrid]->feld[i][j];
								if(d==grids[iGrid]->nodata)
									d=grids[iGrid]->gridmin+delta;
								Centroids[iCluster][iGrid]=
										(n_iK * Centroids[iCluster][iGrid] - d)
										* V1;
								Centroids[kCluster][iGrid]=
										(n_jK * Centroids[kCluster][iGrid] + d)
										* V2;
							}
							cFeld[i][j]=kCluster;
							nMembers[iCluster]--;
							nMembers[kCluster]++;
						}
					}
				}
			}
		}
		double res= SP_last < 0.0 ? SP : SP_last - SP;
		cerr << "loop= " << l << "\t" << res
		<< endl;
		if(SP_last > 0 && SP_last-SP<0.01) bcontinue=false;
		SP_last=SP;
	}
}

void cluster_hill::save(char *fname)
{
	ofstream file(fname);
	for(int iCluster=0; iCluster<nClusters; iCluster++){
		for(int iGrid=0; iGrid<nGrids; iGrid++){
			file << Centroids[iCluster][iGrid] << " ";
		}
		file << endl;
	}
	file.close();
}

grid* cluster_hill::get_cluster()
{
	grid* gout=new grid(100);
	gout = grids[0]->grid_copy();
	for(int i=0; i<gout->nrows; i++)
		for(int j=0; j<gout->ncols; j++)
			if(gout->feld[i][j]!=gout->nodata)
				gout->feld[i][j]=cFeld[i][j];
	return gout;
}


region::region(int r, int character)
{
	fprintf(stderr,"region: r=%d char=%d\n",r,character);
	radius=r;
	ncols=2*r+1;
	nrows=2*r+1;
	int r2=(r*r);
	maske=(int**) new int*[nrows];
	for(int i=0; i<nrows; i++){
		if((maske[i]=new int[ncols])==NULL){
			cerr << "error (region): no sufficient memory" << endl;
		}
	}
	if(character==CIRCLE){
		for(int i=0; i<nrows; i++){
			for(int j=0; j<ncols; j++){
				if(((i-r)*(i-r)+(j-r)*(j-r))<=r2) maske[i][j]=1;
				else maske[i][j]=0;
			}
		}
	}
	if(character==MOORE){
		for(int i=0; i<nrows; i++){
			for(int j=0; j<ncols; j++){
				maske[i][j]=1;
			}
		}
	}
	if(character==MOORE2){
		for(int i=0; i<nrows; i++){
			for(int j=0; j<ncols; j++){
				maske[i][j]=1;
			}
		}
		maske[radius][radius]=0;
	}
	if(character==MOORE1){
		for(int i=0; i<nrows; i++){
			for(int j=0; j<ncols; j++){
				maske[i][j]=1;
				if(i==0 && j==0) maske[i][j]=0;
				if(i==0 && j==ncols-1) maske[i][j]=0;
				if(i==nrows-1 && j==0) maske[i][j]=0;
				if(i==nrows-1 && j==ncols-1) maske[i][j]=0;
			}
		}
	}
}

region::~region()
{
	if(maske!=(int**)NULL){
		for(int i=0; i<nrows; i++){
			delete [] maske[i];
		}
	}
}

void region::calc_sum(grid* g1, grid* ng)
{
	int ax,ay,ex,ey,zeile,spalte;
	float val,erg;
	for(zeile=0; zeile<g1->nrows; zeile++){
		ax=zeile-radius;
		if(ax<0) ax=0;
		ex=zeile+radius;
		if(ex>=g1->nrows) ex=g1->nrows-1;
		for(spalte=0; spalte<g1->ncols; spalte++){
			ay=spalte-radius;
			if(ay<0) ay=0;
			ey=spalte+radius;
			if(ey>=g1->ncols)ey=g1->ncols-1;
			for(int i=ax; i<=ex; i++){
				for(int j=ay; j<=ey; j++){
					val = g1->feld[i][j];
					if(int(val)!=g1->nodata)
						erg+=(maske[i-ax][j-ay]>0) ? val : 0;
				}
			}
			if(ng->feld[zeile][spalte]!=ng->nodata)
				ng->feld[zeile][spalte]=erg;
			erg=0.0;
		}
	}
}

double region::apen(vector<double> test, int mm, double r)
{
	double phi1,phi2;
	double v;
	int m=mm;
	int lll=test.size()-m+1;
	double v1;
	phi1=phi2=0.0;
	for(int i=0; i<lll; i++){
		v=0.0;
		for(int j=i; j<lll; j++){
			v1=0.0;
			for(int p=0; p<m; p++){
				v1+=abs(test[i+p]-test[j+p]);
			}
			if(r>v1) v1=1.0;
			else v1=0.0;
			v+=v1;
		}
		phi1-=v/(lll*(lll-1)/2)*log(v/(lll*(lll-1)/2));
	}
	// phi1/=(lll-m+1);
	m++;
	lll--;
	for(int i=0; i<lll; i++){
		v=0.0;
		for(int j=i; j<lll; j++){
			v1=0.0;
			for(int p=0; p<m; p++){
				v1+=abs(test[i+p]-test[j+p]);
			}
			if(r>v1) v1=1.0;
			else v1=0.0;
			v+=v1;
		}
		phi2-=v/(lll*(lll-1)/2)*log(v/(lll*(lll-1)/2));
	}
	// phi2/=(lll-m+1);
	return(phi1-phi2);
}

void region::calc_apen(grid* g1, grid* ng, int m)
{
	int ax,ay,ex,ey,zeile,spalte;
	float val,erg;
	double sd,mean;
	// calculate the standard deviation sd of the input grid
	sd=mean=0.0;
	int count=0;
	for(int i=0; i<g1->nrows; i++){
		for(int j=0; j<g1->ncols; j++){
			if(g1->feld[i][j]!=g1->nodata){
				mean+=g1->feld[i][j];
				count++;
			}
		}
	}
	if(count<=1) return;
	mean/=count;
	for(int i=0; i<g1->nrows; i++){
		for(int j=0; j<g1->ncols; j++){
			if(g1->feld[i][j]!=g1->nodata){
				sd+=(g1->feld[i][j]-mean)*(g1->feld[i][j]-mean);
			}
		}
	}
	sd/=(count-1);
	sd=sqrt(sd);
	// calc apen in maks
	vector<double> test(ncols*nrows);
	count=0;
	for(zeile=0; zeile<g1->nrows; zeile++){
		ax=zeile-radius;
		if(ax<0) ax=0;
		ex=zeile+radius;
		if(ex>=g1->nrows) ex=g1->nrows-1;
		for(spalte=0; spalte<g1->ncols; spalte++){
			ay=spalte-radius;
			if(ay<0) ay=0;
			ey=spalte+radius;
			if(ey>=g1->ncols)ey=g1->ncols-1;
			count=0;
			for(int i=ax; i<=ex; i++){
				for(int j=ay; j<=ey; j++){
					val = g1->feld[i][j];
					if(int(val)!=g1->nodata)
						test[count]=(double)val;
					else
						test[count]=mean;
					count++;
				}
			}
			erg=apen(test,m,0.2*sd);
			if(ng->feld[zeile][spalte]!=ng->nodata)
				ng->feld[zeile][spalte]=erg;
			erg=0.0;
		}
	}
}

void region::calc_hist(grid* g1, grid* ng)
{
	int ax,ay,ex,ey,zeile,spalte;
	float val,erg=1.0;
	mapType diversity;
	mapType::iterator i_div;
	for(zeile=0; zeile<g1->nrows; zeile++){
		ax=zeile-radius;
		if(ax<0) ax=0;
		ex=zeile+radius;
		if(ex>=g1->nrows) ex=g1->nrows-1;
		for(spalte=0; spalte<g1->ncols; spalte++){
			ay=spalte-radius;
			if(ay<0) ay=0;
			ey=spalte+radius;
			if(ey>=g1->ncols)ey=g1->ncols-1;
			diversity.clear();
			for(int i=ax; i<=ex; i++){
				for(int j=ay; j<=ey; j++){
					val = g1->feld[i][j];
					if(int(val)!=g1->nodata){
						if(diversity.end()==diversity.find((int)val)){
							diversity.insert(ValuePair((int)val,1));
							// erg+=(maske[i-ax][j-ay]>0) ? 1 : 0;
							erg+=1;
						}
					}
				}
			}
			if(ng->feld[zeile][spalte]!=ng->nodata)
				ng->feld[zeile][spalte]=erg;
			erg=1.0;
		}
	}
}

void region::calc_count(grid* g1, grid* ng, int value)
{
	int ax,ay,ex,ey,zeile,spalte;
	float val,erg;
	for(zeile=0; zeile<g1->nrows; zeile++){
		ax=zeile-radius;
		if(ax<0) ax=0;
		ex=zeile+radius;
		if(ex>=g1->nrows) ex=g1->nrows-1;
		for(spalte=0; spalte<g1->ncols; spalte++){
			ay=spalte-radius;
			if(ay<0) ay=0;
			ey=spalte+radius;
			if(ey>=g1->ncols)ey=g1->ncols-1;
			for(int i=ax; i<=ex; i++){
				for(int j=ay; j<=ey; j++){
					val = g1->feld[i][j];
					if(int(val)!=g1->nodata && (int)val==value)
						erg+=
								(maske[i-zeile+radius][j-spalte+radius]>0)
								? val : 0;
				}
			}
			if(ng->feld[zeile][spalte]!=ng->nodata)
				ng->feld[zeile][spalte]=erg;
			erg=0.0;
		}
	}
}

void region::calc_mean(grid* g1,grid* ng)
{
	int ax,ay,ex,ey,zeile,spalte,count=0;
	float val,erg;
	erg=0.0;
	for(zeile=0; zeile<g1->nrows; zeile++){
		ax=zeile-radius;
		if(ax<0) ax=0;
		ex=zeile+radius;
		if(ex>=g1->nrows) ex=g1->nrows-1;
		for(spalte=0; spalte<g1->ncols; spalte++){
			ay=spalte-radius;
			if(ay<0) ay=0;
			ey=spalte+radius;
			if(ey>=g1->ncols)ey=g1->ncols-1;
			for(int i=ax; i<=ex; i++){
				for(int j=ay; j<=ey; j++){
					val = g1->feld[i][j];
					if(int(val)!=g1->nodata){
						if(maske[i-zeile+radius][j-spalte+radius]>0){
							erg+=g1->feld[i][j];
							count++;
						}
					}
				}
			}
			if(ng->feld[zeile][spalte]!=ng->nodata){
				if(count!=0)ng->feld[zeile][spalte]=erg/count;
				else ng->feld[zeile][spalte]=0.0;
			}
			count=0;
			erg=0.0;
		}
	}
}

int Grids::compare(const void* e1, const void* e2)
{
	float* v1=(float*)e1;
	float* v2=(float*)e2;
	return (*v1<*v2) ? -1 : (*v1>*v2) ? 1 : 0;
}

void region::calc_median(grid* g1, grid* ng)
{
	int ax,ay,ex,ey,zeile,spalte,count=0;
	float val,erg;
	float* vec=new float[4*(radius+1)*(radius+1)];
	for(int k=0; k<4*(radius+1)*(radius+1); k++)
		vec[k]=0.0;
	for(zeile=0; zeile<g1->nrows; zeile++){
		ax=zeile-radius;
		if(ax<0) ax=0;
		ex=zeile+radius;
		if(ex>=g1->nrows) ex=g1->nrows-1;
		for(spalte=0; spalte<g1->ncols; spalte++){
			ay=spalte-radius;
			if(ay<0) ay=0;
			ey=spalte+radius;
			if(ey>=g1->ncols)ey=g1->ncols-1;
			for(int i=ax; i<=ex; i++){
				for(int j=ay; j<=ey; j++){
					val = g1->feld[i][j];
					if(int(val)!=g1->nodata){
						vec[count++]=
								(maske[i-zeile+radius][j-spalte+radius]>0)
								? val : 0;
					}
				}
			}
			qsort((void*)vec,(size_t)count,sizeof(float),compare);
			if(ng->feld[zeile][spalte]!=ng->nodata){
				if(count%2==0)
					ng->feld[zeile][spalte]=0.5*(vec[count/2]+vec[count/2-1]);
				else
					ng->feld[zeile][spalte]=vec[count/2];
			}
			count=0;
			erg=0.0;
		}
	}
	delete [] vec;
}

void region::calc_std(grid* g1,grid* ng)
{
	int ax,ay,ex,ey,zeile,spalte,count=0;
	float val,erg,mean;
	erg=0.0;
	mean=0.0;
	for(zeile=0; zeile<g1->nrows; zeile++){
		ax=zeile-radius;
		if(ax<0) ax=0;
		ex=zeile+radius;
		if(ex>=g1->nrows) ex=g1->nrows-1;
		for(spalte=0; spalte<g1->ncols; spalte++){
			ay=spalte-radius;
			if(ay<0) ay=0;
			ey=spalte+radius;
			if(ey>=g1->ncols)ey=g1->ncols-1;
			for(int i=ax; i<=ex; i++){
				for(int j=ay; j<=ey; j++){
					val = g1->feld[i][j];
					if(int(val)!=g1->nodata){
						mean+=(maske[i-zeile+radius][j-spalte-radius]>0)
			    		? val : 0;
						count++;
					}
				}
			}
			if(count!=0) mean/=count;
			else mean=0;
			for(int i=ax; i<ex; i++){
				for(int j=ay; j<ey; j++){
					val = g1->feld[i][j];
					if(int(val)!=g1->nodata)
						erg+=(maske[i-ax][j-ay]>0) ?
								(val-mean)*(val-mean) : 0;
				}
			}
			if(ng->feld[zeile][spalte]!=ng->nodata)
				ng->feld[zeile][spalte]=sqrt(erg/(count-1));
			mean=0.0;
			count=0;
			erg=0.0;
		}
	}
}

void  region::calc_min(grid* g1,grid* ng)
{
	int ax,ay,ex,ey,zeile,spalte;
	float val,zwsp,erg=FLT_MAX;
	for(zeile=0; zeile<g1->nrows; zeile++){
		ax=zeile-radius;
		if(ax<0) ax=0;
		ex=zeile+radius;
		if(ex>=g1->nrows) ex=g1->nrows-1;
		for(spalte=0; spalte<g1->ncols; spalte++){
			ay=spalte-radius;
			if(ay<0) ay=0;
			ey=spalte+radius;
			if(ey>=g1->ncols)ey=g1->ncols-1;
			for(int i=ax; i<=ex; i++){
				for(int j=ay; j<=ey; j++){
					val = g1->feld[i][j];
					if(int(val)!=g1->nodata){
						zwsp = (maske[i-zeile+radius][j-spalte+radius]>0)
			    		? val : FLT_MAX;
						erg = (erg < zwsp) ? erg : zwsp;
					}
				}
			}
			if(ng->feld[zeile][spalte]!=ng->nodata)
				ng->feld[zeile][spalte]=erg;
			erg=FLT_MAX;
		}
	}
}

void region::calc_max(grid* g1, grid* ng)
{
	int ax,ay,ex,ey,zeile,spalte;
	float val,zwsp,erg=-FLT_MAX;
	for(zeile=0; zeile<g1->nrows; zeile++){
		ax=zeile-radius;
		if(ax<0) ax=0;
		ex=zeile+radius;
		if(ex>=g1->nrows) ex=g1->nrows-1;
		for(spalte=0; spalte<g1->ncols; spalte++){
			ay=spalte-radius;
			if(ay<0) ay=0;
			ey=spalte+radius;
			if(ey>=g1->ncols)ey=g1->ncols-1;
			for(int i=ax; i<=ex; i++){
				for(int j=ay; j<=ey; j++){
					val = g1->feld[i][j];
					if(int(val)!=g1->nodata){
						zwsp = (maske[i-zeile+radius][j-spalte+radius]>0)
			    		? val : 0;
						erg = (erg > zwsp) ? erg : zwsp;
					}
				}
			}
			if(ng->feld[zeile][spalte]!=ng->nodata)
				ng->feld[zeile][spalte]=erg;
			erg=-FLT_MAX;
		}
	}
}

// Interpolations 21.10.2002

grid* grid::shepard(int c, int r, int R, float mu)
{
	// init
	float hq=sqrt((double)nrows*ncols)/2;
	stat();
	if(c>ncols*ncols || r>nrows*nrows){
		cerr << "error (shepard): faktor too big: "
		<< c/ncols << " or " << r/nrows
		<< " must be smaller as: " << sqrt((double)ncols*ncols)
		<< " or " << sqrt((double)nrows*nrows) << endl;
		return 0;
	}
	if(R<0 || R>hq || R>=ncols || R>=nrows){
		cerr << "error (shepard): R=" << R << " too big\n";
		return 0;
	}
	if(mu<2 || mu>6){
		cerr << "error (shepard): mu=" << mu << " must be between 2..6\n";
		return 0;
	}
	cerr << "shepard: " << gridmean << endl;
	grid* gx = new grid((int)csize);
	gx->ncols = c;
	gx->nrows = r;
	gx->xcorner = xcorner;
	gx->ycorner = ycorner;
	gx->csize = csize*sqrt((double((nrows*ncols))/(c*r)));
	gx->nodata = nodata;
	gx->feld = (float**) new float*[gx->nrows];
	for(int i=0; i<gx->nrows; i++){
		if((gx->feld[i]=new float[gx->ncols])==NULL){
			cerr << "error (grid::shepard): no sufficient memory" << endl;
		}
	}
	float wxy, wz, dist, radius, phi;//, sci;
	int lx,ly;
	float dly,dlx;
	int fx,fy;
	fx=c/ncols;
	fy=r/nrows;
	radius=sqrt((double)(R+1)*(R+1)+(R+1)*(R+1));
	cerr << " radius: " << radius
	<< " R: " << R << " mu: " << mu << endl;
	for(int i=0; i<gx->nrows; i++){
		for(int j=0; j<gx->ncols; j++){
			wxy=wz=0.0;
			lx=(int)i/fy;
			ly=(int)j/fx;
			dlx=(float)(i%fy)/fy;
			dly=(float)(j%fx)/fx;
			// cerr << "dlx " << dlx << " dly: " << dly << endl;
			for(int k=-R; k<=R; k++){
				for(int l=-R; l<=R; l++){
					dist=sqrt((k-dlx)*(k-dlx)+(l-dly)*(l-dly));
					if(dist<radius)phi=1.0-dist/radius;
					if(lx+k>=0 && lx+k<nrows && ly+l>=0
							&& ly+l<ncols && phi>0){
						if(int(feld[lx+k][ly+l])!=nodata){
							wz+=pow(phi,mu)*feld[lx+k][ly+l];
							wxy+=pow(phi,mu);
						}
					}
				}
			}
			if(wz>0 && wxy>0)
				gx->feld[i][j]=wz/wxy;
			else
				gx->feld[i][j]=nodata;
		}
	}
	return gx;
}

line::line()
{
}

line::~line()
{
	x.clear();
	y.clear();
	z.clear();
	flxname.clear();
	id.clear();
}

int line::read_line(char *name)
{
	FILE *fp;
	int length, fluss;
	double a,b,c;

	if((fp=fopen(name,"r"))==0){
		fprintf(stderr,"line error: could not open input file:%s\n",name);
		return -1;
	}
	while(fscanf(fp,"%d %d\n",&fluss,&length)!=EOF){
		for(int i=0; i<length;i++){
			if(fscanf(fp,"%lf %lf %lf\n",&a,&b,&c)==EOF){
				fprintf(stderr,"line error: unexpected EOF while read\n");
				fclose(fp);
				return(-2);
			}
			flxname.push_back(fluss); // same fluss for all segments
			id.push_back(i);          // individual id in segment
			x.push_back(a);           // values
			y.push_back(b);
			z.push_back(c);
		}
	}
	// test
	for(unsigned int i=0; i<id.size(); i++){
		fprintf(stderr,"%d %d %lf %lf %lf\n",
		        id[i],flxname[i],x[i],y[i],z[i]);
	}
	fclose(fp);
	return 0;
}

grid* line::l2g(grid *gx)
{
	int i,j;//,k,l;
	grid* nx=gx->grid_copy();
	// reset grid
	for(i=0; i<gx->nrows; i++)
		for(j=0; j<gx->ncols; j++)
			nx->feld[i][j]=nx->nodata;
	double x0,x1,y0,y1;
	int lx,ly;//,bins;
	int anfx, anfy, endx, endy;
	for(unsigned int l=0; l<flxname.size(); l++)
		for(unsigned int k=0; k<id.size(); k++){
			x0=x[k];
			y0=y[k];
			x1=x[k+1];
			y1=y[k+1];
			if(x0<gx->xcorner || x0>gx->xcorner+gx->csize*gx->ncols ||
					x1<gx->xcorner || x1>gx->xcorner+gx->csize*gx->ncols ||
					y0<gx->ycorner || y0>gx->ycorner+gx->csize*gx->nrows ||
					y1<gx->ycorner || y1>gx->ycorner+gx->csize*gx->nrows)
				continue; // out of range
				else{
					anfx=(int)((x0-gx->xcorner)/gx->csize);
					anfy=gx->nrows-1-(int)((y0-gx->ycorner)/gx->csize);
					endx=(int)((x1-gx->xcorner)/gx->csize);
					endy=gx->nrows-1-(int)((y1-gx->ycorner)/gx->csize);
					lx=endx-anfx;
					ly=endy-anfy;
					fprintf(stderr,"%d: %d %d %d %d\n",
					        k,anfx,anfy,endx,endy);
					if(lx==0){  // parallel to y-axes
						if(anfy<endy){
							for(i=anfy,j=0; i!=endy; i++,j++){
								nx->feld[i][anfx]=z[k];
							}
						}
						else{
							for(i=anfy,j=0; i!=endy; i--,j++){
								nx->feld[i][anfx]=z[k];
							}
						}
					}
					if(ly==0){ // parallel to y-axes
						if(anfx<endx){
							for(i=anfx,j=0; i!=endx; i++,j++){
								nx->feld[anfy][i]=z[k];
							}
						}
						else{
							for(i=anfx,j=0; i!=endx; i--,j++){
								nx->feld[anfy][i]=z[k];
							}
						}
					}
					int d1;
					if(lx!=0 && ly!=0){
						double m;
						m=(double)ly/lx;
						fprintf(stderr,"m=%f\n",m);
						if(fabs(m)<=1){ // x quantization
							if(anfx<endx && anfy<endy){ // m>0
								for(i=anfx,j=0;i!=endx;i++,j++){
									d1=anfy+(int)(m*j);
									nx->feld[d1][i]=z[k];
								}
							}
							if(anfx<endx && anfy>endy){ // m<0
								for(i=anfx,j=0;i!=endx;i++,j++){
									d1=anfy+(int)(m*j);
									nx->feld[d1][i]=z[k];
								}
							}
							if(anfx>endx && anfy<endy){ // m<0
								m*=-1;
								for(i=anfx,j=0;i!=endx;i--,j++){
									d1=anfy+(int)(m*j);
									nx->feld[d1][i]=z[k];
								}
							}
							if(anfx>endx && anfy>endy){ // m>0
								m*=-1;
								for(i=anfx,j=0;i!=endx;i--,j++){
									d1=anfy+(int)(m*j);
									nx->feld[d1][i]=z[k];
								}
							}
						}
						else{  // y quatization
							m=1/m;
							fprintf(stderr,"m=%f\n",m);
							if(anfy<endy && anfx<endx){ // m>0
								for(i=anfy, j=0; i!=endy; i++,j++){
									d1=anfx+(int)(m*j);
									nx->feld[i][d1]=z[k];
								}
							}
							if(anfy<endy && anfx>endx){ // m<0
								for(i=anfy, j=0; i!=endy; i++,j++){
									d1=anfx+(int)(m*j);
									nx->feld[i][d1]=z[k];
								}
							}
							if(anfy>endy && anfx<endx){ // m<0
								m*=-1;
								for(i=anfy, j=0; i!=endy; i--,j++){
									d1=anfx+(int)(m*j);
									nx->feld[i][d1]=z[k];
								}
							}
							if(anfy>endy && anfx>endx){ // m>0
								m*=-1;
								for(i=anfy, j=0; i!=endy; i--,j++){
									d1=anfx+(int)(m*j);
									nx->feld[i][d1]=z[k];
								}
							}
						}
					}
				}
		}
	return nx;
}

// kadane algorithm selects the brightes region of a grid

void grid::grid_kadane(float diff)
{
	float *pr=new float[ncols];
	int k,l,x1,x2,y1,y2,j,i;
	float s,ss,t;
	ss=-FLT_MAX;
	for(i=0; i<nrows; i++)
		for(j=0; j<ncols; j++)
			feld[i][j]-=diff;
	x1=x2=y1=y2=0;
	for(int z=0; z<nrows;z++){
		for(i=0; i<ncols; i++) pr[i]=0.0;
		for(int x=z; x<nrows; x++){
			t=0.0;
			s=-FLT_MAX;
			j=k=l=0;
			for(i=0; i<ncols; i++){
				pr[i]+=feld[x][i];
				t+=pr[i];
				if(t>s){
					s=t;
					k=i;
					l=j;
				}
				if(t<0){
					t=0.0;
					j=i+1;
				}
			}
			if(s>ss){
				ss=s;
				x1=x;
				y1=k;
				x2=z;
				y2=l;
			}
		}
	}
	cerr << "x1: " << x1 << " y1: " << y1 << " x2: "
	<< x2 << " y2: " << y2 << endl;
	// output region
	for(i=0; i<nrows;i++)
		for(j=0; j<ncols; j++)
			if(!(i>=x2 && i<x1 && j>=y2 && j<y1))feld[i][j]=nodata;
			else feld[i][j]+=diff;
}

/*
// new tree algorithm
tree::tree(grid* gx)
{
	// fill the map
	int k=0;
	for(int i=0; i<gx->nrows; i++){
		for(int j=0; j<gx->ncols; j++){
			// cerr << (int)gx->feld[i][j] << " ";
			if((int)gx->feld[i][j]!=gx->nodata
					&& code.find((int)gx->feld[i][j])==code.end()){
				code.insert(ValuePair((int)gx->feld[i][j],k));
				decode.insert(ValuePair(k++,(int)gx->feld[i][j]));
			}
		}
	}
	// print code
	CodeType::iterator iter;
	for(iter=code.begin(); iter!=code.end(); iter++)
		cout << (*iter).first << " : " << (*iter).second << endl;
	// build the adjacent matrix
	cerr << "max number+1=" << k << endl;
	lmatrix=(long**) new long*[k];
	for(int i=0; i<k; i++){
		lmatrix[i]=new long[k];
		if(lmatrix[i]==0){
			cerr << "(error in tree): no space left on device\n";
			exit(1);
		}
	}
	n=k;
	// fill it
	for(int i=0; i<n; i++)
		for(int j=0; j<n; j++)
			lmatrix[i][j]=0;
	g1=gx;
	for(int i=0; i<gx->nrows; i++){
		for(int j=0; j<gx->ncols; j++){
			if((int)gx->feld[i][j]!=gx->nodata){
				if(j+1<gx->ncols && (int)gx->feld[i][j+1]!=gx->nodata &&
						gx->feld[i][j+1]!=gx->feld[i][j]){
					lmatrix[min(code[(int)gx->feld[i][j+1]],
					            code[(int)gx->feld[i][j]])][
					                                        max(code[(int)gx->feld[i][j+1]],
					                                            code[(int)gx->feld[i][j]])]++;
				}
				if(j-1>=0 && (int)gx->feld[i][j-1]!=gx->nodata
						&& gx->feld[i][j-1]!=gx->feld[i][j]){
					lmatrix[min(code[(int)gx->feld[i][j-1]],
					            code[(int)gx->feld[i][j]])][
					                                        max(code[(int)gx->feld[i][j-1]],
					                                            code[(int)gx->feld[i][j]])]++;
				}
				if(i+1<gx->nrows && (int)gx->feld[i+1][j]!=gx->nodata
						&& gx->feld[i+1][j]!=gx->feld[i][j]){
					lmatrix[min(code[(int)gx->feld[i+1][j]],
					            code[(int)gx->feld[i][j]])][
					                                        max(code[(int)gx->feld[i+1][j]],
					                                            code[(int)gx->feld[i][j]])]++;
				}
				if(i-1>=0 && (int)gx->feld[i-1][j]!=gx->nodata
						&& gx->feld[i-1][j]!=gx->feld[i][j]){
					lmatrix[min(code[(int)gx->feld[i-1][j]],
					            code[(int)gx->feld[i][j]])][
					                                        max(code[(int)gx->feld[i-1][j]],
					                                            code[(int)gx->feld[i][j]])]++;
				}
				if(j+1<gx->ncols && i+1<gx->nrows
						&& (int)gx->feld[i+1][j+1]!=gx->nodata
						&& gx->feld[i+1][j+1]!=gx->feld[i][j]){
					lmatrix[min(code[(int)gx->feld[i+1][j+1]],
					            code[(int)gx->feld[i][j]])][
					                                        max(code[(int)gx->feld[i+1][j+1]],
					                                            code[(int)gx->feld[i][j]])]++;
				}
				if(j-1>=0 && i+1<gx->nrows
						&& (int)gx->feld[i+1][j-1]!=gx->nodata
						&& gx->feld[i+1][j-1]!=gx->feld[i][j]){
					lmatrix[min(code[(int)gx->feld[i+1][j-1]],
					            code[(int)gx->feld[i][j]])][
					                                        max(code[(int)gx->feld[i+1][j-1]],
					                                            code[(int)gx->feld[i][j]])]++;
				}
				if(j+1<gx->ncols && i-1>=0
						&& (int)gx->feld[i-1][j+1]!=gx->nodata
						&& gx->feld[i-1][j+1]!=gx->feld[i][j]){
					lmatrix[min(code[(int)gx->feld[i-1][j+1]],
					            code[(int)gx->feld[i][j]])][
					                                        max(code[(int)gx->feld[i-1][j+1]],
					                                            code[(int)gx->feld[i][j]])]++;
				}
				if(j-1>=0 && i-1>=0
						&& (int)gx->feld[i-1][j-1]!=gx->nodata
						&& gx->feld[i-1][j-1]!=gx->feld[i][j]){
					lmatrix[min(code[(int)gx->feld[i-1][j-1]],
					            code[(int)gx->feld[i][j]])][
					                                        max(code[(int)gx->feld[i-1][j-1]],
					                                            code[(int)gx->feld[i][j]])]++;
				}

			}
		}
	}
	// calc the sum of all
	total = 0;
	for(int i=0; i<n; i++)
		for(int j=0; j<n; j++)
			total+=lmatrix[i][j];
	assoc.resize(n);
	for(int i=0; i<n; i++)assoc[i]=-1;
	cerr << "total=" << total << endl;
}

tree::~tree()
{
	if(lmatrix!=0){
		for(int i=0; i<n; i++)
			delete [] lmatrix[i];
		delete [] lmatrix;
	}
}


void tree::print_tree()
{
	for(int i=0; i<n; i++){
		cout << i << ": ";
		for(int j=0; j<n; j++){
			cout << lmatrix[i][j] << " ";
		}
		cout << endl;
	}
}

void tree::print_assoc()
{
	for(int i=0; i<n; i++)
		if(assoc[i]<0)
			cout << decode[i] << " : " << decode[i] << endl;
		else
			cout << decode[i] << " : " << decode[assoc[i]] << endl;
}

void tree::deep_search(int global, int key,double val)
{
	for(int j=key+1; j<n; j++){
		if((double)lmatrix[key][j]/(double(total))>val){
			assoc[j]=global;
			lmatrix[key][j]=0;
			deep_search(global,j,val);
		}
	}
}

void tree::thresh(double val)
{
	//int key;
	// the cluster is used to address the associations
	for(int i=0; i<n; i++){
		deep_search(i,i,val);
		// print_tree();
	}
}

grid* tree::map_tree()
{
	grid* out=g1->grid_copy();
	if(out==NULL) return(NULL);
	for(int i=0; i<g1->nrows; i++){
		for(int j=0; j<g1->ncols; j++){
			if((int)g1->feld[i][j]!=g1->nodata){
				if(assoc[code[(int)g1->feld[i][j]]]!=-1)
					out->feld[i][j]=decode[assoc[code[(int)g1->feld[i][j]]]];
			}
		}
	}
	return(out);
}
*/

// water::water(grid* g)
// {
//     g1=g;
//     ncols = g1->ncols;
//     nrows = g1->nrows;
//     dir = new (int*)[nrows];
//     if(dir==NULL){
// 	cerr << " no space on device" << endl;
// 	exit(2);
//     }
//     tag = new (int*)[nrows];
//     if(tag==NULL){
// 	cerr << "error (water): no space on device" << endl;
// 	exit(2);
//     }
//     lock = new (int*)[nrows];
//     if(lock==NULL){
// 	cerr << "error (water): no space on device" << endl;
// 	exit(2);
//     }
//     hw = new (float*)[nrows];
//     if(hw==NULL){
// 	cerr << "error (water): no space on device" << endl;
// 	exit(2);
//     }
//     for(int i=0; i<nrows; i++){
// 	dir[i]=new int[ncols];
// 	tag[i]=new int[ncols];
// 	lock[i]=new int[ncols];
// 	hw[i]=new float[ncols];
// 	if(lock[i]==NULL || dir[i]==NULL || tag[i]==NULL
// 	   || hw[i]==NULL){
// 	cerr << "error (water): no space on device" << endl;
// 	exit(2);
// 	}
//     }
//     #ifdef FLUX
//     flux1 = new (struct flux *)[nrows];
//     for(int i=0; i<nrows; i++){
// 	flux1[i] = new (struct flux)[ncols];
// 	if(flux1[i]==NULL){
// 	cerr << "error (water): no space on device" << endl;
// 	exit(2);
// 	}
//     }
//     for(int i=0; i<nrows; i++){
// 	for(int j=0; j<ncols; j++){
// 	    flux1[i][j].ol=flux1[i][j].o=flux1[i][j].or=
// 		flux1[i][j].l=flux1[i][j].r=
// 		flux1[i][j].ul=flux1[i][j].u=flux1[i][j].ur=0.0;
// 	}
//     }
//     #endif
//     for(int i=0; i<nrows; i++){
// 	for(int j=0; j<ncols; j++){
// 	    dir[i][j]=ND;
// 	    lock[i][j]=0;
// 	    tag[i][j]=0;
// 	    hw[i][j]=0.0;
// 	}
//     }
// }

// water::~water()
// {
//     if(dir!=(int**)NULL){
// 	for(int i=0; i<nrows; i++){
// 	    delete [] dir[i];
// 	}
//     }
//     if(tag!=(int**)NULL){
// 	for(int i=0; i<nrows; i++){
// 	    delete [] tag[i];
// 	}
//     }
//     if(lock!=(int**)NULL){
// 	for(int i=0; i<nrows; i++){
// 	    delete [] lock[i];
// 	}
//     }
//     if(hw!=(float**)NULL){
// 	for(int i=0; i<nrows; i++){
// 	    delete [] hw[i];
// 	}
//     }
//     #ifdef FLUX
//     if(flux1!=(struct flux **)NULL){
// 	for(int i=0; i<nrows; i++)
// 	    delete [] flux1[i];
//     }
//     #endif
// }

// void water::calc_dir(long count)
// {
//     time_t now = time(NULL);
//     srand(now);
//     int flag=1;
//     float sum;
//     int x,y;
//     int count1=0;
//     int layer;
//     float val;
//     float random;
//     float grad[DIRS];     // gradient
//     for(long i=0; i<count; i++){
// 	x=rand()%ncols;   // choose on point
// 	y=rand()%nrows;
// 	//
// 	flag=1;
// 	while(flag==1){
// 	    tag[y][x]+=1;     // add the rain drop
// 	    layer=0;
// 	    sum=0;
// 	    for(int j=0; j<DIRS; j++){
// 		grad[j]=0.0;
// 	    }
// 	    // calculate the gradient
// 	    if(dir[y][x]==ND){
// 		if((x-1>=0) && g1->feld[y][x-1]<g1->feld[y][x]-DELTA){
// 		    grad[0]=g1->feld[y][x]-g1->feld[y][x-1];
// 		    sum+=grad[0];
// 		    layer=1;
// 		}
// 		if((x+1<ncols) && g1->feld[y][x+1]<g1->feld[y][x]-DELTA){
// 		    grad[1]=g1->feld[y][x]-g1->feld[y][x+1];
// 		    sum+=grad[1];
// 		    layer=1;
// 		}
// 		if((x-1>=0) && (y-1>=0)
// 		   && g1->feld[y-1][x-1]<g1->feld[y][x]-DELTA){
// 		    grad[2]=(g1->feld[y][x]-g1->feld[y-1][x-1])/1.41;
// 		    sum+=grad[2];
// 		    layer=1;
// 		}
// 		if((x+1<ncols) && (y-1>=0)
// 		   && g1->feld[y-1][x+1]<g1->feld[y][x]-DELTA){
// 		    grad[3]=(g1->feld[y][x]-g1->feld[y-1][x+1])/1.41;
// 		    sum+=grad[3];
// 		    layer=1;
// 		}
// 		if((x-1>=0) && (y+1<nrows)
// 		   && g1->feld[y+1][x-1]<g1->feld[y][x]-DELTA){
// 		    grad[4]=(g1->feld[y][x]-g1->feld[y+1][x-1])/1.41;
// 		    sum+=grad[4];
// 		    layer=1;
// 		}
// 		if((x+1<ncols) && (y+1<nrows)
// 		   && g1->feld[y+1][x+1]<g1->feld[y][x]-DELTA){
// 		    grad[5]=(g1->feld[y][x]-g1->feld[y+1][x+1])/1.41;
// 		    sum+=grad[5];
// 		    layer=1;
// 		}
// 		if((y-1>=0) && g1->feld[y-1][x]<g1->feld[y][x]-DELTA){
// 		    grad[6]=g1->feld[y][x]-g1->feld[y-1][x];
// 		    sum+=grad[6];
// 		    layer=1;
// 		}
// 		if((y+1<nrows) && g1->feld[y+1][x]<g1->feld[y][x]-DELTA){
// 		    grad[7]=g1->feld[y][x]-g1->feld[y+1][x];
// 		    sum+=grad[7];
// 		    layer=1;
// 		}
// 		if(layer==0){flag=0; continue;}
// 		// monte carlo simulation
// 		random=sum*(float)rand()/RAND_MAX;
// 		val=grad[0];
// 		if(random<val){
// 		    if((x-1>=0) && dir[y][x-1]!=rr) dir[y][x]=ll;
// 		    if(x-1<0) dir[y][x]=ll;
// 		    flag=0;
// 		    continue;
// 		}
// 		val+=grad[1];
// 		if(random<val){
// 		    if((x+1)<ncols && dir[y][x+1]!=ll)dir[y][x]=rr;
// 		    if((x+1)==ncols) dir[y][x]=rr;
// 		    flag=0;
// 		    continue;
// 		}
// 		val+=grad[2];
// 		if(random<val){
// 		    if((x-1)>=0 && (y-1)>=0 && dir[y-1][x-1]!=Lr)dir[y][x]=Ul;
// 		    if((x-1)<0 || (y-1)<0) dir[y][x]=Ul;
// 		    flag=0;
// 		    continue;
// 		}
// 		val+=grad[3];
// 		if(random<val){
// 		    if((x+1)<ncols && (y-1)>=0 && dir[y-1][x+1]!=Ll)
// 			dir[y][x]=Ur;
// 		    if((x+1)==ncols || (y-1)<0) dir[y][x]=Ur;
// 		    flag=0;
// 		    continue;
// 		}
// 		val+=grad[4];
// 		if(random<val){
// 		    if((x-1)>=0 && (y+1)<nrows && dir[y+1][x-1]!=Ur)
// 			dir[y][x]=Ll;
// 		    if((x-1)<0 || (y+1)==ncols) dir[y][x]=Ll;
// 		    flag=0;
// 		    continue;
// 		}
// 		val+=grad[5];
// 		if(random<val){
// 		    if((x+1)<ncols && (y+1)<nrows && dir[y+1][x+1]!=Ul)
// 			dir[y][x]=Lr;
// 		    if((x+1)==ncols || (y+1)==nrows)dir[y][x]=Lr;
// 		    flag=0;
// 		    continue;
// 		}
// 		val+=grad[6];
// 		if(random<val){
// 		    if((y-1)>=0 && dir[y-1][x]!=LL)
// 			dir[y][x]=UU;
// 		    if((y-1)<0) dir[y][x]=UU;
// 		    flag=0;
// 		    continue;
// 		}
// 		val+=grad[7];
// 		if(random<=val){
// 		    if((y+1)<ncols && dir[y+1][x]!=UU)
// 			dir[y][x]=LL;
// 		    if((y+1)==ncols) dir[y][x]=LL;
// 		    flag=0;
// 		    continue;
// 		}
// 		flag=0;
// 	    }
// 	    else
// 	    { // there was an ond direction from x y
// 		flag=1;
// 		if(dir[y][x]==ll){
// 		    x--;
// 		}
// 		if(dir[y][x]==rr){
// 		    x++;
// 		}
// 		if(dir[y][x]==UU){
// 		    y--;
// 		}
// 		if(dir[y][x]==LL){
// 		    y++;
// 		}
// 		if(dir[y][x]==Ul){
// 		    y--;
// 		    x--;
// 		}
// 		if(dir[y][x]==Ur){
// 		    y--;
// 		    x++;
// 		}
// 		if(dir[y][x]==Ll){
// 		    y++;
// 		    x--;
// 		}
// 		if(dir[y][x]==Lr){
// 		    y++;
// 		    x++;
// 		}
// 		if(x<=0 || y<=0 || x>=ncols || y>=nrows) flag=0;
// 	    }
// 	}
//     }
//     for(int i=0; i<nrows; i++){
// 	for(int j=0; j<ncols; j++){
// 	    if(dir[i][j]==ND) lock[i][j]=1;
// 	}
//     }
// }

// // Die Flux-Berechnung basiert auf den mittels calc_dir berechneten
// // Stromlinien.


// void water::calc_fluxs(long count)
// {
//     time_t now = time(NULL);
//     srand(now);
//     int flag=1;
//     float sum,val;
//     int flagl;
//     int x,y,x1,y1;
//     long int counter=0;
//     int layer;
//     float w=0.0;  // kin. energy
//     for(int i=0; i<nrows; i++)
// 	for(int j=0; j<ncols; j++)
// 	    tag[i][j]=0;
//     cerr << "calc_fluxs: " << count << endl;
//     for(long i=0; i<count; i++){
// 	x=rand()%ncols;   // choose on point
// 	y=rand()%nrows;
// 	flag=1;
// 	w=0.0;
// 	counter++;
// 	tag[y][x]+=1;     // add the rain drop
// 	while(flag==1){
// 	    if(dir[y][x]==ND){hw[y][x]+=ALPHA; flag=0; continue;}
// 	    if(dir[y][x]==Ul && x-1>=0 && y-1>=0){
// 		x1=x-1;
// 		y1=y-1;
// 		val=(g1->feld[y][x])-(g1->feld[y-1][x-1]);
// 		w+=sqrt(val)-BETA*w;
// 		if(w<0)w=0.0;
// #ifdef FLUX
// 		flux1[y][x].ol+=w/1.41;
// #endif
// 		flagl++;
// 	    }
// 	    if(dir[y][x]==Ur && x+1<ncols && y-1>=0){
// 		x1=x+1;
// 		y1=y-1;
// 		val=(g1->feld[y][x])-(g1->feld[y-1][x+1]);
// 		w+=sqrt(val)-BETA*w;
// 		if(w<0)w=0.0;
// #ifdef FLUX
// 		flux1[y][x].or+=w/1.41;
// #endif
// 		flagl++;
// 	    }
// 	    if(dir[y][x]==Ll && x-1>=0 && y+1<nrows){
// 		x1=x-1;
// 		y1=y+1;
// 		val=(g1->feld[y][x])-(g1->feld[y+1][x-1]);
// 		w+=sqrt(val)-BETA*w;
// 		if(w<0)w=0.0;
// #ifdef FLUX
// 		flux1[y][x].ul+=w/1.41;
// #endif
// 		flagl++;
// 	    }
// 	    if(dir[y][x]==Lr && x+1<ncols && y+1<nrows){
// 		x1=x+1;
// 		y1=y+1;
// 		val=(g1->feld[y][x])-(g1->feld[y+1][x+1]);
// 		w+=sqrt(val)-BETA*w;
// 		if(w<0)w=0.0;
// #ifdef FLUX
// 		flux1[y][x].ur+=w/1.41;
// #endif
// 		flagl++;
// 	    }
// 	    if(dir[y][x]==UU && y-1>=0){
// 		y1=y-1;
// 		x1=x;
// 		val=(g1->feld[y][x])-(g1->feld[y-1][x]);
// 		w+=sqrt(val)-BETA*w;
// 		if(w<0)w=0.0;
// #ifdef FLUX
// 		flux1[y][x].o+=w;
// #endif
// 		flagl++;
// 	    }
// 	    if(dir[y][x]==LL && y+1<nrows){
// 		y1=y+1;
// 		x1=x;
// 		val=(g1->feld[y][x])-(g1->feld[y+1][x]);
// 		w+=sqrt(val)-BETA*w;
// 		if(w<0)w=0.0;
// #ifdef FLUX
// 		flux1[y][x].u+=w;
// #endif
// 		flagl++;
// 	    }
// 	    if(dir[y][x]==ll && x-1>=0){
// 		x1=x-1;
// 		y1=y;
// 		val=(g1->feld[y][x])-(g1->feld[y][x-1]);
// 		w+=sqrt(val)-BETA*w;
// 		if(w<0)w=0.0;
// #ifdef FLUX
// 		flux1[y][x].l+=w;
// #endif
// 		flagl++;
// 	    }
// 	    if(dir[y][x]==rr && x+1<ncols){
// 		x1=x+1;
// 		y1=y;
// 		val=(g1->feld[y][x])-(g1->feld[y][x+1]);
// 		w+=sqrt(val)-BETA*w;
// 		if(w<0)w=0.0;
// #ifdef FLUX
// 		flux1[y][x].r+=w;
// #endif
// 		flagl++;
// 	    }
// 	    if(flagl>0){
// 		x=x1;
// 		y=y1;
// 	    }
// 	    else{
// 		flag=0;
// 		continue;
// 	    }
// 	    if(x>0 && y>0 && x<ncols && y<nrows){
// 		flag=1;
// 		continue;
// 	    }
// 	    else{
// 		flag=0;
// 		continue;
// 	    }
// 	} // while(flag==1)
//     } // for(...
//     cerr << "flux counter: " << counter << endl;
// }

// grid* water::get_dir()
// {
//     grid* ng=g1->grid_copy();
//     for(int i=0; i<nrows; i++)
// 	for(int j=0; j<ncols; j++)
// 	    ng->feld[i][j]=(float)dir[i][j];
//     return ng;
// }

// grid* water::get_lock()
// {
//     grid* ng=g1->grid_copy();
//     for(int i=0; i<nrows; i++)
// 	for(int j=0; j<ncols; j++)
// 	    ng->feld[i][j]=(float)lock[i][j];
//     return ng;
// }

// grid* water::get_tag()
// {
//     grid* ng=g1->grid_copy();
//     for(int i=0; i<nrows; i++)
// 	for(int j=0; j<ncols; j++)
// 	    ng->feld[i][j]=(float)tag[i][j];
//     return ng;
// }

// void water::read_tag(grid& g)
// {
//     if(g.ncols!=ncols || g.nrows!=nrows){
// 	cerr << "read_tag: dimension must be the same!" << endl;
// 	exit(2);
//     }
//     for(int i=0; i<nrows; i++)
// 	for(int j=0; j<ncols; i++)
// 	    tag[i][j]=(int)g.feld[i][j];
// }

// void water::read_dir(grid& g)
// {
//     if(g.ncols!=ncols || g.nrows!=nrows){
// 	cerr << "read_tag: dimension must be the same!" << endl;
// 	exit(2);
//     }
//     for(int i=0; i<nrows; i++)
// 	for(int j=0; j<ncols; i++)
// 	    dir[i][j]=(int)g.feld[i][j];
// }

// void water::read_lock(grid& g)
// {
//     if(g.ncols!=ncols || g.nrows!=nrows){
// 	cerr << "read_tag: dimension must be the same!" << endl;
// 	exit(2);
//     }
//     for(int i=0; i<nrows; i++)
// 	for(int j=0; j<ncols; i++)
// 	    lock[i][j]=(int)g.feld[i][j];
// }
