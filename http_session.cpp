#include "http_session.h"
#include "http_server.h"
#include "public_define.h"
#include <cwchar>
#include <locale>
#include <stdlib.h>

void http_session::start()
{
    boost::system::error_code ec;

#ifdef DEBUG
    boost::asio::socket_base::debug option_debug(true);
    socket_->set_option(option_debug, ec);
    if (ec)
        std::cout << ec.message() << std::endl;
#endif

    /*boost::asio::socket_base::linger option_linger(true);
    socket_->set_option(option_linger, ec);
    if (ec)
        std::cout << ec.message() << std::endl;*/

    http_parser_init(&parser_, HTTP_REQUEST);
    parser_.data = this;

    /*
    
    //on_url
    parser_settings_.on_url = [](http_parser* parser, const char *at, size_t len) {
        auto psession = reinterpret_cast<http_session*>(parser->data);
        psession->request_.url_.from_buf(at, len);
        //std::cout << "on_url: " << std::string(at, len) << std::endl;
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

        //std::cout << "on_header_field: " << std::string(at, len) << std::endl;
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
        //std::cout << "on_header_value: " << std::string(at, len) << std::endl;
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
    */
    
    //on_body
    parser_settings_.on_body = [](http_parser* parser, const char* at, size_t len) {
        auto psession = reinterpret_cast<http_session*>(parser->data);

        psession->request_.body_ = const_cast<char*>(at);
        psession->request_.len_ = len;
        //std::cout << "on_body: " << std::string(at, len) << std::endl;
        return 0;
    };

    parser_settings_.on_message_complete = [](http_parser* parser) {
        auto psession = reinterpret_cast<http_session*>(parser->data);

        bool bOK = psession->server_.invoke(psession->request_, psession->response_);
        psession->process_ = true;
        psession->do_write(psession->response_.end());
        //std::cout << "on_message_complete: "  << std::endl;
        return 0;
    };

    
    //启动一个业务超时定时器，一个客户端，只允许他连接30秒，超过30秒就断开，无论有没有做完业务，时间可调。
    std::weak_ptr<http_session> wp(shared_Derived_from_this<http_session>());
    timer_.async_wait([wp](const boost::system::error_code& ec){

        if (ec != boost::asio::error::operation_aborted)
        {
            //定时器回调函数返回，但是不是定时器被取消，那么就关闭socket。
            //定期器被取消，是个正常操作
            auto sp(wp.lock());
            if (sp)
            {
                sp->close();
            }
        }
    });
    

    do_read();
}

void http_session::do_read()
{
    std::weak_ptr<http_session> wp(shared_Derived_from_this<http_session>());
    //开始异步读取
    socket_->async_read_some(boost::asio::buffer(data_ + pos_, max_length - pos_),
        [wp](const boost::system::error_code &ec, std::size_t length){

        auto sp(wp.lock());
        if (sp)
        {
            sp->onRead(ec, length);
        }
    });
}

void http_session::onRead(const boost::system::error_code &ec, std::size_t length)
{
    if (!ec)
    {
        pos_ += length;
        http_parser_execute(&parser_, &parser_settings_, data_, length);

        //如果还没有处理报文，缓冲区就满了，那么我们认为是非法，直接断开连接。
        if (!process_)
        {
            if (pos_ >= max_length)
                close();
            else
                do_read();
        }
    }
    else
    {
        if (ec == boost::asio::error::try_again)
        {
            std::cout << "http_session::onRead. ec(boost::asio::error::try_again): [" << ec.value() << "] Msg: [" << ec.message() << "]" << std::endl;
        }
        if (ec == boost::asio::error::would_block)
        {
            std::cout << "http_session::onRead. ec(ec == boost::asio::error::would_block): [" << ec.value() << "] Msg: [" << ec.message() << "]" << std::endl;
        }

        LOG("http_session::onRead. ec: [%d], Msg: [%s]\n", ec.value(), ec.message().c_str());
        close();
    }
}

void http_session::do_write(std::string &&respons)
{
    std::weak_ptr<http_session> wp(shared_Derived_from_this<http_session>());
    boost::asio::async_write(*socket_, boost::asio::buffer(respons.c_str(), respons.length()),
        [wp](const boost::system::error_code &ec, std::size_t length){
        auto sp(wp.lock());
        if (sp)
        {
            //业务处理完成，取消定时器
            boost::system::error_code error;
            sp->timer_.cancel(error);

            sp->onWrite(ec, length);
        }
    });
}

void http_session::onWrite(const boost::system::error_code &ec, std::size_t length)
{
    if (ec)
    {
        if (ec == boost::asio::error::try_again)
        {
            std::cout << "http_session::onRead. ec(boost::asio::error::try_again): [" << ec.value() << "] Msg: [" << ec.message() << "]" << std::endl;
        }
        if (ec == boost::asio::error::would_block)
        {
            std::cout << "http_session::onRead. ec(ec == boost::asio::error::would_block): [" << ec.value() << "] Msg: [" << ec.message() << "]" << std::endl;
        }

        LOG("http_session::onWrite. ec: [%d], Msg: [%s]\n", ec.value(), ec.message().c_str());
    }
        

    close();
}

void http_session::close()
{
    boost::system::error_code ec;
    socket_->shutdown(boost::asio::socket_base::shutdown_both, ec);
    socket_->close(ec);
    server_.erase(shared_from_this());
}
