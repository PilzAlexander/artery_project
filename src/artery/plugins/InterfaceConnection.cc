//
// Created by vagrant on 10/21/21.
//


#include "artery/traci/VehicleMobility.h"
#include "artery/traci/VehicleController.h"
#include "artery/traci/Cast.h"
#include "artery/traci/VehicleMobility.h"
#include <omnetpp/cwatch.h>
#include "InterfaceConnection.h"

#include <fstream>
#include <zmq.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>


std::ofstream myfile;

InterfaceConnection::InterfaceConnection() {

}

InterfaceConnection::~InterfaceConnection() {
    myfile.close();
}

void InterfaceConnection::closeFile() {
    myfile.close();
}

void InterfaceConnection::openFile(const std::string path) {
    myfile.open(path);

    /*
    //  Socket to talk to clients
    void *context = zmq_ctx_new ();
    void *responder = zmq_socket (context, ZMQ_REP);
    int rc = zmq_bind (responder, "tcp://*:5555");
    assert (rc == 0);
     */
}

void InterfaceConnection::writeToFile(const std::string vehicleID, TraCIAPI::VehicleScope traci) {

    std::cout << vehicleID;
    //auto& traci = m_api->vehicle;

    std::cout << "NodeID: " << vehicleID << "\n";
    std::cout << "Speed: " << traci.getSpeed("flow0.0") << "\n";
    std::cout << "Acceleration: " << traci.getAcceleration("flow0.0") << "\n";
    std::cout << "\n";

    myfile << "Vehicle ID: " << vehicleID << "\n";
    myfile << "Speed: " << traci.getSpeed(vehicleID) << "\n";
    myfile << "Acceleration: " << traci.getAcceleration(vehicleID) << "\n";
    myfile << "Angle: " << traci.getAngle(vehicleID) << "\n";
    myfile << "Distance: " << traci.getDistance(vehicleID) << "\n";
    myfile<< "Height: " << traci.getHeight(vehicleID) << "\n";
    myfile << "Length: " << traci.getLength(vehicleID) << "\n";
    myfile << "Width: " << traci.getWidth(vehicleID) << "\n";
    myfile << "LanePosition: " << traci.getLanePosition(vehicleID) << "\n";
    myfile << "Line: " << traci.getLine(vehicleID) << "\n";
    myfile << "Signals: " << traci.getSignals(vehicleID) << "\n";

    auto signalsNode = traci.getSignals(vehicleID);

    traci.setSignals("flow0.0", 255);

    std::string binary = std::bitset<8>(signalsNode).to_string(); //to binary
    std::cout << "SignalBinary: " << binary << "\n";

    if (signalsNode & 1){
        std::cout << "lowBeamHeadlightsOn \n";
    }

    if (signalsNode & 2){
        std::cout << "HighBeamHeadlightsOn \n";
    }

    if (signalsNode & 4){
        std::cout << "leftTurnSignalOn \n";
    }

    if (signalsNode & 8){
        std::cout << "rightTurnSignalOn \n";
    }

    if (signalsNode & 16){
        std::cout << "dayTimeRunningLightsOn \n";
    }

    if (signalsNode & 32){
        std::cout << "reverseLightOn \n";
    }

    if (signalsNode & 64){
        std::cout << "fogLightOn \n";
    }

    if (signalsNode & 128){
        std::cout << "parkingLightOn \n";
    }

    std::cout << "\n \n";

    /*
    //while (1) {
    char buffer [10];
    zmq_recv (responder, buffer, 10, 0);
    printf ("Received Hello\n");
    sleep (1);          //  Do some 'work'
    zmq_send (responder, "Feinstes Schweeeeeiiiiiiinnn", 5, 0);
    //}
     */
}