
#include <boost/test/unit_test.hpp>
#include <thread>

#include "zmqpp/context.hpp"
#include "zmqpp/message.hpp"
#include "zmqpp/actor.hpp"
#include "zmqpp/poller.hpp"
#include "zmqpp/zap.hpp"
#include "zmqpp/zap_auth.hpp"
#include <iostream>

BOOST_AUTO_TEST_SUITE( auth )

class MyDummyAuth : public zmqpp::zap::iauthenticator
{
public:
  zmqpp::zap::response process_request(const zmqpp::zap::request &r)
  {
    BOOST_CHECK_EQUAL(r.domain, "auth_domain");
    BOOST_CHECK_EQUAL(r.identity, "MY_SERVER_IDENTITY");
    if (r.username == "toto" && r.password == "toto_secret")
      {
	return zmqpp::zap::response(r.request_id, "200", "Login OK", "llama", "");
      }
    return zmqpp::zap::response(r.request_id, "400", "", "", "");
  }
};

BOOST_AUTO_TEST_CASE(test_plain_ok)
{
  zmqpp::context ctx;
  zmqpp::zap::handler zap_handler(ctx, new MyDummyAuth());
  zmqpp::socket srv(ctx, zmqpp::socket_type::rep);
  zmqpp::socket client(ctx, zmqpp::socket_type::req);
							
  srv.set(zmqpp::socket_option::plain_server, true);
  srv.set(zmqpp::socket_option::identity, "MY_SERVER_IDENTITY");
  srv.set(zmqpp::socket_option::zap_domain, "auth_domain");
  srv.bind("tcp://*:45451");

  client.set(zmqpp::socket_option::plain_username, "toto");
  client.set(zmqpp::socket_option::plain_password, "toto_secret");
  client.connect("tcp://localhost:45451");

  client.send("toto");

  zmqpp::message ret;
  std::string content;
  srv.receive(ret);
  ret >> content;

  // if we have >= 4.1 we can access msg property and check user id
#if (ZMQ_VERSION_MAJOR == 4 && ZMQ_VERSION_MINOR >= 1)
  BOOST_CHECK_EQUAL(ret.get_property("User-Id"), "llama");
#endif
  BOOST_CHECK_EQUAL(content, "toto");
}

BOOST_AUTO_TEST_SUITE_END()
