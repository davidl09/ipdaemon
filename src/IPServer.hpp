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

template <typename Streamable>
    concept Stremable = requires(std::ostream& ostream, Streamable S) {ostream << S;};

class IPServer {
public:
    explicit IPServer(const std::string& configFile, bool logging = true);

    virtual void run(chrono::seconds interval);

    void notifyError(const std::string& errMessage);

    virtual ~IPServer();

protected:
    void runOnce();

    [[nodiscard]] bool configFileChanged();

    [[nodiscard]] bool configFileExists() const;

    static void handleConfigFileError(const std::string& message) ;

    void readConfig();

    bool updateIP();

    void sendIp();

    void sendToRecipient(const std::string& message);

    void sendToSource(const std::string& errMessage);

    std::string getIP();

    static std::string getPcName();

    static void makeHiddenDir();

    std::ofstream openLogFile();

    template <class Streamable>
    friend std::ostream& operator<<(IPServer& server, Streamable output) {

        auto getTime = []() -> std::string {
            const time_t now = chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            const std::tm* time_info = std::localtime(&now);
            char time_buf[256];
            std::strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", time_info);
            return {time_buf, time_buf + sizeof(time_buf)};
        };

        if (server.logger && server.log) {

            server.logger << "[" << getTime() + "]\t\t";
            server.logger << output << std::endl;
        }
        return server.logger;
    }

    const fs::path configDir;
    const fs::path configFile;
    nlo::json data;
    std::string ip;
    std::ofstream logger;
    bool log;
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
