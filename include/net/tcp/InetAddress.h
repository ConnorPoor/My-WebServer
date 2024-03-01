#ifndef INET_ADDRESS_H
#define INET_ADDRESS_H

#include <arpa/inet.h>
#include <netinet/in.h>
#include <string>
#include <string.h>

class InetAddress {
public:
    explicit InetAddress(uint16_t port = 0);
    explicit InetAddress(std::string ip, uint16_t port);
    explicit InetAddress(const sockaddr_in &addr)
        : addr_(addr)
    {}

    // x.x.x.x
    std::string toIp() const;
    // x.x.x.x:xxxx
    std::string toIpPort() const;
    // xxxx
    uint16_t toPort() const;

    const sockaddr_in *getSockAddr() const { return &addr_; }
    void setSockAddr(const sockaddr_in &addr) { addr_ = addr; }
private:
    sockaddr_in addr_;
};

#endif // INET_ADDRESS_H