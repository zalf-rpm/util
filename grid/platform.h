/**
File platform.cpp - platform specific code for SAMT

Created 9.3.2008, Maximilian Matthe

Authors:
Maximilian Matthe

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

/*
Definiert plattformspezifische Dinge.
*/

#ifndef PLATFORM_H__
#define PLATFORM_H__


// Replaces getOption in windows but uses option file in windows
const char* getOption(const char* optionname);

// replaces different commands in windows (e.g. rm -> del)
const char* getCommand(const char* commandname);

// return filename of the temporary hdf-file used to move data from samt to tools
const char* getOutTempHDF();
// return filename of the temporary hdf-file used to move data from tools to samt
const char* getInTempHDF();


#ifdef WIN32
typedef unsigned int uint;
#endif

#endif
