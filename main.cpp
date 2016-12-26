#include <iostream>
#include <string>
#include <stdio.h>
#include <door_bridge/door_bridge.h>
#include <door_bridge/unix_domain_socket_client.h>
#include <door_shared_memory/dpi.h>

int
main() {
    DoorBridge bridge = DoorBridge();
    Dpi *dpi = NULL;
    std::string keyword = "hoge";
    bridge.getAllInformation(dpi, keyword);
    delete dpi;
    return 0;
};

