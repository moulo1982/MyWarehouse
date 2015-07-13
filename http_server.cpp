#include "http_server.h"
#include "http_session.h"
#include <sys/sysinfo.h>

void http_server::run()
{
    std::vector<std::shared_ptr<std::thread> > threads;
    for (std::size_t i = 0; i < get_nprocs(); ++i)
    {
        std::shared_ptr<std::thread> thread(new std::thread([this](){
            io_service_.run();
        }));
        threads.push_back(thread);
    }

    for (std::size_t i = 0; i < 2; ++i)
        threads[i]->join();
}

void http_server::do_accept()
{
    std::shared_ptr<session> new_session(new http_session(*this, pool_.get_io_service()));
    acceptor_.async_accept(new_session->socket(),
        std::bind(&http_server::OnAccept, this, new_session, std::placeholders::_1));
}

void http_server::OnAccept(std::shared_ptr<session> new_session, const boost::system::error_code &ec)
{
    if(!ec)
    {
        do_accept();

        insert(new_session);
        new_session->start();
    }
    else
    {
        if (ec == boost::asio::error::try_again)
        {
            std::cout << "http_server::OnAccept::. ec(boost::asio::error::try_again): [" << ec.value() << "] Msg: [" << ec.message() << "]" << std::endl;
        }
        if (ec == boost::asio::error::would_block)
        {
            std::cout << "http_server::acceptor_::. ec(boost::asio::error::would_block): [" << ec.value() << "] Msg: [" << ec.message() << "]" << std::endl;
        }

        new_session.reset();
        std::cout << "http_server::acceptor_::async_accept. ec: [" << ec.value() << "] Msg: [" << ec.message() << "]" << std::endl;
    }
}



