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

#ifndef COMMON_TYPEDEFS_H
#define COMMON_TYPEDEFS_H

#include <string>

//! numeric code for standorttyp
typedef int STTCode;

//! alphanumeric code for standorttyp
typedef std::string STT;

typedef int Year;

//------------------------------------------------------------------------------
/**
@class tools/types.h

define type shortcuts
*/
typedef unsigned long  ulong;       // 32 bit on ia32, llp64, 64 bit on lp64
typedef unsigned int   uint;        // 32 bit
typedef unsigned short ushort;      // 16 bit
typedef unsigned char  uchar;
typedef unsigned char  ubyte;

#endif // TYPEDEFS_H
