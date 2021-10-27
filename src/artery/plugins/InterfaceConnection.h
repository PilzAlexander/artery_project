//
// Created by vagrant on 10/21/21.
//



#ifndef ARTERY_INTERFACECONNECTION_H
#define ARTERY_INTERFACECONNECTION_H
#include "../../../../../../../../usr/include/c++/8/fstream"
#include "../../../../../../../../usr/include/c++/8/iostream"
#include <zmq.hpp>


class InterfaceConnection {

public:
    ~InterfaceConnection();
    InterfaceConnection();
    static void openSocket(int c);
    static void sendMessage(zmq::context_t context, zmq::socket_t socket, TraCIAPI::VehicleScope);
    static int writeToFile(const std::string path, const std::string vehicleID, TraCIAPI::VehicleScope traci);
    static void closeFile(const std::string path);
    static void openFile(const std::string path);

protected:
    zmq::context_t context;
    zmq::socket_t socket;
    const std::string data_zmq;
};


#endif /* ARTERY_INTERFACECONNECTION_H */
