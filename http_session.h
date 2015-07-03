#pragma once
#include <memory>
#include "session_interface.h"
#include "http_server.h"

class http_session : public session
{
public:

    http_session(http_server &s, boost::asio::io_service &ios)
        : server_(s)
        , io_service_(ios)
        , socket_(new tcp::socket(io_service_))
    {
    }

    void start()
    {
        do_read();
    }

    tcp::socket& socket()
    {
        return *socket_;
    }

public:
    void do_read();

    void onRead(boost::system::error_code ec, std::size_t length);

    void do_write(std::size_t length);

    void onWrite(boost::system::error_code ec, std::size_t length);

    void close();

    http_server& server_;
    boost::asio::io_service &io_service_;
    std::shared_ptr<tcp::socket> socket_;
    enum { max_length = 2048 };
    char data_[max_length];
};

