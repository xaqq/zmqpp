#pragma once

#include "zap.hpp"

namespace zmqpp
{
namespace zap
{
  /**
   * This is the base interface for the implementation of a ZAP request verification.
   * 
   * A subclass of the interface is called by the zap::handler for each requests
   * that arrives. This class is responsible for providing
   * a response, eg by checking credentials from file or a database.
   * 
   * The zmqpp::zap::handler's constructor receives a subclass of this interface.
   */
  class iauthenticator
  {
  public:
    virtual ~iauthenticator() = default;

    /**
     * Process the request and returns a response.
     */
    virtual response process_request(const request &r) = 0;
  };
};

};
