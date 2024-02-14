#include <future>
#include <iostream>
#include <SMTPClient.h>
#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>
#include <atomic>
#include <utility>

using namespace std::chrono_literals;
namespace chrono = std::chrono;

class IPServer {
public:
    IPServer(std::string notifyWho, std::string sendFrom)
        : mailTo(std::move(notifyWho)), mailFrom(std::move(sendFrom)), running(false)
    {}

    static std::string IP() {
        return getIP();
    }

    void stop() {
        running = false;
    }

    void start() {
        if (running) return;
        auto unused = std::async(&IPServer::run, this);
    }

    void start(chrono::duration<chrono::seconds> time) {
        if (running) return;
        run();
    }

private:

    void run(const chrono::duration<chrono::seconds> time = chrono::duration<chrono::seconds>{}.zero(), chrono::duration<chrono::seconds> checkFreq = {}) {
        if (time.count().count()) {
            
        }
        /*while ((time > 0 ? running )) {
            const auto temp = getIP();
            if (ip != temp) {
                ip = temp;
                sendIp();
            }
            std::this_thread::sleep_for(checkFreq.count().zero() ? checkFreq : 1h);;
        }*/
    }

    void sendIp() const {
        try {
            CSMTPClient client{[](const std::string& str){std::clog << str << std::endl;}};
            client.InitSession("smtp.gmail.com:587", "dalaeer@gmail.com", gmailAppPassword,
                CMailClient::SettingsFlag::ALL_FLAGS, CMailClient::SslTlsFlag::ENABLE_TLS);

            client.SendString(mailFrom, mailTo, "", getPcName() + " now has IP address " + ip);
        } catch (std::exception& e) {
            std::cerr << "Exception: " << e.what() << std::endl;
            throw;
        }
    }

    static std::string getIP() {
        try
        {
            curlpp::Easy myRequest;

            std::string ss;
            ss.reserve(400);

            myRequest.setOpt<curlpp::options::Url>(ipApi);
            myRequest.setOpt<curlpp::options::WriteFunction>(
                [&ss](const char* str, long size, long nmemb)
                {std::copy_n(str, size * nmemb, std::back_inserter(ss)); return ss.length();}
                );
            myRequest.perform();
#ifndef NDEBUG
            std::cout << ss << '\n';
#endif
            return ss;
        }
        catch(curlpp::RuntimeError & e)
        {
            std::cerr << e.what() << std::endl;
            throw;
        }
        catch(curlpp::LogicError & e)
        {
            std::cerr << e.what() << std::endl;
            throw;
        }
    }

    static std::string getPcName() {
        static constexpr auto BUFLEN = 200u;
        std::string buffer;
        buffer.reserve(BUFLEN);

        gethostname(&buffer[0], BUFLEN - 1);
        return buffer;
    }

    constexpr static auto ipApi{"https://ipinfo.io/ip"};
    constexpr static auto gmailAppPassword{"zerj pyng aana pflq"};
    std::string mailTo, mailFrom;
    std::string ip;
    std::atomic<bool> running;
};

int main() {
    curlpp::initialize();
    IPServer server{"davidlaeer@gmail.com", "dalaeer@gmail.com"};
    server.start();

}
