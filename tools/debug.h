/* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/*
Authors: 
Xenia Specka <xenia.specka@zalf.de>
Michael Berg <michael.berg@zalf.de>

Maintainers: 
Currently maintained by the authors.

This file is part of the MONICA model. 
Copyright (C) Leibniz Centre for Agricultural Landscape Research (ZALF)
*/

#ifndef TOOLS_DEBUG_H_
#define TOOLS_DEBUG_H_

/**
 * @file debug.h
 * @author Xenia Specka, Michael Berg
 */

#include <iostream>
#include <streambuf>
#include <ostream>

namespace Tools
{
  std::ostream& debug();
  //global flag to activate debug function
  extern bool activateDebug;

  class DebugBuffer : public std::streambuf
  {
  public:
    DebugBuffer(int=5000){}
    virtual ~DebugBuffer(){}

  protected:
    int_type overflow(int_type){ return 0; }
    int_type sync(){ return 0; }
  };

  class Debug : public std::ostream
  {
  public:
    Debug() : std::ostream(new DebugBuffer()) {}
    Debug(int i) : std::ostream(new DebugBuffer(i)) {}
    ~Debug();
  };
}

#endif /* TOOLS_DEBUG_H_ */
