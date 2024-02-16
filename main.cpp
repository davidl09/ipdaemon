#include "src/IPServer.hpp"

int main(int argc, char* argv[]) {
    curlpp::initialize();
    chrono::seconds interval = 1h;
    if (argc > 1) {
        auto temp = 0l;
        try {
            std::stringstream(argv[1]) >> temp;
        }
        catch (std::exception& e) {
#ifndef NDEBUG
            std::clog << "Parsing Exception: " << e.what() << std::endl;
#endif
        }

        if (temp) interval = chrono::seconds(temp);
    }
    std::cout << "Monitoring IP address changes every " << interval << std::endl;
    IPServer server{"ipcheck-conf.json", true};
    try {
        server.run(interval);
    }
    catch (std::exception& e) {
        server.notifyError("An error has occured: " + std::string{e.what()});
    }
}
