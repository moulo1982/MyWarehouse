#include <iostream>
#include <sys/sysinfo.h>


#include "http_server.h"
#include "io_service_pool.h"

int main(int argc, char* argv[])
{
    try
    {
        http_server s(tcp::v4(), 8080, get_nprocs() * 2, [](request &request_, response &response_){
            Json::Reader reader;
            Json::Value root;

            if (!reader.parse(request_.get_body(), root, false))
            {
                return -1;
            }

            try
            {
                std::string ClientVersion = root["ClientVersion"].asString();
                std::string SceneId = root["SceneId"].asString();
                int Value = root["Value"].asInt();

                /*std::cout << ClientVersion << std::endl;
                std::cout << SceneId << std::endl;
                std::cout << Value << std::endl;*/
            }
            catch (const std::exception &e)
            {
                std::cout << e.what() << std::endl;
            }



            response_.getValue()["name"] = "My name is haha.";
            response_.getValue()["age"] = "30";
        });

        std::cout << "Server running at http://0.0.0.0:8080/" << std::endl;

        s.run();
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}