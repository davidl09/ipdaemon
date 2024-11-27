#include "IPServer.hpp"
#include <unistd.h>

IPServer::IPServer(const std::string& configFile, bool logging)
    : configDir(fs::path(getenv("HOME")) / ".ipcheck"), configFile(configDir / configFile), log(logging) {
    makeHiddenDir();
    readConfig();
    ip = getIP();
    if (log) {
        openLogFile();
    }
}

void IPServer::run(const chrono::seconds interval) {
    while (true) {
        try {
            runOnce();
        } catch (const std::exception& e) {
            notifyError(e.what());
            break;
        }
        std::this_thread::sleep_for(interval);
    }
}

void IPServer::runOnce() {
    if (const char* home = getenv("HOME"); !home) {
        throw std::runtime_error("HOME environment variable not set");
    }
    
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

[[nodiscard]] bool IPServer::configFileChanged() {
    try {
        nlo::json temp;
        std::ifstream config(configFile);
        if (!config.is_open()) {
            throw std::runtime_error("Failed to open config file for reading");
        }
        config >> temp;
        
        *this << "Read updated json: " << temp << '\n';
        
        return temp != data;
    } catch (const nlo::json::exception& e) {
        throw std::runtime_error(std::string("JSON parsing error: ") + e.what());
    }
}

bool IPServer::configFileExists() const {
    return fs::exists(configFile);
}

void IPServer::handleConfigFileError(const std::string& message) {

    throw std::invalid_argument(message);
}

void IPServer::readConfig() {
    if (not configFileExists())
        handleConfigFileError(
            "Aborting: config file '" + configFile.string() + "' could not be found. Please place the config file into '" + configDir.string() + "/.ipcheck/', then restart ipcheck."
            );
    std::fstream(configFile) >> data;

    *this << "Read from " + configFile.string() + ": " << data << '\n';

    static constexpr std::array requiredKeys = {"gmailAppPassWord", "mailTo", "mailFrom", "ipApi", "smtpServer"};
    ranges::for_each(requiredKeys, [this](const auto& key) {
        if (not data.contains(key)) throw std::invalid_argument("Error reading '" + configFile.string() + "': missing '" + key + "'");
    });
}

bool IPServer::updateIP() {
    if (const auto temp = getIP(); temp != ip) {
        ip = temp;
        return true;
    }
    return false;
}

void IPServer::sendIp() {
    sendToRecipient("Subject: " + getPcName() + "\n\n Updated IP address " + ip);
}

void IPServer::sendToRecipient(const std::string& message) {
    try {
        CSMTPClient client{[](const std::string& str) {}};
        client.InitSession(data["smtpServer"], "dalaeer@gmail.com", data["gmailAppPassWord"],
            CMailClient::SettingsFlag::ALL_FLAGS, CMailClient::SslTlsFlag::ENABLE_TLS);

        client.SendString(data["mailFrom"], data["mailTo"], "", message);
    } catch (std::exception& e) {
        *this << "Exception: " << e.what() << std::endl;
        throw;
    }
}

void IPServer::sendToSource(const std::string& errMessage) {
    try {
        CSMTPClient client{[](const std::string& str) {
        }};
        client.InitSession(data["smtpServer"], "dalaeer@gmail.com", data["gmailAppPassWord"],
            CMailClient::SettingsFlag::ALL_FLAGS, CMailClient::SslTlsFlag::ENABLE_TLS);

        client.SendString(data["mailFrom"], data["mailFrom"], "", errMessage);
    } catch (std::exception& e) {
        *this << "Exception: " << e.what() << std::endl;
        throw;
    }
}

void IPServer::notifyError(const std::string& errMessage) {
    *this << "Error: " << errMessage << '\n';
    sendToSource(errMessage);
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
        *this << ss << '\n';
        return ss;
    }
    catch(curlpp::RuntimeError & e)
    {
        *this << e.what() << std::endl;
        throw;
    }
    catch(curlpp::LogicError & e)
    {
        *this << e.what() << std::endl;
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

void IPServer::makeHiddenDir() {
    fs::create_directory(fs::path(getenv("HOME")) / ".ipcheck");
}

std::ofstream IPServer::openLogFile() {
    return std::ofstream{configDir / "logs.txt", std::ios::out};

}

AsyncIPServer::AsyncIPServer(const std::string& configFile)
    : IPServer(configFile), running(false) {
}

void AsyncIPServer::run(chrono::seconds interval) {
    if (running) return; //do not start more than one thread
    running = true;
    auto task = [this, interval]() {
        while (running) {
            try {
                runOnce();
            } catch (const std::exception& e) {
                notifyError(e.what());
                running = false;
                break;
            }
            std::this_thread::sleep_for(interval);
        }
    };

    std::thread(task).detach();
}

void AsyncIPServer::stop() {
    *this << "Stopping" << std::endl;
    running = false;
}

AsyncIPServer::~AsyncIPServer() = default;
