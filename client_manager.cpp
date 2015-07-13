#include "client_manager.h"
#include "client.h"

client_manager::client_manager()
{
}


client_manager::~client_manager()
{
}

void client_manager::process_login(boost::asio::io_service &ios, int value, Json::Value &respons)
{
    std::shared_ptr<client_interface> sc;
    ERROR_CODE ec = find_client_use_id(value, sc);

    if (ec)
        respons["errorcode"] = insert_client_use_id(ios, value, sc);

    if (sc)
        sc->get_session_data(respons);
}

void client_manager::process_logic(Json::Value &root, Json::Value &respons)
{
    std::shared_ptr<client_interface> sc;
    ERROR_CODE ec = find_client_use_id(root["UsrID"].asInt(), sc);

    if (!ec)
    {
        ec = sc->do_some(root, respons);
        sc->get_session_data(respons);
    }

    respons["errorcode"] = ec;
}

ERROR_CODE client_manager::find_client_use_id(int id, std::shared_ptr<client_interface> &sc)
{
    ERROR_CODE isfind = USER_NOT_LOGIN;

    std::lock_guard<std::mutex> lock(m_lock);
    auto it = m_client.find(id);
    if (it != m_client.end())
    {
        isfind = SUCCEEDED;
        sc = it->second;
    }

    return isfind;
}

ERROR_CODE client_manager::insert_client_use_id(boost::asio::io_service &ios, int id, std::shared_ptr<client_interface> &sc)
{
    ERROR_CODE ec = find_client_use_id(id, sc);
    if (ec)
    {
        sc = std::shared_ptr<client_interface>(new client(ios, shared_from_this()));
        ec = sc->init_user_info(id);

        if (ec)
            sc.reset();
        else
            return !insert(id, sc) ? UNKNOWN_ERROR : SUCCEEDED;
    }

    return ec;
}
