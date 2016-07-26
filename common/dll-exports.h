/* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/*
Authors: 
Michael Berg <michael.berg@zalf.de>
//taken from https://gcc.gnu.org/wiki/Visibility

Maintainers: 
Currently maintained by the authors.

This file is part of the util library used by models created at the Institute of
Landscape Systems Analysis at the ZALF.
Copyright (C) Leibniz Centre for Agricultural Landscape Research (ZALF)
*/

#ifndef DLL_EXPORTS_H
#define DLL_EXPORTS_H

// Generic helper definitions for shared library support
#if defined _WIN32 || defined __CYGWIN__
#define HELPER_DLL_IMPORT __declspec(dllimport)
#define HELPER_DLL_EXPORT __declspec(dllexport)
#define HELPER_DLL_LOCAL
#else
#if __GNUC__ >= 4
#define HELPER_DLL_IMPORT __attribute__ ((visibility ("default")))
#define HELPER_DLL_EXPORT __attribute__ ((visibility ("default")))
#define HELPER_DLL_LOCAL  __attribute__ ((visibility ("hidden")))
#else
#define HELPER_DLL_IMPORT
#define HELPER_DLL_EXPORT
#define HELPER_DLL_LOCAL
#endif
#endif

// Now we use the generic helper definitions above to define DLL_API and DLL_LOCAL.
// DLL_API is used for the public API symbols. It either DLL imports or DLL exports (or does nothing for static build)
// DLL_LOCAL is used for non-api symbols.

#ifdef USE_DLL // defined if code is compiled as a DLL
#ifdef DLL_EXPORTS // defined if we are building the DLL (instead of using it)
#define DLL_API HELPER_DLL_EXPORT
#else
#define DLL_API HELPER_DLL_IMPORT
#endif // DLL_EXPORTS
#define DLL_LOCAL HELPER_DLL_LOCAL
#else // MONICA_DLL is not defined: this means code is a static lib.
#define DLL_API
#define DLL_LOCAL
#endif // USE_DLL

#endif // DLL_EXPORTS_H
