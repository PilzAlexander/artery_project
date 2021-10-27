//
// Created by vagrant on 10/21/21.
//



#ifndef ARTERY_INTERFACECONNECTION_H
#define ARTERY_INTERFACECONNECTION_H
#include "../../../../../../../../usr/include/c++/8/fstream"
#include "../../../../../../../../usr/include/c++/8/iostream"


class InterfaceConnection {

public:
    static void writeToFile(const std::string path, const std::string vehicleID, TraCIAPI::VehicleScope traci);
    static void closeFile(const std::string path);
    static void openFile(const std::string path);

};


#endif /* ARTERY_INTERFACECONNECTION_H */
