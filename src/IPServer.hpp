#ifndef IPSERVER_HPP
#define IPSERVER_HPP

#include <iostream>
#include <thread>
#include <sstream>
#include <chrono>
#include <condition_variable>
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

    virtual void run(chrono::seconds interval);

    void notifyError(const std::string& errMessage) const;

    virtual ~IPServer();

protected:
    void runOnce();

    [[nodiscard]] bool configFileChanged() const;

    [[nodiscard]] bool configFileExists() const;

    static void handleConfigFileError(const std::string& message) ;

    void readConfig();

    bool updateIP();

    void sendIp() const;

    void sendToRecipient(const std::string& message) const;

    void sendToSource(const std::string& errMessage) const;

    std::string getIP();

    static std::string getPcName();

    std::string configFile;
    nlo::json data;
    std::string ip;
};

class AsyncIPServer final : public IPServer {
public:
    explicit AsyncIPServer(const std::string& configFile);

    virtual ~AsyncIPServer();

    void run(chrono::seconds interval);

    void stop();

private:
    std::atomic_bool running;
};

#endif // IPSERVER_HPP
