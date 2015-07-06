#pragma once
#include <memory>
#include "http_parser.h"
#include "session_interface.h"
#include "http_server.h"
#include "request_respons.h"

class http_session : public session
{
public:

    http_session(http_server &s, boost::asio::io_service &ios)
        : server_(s)
        , io_service_(ios)
        , socket_(new tcp::socket(io_service_))
    {
    }

    ~http_session(){}

    void start();

    tcp::socket& socket()
    {
        return *socket_;
    }

private:
    void do_read();

    void onRead(boost::system::error_code ec, std::size_t length);

    void do_write(std::string &&respons);

    void onWrite(boost::system::error_code ec, std::size_t length);

    void close();

private:
    http_parser parser_;
    http_parser_settings parser_settings_;

    http_server& server_;
    boost::asio::io_service &io_service_;
    std::shared_ptr<tcp::socket> socket_;
    enum { max_length = 2048 };
    char data_[max_length];

    std::string body_;
    request request_;
    response response_;
    bool was_header_value_;
    std::string last_header_field_;
    std::string last_header_value_;
};

