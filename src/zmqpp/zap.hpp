#pragma once

#include <string>
#include <vector>
#include "socket.hpp"
#include "actor.hpp"
#include "reactor.hpp"

namespace zmqpp
{
  namespace zap
  {
    class iauthenticator;

    /**
     * This class is the representation of a ZAP request.
     * It is serialize to a zmqpp::message object, and can be extracted from one.
     * See ZMQ's RFC 27
     */
    class request
    {
    public:
      std::string version;              //  Version number, must be "1.0"
      std::string request_id;             //  Sequence number of request
      std::string domain;               //  Server socket domain
      std::string address;              //  Client IP address
      std::string identity;             //  Server socket idenntity
      std::string mechanism;            //  Security mechansim
      std::string username;             //  PLAIN user name
      std::string password;             //  PLAIN password, in clear text
      std::string client_key;           //  CURVE client public key in ASCII
      std::string principal;            //  GSSAPI client principal
    };

    /**
     * A ZAP response object
     */
    class response
    {
    public:
      response(const std::string &request_id = "",
	       const std::string &status_code = "",
	       const std::string &status_text = "",
	       const std::string &user_id = "",
	       const std::string &metadata = "") :
	request_id_(request_id),
	status_code_(status_code),
	status_text_(status_text),
	user_id_(user_id),
	metadata_(metadata)
      {
	version_ = "1.0";
      }

      std::string version_;
      std::string request_id_;
      std::string status_code_;
      std::string status_text_;
      std::string user_id_;
      std::string metadata_;
    };

    /**
     * This is a base class for a ZAP handler implementation.
     * 
     */
    class handler
    {

    public:
      /**
       * Construct a handler object from a zmqpp::context and a 
       * iauthenticator object that will authenticate the request.
       * The handler takes ownership of the iauthenticator.
       */
      handler(context_t &ctx, iauthenticator *auth);
      virtual ~handler();

    protected:
      /**
       * Rep socket that reiceve auth request from libzmq
       */
      socket rep_;
      actor *me_;

      iauthenticator *authenticator_;

      bool run(socket *pipe);


      /**
       * Called by the internal reactor when something happens on the actor's pipe.
       */
      void handle_pipe(socket *pipe);

      /**
       * Called by internal reactor when a authentication request arrives.
       */
      void handle_router();

    private:
      /**
       * Main loop runs while this is true.
       * The shutdown order come from the actor's pipe.
       */
      bool is_running_;
      reactor reactor_;

      request build_request(message_t &msg);
    };
  };


  message &operator<<(message &msg, const zmqpp::zap::response &rep);
};
