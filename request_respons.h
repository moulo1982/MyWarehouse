#pragma once
#include <map>
#include <iosfwd>
#include "http_parser.h"
#include <json/json.h>

class http_session;

class url_obj
{
    friend class http_session;

public:
    url_obj()
        : handle_(), buf_()
    {
        //printf("url_obj() %x\n", this);
    }

    url_obj(const url_obj& c)
        : handle_(c.handle_), buf_(c.buf_)
    {
        //printf("url_obj(const url_obj&) %x\n", this);
    }

    url_obj& operator =(const url_obj& c)
    {
        //printf("url_obj::operator =(const url_obj&) %x\n", this);
        handle_ = c.handle_;
        buf_ = c.buf_;
        return *this;
    }

    ~url_obj()
    {
        //printf("~url_obj() %x\n", this);
    }

public:
    std::string schema() const
    {
        if (has_schema()) return buf_.substr(handle_.field_data[UF_SCHEMA].off, handle_.field_data[UF_SCHEMA].len);
        return "HTTP";
    }

    std::string host() const
    {
        // TODO: if not specified, use host name
        if (has_schema()) return buf_.substr(handle_.field_data[UF_HOST].off, handle_.field_data[UF_HOST].len);
        return std::string("localhost");
    }

    int port() const
    {
        if (has_path()) return static_cast<int>(handle_.port);
        return (schema() == "HTTP" ? 80 : 443);
    }

    std::string path() const
    {
        if (has_path()) return buf_.substr(handle_.field_data[UF_PATH].off, handle_.field_data[UF_PATH].len);
        return std::string("/");
    }

    std::string query() const
    {
        if (has_query()) return buf_.substr(handle_.field_data[UF_QUERY].off, handle_.field_data[UF_QUERY].len);
        return std::string();
    }

    std::string fragment() const
    {
        if (has_query()) return buf_.substr(handle_.field_data[UF_FRAGMENT].off, handle_.field_data[UF_FRAGMENT].len);
        return std::string();
    }

private:
    void from_buf(const char* buf, std::size_t len, bool is_connect = false)
    {
        // TODO: validate input parameters

        buf_ = std::string(buf, len);
        if (http_parser_parse_url(buf, len, is_connect, &handle_) != 0)
        {
            // failed for some reason
            // TODO: let the caller know the error code (or error message)
            //throw url_parse_exception();
        }
    }

    bool has_schema() const { return handle_.field_set & (1 << UF_SCHEMA); }
    bool has_host() const { return handle_.field_set & (1 << UF_HOST); }
    bool has_port() const { return handle_.field_set & (1 << UF_PORT); }
    bool has_path() const { return handle_.field_set & (1 << UF_PATH); }
    bool has_query() const { return handle_.field_set & (1 << UF_QUERY); }
    bool has_fragment() const { return handle_.field_set & (1 << UF_FRAGMENT); }

private:
    http_parser_url handle_;
    std::string buf_;
};

struct ci_less : std::binary_function < std::string, std::string, bool >
{
    // case-independent (ci) compare_less binary function
    struct nocase_compare : public std::binary_function < unsigned char, unsigned char, bool >
    {
        bool operator()(const unsigned char& c1, const unsigned char& c2) const
        {
            return tolower(c1) < tolower(c2);
        }
    };

    bool operator()(const std::string & s1, const std::string & s2) const
    {
        return std::lexicographical_compare(s1.begin(), s1.end(), // source range
            s2.begin(), s2.end(), // dest range
            nocase_compare()); // comparison
    }
};

class request
{
    friend class http_session;
private:
    request()
        : url_()
        , headers_()
        , body_("")
        , len_(0)
    {
    }

    ~request()
    {
        //printf("~request() %x\n", this);
    }

public:
    const url_obj& url() const { return url_; }

    const std::string& get_header(const std::string& key) const
    {
        auto it = headers_.find(key);
        if (it != headers_.end()) return it->second;
        return default_value_;
    }

    bool get_header(const std::string& key, std::string& value) const
    {
        auto it = headers_.find(key);
        if (it != headers_.end())
        {
            value = it->second;
            return true;
        }
        return false;
    }

    const std::string& get_body(void)
    {
        return body_;
    }

private:
    url_obj url_;
    std::map<std::string, std::string, ci_less> headers_;
    std::string body_;
    //std::vector<char> body_;
    unsigned int len_;
    std::string default_value_;
};



class response
{
    friend class http_session;

private:
    response()
        : headers_()
        , status_(200)
    {
        headers_["Content-Type"] = "text/html";
    }

    ~response()
    {}

public:
    std::string end()
    {
        Json::FastWriter writer;
        body_ = writer.write(root_);

        // Content-Length
        if (headers_.find("Content-Length") == headers_.end())
        {
            std::stringstream ss;
            ss << body_.length();
            headers_["Content-Length"] = ss.str();
        }

        std::stringstream response_text;
        response_text << "HTTP/1.1 ";
        response_text << status_ << " " << get_status_text(status_) << "\r\n";
        for (auto h : headers_)
        {
            response_text << h.first << ": " << h.second << "\r\n";
        }
        response_text << "\r\n";
        response_text << body_;

        return response_text.str();
    }

    void set_status(int status_code)
    {
        status_ = status_code;
    }

    void set_header(const std::string& key, const std::string& value)
    {
        headers_[key] = value;
    }

    static std::string get_status_text(int status)
    {
        switch (status)
        {
        case 100: return "Continue";
        case 101: return "Switching Protocols";
        case 200: return "OK";
        case 201: return "Created";
        case 202: return "Accepted";
        case 203: return "Non-Authoritative Information";
        case 204: return "No Content";
        case 205: return "Reset Content";
        case 206: return "Partial Content";
        case 300: return "Multiple Choices";
        case 301: return "Moved Permanently";
        case 302: return "Found";
        case 303: return "See Other";
        case 304: return "Not Modified";
        case 305: return "Use Proxy";
            //case 306: return "(reserved)";
        case 307: return "Temporary Redirect";
        case 400: return "Bad Request";
        case 401: return "Unauthorized";
        case 402: return "Payment Required";
        case 403: return "Forbidden";
        case 404: return "Not Found";
        case 405: return "Method Not Allowed";
        case 406: return "Not Acceptable";
        case 407: return "Proxy Authentication Required";
        case 408: return "Request Timeout";
        case 409: return "Conflict";
        case 410: return "Gone";
        case 411: return "Length Required";
        case 412: return "Precondition Failed";
        case 413: return "Request Entity Too Large";
        case 414: return "Request-URI Too Long";
        case 415: return "Unsupported Media Type";
        case 416: return "Requested Range Not Satisfiable";
        case 417: return "Expectation Failed";
        case 500: return "Internal Server Error";
        case 501: return "Not Implemented";
        case 502: return "Bad Gateway";
        case 503: return "Service Unavailable";
        case 504: return "Gateway Timeout";
        case 505: return "HTTP Version Not Supported";
        //default: throw response_exception("Not supported status code.");
        }
    }

    Json::Value& getValue()
    {
        return root_;
    }
private:

    std::map<std::string, std::string, ci_less> headers_;
    int status_;
    std::string body_;
    Json::Value root_;
};
