#ifndef IPSERVER_HPP
#define IPSERVER_HPP

#include <iostream>
#include <thread>
#include <sstream>
#include <chrono>
#include <SMTPClient.h>
#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>
#include <nlohmann/json.hpp>

using namespace std::chrono_literals;
namespace chrono = std::chrono;
namespace nlo = nlohmann;
namespace fs = std::filesystem;
namespace ranges = std::ranges;

class IPServer {
public:
    explicit IPServer(std::string configFile);

    void run(const chrono::seconds interval);

private:
    [[nodiscard]] bool configFileChanged() const;

    [[nodiscard]] bool configFileExists() const;

    static void handleConfigFileError(const std::string& message) ;

    void readConfig();

    bool updateIP();

    void sendIp() const;

    std::string getIP();

    static std::string getPcName();

    std::string configFile;
    nlo::json data;
    std::string ip;
};

#endif // IPSERVER_HPP
