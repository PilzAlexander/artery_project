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
#include <iostream>
#include <map>
#include <string>
#include <string_view>

std::ofstream myfile;

std::map<std::string, double> data { {"VehicleID", 0},
                                     {"Speed", 0},
                                     {"Acceleration", 0},
                                     {"Angle", 0},
                                     {"Distance", 0},
                                     {"Height", 0},
                                     {"Length", 0},
                                     {"Width", 0},
                                     {"Angle", 0},
                                     {"LanePosition", 0},
                                     {"Line", 0},
                                     {"Signals", 0},};

void InterfaceConnection::closeFile(const std::string path) {
    myfile.close();
}

void InterfaceConnection::openFile(const std::string path) {
    myfile.open(path);

    std::cout << "######################################################## \n";

    /*
    //  Socket to talk to clients
    void *context = zmq_ctx_new ();
    void *responder = zmq_socket (context, ZMQ_REP);
    int rc = zmq_bind (responder, "tcp://*:5555");
    assert (rc == 0);
     */
}

int InterfaceConnection::writeToFile(std::string path, std::string vehicleID, TraCIAPI::VehicleScope traci, int cnt) {

    if (cnt == 0){
        std::cout << "drinnen \n";
        openFile(path);
        cnt++;
    }

/*
    std::string vehicleID_string = vehicleID;
    int vehicleID_int;
    std::string vehicleID_int_tmp;
    int tmp_cnt = vehicleID_string.find('w')

    vehicleID_int_tmp = vehicleID_string.substr(tmp_cnt,5);
    std::cout << vehicleID << "\n";
    std::cout << vehicleID_int_tmp;
    //auto& traci = m_api->vehicle;
*/
    //data["VehicleID"] = vehicleID;
    data["Speed"] = traci.getSpeed(vehicleID);
    data["Acceleration"] = traci.getAcceleration(vehicleID);
    data["Angle"] = traci.getAngle(vehicleID);
    data["Distance"] = traci.getDistance(vehicleID);
    data["Height"] = traci.getHeight(vehicleID);
    data["Length"] = traci.getLength(vehicleID);
    data["Width"] = traci.getWidth(vehicleID) ;
    data["LanePosition"] = traci.getLanePosition(vehicleID);
    //data["Line"] = traci.getLine(vehicleID);
    data["Signals"] = traci.getSignals(vehicleID);

    /*
    for(const auto& elem : data)
    {
        std::cout << elem.first << " " << elem.second.first << " " << elem.second.second << "\n";
    }
     */

    std::cout << "\n \n";

    myfile << "Vehicle ID: " << vehicleID << "\n";
    myfile << "Speed: " << traci.getSpeed(vehicleID) << "\n";
    myfile << "Acceleration: " << traci.getAcceleration(vehicleID) << "\n";
    myfile << "Angle: " << traci.getAngle(vehicleID) << "\n";
    myfile << "Distance: " << traci.getDistance(vehicleID) << "\n";
    myfile << "Height: " << traci.getHeight(vehicleID) << "\n";
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
        myfile << "lowBeamHeadlightsOn \n";
    }

    if (signalsNode & 2){
        myfile << "HighBeamHeadlightsOn \n";
    }

    if (signalsNode & 4){
        myfile << "leftTurnSignalOn \n";
    }

    if (signalsNode & 8){
        myfile << "rightTurnSignalOn \n";
    }

    if (signalsNode & 16){
        myfile << "dayTimeRunningLightsOn \n";
    }

    if (signalsNode & 32){
        myfile << "reverseLightOn \n";
    }

    if (signalsNode & 64){
        myfile << "fogLightOn \n";
    }

    if (signalsNode & 128){
        myfile << "parkingLightOn \n";
    }

    myfile << "\n \n";

    /*
    //while (1) {
    char buffer [10];
    zmq_recv (responder, buffer, 10, 0);
    printf ("Received Hello\n");
    sleep (1);          //  Do some 'work'
    zmq_send (responder, "Feinstes Schweeeeeiiiiiiinnn", 5, 0);
    //}
     */

    return cnt;
}
