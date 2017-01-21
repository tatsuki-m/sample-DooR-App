#include <iostream>
#include <string>
#include <stdio.h>
#include <fstream>
#include <iomanip>
#include <stdlib.h>
#include <time.h>

#include "door_bridge/door_bridge.h"
#include "door_unix_domain_socket_client/unix_domain_socket_client.h"
#include "door_ipc/dpi.h"
#include "door_ipc/shared_packet_information.h"

const int MAX_COUNT = 1000;
std::string BASE_RECORDER_DIR = "/tmp/recorder/";


// 1st argument env docker = 0, native = 1;
// 2nd argument ipc-metor  0: SHM, 1: UDS, 2: TCP, 3: ZERO-SHM
// 3rd argument number
// 4th argument timer flag  0: no timer, 1: timer
int
main(int argc, char* argv[]) {
    if (argc != 5) {
        std::cerr << "Please input argument" << std::endl;
        return 1;
    }

    std::string env;
    std::string ipc;
    std::string fileName;

    // init
    unsigned int counter = 0;
    struct timespec startTime, endTime;
    //init DoorBridge
    DoorBridge bridge = DoorBridge();
    Dpi *dpi;

    switch (atoi(argv[0])) {
        case 0:
            env = "docker";
            break;
        case 1:
            env = "native";
            break;
        default:
            std::cerr << "invalid 1st argument number";
            return 1;
    }

    time_t endwait;
    time_t start = time(NULL);
    time_t seconds = 10; // after 60s, end loop.
    endwait = start + seconds;
    printf("start time is : %s", ctime(&start));

    switch (atoi(argv[2])) {
        case 0:
            ipc = "shm";
            std::cout << "reading start" << std::endl;
            std::cout << "ipc: shm" << std::endl;

            if (atoi(argv[4]) == 0) {
                std::cout << "==================1000 times=================" << std::endl;
                clock_gettime(CLOCK_MONOTONIC, &startTime);
                bridge.callDoorWithSem();
                while(counter<MAX_COUNT) {
                    bridge.getPacketDataWithSem(dpi);
                    counter++;
                }
                clock_gettime(CLOCK_MONOTONIC, &endTime);
            } else {
                std::cout << "==================60 sec timer=================" << std::endl;
                bridge.callDoorWithSem();
                while(time(NULL) < endwait) {
                    bridge.getPacketDataWithSem(dpi);
                }
            }
            break;
        case 1:
            ipc = "uds";
            clock_gettime(CLOCK_MONOTONIC, &startTime);
            clock_gettime(CLOCK_MONOTONIC, &endTime);
            break;
        case 2:
            ipc = "tcp";
            clock_gettime(CLOCK_MONOTONIC, &startTime);
            clock_gettime(CLOCK_MONOTONIC, &endTime);
            break;
        case 3:
            ipc = "zero-shm";
            clock_gettime(CLOCK_MONOTONIC, &startTime);
            clock_gettime(CLOCK_MONOTONIC, &endTime);
            break;
        default:
            std::cerr << "invalid 2nd argument number";
            return 1;
    }

    // create timer
    fileName = BASE_RECORDER_DIR + env + "_" + std::to_string(SharedPacketInformation::getSharedDataSize()) + "_" + ipc + "/"  "throuput_" + argv[3] +".csv";
    std::ofstream ofs(fileName.c_str());
    ofs << "start_time, end_time, interval" << std::endl;
    ofs << std::setfill('1') << std::setw(6) << startTime.tv_nsec << ",";
    ofs << std::setfill('0') << std::setw(6) << endTime.tv_nsec << ",";
    ofs << endTime.tv_nsec - startTime.tv_nsec << std::endl;

    delete dpi;
    std::cout << "fin" << std::endl;
    return 0;
};

