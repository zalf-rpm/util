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

#include "platform.h"
#include "grid.h"

using namespace std;
using namespace Grids;

hdf5::hdf5()
{
	size=0;
	i1 = (int*)NULL;
	f1 = (float*)NULL;
}

hdf5::~hdf5()
{
	herr_t ret;
	if(f1!=NULL) free(f1);
	if(i1!=NULL) free(i1);
	ret = H5Dclose(dataset);
	ret = H5Fclose(file);
}

int hdf5::open_f(const char* name)
{
	H5Eset_auto2(H5E_DEFAULT,NULL,NULL);
	file = H5Fopen(name, H5F_ACC_RDWR, H5P_DEFAULT);
	if(file<0){
		return 1; // error: could not open
	}
	return 0;
}

int hdf5::closeFile()
{
	if(H5Fclose(file) <0)
		return 1; // error: could not open
	return 0;
}

int hdf5::create_f(const char* name)
{
	file = H5Fcreate(name, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
	if(file==0){//(hid_t)NULL){
		fprintf(stderr,"can not open hdf_filesystem: %s\n",name);
		exit(2);
	}
	return 0;
}

int hdf5::open_d(const char* name)
{
	H5Eset_auto(H5E_DEFAULT,NULL,NULL);
	if((dataset = H5Dopen(file,name,H5P_DEFAULT))<0){
		return 1; // error: could not open dataset
	}
	return 0;
}

herr_t h5gIterator(hid_t /*group*/, const char* name, void* op_data){
	list<string>* dsns = static_cast<list<string>*>(op_data);
	dsns->push_back(name);
	return 0;
}

list<string> hdf5::allDatasetNames(const char* fileName){
	list<string> dsns;

	hid_t fh = H5Fopen(fileName, H5F_ACC_RDONLY, H5P_DEFAULT);
	int idx = 0;
	//herr_t e =
	H5Giterate(fh, "/", &idx, &h5gIterator, (void*) & dsns);
	//cout << "e: " << e << endl;
	H5Fclose(fh);

	return dsns;
}

void hdf5::write_i_feld(const char* name, int* feld,int nx,int ny)
{
	//int *f1;
	hid_t dataspace, datatype;
	herr_t status;
	hsize_t dims[1];

	dims[0]=nx*ny;
	dataspace=H5Screate_simple(1,dims,NULL);
	datatype=H5Tcopy(H5T_NATIVE_INT);
	status=H5Tset_order(datatype,H5T_ORDER_LE);
	dataset=H5Dcreate(file,name,datatype,dataspace,H5P_DEFAULT,H5P_DEFAULT,H5P_DEFAULT);
	status=H5Dwrite(dataset
	                ,H5T_NATIVE_INT,H5S_ALL,H5S_ALL
	                ,H5P_DEFAULT,feld);
	H5Sclose(dataspace);
	H5Tclose(datatype);
}

void hdf5::write_f_feld(const char* name, float* feld,int nx,int ny)
{
	//float *f1;
	hid_t dataspace, datatype;
	herr_t status;
	hsize_t dims[1];

	dims[0]=nx*ny;
	dataspace=H5Screate_simple(1,dims,NULL);
	datatype=H5Tcopy(H5T_NATIVE_FLOAT);
	status=H5Tset_order(datatype,H5T_ORDER_LE);
	dataset=H5Dcreate(file,name,datatype,dataspace,H5P_DEFAULT,H5P_DEFAULT,H5P_DEFAULT);
	status=H5Dwrite(dataset
	                ,H5T_NATIVE_FLOAT,H5S_ALL,H5S_ALL
	                ,H5P_DEFAULT,feld);
	H5Sclose(dataspace);
	H5Tclose(datatype);
}

int* hdf5::read_i_feld(const char* name)
{
	hid_t dataspace;//, datatype;
	int rank;
	herr_t status;
	hsize_t dims[1];
	herr_t  ret;

	if((dataset = H5Dopen(file, name,H5P_DEFAULT))<0){
		fprintf(stderr,"No Dataset : %s\n",name);
		exit(2);
	}
	dataspace = H5Dget_space(dataset);    /* dataspace handle */
	rank      = H5Sget_simple_extent_ndims(dataspace);
	status    = H5Sget_simple_extent_dims(dataspace, dims, NULL);

	i1=(int*) malloc(dims[0]*sizeof(int));
	if(i1==NULL){
		fprintf(stderr,"not space on device\n");
		exit(2);
	}
	size=dims[0];
	ret = H5Dread(dataset, H5T_NATIVE_INT, H5S_ALL, H5S_ALL,
	              H5P_DEFAULT, i1);
	H5Sclose(dataspace);
	return i1;
}

float* hdf5::read_f_feld(const char* name)
{
	hid_t dataspace;//, datatype;
	int rank;
	herr_t status;
	hsize_t dims[1];
	herr_t  ret;

	if((dataset = H5Dopen(file, name,H5P_DEFAULT))<0){
		fprintf(stderr,"No Dataset : %s\n",name);
		exit(2);
	}
	dataspace = H5Dget_space(dataset);    /* dataspace handle */
	rank      = H5Sget_simple_extent_ndims(dataspace);
	status    = H5Sget_simple_extent_dims(dataspace, dims, NULL);
  //cerr << "read_dims: " << dims[0] << endl;
	f1=(float*) malloc(dims[0]*sizeof(float));
	if(f1==NULL){
		fprintf(stderr,"not space on device\n");
		exit(2);
	}
	size=dims[0];
	ret = H5Dread(dataset, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
	              H5P_DEFAULT, f1);
	H5Sclose(dataspace);
	return f1;
}

int hdf5::write_i_attribute(const char* name, int val)
{
	hid_t aid1,attr1;
	herr_t ret;

	H5Eset_auto(NULL,NULL,NULL);
	aid1 = H5Screate(H5S_SCALAR);
	if((attr1=H5Aopen_name(dataset,name))<0)
		attr1 = H5Acreate(dataset, name, H5T_NATIVE_INT, aid1,
											H5P_DEFAULT, H5P_DEFAULT);
	ret = H5Awrite(attr1, H5T_NATIVE_INT, (void*)&val);
	ret = H5Sclose(aid1);
	ret = H5Aclose(attr1);
	return 0;
}

int hdf5::write_l_attribute(const char* name, long val)
{
	hid_t aid1,attr1;
	herr_t ret;

	H5Eset_auto(H5E_DEFAULT,NULL,NULL);
	aid1 = H5Screate(H5S_SCALAR);
	if((attr1=H5Aopen_name(dataset,name))<0)
		attr1 = H5Acreate(dataset, name, H5T_NATIVE_LONG, aid1,
											H5P_DEFAULT,H5P_DEFAULT);
	ret = H5Awrite(attr1, H5T_NATIVE_LONG, (void*) &val);
	ret = H5Sclose(aid1);
	ret = H5Aclose(attr1);
	return 0;
}

int hdf5::write_f_attribute(const char* name, float val)
{
	hid_t aid1,attr1;
	herr_t ret;

	H5Eset_auto(H5E_DEFAULT,NULL,NULL);
	aid1 = H5Screate(H5S_SCALAR);
	if((attr1=H5Aopen_name(dataset,name))<0)
		attr1 = H5Acreate(dataset, name, H5T_NATIVE_FLOAT, aid1,
											H5P_DEFAULT,H5P_DEFAULT);
	ret = H5Awrite(attr1, H5T_NATIVE_FLOAT, (void*) &val);
	ret = H5Sclose(aid1);
	ret = H5Aclose(attr1);
	return 0;
}

int hdf5::write_d_attribute(const char* name, double val)
{
	hid_t aid1,attr1;
	herr_t ret;

	H5Eset_auto(H5E_DEFAULT,NULL,NULL);
	aid1 = H5Screate(H5S_SCALAR);
	if((attr1=H5Aopen_name(dataset,name))<0)
		attr1 = H5Acreate(dataset, name, H5T_NATIVE_DOUBLE, aid1,
											H5P_DEFAULT,H5P_DEFAULT);
	ret = H5Awrite(attr1, H5T_NATIVE_DOUBLE, (void*) &val);
	ret = H5Sclose(aid1);
	ret = H5Aclose(attr1);
	return 0;
}

int hdf5::write_s_attribute(const char* name, const char* val)
{
	hid_t aid1,attr1,atype;
	herr_t ret;

	H5Eset_auto(H5E_DEFAULT,NULL,NULL);
	aid1  = H5Screate(H5S_SCALAR);
	atype = H5Tcopy(H5T_C_S1);
	H5Tset_size(atype, 80);
	H5Tset_strpad(atype,H5T_STR_NULLTERM);
	if((attr1=H5Aopen_name(dataset,name))<0)
		attr1 = H5Acreate(dataset, name, atype, aid1, H5P_DEFAULT,H5P_DEFAULT);
	// Write string attribute.
	ret = H5Awrite(attr1, atype, (void*) val);
	ret = H5Tclose(atype);
	ret = H5Sclose(aid1);
	ret = H5Aclose(attr1);
	return 0;
}


char* hdf5::get_s_attribute(const char* name)
{
	hid_t attr,atype;//,aid1;
	herr_t ret;
	char *cp;

	cp = (char*)malloc(80);
	atype = H5Tcopy(H5T_C_S1);
	H5Tset_size(atype, 80);
	H5Tset_strpad(atype,H5T_STR_NULLTERM);
	attr=H5Aopen_name(dataset,name);
	ret = H5Aread(attr, atype, cp);
	ret = H5Tclose(atype);
	ret = H5Aclose(attr);
	return cp;
}

float hdf5::get_f_attribute(const char* name)
{
	hid_t attr, atype;
	herr_t ret;
	float x;

	attr  = H5Aopen_name(dataset, name);
	atype = H5Aget_type(attr);
	ret   = H5Aread(attr,atype,&x);
	ret   = H5Aclose(attr);
	ret   = H5Tclose(atype);
	return x;
}

double hdf5::get_d_attribute(const char* name)
{
	hid_t attr, atype;
	herr_t ret;
	double x;

	attr  = H5Aopen_name(dataset, name);
	atype = H5Aget_type(attr);
	ret   = H5Aread(attr,atype,&x);
	ret   = H5Aclose(attr);
	ret   = H5Tclose(atype);
	return x;
}

int hdf5::get_i_attribute(const char* name)
{
	hid_t attr, atype;
	herr_t ret;
	int x;

	attr  = H5Aopen_name(dataset, name);
	atype = H5Aget_type(attr);
	ret   = H5Aread(attr,atype,&x);
	ret   = H5Aclose(attr);
	ret   = H5Tclose(atype);
	return x;
}

long hdf5::get_l_attribute(const char* name)
{
	hid_t attr, atype;
	herr_t ret;
	long x;

	attr  = H5Aopen_name(dataset, name);
	atype = H5Aget_type(attr);
	ret   = H5Aread(attr,atype,&x);
	ret   = H5Aclose(attr);
	ret   = H5Tclose(atype);
	return x;
}
