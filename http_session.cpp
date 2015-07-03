#include "http_session.h"
#include "http_server.h"

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
        //TO DO:  process buf_ & length there, Example: http_parser
        do_write(length);
    }
    else
    {
        //ec == 2, End of file
        if (ec.value() != 2)
            std::cout << "http_session::onRead. ec: [" << ec.value() << "] Msg: [" << ec.message() << "]" << std::endl;
        close();
    }
}

void http_session::do_write(std::size_t length)
{
    std::weak_ptr<http_session> wp(shared_Derived_from_this<http_session>());
    boost::asio::async_write(*socket_, boost::asio::buffer(data_, length),
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
        do_read();
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