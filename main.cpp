#include <iostream>
#include <string>
#include <stdio.h>
#include <door_bridge/door_bridge.h>
#include <door_bridge/unix_domain_socket_client.h>
#include <door_shared_memory/dpi.h>

int
main() {
    DoorBridge bridge = DoorBridge();
    Dpi *dpi;
    std::string keyword = "hoge";
    std::cout << "dpi address:" << &dpi << std::endl;
    bool is_success = bridge.getAllInformation(dpi, keyword);

    if (is_success) {
        std::cout << "Sample App" <<  dpi->id_ << std::endl;
    } else {
        std::cout << "false" << std::endl;
    }
    delete dpi;
    return 0;
};

