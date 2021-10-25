//
// Created by vagrant on 10/21/21.
//

#include "artery/traci/Cast.h"
#include "InterfaceConnection.h"
#include <fstream>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <iostream>
#include <map>
#include <string>
#include <string_view>

std::ofstream myfile;

std::map<std::string, double> data;

// Constructor without args
InterfaceConnection::InterfaceConnection() {}

// Deconstructor
InterfaceConnection::~InterfaceConnection() {

}


void InterfaceConnection::closeFile(const std::string path) {
    myfile.close();
}

void InterfaceConnection::openFile(const std::string path) {
    myfile.open(path);

    std::cout << "######################################################## \n";

    //Initialize Map
    //data["VehicleID"] =  0;
    data["Speed"] =  0;
    data["Acceleration"] =  0;
    data["Angle"] =  0;
    data["Distance"] =  0;
    data["Height"] =  0;
    data["Length"] =  0;
    data["Width"] =  0;
    data["LanePosition"] =  0;
    //data["Line"] =  0;
    data["Signals"] =  0;

}

void InterfaceConnection::writeToFile(std::string path, std::string vehicleID, TraCIAPI::VehicleScope traci) {

    //Check if file is already open
    //if not -> open the file
    if (!myfile.is_open()){
        std::cout << "Drinnen \n";
        openFile(path);
    }

    //extract vehicle_ID (string to double)
    std::string vehicleID_string = vehicleID; //get vehicle id
    double vehicleID_double = 0.0;
    vehicleID_string = vehicleID.substr(4,5);
    vehicleID_double = stod(vehicleID_string);
    std::cout << "################ \n" << vehicleID_string << "\n" << vehicleID_double << "\n" << "########### \n";

    //write data into map
    data["VehicleID"] = vehicleID_double;
    data["Speed"] = traci.getSpeed("flow0.0");
    data["Acceleration"] = traci.getAcceleration("flow0.0");
    data["Angle"] = traci.getAngle("flow0.0");
    data["Distance"] = traci.getDistance("flow0.0");
    data["Height"] = traci.getHeight("flow0.0");
    data["Length"] = traci.getLength("flow0.0");
    data["Width"] = traci.getWidth("flow0.0") ;
    data["LanePosition"] = traci.getLanePosition("flow0.0");
    //data["Line"] = traci.getLine(vehicleID);
    data["Signals"] = traci.getSignals("flow0.0");

    //transform data from map into .txt file
    myfile << "Speed: " << data["Speed"] << "\n";
    myfile << "Acceleration: " <<  data["Acceleration"] << "\n";
    myfile << "Angle: " << data["Angle"] << "\n";
    myfile << "Distance: " << data["Distance"] << "\n";
    myfile << "Height: " << data["Height"] << "\n";
    myfile << "Length: " << data["Length"] << "\n";
    myfile << "Width: " << data["Width"] << "\n";
    myfile << "LanePosition: " << data["LanePosition"] << "\n";
    myfile << "Signals: " << data["Signals"] << "\n";
    std::cout << "\n \n";

    //extract signals
    auto signalsNode = traci.getSignals(vehicleID);
    //traci.setSignals("flow0.0", 255);

    //Convert 8-bit set to string
    std::string binary = std::bitset<8>(signalsNode).to_string(); //to binary
    std::cout << "SignalBinary: " << binary << "\n";

    //check, which bit is set to 1 (on)
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

}

// EOF











