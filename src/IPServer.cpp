#include "IPServer.hpp"

#include <utility>

IPServer::IPServer(std::string configFile)
    : configFile(std::move(configFile))
{
    readConfig();
    ip = getIP();
}

IPServer::~IPServer() {

}


void IPServer::run(const chrono::seconds interval) {
    while (true) {
        runOnce();
        std::this_thread::sleep_for(interval);
    }
}

void IPServer::notifyError(const std::string& errMessage) const {
    sendToSource(errMessage);
}


void IPServer::runOnce() {
    if (not configFileExists()) {
        handleConfigFileError("Could not open config file, was it deleted?");
    }
    if (configFileChanged()) {
        readConfig();
    }
    if (updateIP()) {
        sendIp();
    }
}

[[nodiscard]] bool IPServer::configFileChanged() const {
    nlo::json temp;
    std::fstream(configFile) >> temp;
#ifndef NDEBUG
    std::clog << "Read updated json: " << temp << '\n';
#endif
    return temp != data;
}

bool IPServer::configFileExists() const {
    return fs::exists(configFile);
}

void IPServer::handleConfigFileError(const std::string& message) {
    throw std::invalid_argument(message);
}

void IPServer::readConfig() {
    if (not configFileExists()) handleConfigFileError("Aborting, config file '" + configFile + "' could not be found");
    std::fstream(configFile) >> data;
#ifndef NDEBUG
    std::clog << "Read from ipcheck-conf.json: " << data << '\n';
#endif
    static constexpr std::array requiredKeys = {"gmailAppPassWord", "mailTo", "mailFrom", "ipApi", "smtpServer"};
    ranges::for_each(requiredKeys, [this](const auto& key) {
        if (not data.contains(key)) throw std::invalid_argument("Error reading '" + configFile + "': missing '" + key + "'");
    });
}

bool IPServer::updateIP() {
    const auto temp = getIP();
    if (temp != ip) {
        ip = temp;
        return true;
    }
    return false;
}

void IPServer::sendIp() const {
    sendToRecipient("Subject: " + getPcName() + "\n\n Updated IP address " + ip);
}

void IPServer::sendToRecipient(const std::string& message) const {
    try {
        CSMTPClient client{[](const std::string& str) {
#ifndef NDEBUG
            std::clog << str << std::endl;
#endif
        }};
        client.InitSession(data["smtpServer"], "dalaeer@gmail.com", data["gmailAppPassWord"],
            CMailClient::SettingsFlag::ALL_FLAGS, CMailClient::SslTlsFlag::ENABLE_TLS);

        client.SendString(data["mailFrom"], data["mailTo"], "", message);
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        throw;
    }
}

void IPServer::sendToSource(const std::string& errMessage) const {
    try {
        CSMTPClient client{[](const std::string& str) {
#ifndef NDEBUG
            std::clog << str << std::endl;
#endif
        }};
        client.InitSession(data["smtpServer"], "dalaeer@gmail.com", data["gmailAppPassWord"],
            CMailClient::SettingsFlag::ALL_FLAGS, CMailClient::SslTlsFlag::ENABLE_TLS);

        client.SendString(data["mailFrom"], data["mailFrom"], "", errMessage);
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        throw;
    }
}

std::string IPServer::getIP() {
    try
    {
        curlpp::Easy myRequest;

        std::string ss;
        ss.reserve(400);

        myRequest.setOpt<curlpp::options::Url>(data["ipApi"]);
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

std::string IPServer::getPcName() {
    constexpr size_t MAX_HOSTNAME_LEN = 256; // Maximum length for a hostname (RFC 1035)
    std::string buffer(MAX_HOSTNAME_LEN, '\0'); // Initialize with null characters
    const int ret = gethostname(&buffer[0], MAX_HOSTNAME_LEN);
    if (ret == -1) {
        throw std::runtime_error("Failed to retrieve hostname");
    }

    return buffer;
}

AsyncIPServer::AsyncIPServer(const std::string& configFile)
    : IPServer(configFile), running(false) {

}

void AsyncIPServer::run(chrono::seconds interval) {
    if (running) return; //do not start more than one thread
    running = true;
    auto task = [this, interval]() {
        while (running) {
            runOnce();
            std::this_thread::sleep_for(interval);
        }
    };

    std::thread(task).detach();
}

void AsyncIPServer::stop() {
#ifndef NDEBUG
    std::clog << "Stopping" << std::endl;
#endif
    running = false;
}

AsyncIPServer::~AsyncIPServer() {

}




