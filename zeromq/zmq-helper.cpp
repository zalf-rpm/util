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

#include <sstream>

#include "zmq-helper.h"

using namespace Tools;
using namespace std;
using namespace json11;

#ifndef NO_ZMQ
Msg Tools::receiveMsg(zmq::socket_t& socket, int topicCharCount, bool nonBlockingMode)
{
  zmq::message_t message;
  if(socket.recv(&message, nonBlockingMode ? ZMQ_NOBLOCK : 0))
  {
    std::string strMsg(static_cast<char*>(message.data()), message.size());
		std::string topic = topicCharCount > 0 ? strMsg.substr(0, topicCharCount) : "";
		strMsg = strMsg.substr(topicCharCount);

    //cout << "receiveMsg: " << strMsg << endl;

    //    string strMsg = s_recv(pullSocket);
    string err;
    const Json& jsonMsg = Json::parse(strMsg, err);
    return Msg{jsonMsg, err, topic, (err.empty() ? "" : strMsg), true};
  }
  return Msg{Json(), "", "", "", false};
}

bool Tools::sendMsg(zmq::socket_t& pushSocket, json11::Json msg) { 
  auto msgStr = msg.dump();
  zmq::message_t message(msgStr.size());
  memcpy(message.data(), msgStr.data(), msgStr.size());
  return pushSocket.send(message);
}

std::string Tools::s_recv(zmq::socket_t& socket) {
  zmq::message_t message;
  socket.recv(&message);
  return std::string(static_cast<char*>(message.data()), message.size());
}

bool Tools::s_send(zmq::socket_t& socket, const std::string& string) {
  zmq::message_t message(string.size());
  memcpy(message.data(), string.data(), string.size());

  bool rc = socket.send(message);
  return (rc);
}

bool Tools::s_sendmore(zmq::socket_t& socket, const std::string& string) {
  zmq::message_t message(string.size());
  memcpy(message.data(), string.data(), string.size());

  bool rc = socket.send(message, ZMQ_SNDMORE);
  return (rc);
}

#endif


