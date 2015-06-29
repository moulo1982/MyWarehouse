#include "DBConnectPool.h"

const char* keys[] = { "host", "port", "dbname", "user", "password", "connect_timeout", NULL };
//const char* values[] = { "120.132.85.254", "5432", "postgres", "postgres", "Gonzo1982", "10", NULL };
const char* values[] = { "127.0.0.1", "5432", "postgres", "postgres", "Gonzo1982", "10", NULL };

static void
exit_nicely(PGconn *conn)
{
    PQfinish(conn);
    exit(1);
}

DBConnectPool::DBConnectPool(boost::circular_buffer<PGconn*>::capacity_type n) : m_connPool(n)
{
    LOG("Begin DBConnectPool(%d)... \n", n);
    for (size_t i = 1; i <= n; i++)
    {
        PGconn *conn = PQconnectdbParams(keys, values, 0);
        ConnStatusType s = PQstatus(conn);
        if (s != CONNECTION_OK)
        {
            LOG("Connection to database failed: code:%d, msg:%s \n",
                s, PQerrorMessage(conn));
            exit_nicely(conn);
        }

        m_connPool.push_back(conn);
    }
}

std::shared_ptr<PGconn> DBConnectPool::getConn()
{
    std::shared_ptr<PGconn> conn;
    std::unique_lock<std::mutex> lk(m_lock);
    m_empty_cv.wait(lk, [this]{return !m_connPool.empty(); });

    std::weak_ptr<DBConnectPool> wp(shared_from_this());
    conn.reset(m_connPool.front(), [wp](PGconn *conn)
    {
        std::shared_ptr<DBConnectPool> sp(wp.lock());
        if (sp)
            sp->releaseConn(conn);
        else
            PQfinish(conn);
    });
    m_connPool.pop_front(); 

    m_full_cv.notify_one();

    LOG("Begin getConn... conn=%x\n", conn.get());

    return conn;
}

void DBConnectPool::releaseConn(PGconn *conn)
{
    LOG("Begin releaseConn... conn=%x\n", conn);
    std::unique_lock<std::mutex> lk(m_lock);
    m_full_cv.wait(lk, [this]{return !m_connPool.full(); });

    m_connPool.push_back(conn);

    m_empty_cv.notify_one();
}
