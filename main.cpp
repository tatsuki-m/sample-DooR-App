#include <iostream>
#include <string>
#include <stdio.h>
#include <fstream>
#include <iomanip>
#include <stdlib.h>
#include <time.h>

#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/sync/interprocess_semaphore.hpp>

#include "door_bridge/door_bridge.h"
#include "door_unix_domain_socket_client/unix_domain_socket_client.h"
#include "door_ipc/dpi.h"
#include "door_ipc/shared_packet_information.h"
#include "door_ipc/shared_packet_information.h"
#include "door_ipc/dpi.h"

const int MAX_COUNT = 1000;
std::string BASE_RECORDER_DIR = "/tmp/recorder/";

// 1st argument env docker = 0, native = 1;
// 2nd argument ipc-metor  0: SHM, 1: UDS, 2: TCP, 3: ZERO-SHM
// 3rd argument timer flag  0: no timer, other
int
main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cerr << "3 arguments are needed" << std::endl;
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

    std::cout << atoi(argv[1]);

    switch (atoi(argv[1])) {
        case 0:
            env = "native";
            break;
        case 1:
            env = "docker";
            break;
        default:
            std::cerr << "invalid 1st argument number";
            return 1;
    }

    time_t endwait;
    time_t start = time(NULL);
    time_t seconds = atoi(argv[3]);
    endwait = start + seconds;
    std::cout << "reading start" << std::endl;
    printf("%s\n", ctime(&start));

    switch (atoi(argv[2])) {
        case 0:
            ipc = "shm";
            std::cout << "======ipc: Shared Memory======" << std::endl;

            if (atoi(argv[3]) == 0) {
                std::cout << "==================1000 times=================" << std::endl;
                clock_gettime(CLOCK_MONOTONIC, &startTime);
                bridge.callDoorWithSem();
                while(counter<MAX_COUNT) {
                    Dpi *dpi;
                    bridge.getPacketDataWithSem(dpi);
                    delete dpi;
                    counter++;
                }
                clock_gettime(CLOCK_MONOTONIC, &endTime);
            } else {
                std::cout << "==================" << seconds << "se sec timer=================" << std::endl;
                bridge.callDoorWithSem();
                while(time(NULL) < endwait) {
                    Dpi *dpi;
                    bridge.getPacketDataWithSem(dpi);
                    delete dpi;
                }
            }
            break;
        case 1:
            std::cout << "======ipc: Unix Domain Socket======" << std::endl;
            {
                ipc = "uds";
                if (atoi(argv[3]) == 0) {
                    clock_gettime(CLOCK_MONOTONIC, &startTime);
                    bridge.callDoorWithUds();
                    //get shared memory name
                    std::string sharedKey = bridge.shareKey();

                    std::cout << "==================1000 times=================" << std::endl;
                    while(counter<MAX_COUNT) {
                        SocketClient socket = SocketClient(sharedKey);
                        socket.run();
                        socket.handle();
                        socket.getDpi(dpi);
                        socket.closeSocket();
                        counter++;
                    }
                    clock_gettime(CLOCK_MONOTONIC, &endTime);
                } else {
                    std::cout << "==================" << seconds << "se sec timer=================" << std::endl;
                    bridge.callDoorWithUds();
                    //get shared memory name
                    std::string sharedKey = bridge.shareKey();
                    while(time(NULL) < endwait) {
                        bridge.getPacketDataWithUds(dpi);
                        SocketClient socket = SocketClient(sharedKey);
                        socket.run();
                        socket.handle();
                        socket.getDpi(dpi);
                        socket.closeSocket();
                    }
                }
            }
            break;
        case 2:
            ipc = "tcp";
            clock_gettime(CLOCK_MONOTONIC, &startTime);
            clock_gettime(CLOCK_MONOTONIC, &endTime);

            if (atoi(argv[3]) == 0) {
            } else {
            }
            break;
        case 3:
            ipc = "zero-shm";
            std::cout << "======ipc: Zero-Copy-Shm======" << std::endl;
            {
                clock_gettime(CLOCK_MONOTONIC, &startTime);
                bridge.callDoorWithSem();

                //get shared memory name
                std::string sharedKey = bridge.shareKey();
                shared_memory_object shm(open_only, sharedKey.c_str(), read_write);
                mapped_region region(shm, read_write);
                void *addr = region.get_address();
                SharedPacketInformation* sharedBuffer = static_cast<SharedPacketInformation*>(addr);
                if (atoi(argv[3]) == 0) {
                    std::cout << "==================1000 times=================" << std::endl;
                    while(counter<MAX_COUNT) {
                        sharedBuffer->reader_.wait();
                            dpi = &(sharedBuffer->sharedData_);
                        sharedBuffer->writer_.post();
                        counter++;
                    }
                    clock_gettime(CLOCK_MONOTONIC, &endTime);
                } else if (atoi(argv[3]) < 60) {
                    std::cout << "=================="<< seconds << "sec timer=================" << std::endl;
                    bridge.callDoorWithSem();
                    while(time(NULL) < endwait) {
                        sharedBuffer->reader_.wait();
                            dpi = &(sharedBuffer->sharedData_);
                        sharedBuffer->writer_.post();
                    }
                } else {
                    std::cout << "================== throughput evaluation: 1000 times =================" << std::endl;
                    while(counter<MAX_COUNT) {
                        sharedBuffer->reader_.wait();
                            dpi = &(sharedBuffer->sharedData_);
                        sharedBuffer->writer_.post();
                        counter++;
                    }
                    clock_gettime(CLOCK_MONOTONIC, &endTime);
                    std::cout << "============= throughput evalucation fin==============" << std::endl;

                    fileName = BASE_RECORDER_DIR + "multi_" + env + "_" + std::to_string(SharedPacketInformation::getSharedDataSize()) + "_" + ipc + "/throuput" + ".csv";
                    std::cout << fileName << std::endl;
                    std::ofstream ofs(fileName.c_str(), std::ios::app);
                    ofs << std::setfill('0') << std::setw(6) << startTime.tv_nsec << ",";
                    ofs << std::setfill('0') << std::setw(6) << endTime.tv_nsec << ",";
                    ofs << endTime.tv_nsec - startTime.tv_nsec << std::endl;

                    std::cout << "============= loop start==============" << std::endl;
                    while(time(NULL) < endwait) {
                        sharedBuffer->reader_.wait();
                            dpi = &(sharedBuffer->sharedData_);
                        sharedBuffer->writer_.post();
                    }
                    std::cout << "============= loop fin==============" << std::endl;
                }
            }
            break;
        default:
            std::cerr << "invalid 2nd argument number";
            return 1;
    }

    // no timer
    if (atoi(argv[3]) == 0) {
        // create timer
        fileName = BASE_RECORDER_DIR + env + "_" + std::to_string(SharedPacketInformation::getSharedDataSize()) + "_" + ipc + "/"  "throuput" + ".csv";
        std::cout << fileName << std::endl;
        std::ofstream ofs(fileName.c_str(), std::ios::app);
        if (atoi(argv[2]) == 1) {
            // socket 1MB takes more than 1sec
            ofs << std::setfill('1') << std::setw(6) << startTime.tv_sec << "." << std::setfill('1') << std::setw(6) << startTime.tv_nsec << ",";
            ofs << std::setfill('1') << std::setw(6) << endTime.tv_sec << "." << std::setfill('1') << std::setw(6) << endTime.tv_nsec << "," << std::endl;
        } else {
            ofs << std::setfill('0') << std::setw(6) << startTime.tv_nsec << ",";
            ofs << std::setfill('0') << std::setw(6) << endTime.tv_nsec << ",";
            ofs << endTime.tv_nsec - startTime.tv_nsec << std::endl;
        }
    }

    /*
    // measurement for multi-container
    if (atoi(argv[3]) >= 60) {
        // create timer
        fileName = BASE_RECORDER_DIR + "multi_" + env + "_" + std::to_string(SharedPacketInformation::getSharedDataSize()) + "_" + ipc + "/throuput" + ".csv";
        std::cout << fileName << std::endl;
        std::ofstream ofs(fileName.c_str(), std::ios::app);
        ofs << std::setfill('0') << std::setw(6) << startTime.tv_nsec << ",";
        ofs << std::setfill('0') << std::setw(6) << endTime.tv_nsec << ",";
        ofs << endTime.tv_nsec - startTime.tv_nsec << std::endl;
    }
    */

    dpi=NULL;
    delete dpi;
    std::cout << "===================fin====================" << std::endl;
    return 0;
};

