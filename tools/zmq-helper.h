#ifndef ZMQ_HELPER_H_
#define ZMQ_HELPER_H_

#include <string>
#include <functional>
#include <initializer_list>

#include "json11/json11.hpp"

#ifndef NO_ZMQ
#include "zmq.hpp"
#include "zhelpers.hpp"
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
    bool valid;
  };

#ifndef NO_ZMQ
  Msg receiveMsg(zmq::socket_t& pullSocket, bool nonBlockingMode = false);

  inline bool sendMsg(zmq::socket_t& pushSocket, json11::Json msg){ return s_send(pushSocket, msg.dump()); }
#endif
}

#endif
