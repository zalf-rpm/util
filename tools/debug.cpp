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

#include "debug.h"

using namespace Tools;
using namespace std;

bool Tools::activateDebug = false;

ostream& Tools::debug()
{
  static Debug dummy;
  return activateDebug ? cout : dummy;
}

Debug::~Debug()
{
  DebugBuffer* buf = (DebugBuffer*) rdbuf();
  if (buf)
    delete buf;
}
