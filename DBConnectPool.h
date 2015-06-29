#pragma once
#include <memory>
#include <mutex>
#include <condition_variable>
//µÚÈý·½¿â
#include <boost/circular_buffer.hpp>
#include <libpq-fe.h>
//
#include "LogWrapper.h"

class DBConnectPool : public std::enable_shared_from_this<DBConnectPool>
{
public:
    DBConnectPool(boost::circular_buffer<PGconn*>::capacity_type n);

    ~DBConnectPool()
    {
        LOG("Begin ~DBConnectPool()... \n");
        for (auto conn : m_connPool)
            PQfinish(conn);
        m_connPool.clear();
    }

    std::shared_ptr<PGconn> getConn();

private:
    void releaseConn(PGconn *conn);
private:
    boost::circular_buffer<PGconn*> m_connPool;
    std::mutex m_lock;
    std::condition_variable m_empty_cv;
    std::condition_variable m_full_cv;
};

