#pragma once
#include <json/json.h>
#include <memory>
#include "error_code.h"

class client_interface : public std::enable_shared_from_this < client_interface >
{
public:
    template<typename T>
    std::shared_ptr<T> shared_Derived_from_this() {
        return std::static_pointer_cast<T>(shared_from_this());
    }

    template<typename T>
    std::shared_ptr<T const> shared_Derived_from_this() const {
        return std::static_pointer_cast<T const>(shared_from_this());
    }

    virtual ERROR_CODE init_user_info(int id) = 0;

    virtual ERROR_CODE do_some(Json::Value &root, Json::Value &respons) = 0;

    //服务器统一回复内容
    virtual void get_session_data(Json::Value &respons) = 0;

    virtual ~client_interface(){}
};