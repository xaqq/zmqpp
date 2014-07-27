
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
    std::cout << "U = " << r.username << ";; req id = " << r.request_id << std::endl;
    if (r.username == "toto" && r.password == "toto_secret")
      {
	std::cout << "test2" << std::endl;
	return zmqpp::zap::response(r.request_id, "200", "gj", "toto", "");
      }
    return zmqpp::zap::response(r.request_id, "400", "", "", "");
  }
};

BOOST_AUTO_TEST_CASE(test_plain)
{
  zmqpp::context ctx;
  zmqpp::zap::handler zap_handler(ctx, new MyDummyAuth());
  zmqpp::socket srv(ctx, zmqpp::socket_type::router);
  zmqpp::socket client(ctx, zmqpp::socket_type::dealer);
							
  srv.set(zmqpp::socket_option::plain_server, true);
  srv.bind("tcp://*:45451");

  client.set(zmqpp::socket_option::plain_username, "toto");
  client.set(zmqpp::socket_option::plain_password, "toto_secret");
  client.connect("tcp://localhost:45451");

  client.send("toto");

  zmqpp::message ret;
  std::string content;
  srv.receive(ret);
  
  ret >> content; // flush identity
  ret >> content;
  
  BOOST_CHECK_EQUAL(content, "toto");

}

BOOST_AUTO_TEST_SUITE_END()
