#include "catch.hpp"
#include <boost\thread.hpp>
#include <boost\asio.hpp>

///////////////////////////////////////////////////////////////////////////////////////////
class BaseChannel : public std::enable_shared_from_this<BaseChannel>
{
public:
	using IOService = boost::asio::io_service;
	using ThreadGroup = boost::thread_group;

	BaseChannel();
	virtual ~BaseChannel();

	virtual void start();
	virtual void join();
	virtual void stop();

public:
	ThreadGroup threadPool_;
	IOService ioService_;
};

BaseChannel::BaseChannel()
{
}

BaseChannel::~BaseChannel()
{
}

void BaseChannel::start()
{
	for (unsigned int i = 0; i < 2; i++)
	{
		threadPool_.create_thread(boost::bind(&boost::asio::io_service::run, &ioService_));
	}
}

void BaseChannel::join()
{
	threadPool_.join_all();
}

void BaseChannel::stop()
{
	ioService_.stop();
}

class TCPServerHandler : public std::enable_shared_from_this<TCPServerHandler>
{
public:
	using Ptr = std::shared_ptr<TCPServerHandler>;

public:
	boost::asio::ip::tcp::socket socket_;

public:
	TCPServerHandler(BaseChannel& channel)
		: socket_(channel.ioService_)
	{
	}
};

class TCPServer : public std::enable_shared_from_this<TCPServer>
{
public:
	using Ptr = std::shared_ptr<TCPServer>;

public:
	BaseChannel & channel_;
	boost::asio::io_context::strand strand_;
	boost::asio::ip::tcp::acceptor acceptor_v4;
	boost::asio::ip::tcp::acceptor acceptor_v6;
	std::vector<TCPServerHandler::Ptr> vecHandler_;

public:
	TCPServer(BaseChannel& channel)
		: channel_(channel)
		, strand_(channel.ioService_)
		, acceptor_v4(channel.ioService_, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), 6001))
		, acceptor_v6(channel.ioService_, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v6(), 6001))
	{
	}
	void start_accept_v4()
	{
		TCPServerHandler::Ptr handler = std::make_shared<TCPServerHandler>(channel_);
		try
		{
			printf("start_accept_v4...\n");
			acceptor_v4.async_accept(handler->socket_, strand_.wrap(boost::bind(&TCPServer::handle_accept_v4, this, handler, boost::asio::placeholders::error)));
		}
		catch (boost::system::error_code& e)
		{
			printf("acceptor_v4 async_accept failed [error_code: %d, %s]\n", e.value(), e.message().c_str());
		}
	}
	void start_accept_v6()
	{
		TCPServerHandler::Ptr handler = std::make_shared<TCPServerHandler>(channel_);
		try
		{
			printf("start_accept_v6...\n");
			acceptor_v6.async_accept(handler->socket_, strand_.wrap(boost::bind(&TCPServer::handle_accept_v6, this, handler, boost::asio::placeholders::error)));
		}
		catch (boost::system::error_code& e)
		{
			printf("acceptor_v6 async_accept failed [error_code: %d, %s]\n", e.value(), e.message().c_str());
		}

	}
	void handle_accept_v4(TCPServerHandler::Ptr handler, const boost::system::error_code& e)
	{
		if (!e)
		{
			vecHandler_.push_back(handler);
			printf("handle_accept_v4 ok [vecHandler_ size = %d]\n", vecHandler_.size());
			start_accept_v4();
		}
		else
		{
			printf("handle_accept_v4 failed [error_code: %d, %s]\n", e.value(), e.message().c_str());
		}
	}
	void handle_accept_v6(TCPServerHandler::Ptr handler, const boost::system::error_code& e)
	{
		if (!e)
		{
			vecHandler_.push_back(handler);
			printf("handle_accept_v6 ok [vecHandler_ size = %d]\n", vecHandler_.size());
			start_accept_v6();
		}
		else
		{
			printf("handle_accept_v6 failed [error_code: %d, %s]\n", e.value(), e.message().c_str());
		}
	}
};

///////////////////////////////////////////////////////////////////////////////////////////
TEST_CASE("tcp_server", "[tcp_server][.hide]")
{
	BaseChannel channel;
	TCPServer::Ptr server = std::make_shared<TCPServer>(channel);

	server->start_accept_v4();
	server->start_accept_v6();

	channel.start();
	channel.join();
}

