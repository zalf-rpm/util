#include <string>
#include <vector>

#include <boost/foreach.hpp>

#include "grid/grid.h"

using namespace std;

int main(int argc, char **argv){
	if(argc == 2){
		BOOST_FOREACH(const string& dsn, hdf5::allDatasetNames(argv[1])){
			cout << dsn << endl;
		}
	} else {
		cout << "usage: list-hdf HDF5-filename" << endl;
	}

	return 0;
}
