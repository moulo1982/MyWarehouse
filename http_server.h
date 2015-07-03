#ifndef __HTTPSERVER_H__
#define __HTTPSERVER_H__

#include <iostream>
#include <memory>
#include <unordered_set>
#include <thread>
#include <condition_variable>

#include <boost/circular_buffer.hpp>
#include <boost/asio.hpp>

#include "io_service_pool.h"
#include "session_interface.h"

using boost::asio::ip::tcp;
using boost::asio::socket_base;

class http_server
{
public:
    http_server(tcp v, short port, short threadnum = 1) 
        : acceptor_(io_service_, tcp::endpoint(v, port))
        , pool_(threadnum)
        , thread_(&io_service_pool::run, std::ref(pool_))
    {
        pool_.wait_init();
        do_accept();
    }

    ~http_server()
    {
        pool_.stop();
        thread_.join();
    }

    void run()
    {
        io_service_.run();
    }

    void erase(std::shared_ptr<session> p);

private:

    void do_accept();
    void insert(std::shared_ptr<session> &p);

private:
    std::unordered_set<std::shared_ptr<session>> session_;
    boost::asio::io_service io_service_;
    tcp::acceptor acceptor_;
    
    io_service_pool pool_;
    std::thread thread_;

    std::mutex m_mutex;
};

#endif
