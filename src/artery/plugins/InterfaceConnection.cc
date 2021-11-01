//
// Created by vagrant on 10/21/21.
//

#include "artery/traci/Cast.h"
#include "InterfaceConnection.h"

#include "inet/common/INETMath.h"
#include "inet/mobility/single/TractorMobility.h"

#include <iostream>
#include <map>
#include <string>
#include <string_view>
#include "json.hpp"


std::ofstream myfile;
std::map<std::string, double> data;

// for convenience
using json = nlohmann::json;

// write prettified JSON to another file
std::ofstream o("/home/vagrant/Desktop/fork_repo/NodeData.json");


// Constructor without args
InterfaceConnection::InterfaceConnection() {
}

// Deconstructor
InterfaceConnection::~InterfaceConnection() {
    //auto path = "/home/vagrant/Desktop/fork_repo/Test1.txt";
    //std::cout << "Close Path \n";
    //std::cout << path << "\n";

    //closeFile(path);
}


void InterfaceConnection::closeFile(const std::string path) {
    std::cout << "Close Path \n";
    std::cout << path << "\n";
    myfile.close();

    //Close jSON file
    o.close();
}

void InterfaceConnection::openFile(const std::string path) {
    myfile.open(path);

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
    /*
    if (!myfile.is_open()){
        std::cout << "Drinnen \n";
        openFile(path);
    }*/

    // create an empty structure (null)
    json j;

    //extract vehicle_ID (string to double)
    std::string vehicleID_string = vehicleID; //get vehicle id
    double vehicleID_double = 0.0;
    vehicleID_string = vehicleID.substr(4,5);
    vehicleID_double = stod(vehicleID_string);
    std::cout << "################ \n" << vehicleID_string << "\n" << vehicleID_double << "\n" << "########### \n";

    //write data into map
    data["VehicleID"] = vehicleID_double;
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

     */
    //extract signals
    auto signalsNode = traci.getSignals(vehicleID);
    //traci.setSignals("flow0.0", 255);

    //Convert 8-bit set to string
    std::string binary = std::bitset<8>(signalsNode).to_string(); //to binary
    //std::cout << "SignalBinary: " << binary << "\n";

    /*
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
    //closeFile(path);
     */

    // add a number that is stored as double (note the implicit conversion of j to an object)
    j["Speed"] = traci.getSpeed(vehicleID);
    j["Acceleration"] = traci.getAcceleration(vehicleID);
    j["Angle"] = traci.getAngle(vehicleID);
    j["Distance"] = traci.getDistance(vehicleID);
    j["Height"] = traci.getHeight(vehicleID);
    j["Length"] = traci.getLength(vehicleID);
    j["Width"] = traci.getWidth(vehicleID);
    j["LanePosition"] = traci.getLanePosition(vehicleID);
    j["Signals"] = traci.getSignals(vehicleID);
    j["Position X-Coordinate"] = traci.getPosition(vehicleID).x;
    j["Position Y-Coordinate"] = traci.getPosition(vehicleID).y;
    j["Position Z-Coordinate"] = traci.getPosition(vehicleID).z;
    j["Route"] = traci.getRoute(vehicleID);

    if(simTime() > 30){
        //j["Follower (5m)"] = traci.getFollower(vehicleID, 5);
        //j["Follower (10m)"] = traci.getFollower(vehicleID, 10.0);
        //j["Follower (25m)"] = traci.getFollower(vehicleID, 25.0);
        //j["Follower (50m)"] = traci.getFollower(vehicleID, 50.0);
    }

    j["Decel"] = traci.getDecel(vehicleID);
    //j["ApparentDecel"] = traci.getApparentDecel(vehicleID);
    //j["EmergencyDecel"] = traci.getEmergencyDecel(vehicleID);
    //j["PersonCapacity"] = traci.getPersonCapacity(vehicleID);
    j["RoadID"] = traci.getRoadID(vehicleID);
    j["RouteIndex"] = traci.getRouteIndex(vehicleID);
    j["LaneChangeState"] = traci.getLaneChangeState(vehicleID, 1);
    //j["BestLane"] = traci.getBestLanes(vehicleID);
    j["LaneID"] = traci.getLaneID(vehicleID);
    j["LaneIndex"] = traci.getLaneIndex(vehicleID);
    j["Leader"] = traci.getLeader(vehicleID, 10.0);
    j["3DPos X-Coordiante"] = traci.getPosition3D(vehicleID).x;
    j["3DPos Y-Coordiante"] = traci.getPosition3D(vehicleID).y;
    j["3DPos Z-Coordiante"] = traci.getPosition3D(vehicleID).z;

    if (o.is_open()) {
        o << std::setw(2) << j << std::endl << "\n";
    }
}

// EOF











