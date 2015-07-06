#include "http_session.h"
#include "http_server.h"
#include <cwchar>
#include <locale>
#include <stdlib.h>

void http_session::start()
{
    http_parser_init(&parser_, HTTP_REQUEST);
    parser_.data = this;

    //on_url
    parser_settings_.on_url = [](http_parser* parser, const char *at, size_t len) {
        auto psession = reinterpret_cast<http_session*>(parser->data);
        psession->request_.url_.from_buf(at, len);
        return 0;
    };

    //on_header_field
    parser_settings_.on_header_field = [](http_parser* parser, const char* at, size_t len) {
        auto psession = reinterpret_cast<http_session*>(parser->data);

        if (psession->was_header_value_)
        {
            // new field started
            if (!psession->last_header_field_.empty())
            {
                // add new entry
                psession->request_.headers_[psession->last_header_field_] = psession->last_header_value_;
                psession->last_header_value_.clear();
            }

            psession->last_header_field_ = std::string(at, len);
            psession->was_header_value_ = false;
        }
        else
        {
            // appending
            psession->last_header_field_ += std::string(at, len);
        }
        return 0;
    };

    //on_header_value
    parser_settings_.on_header_value = [](http_parser* parser, const char* at, size_t len) {
        auto psession = reinterpret_cast<http_session*>(parser->data);

        if (!psession->was_header_value_)
        {
            psession->last_header_value_ = std::string(at, len);
            psession->was_header_value_ = true;
        }
        else
        {
            // appending
            psession->last_header_value_ += std::string(at, len);
        }
        return 0;
    };

    //on_headers_complete
    parser_settings_.on_headers_complete = [](http_parser* parser) {
        auto psession = reinterpret_cast<http_session*>(parser->data);

        if (!psession->last_header_field_.empty()) {
            // add new entry
            psession->request_.headers_[psession->last_header_field_] = psession->last_header_value_;
        }

        return 0;
    };


    //on_body
    parser_settings_.on_body = [](http_parser* parser, const char* at, size_t len) {
        auto psession = reinterpret_cast<http_session*>(parser->data);


        psession->request_.body_ = std::string(at, len);

        return 0;
    };
    parser_settings_.on_message_complete = [](http_parser* parser) {

        auto psession = reinterpret_cast<http_session*>(parser->data);
        psession->server_.invoke(psession->request_, psession->response_);
        psession->do_write(psession->response_.end());

        return 1;
    };

    do_read();
}

void http_session::do_read()
{
    std::weak_ptr<http_session> wp(shared_Derived_from_this<http_session>());
    socket_->async_read_some(boost::asio::buffer(data_, max_length),
        [wp](boost::system::error_code ec, std::size_t length){
        auto sp(wp.lock());
        if (sp)
        {
            sp->onRead(ec, length);
        }
    });
}

void http_session::onRead(boost::system::error_code ec, std::size_t length)
{
    if (!ec)
    {
        http_parser_execute(&parser_, &parser_settings_, data_, length);
    }
    else
    {
        //ec == 2, End of file
        if (ec.value() != 2)
            std::cout << "http_session::onRead. ec: [" << ec.value() << "] Msg: [" << ec.message() << "]" << std::endl;
        close();
    }
}

void http_session::do_write( std::string &&respons)
{
    std::weak_ptr<http_session> wp(shared_Derived_from_this<http_session>());
    boost::asio::async_write(*socket_, boost::asio::buffer(respons.c_str(), respons.length()),
        [wp](boost::system::error_code ec, std::size_t length){
        auto sp(wp.lock());
        if (sp)
        {
            sp->onWrite(ec, length);
        }
    });
}

void http_session::onWrite(boost::system::error_code ec, std::size_t length)
{
    if (!ec)
    {
        close();
    }
    else
    {
        std::cout << "http_session::onWrite. ec: [" << ec.value() << "] Msg: [" << ec.message() << "]" << std::endl;
        close();
    }
}

void http_session::close()
{
    boost::system::error_code ec;
    socket_->shutdown(boost::asio::socket_base::shutdown_both, ec);
    socket_->close(ec);
    server_.erase(shared_from_this());
}