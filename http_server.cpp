#include "http_server.h"
#include "http_session.h"

void http_server::do_accept()
{
    while (1)
    {
        session* new_session = new http_session(*this, pool_.get_io_service());
        boost::system::error_code ec;
        acceptor_.accept(new_session->socket(), ec);
        if (!ec)
        {
            std::shared_ptr<session> ss(new_session);
            insert(ss);
            ss->start();
        }
        else
        {
            std::cout << "http_server::acceptor_::async_accept. ec: [" << ec.value() << "] Msg: [" << ec.message() << "]" << std::endl;
        }
    }
}

void http_server::erase(std::shared_ptr<session> p)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = session_.find(p);
    if (it != session_.end())
        session_.erase(it);
}

void http_server::insert(std::shared_ptr<session> &p)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = session_.insert(p);
    assert(it.second == true);
}