class TCPClientHandler : public std::enable_shared_from_this<TCPClientHandler>
{
public:
	using Ptr = std::shared_ptr<TCPClientHandler>;

public:
	BaseChannel & channel_;
	boost::asio::io_context::strand strand_;

	boost::asio::ip::tcp::socket socket_v4_;
	boost::asio::ip::tcp::socket socket_v6_;

public:
	TCPClientHandler(BaseChannel& channel)
		: strand_(channel.ioService_)
		, socket_v4_(channel.ioService_)
		, socket_v6_(channel.ioService_)
		, channel_(channel)
	{
	}
	void start_connect_v4()
	{
		boost::asio::ip::tcp::endpoint remote_endpoint_v4(boost::asio::ip::address_v4::from_string("139.24.137.223"), 6001);
		socket_v4_.async_connect(remote_endpoint_v4, strand_.wrap(boost::bind(&TCPClientHandler::handle_connect_v4, this, boost::asio::placeholders::error)));
	}
	void start_connect_v6()
	{
		boost::asio::ip::tcp::endpoint remote_endpoint_v6(boost::asio::ip::address_v6::from_string("fe80::9d8e:6863:ccc5:cbdf"), 6001);
		socket_v6_.async_connect(remote_endpoint_v6, strand_.wrap(boost::bind(&TCPClientHandler::handle_connect_v6, this, boost::asio::placeholders::error)));
	}
	void handle_connect_v4(const boost::system::error_code& e)
	{
		if (!e)
		{
			printf("handle_connect_v4 ok\n");
		}
		else
		{
			printf("handle_connect_v4 failed [error_code: %d, %s]\n", e.value(), e.message().c_str());
		}
	}
	void handle_connect_v6(const boost::system::error_code& e)
	{
		if (!e)
		{
			printf("handle_connect_v6 ok\n");
		}
		else
		{
			printf("handle_connect_v6 failed [error_code: %d, %s]\n", e.value(), e.message().c_str());
		}
	}
};

TEST_CASE("tcp_client", "[tcp_client][.hide]")
{
	BaseChannel channel;
	TCPClientHandler::Ptr clientHandler = std::make_shared<TCPClientHandler>(channel);

	clientHandler->start_connect_v4();
	clientHandler->start_connect_v6();

	channel.start();
	channel.join();
}

class UDPHandler : public std::enable_shared_from_this<UDPHandler>
{
public:
	using Ptr = std::shared_ptr<UDPHandler>;

public:
	BaseChannel & channel_;
	boost::asio::io_context::strand strand_;

	boost::asio::ip::udp::socket socket_v4_;
	boost::asio::ip::udp::socket socket_v6_;

public:
	UDPHandler(BaseChannel& channel)
		: strand_(channel.ioService_)
		, socket_v4_(channel.ioService_)
		, socket_v6_(channel.ioService_)
		, channel_(channel)
	{
	}
	void start_bind_v4()
	{
		socket_v4_.open(boost::asio::ip::udp::v4());
		boost::asio::ip::udp::endpoint remote_endpoint(boost::asio::ip::address_v4::from_string("139.24.137.223"), 6001);
		boost::system::error_code e;
		socket_v4_.bind(remote_endpoint, e);
		if (!e)
		{
			printf("start_bind_v4 ok\n");
		}
		else
		{
			printf("start_bind_v4 failed [error_code: %d, %s]\n", e.value(), e.message().c_str());
		}
	}
	void start_bind_v6()
	{
		socket_v6_.open(boost::asio::ip::udp::v6());
		boost::asio::ip::udp::endpoint remote_endpoint(boost::asio::ip::address_v6::from_string("fe80::9d8e:6863:ccc5:cbdf"), 6001);
		boost::system::error_code e;
		socket_v6_.bind(remote_endpoint, e);
		if (!e)
		{
			printf("start_bind_v6 ok\n");
		}
		else
		{
			printf("start_bind_v6 failed [error_code: %d, %s]\n", e.value(), e.message().c_str());
		}
	}
};

TEST_CASE("udp", "[udp][.hide]")
{
	BaseChannel channel;
	UDPHandler::Ptr clientHandler = std::make_shared<UDPHandler>(channel);
	clientHandler->start_bind_v4();
	clientHandler->start_bind_v6();

	channel.start();
	channel.join();
}

