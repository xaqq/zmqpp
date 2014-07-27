#include "zap.hpp"
#include "message.hpp"
#include "zap_auth.hpp"
#include <functional>

using namespace zmqpp::zap;

handler::handler(context_t &ctx, iauthenticator *auth):
  rep_(ctx, socket_type::router),
  authenticator_(auth)
{
  me_ = new actor(std::bind(&handler::run_, this, std::placeholders::_1));
}

handler::~handler()
{
  delete authenticator_;
  delete me_;
}

bool handler::run_(socket *pipe)
{
  rep_.bind("inproc://zeromq.zap.01");
  pipe->send(signal::ok);
  while (true)
    {
      message_t msg;
      message_t response_msg;
      request req;
      response r;
      std::string identity;

      rep_.receive(msg);
      msg >> identity;

      req = build_request(msg);
      r = authenticator_->process_request(req);

      response_msg << identity;
      response_msg << r;
      rep_.send(response_msg);
    }
  return true;
}

request handler::build_request(message_t &msg)
{
  request req;

  msg.pop_front(); // pop delimiter
  msg >> req.version;
  msg >> req.request_id;             //  Sequence number of request
  msg >> req.domain;               //  Server socket domain
  msg >> req.address;              //  Client IP address
  msg >> req.identity;             //  Server socket idenntity
  msg >> req.mechanism;            //  Security mechansim
  msg >> req.username;             //  PLAIN user name
  msg >> req.password;             //  PLAIN password, in clear text
  //  msg >> req.client_key;           //  CURVE client public key in ASCII
  //  msg >> req.principal;            //  GSSAPI client principal

  std::cout << "V = " << req.version << std::endl;
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
