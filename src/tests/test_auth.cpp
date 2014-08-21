
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

#if (ZMQ_VERSION_MAJOR > 3)
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
#endif

#if (ZMQ_VERSION_MAJOR > 3)
BOOST_AUTO_TEST_CASE(test_invalid_req1)
{
  zmqpp::context ctx;
  zmqpp::message msg;
  zmqpp::zap::handler zap_handler(ctx, nullptr); //handler shouldn't be called
  zmqpp::socket zap_client(ctx, zmqpp::socket_type::req);

  zap_client.connect("inproc://zeromq.zap.01");

  msg << "2.0";
  zap_client.send(msg);

  zmqpp::zap::response rep;
  zap_client.receive(msg);
  
  std::string rep_version, rep_id, rep_status_code, rep_status_txt;
  msg >> rep_version;
  msg >> rep_id; // we wont check that because we failed sooner
  msg >> rep_status_code;
  msg >> rep_status_txt;

  BOOST_CHECK_EQUAL(rep_version, "1.0");
  BOOST_CHECK_EQUAL(rep_status_code, "500");
  BOOST_CHECK_EQUAL(rep_status_txt, "Invalid ZAP request");
}
#endif

#if (ZMQ_VERSION_MAJOR > 3)
BOOST_AUTO_TEST_CASE(test_invalid_req2)
{
  zmqpp::context ctx;
  zmqpp::message msg;
  zmqpp::zap::handler zap_handler(ctx, nullptr);//handler shouldn't be called
  zmqpp::socket zap_client(ctx, zmqpp::socket_type::req);

  zap_client.connect("inproc://zeromq.zap.01");

  msg << "1.0";
  msg << "BLA_ID";
  msg << "random_domain";
  msg << "127.0.0.1";
  msg << "sock_identity";
  msg << "PLAIN";
  msg << "username";
  // we "forgot" password

  zap_client.send(msg);

  zmqpp::zap::response rep;
  zap_client.receive(msg);
  
  std::string rep_version, rep_id, rep_status_code, rep_status_txt;
  msg >> rep_version;
  msg >> rep_id; // we wont check that
  msg >> rep_status_code;
  msg >> rep_status_txt;

  BOOST_CHECK_EQUAL(rep_version, "1.0");
  BOOST_CHECK_EQUAL(rep_status_code, "500");
  BOOST_CHECK_EQUAL(rep_status_txt, "Invalid ZAP request");
}
#endif

BOOST_AUTO_TEST_SUITE_END()
