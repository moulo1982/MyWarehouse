#ifndef __HTTPSERVER_H__
#define __HTTPSERVER_H__

#include <iostream>
#include <memory>
#include <unordered_set>
#include <thread>
#include <vector>
#include <condition_variable>
#include <assert.h>

#include <boost/asio.hpp>
#include "io_service_pool.h"
#include "session_interface.h"
#include "request_respons.h"

using boost::asio::ip::tcp;
using boost::asio::socket_base;

class http_server
{
public:
    http_server(tcp v, unsigned short port, short threadnum, std::function < bool(boost::asio::io_service&, request&, response &) > func)
        : acceptor_(io_service_, tcp::endpoint(v, port))
        , pool_(threadnum)
        , thread_(&io_service_pool::run, std::ref(pool_))
        , func_(func)
    {
        assert(threadnum > 0);
        pool_.wait_init();
        do_accept();
    }

    ~http_server()
    {
        pool_.stop();
        thread_.join();
    }

    void run();

    void erase(std::shared_ptr<session> p)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = session_.find(p);
        if (it != session_.end())
            session_.erase(it);
    }

    bool invoke(request &request_, response &response_)
    {
        return func_(io_service_, request_, response_);
    }
private:

    void do_accept();
    void OnAccept(std::shared_ptr<session> new_session, const boost::system::error_code &ec);

    void insert(std::shared_ptr<session> &p)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = session_.insert(p);
        assert(it.second == true);
    }

private:
    std::unordered_set<std::shared_ptr<session>> session_;
    boost::asio::io_service io_service_;
    tcp::acceptor acceptor_;
    
    io_service_pool pool_;
    std::thread thread_;

    std::mutex m_mutex;

    std::function < bool(boost::asio::io_service&, request&, response &) > func_;
};

#endif
