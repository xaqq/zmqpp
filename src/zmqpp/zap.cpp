#include "zap.hpp"
#include "message.hpp"
#include "zap_auth.hpp"
#include "exception.hpp"
#include <functional>

using namespace zmqpp::zap;

handler::handler(context_t &ctx, iauthenticator *auth):
  rep_(ctx, socket_type::router),
  authenticator_(auth),
  is_running_(true)
{
  me_ = new actor(std::bind(&handler::run, this, std::placeholders::_1));
}

handler::~handler()
{
  delete authenticator_;
  delete me_;
}

bool handler::run(socket *pipe)
{
  rep_.bind("inproc://zeromq.zap.01");
  reactor_.add(*pipe, std::bind(&handler::handle_pipe, this, pipe));
  reactor_.add(rep_, std::bind(&handler::handle_router, this));
  pipe->send(signal::ok);

  while (is_running_)
    {
      reactor_.poll();
    }
  return true;
}

void handler::handle_pipe(socket *pipe)
{
  signal s;

  // we only monitor for the stop signal
  pipe->receive(s);
  if (s == signal::stop)
    {
      is_running_ = false;
    }
}

void handler::handle_router()
{
  message_t msg;
  message_t response_msg;
  request req;
  response r;
  std::string identity;

  rep_.receive(msg);
  msg >> identity;

  try 
    {
      req = build_request(msg);
      r = authenticator_->process_request(req);
    }
  catch (zmqpp::exception &e)
    {
      // request is invalid, send 500 error.
      std::cout << "invalid zap request" << std::endl;
      r = response("UNKNOWN", "500", "Invalid ZAP request");
    }

  response_msg << identity;
  response_msg << r;
  rep_.send(response_msg);
}

request handler::build_request(message_t &msg)
{
  request req;

  msg.pop_front(); // pop delimiter
  msg >> req.version;

  if (req.version != "1.0")
    {
      std::cout << "verzion mismatch" << std::endl;
    throw zap_invalid_request_exception();
    }
  msg >> req.request_id;             //  Sequence number of request
  msg >> req.domain;               //  Server socket domain
  msg >> req.address;              //  Client IP address
  msg >> req.identity;             //  Server socket idenntity
  msg >> req.mechanism;            //  Security mechansim

  // credentials frame(s) vary based on the mechanism
  if (req.mechanism == "PLAIN")
    {
      msg >> req.username;             //  PLAIN user name
      msg >> req.password;             //  PLAIN password, in clear text
    }
  else if (req.mechanism == "CURVE")
    {
      msg >> req.client_key;           //  CURVE client public key in ASCII
      // todo decode binary blob
    }

  std::cout << "V = " << req.version << "; I = " << req.identity << "; A = " << req.address  << "; D = " << req.domain << std::endl;
  return req;
}

zmqpp::message &zmqpp::operator<<(zmqpp::message &msg, const zmqpp::zap::response &rep)
{
  msg << "";
  msg << rep.version_;
  msg << rep.request_id_;
  msg << rep.status_code_;
  msg << rep.status_text_;
  msg << rep.user_id_;
  msg << rep.metadata_;

  return msg;
}
