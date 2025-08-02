#include <iostream>
#include <string>
#include <vector>
#include <map>
#include "PcapFileDevice.h"
#include "Packet.h"
#include "IpAddress.h"
#include "TcpLayer.h"
#include "PayloadLayer.h"

// Base64 解码函数
// C++11 compliant Base64 decoding function
std::string base64_decode(const std::string &in) {
    std::string out;
    std::vector<int> T(256,-1);
    for (int i=0; i<64; i++) T["ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[i]] = i;

    int val=0, valb=-8;
    for (char c : in) {
        if (T[c] == -1) break;
        val = (val << 6) + T[c];
        valb += 6;
        if (valb >= 0) {
            out.push_back(char((val >> valb) & 0xFF));
            valb -= 8;
        }
    }
    return out;
}


// 主解析函数
void parseSmtp(const std::string& pcap_file) {
    pcpp::IFileReaderDevice* reader = pcpp::IFileReaderDevice::getReader(pcap_file);

    if (!reader->open()) {
        std::cerr << "Error opening the pcap file '" << pcap_file << "'" << std::endl;
        return;
    }

    std::cout << "Successfully opened pcap file." << std::endl;

    // 使用 map 来追踪每个 TCP 流的状态
    // Key: source IP + source Port, Value: 状态 (1: 等待用户名, 2: 等待密码)
    std::map<std::string, int> smtp_sessions;

    pcpp::RawPacket rawPacket;
    while (reader->getNextPacket(rawPacket)) {
        pcpp::Packet parsedPacket(&rawPacket);

        if (!parsedPacket.isPacketOfType(pcpp::TCP))
            continue;

        pcpp::TcpLayer* tcpLayer = parsedPacket.getLayerOfType<pcpp::TcpLayer>();
        pcpp::IPv4Layer* ipLayer = parsedPacket.getLayerOfType<pcpp::IPv4Layer>();

        if (tcpLayer == nullptr || ipLayer == nullptr)
            continue;

        // SMTP 通常使用端口 25, 587 (有时 465 用于 SMTPS)
        uint16_t srcPort = tcpLayer->getSrcPort();
        uint16_t dstPort = tcpLayer->getDstPort();
        bool isSmtpPort = (dstPort == 25 || dstPort == 587 || dstPort == 465 ||
                           srcPort == 25 || srcPort == 587 || srcPort == 465);

        if (!isSmtpPort)
            continue;

        pcpp::PayloadLayer* payload = parsedPacket.getLayerOfType<pcpp::PayloadLayer>();
        if (payload == nullptr)
            continue;

        std::string payload_str(reinterpret_cast<const char*>(payload->getPayload()), payload->getPayloadSize());

        // 清理 payload, \r\n 可能会干扰字符串查找
        std::string cleaned_payload;
        for(char c : payload_str) {
            if (c != '\r' && c != '\n') {
                cleaned_payload += c;
            }
        }


        // 服务器响应 "334 VXNlcm5hbWU6" (334 Username:)
        // 客户端发送 AUTH LOGIN
        // 我们将追踪客户端发送的数据
        std::string client_key = ipLayer->getSrcIPAddress().toString() + ":" + std::to_string(srcPort);
        std::string server_key = ipLayer->getDstIPAddress().toString() + ":" + std::to_string(dstPort);

        // 状态 1: 客户端发送了 AUTH LOGIN, 服务器响应 "334 VXNlcm5hbWU6" (Username:)
        if (payload_str.find("AUTH LOGIN") != std::string::npos) {
            std::cout << "Found 'AUTH LOGIN' from " << client_key << ". Waiting for username." << std::endl;
            // 我们需要追踪服务器的响应流，来确认下一步是输入用户名
            smtp_sessions[server_key] = 1; // 标记这个服务器流，下一步期待用户名
        }
        // 服务器响应 "334 UGFzc3dvcmQ6" (334 Password:)
        else if (smtp_sessions.count(client_key) && smtp_sessions[client_key] == 1) {
            // 这通常是服务器对 AUTH LOGIN 的响应 "334 VXNlcm5hbWU6"
            // 或者对用户名的响应 "334 UGFzc3dvcmQ6"
            // 简单起见，我们直接检查 payload 是否是合法的 base64
            if (cleaned_payload.length() > 0) {
                 std::string username = base64_decode(cleaned_payload);
                 std::cout << "----------------------------------------" << std::endl;
                 std::cout << "Potential Username Found!" << std::endl;
                 std::cout << "  From Stream: " << client_key << std::endl;
                 std::cout << "  Base64: " << cleaned_payload << std::endl;
                 std::cout << "  Decoded: " << username << std::endl;
                 std::cout << "----------------------------------------" << std::endl;
                 smtp_sessions[client_key] = 2; // 更新状态为等待密码
            }
        }
        // 客户端发送密码
        else if (smtp_sessions.count(client_key) && smtp_sessions[client_key] == 2) {
             if (cleaned_payload.length() > 0) {
                std::string password = base64_decode(cleaned_payload);
                std::cout << "----------------------------------------" << std::endl;
                std::cout << "Potential Password Found!" << std::endl;
                std::cout << "  From Stream: " << client_key << std::endl;
                std::cout << "  Base64: " << cleaned_payload << std::endl;
                std::cout << "  Decoded: " << password << std::endl;
                std::cout << "----------------------------------------" << std::endl;
                smtp_sessions.erase(client_key); // 会话结束
             }
        }
    }

    reader->close();
    delete reader;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <pcap_file>" << std::endl;
        return 1;
    }

    std::string pcap_file = argv[1];
    parseSmtp(pcap_file);

    return 0;
}
