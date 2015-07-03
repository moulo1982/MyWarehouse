#pragma once
#include <memory>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

class session : public std::enable_shared_from_this < session >
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

    virtual void start() = 0;
    virtual tcp::socket& socket() = 0;
    virtual ~session(){};
};