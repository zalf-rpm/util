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

#ifndef ZMQ_HELPER_H_
#define ZMQ_HELPER_H_

#include <string>
#include <functional>
#include <initializer_list>

#include "json11/json11.hpp"

#ifndef NO_ZMQ
#include "zmq.hpp"
#endif

#include "tools/date.h"

namespace Tools
{
  struct Msg
  {
    std::string type() const { return json["type"].string_value(); }
    std::string toString() const { return std::string("type: ") + type() + " msg: " + json.dump() + (err.empty() ? "" : " err: " + err); }
    json11::Json json;
    std::string err;
		std::string topic;
		std::string msg;
    bool valid;
  };

#ifndef NO_ZMQ
	Msg receiveMsg(zmq::socket_t& socket, int topicCharCount = 0, bool nonBlockingMode = false);

  bool sendMsg(zmq::socket_t& pushSocket, json11::Json msg);

  std::string s_recv(zmq::socket_t& socket);

  bool s_send(zmq::socket_t& socket, const std::string& string);

  bool s_sendmore(zmq::socket_t& socket, const std::string& string);
#endif
}

#endif
