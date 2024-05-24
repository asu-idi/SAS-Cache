// This file is the cache server.
// #include <grpcpp/reflection/proto_server_reflection_plugin.h>

#include "../cache/active_cache_service/cache_server.h"


std::unique_ptr<SimpleCache> CacheServiceImpl::cache_;
Filter* CacheServiceImpl::filter_;

int main(int argc, char *argv[]) {
    int NavySize, DramSize;
    bool use_filter = false;
    std::string server_address;
    // Check if the number of parameters is correct
    if (argc < 4 || argc > 5) {
        std::cerr << "Usage: " << argv[0] << " NavySize DramSize ListenAdrr [use_filter]" << std::endl;
        return 1;
    }

    // Parse the NavySize parameter
    try {
        NavySize = std::stoi(argv[1]);
    } catch (const std::invalid_argument& e) {
        std::cerr << "Invalid NavySize parameter: " << e.what() << std::endl;
        return 1;
    }

    // Parse the DramSize parameter
    try {
        DramSize = std::stoi(argv[2]);
    } catch (const std::invalid_argument& e) {
        std::cerr << "Invalid DramSize parameter: " << e.what() << std::endl;
        return 1;
    }

    try {
       server_address = argv[3]; 
    } catch (const std::invalid_argument& e) {
        std::cerr << "Invalid ListenAdrr parameter: " << e.what() << std::endl;
        return 1;
    }

    // Parse the use_filter parameter, if present
    if (argc == 5) {
        if (std::string(argv[4]) == "true") {
            use_filter = true;
        } else if (std::string(argv[4]) == "false") {
            use_filter = false;
        } else {
            std::cerr << "Invalid use_filter parameter. Must be 'true' or 'false'." << std::endl;
            return 1;
        }
    }

    CacheServiceImpl service(NavySize, DramSize, use_filter);

    ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cout << "Server listening on " << server_address << std::endl;
    server->Wait();

    return 0;
}

