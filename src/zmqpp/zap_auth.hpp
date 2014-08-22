#pragma once

#include "zap.hpp"

namespace zmqpp
{
namespace zap
{
  /**
   * This is the interface for the implementation
   * of a ZAP request authentification.
   * 
   * zmqpp::zap::handler is injected with an imlementation
   * of the interface.
   */
  class iauthenticator
  {
  public:
    virtual ~iauthenticator() = default;
    virtual response process_request(const request &r) = 0;
  };
};

};
