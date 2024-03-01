#include "InetAddress.h"

#include <iostream>

int InetAddress_test() {
    InetAddress address((uint16_t)9090);

    std::cout << "The ip is " << address.toIp() << std::endl;
    std::cout << "The port is " << address.toPort() << std::endl;
    std::cout << "The ipport is " << address.toIpPort() << std::endl;
    
    return 0;
}