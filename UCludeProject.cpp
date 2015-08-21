#include <iostream>
#include <memory.h>
#include <sys/sysinfo.h>
#include <unistd.h> 


#include "http_server.h"
#include "io_service_pool.h"
#include "LogWrapper.h"
#include "client_manager.h"

int main(int argc, char* argv[])
{
    //如果有--fork参数，那么将变成守护进程
    if (argc == 2)
    {
        if (!strcmp(argv[1], "--fork"))
        {
            if (daemon(1, 1) < 0)
            {
                perror("error daemon.../n");
                exit(1);
            }
        }
    }

    std::shared_ptr<client_manager> cm(new client_manager);
    http_server s(boost::asio::ip::tcp::v4(), 8080, get_nprocs() * 2, 
        [&cm](boost::asio::io_service &io_, request &request_, response &response_)->bool
    {
        return cm->do_some(io_, request_, response_);
    });

    std::cout << "Server running at http://0.0.0.0:8080/" << std::endl;

    s.run();

    return 0;
}