/**
 * \file
 *
 * \date   9 Aug 2011
 * \author Ben Gray (\@benjamg)
 */

#ifndef ZMQPP_EXCEPTION_HPP_
#define ZMQPP_EXCEPTION_HPP_

#include <stdexcept>
#include <string>

#include <zmq.h>

namespace zmqpp
{

/** \todo Have a larger variety of exceptions with better state debug information */

/*!
 * Represents the base zmqpp exception.
 *
 * All zmqpp runtime exceptions are children of this class.
 * The class itself does not provide any special access fields but it only
 * for convince when catching exceptions.
 *
 * The class extends std::runtime_error.
 *
 */
class exception : public std::runtime_error
{
public:
	/*!
	 * Standard exception constructor.
	 *
	 * \param message a string representing the error message.
	 */
	exception(std::string const& message)
		: std::runtime_error(message)
	{ }
};

/*!
 * Represents an attempt to use an invalid object.
 *
 * Objects may be invalid initially or after a shutdown or close.
 */
class invalid_instance : public exception
{
public:
	invalid_instance(std::string const& message)
		: exception(message)
	{ }
};

    /**
     * Represents a failed zmqpp::actor initialization.
     */
  class actor_initialization_exception : public exception
  {
  public:
    
    actor_initialization_exception() :
      exception("Actor Initialization Exception")
    {
    }
    
  };
  

  /**
   * Exception when we have trouble accessing a message's property.
   * Either the property doesn't not exist, or the zmqpp::message is (or has been)
   * empty(ed). By empty, we mean the message has 0 frame.
   */
  class message_property_exception : public exception
  {
  public:
    enum reason
      {
	EMPTY_MSG,
	NOT_FOUND
      };

    message_property_exception(enum reason r) :
      exception(r == reason::EMPTY_MSG ?
		"Message has no frame, cannot access property" : 
		"Cannot find property")
    {}
  };

  /**
   * Exception thrown by the ZAP handler when a request is invalid.
   */
  class zap_invalid_request_exception : public exception
  {
  public:
    zap_invalid_request_exception() :
      exception("Invalid ZAP request")
    {}
  };

/*!
 * Represents internal zmq errors.
 *
 * Any error response from the zmq bindings will be wrapped in this error.
 *
 * The class provides access to the zmq error number via zmq_error().
 */
class zmq_internal_exception : public exception
{
public:
	/*!
	 * Uses the zmq functions to pull out error messages and numbers.
	 */
	zmq_internal_exception()
		: exception(zmq_strerror(zmq_errno()))
		, _error(zmq_errno())
	{ }

	/*!
	 * Retrieve the zmq error number associated with this exception.
	 * \return zmq error number
	 */
	int zmq_error() const { return _error; }

private:
	int _error;
};

}

#endif /* ZMQPP_EXCEPTION_HPP_ */
