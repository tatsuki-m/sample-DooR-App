#include <iostream>
#include <string>
#include <stdio.h>
#include <fstream>
#include <iomanip>

#include "door_bridge/door_bridge.h"
#include "door_unix_domain_socket_client/unix_domain_socket_client.h"
#include "door_ipc/dpi.h"
#include "door_ipc/shared_packet_information.h"

const int MAX_COUNT = 1000;
std::string BASE_RECORDER_DIR = "/tmp/recorder/";
std::string ENV = "native";

int
main() {
    // init
    unsigned int counter = 0;
    struct timespec startTime, endTime;
    // init DoorBridge
    DoorBridge bridge = DoorBridge();

    Dpi *dpi;
    // create timer
    std::string fileName = BASE_RECORDER_DIR + ENV + "_" + std::to_string(SharedPacketInformation::getSharedDataSize()) + "_throuput.csv";
    std::ofstream ofs(fileName.c_str());
    ofs << "start_time, end_time, interval" << std::endl;


    clock_gettime(CLOCK_MONOTONIC, &startTime);
    bridge.callDoorWithSem();
    std::cout << "reading start" << std::endl;
    while(counter<MAX_COUNT) {
        bridge.getPacketDataWithSem(dpi);
        counter++;
    }
    clock_gettime(CLOCK_MONOTONIC, &endTime);
    ofs << std::setfill('0') << std::setw(6) << startTime.tv_nsec << ",";
    ofs << std::setfill('0') << std::setw(6) << endTime.tv_nsec << ",";
    ofs << endTime.tv_nsec - startTime.tv_nsec << std::endl;

    delete dpi;
    return 0;
};

