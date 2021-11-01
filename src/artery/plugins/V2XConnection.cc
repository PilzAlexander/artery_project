/********************************************************************************
 * Includes
 *********************************************************************************/
#include "artery/traci/Cast.h"
#include "V2XConnection.h"
#include "inet/common/INETMath.h"
#include <iostream>
#include <map>
#include <string>
#include <string_view>
#include <thread>
#include "json.hpp"
#include "SimSocket.h"

/********************************************************************************
 * Declarations
 ********************************************************************************/
// for convenience
using json = nlohmann::json;

// create an empty structure (null)
json jsonNode;

static std::ofstream myfile;
static std::map<std::string, double> data;
/********************************************************************************
 * Program Code
 ********************************************************************************/
// Constructor without args
V2XConnection::V2XConnection(){
};

// Deconstructor
V2XConnection::~V2XConnection() = default;

//Close txt File
void V2XConnection::closeFile(const std::string path) {
    std::cout << "Close Path \n";
    std::cout << path << "\n";
    myfile.close();
}

//Create txt File and initialize map
void V2XConnection::openFile(const std::string path) {
    myfile.open(path);

    //Initialize Map
    data["Speed"] =  0;
    data["Acceleration"] =  0;
    data["Angle"] =  0;
    data["Distance"] =  0;
    data["Height"] =  0;
    data["Length"] =  0;
    data["Width"] =  0;
    data["LanePosition"] =  0;
    data["Signals"] =  0;
}

void V2XConnection::ConvertToJSONFile(nlohmann::json JSON){

    //open new JSON file
    std::ofstream o("/home/vagrant/Desktop/fork_repo/NodeData.json");

    //check if file is open
    //if (o.is_open()) {
        //std::cout << "##################### \n \n";
        //Convert json object to json file
        o << std::setw(2) << JSON << std::endl << "\n";
    //}

    //Close open json file
    o.close();
}

//extract data and write into json object
void V2XConnection::writeToJSON(std::string vehicleID, TraCIAPI::VehicleScope traci) {

    // add data to json
    jsonNode["Speed"] = traci.getSpeed(vehicleID);
    jsonNode["Acceleration"] = traci.getAcceleration(vehicleID);
    jsonNode["Angle"] = traci.getAngle(vehicleID);
    jsonNode["Distance"] = traci.getDistance(vehicleID);
    jsonNode["Height"] = traci.getHeight(vehicleID);
    jsonNode["Length"] = traci.getLength(vehicleID);
    jsonNode["Width"] = traci.getWidth(vehicleID);
    jsonNode["LanePosition"] = traci.getLanePosition(vehicleID);
    jsonNode["Signals"] = traci.getSignals(vehicleID);
    jsonNode["Position X-Coordinate"] = traci.getPosition(vehicleID).x;
    jsonNode["Position Y-Coordinate"] = traci.getPosition(vehicleID).y;
    jsonNode["Position Z-Coordinate"] = traci.getPosition(vehicleID).z;
    jsonNode["Route"] = traci.getRoute(vehicleID);
    jsonNode["Decel"] = traci.getDecel(vehicleID);
    jsonNode["RoadID"] = traci.getRoadID(vehicleID);
    jsonNode["RouteIndex"] = traci.getRouteIndex(vehicleID);
    jsonNode["LaneChangeState"] = traci.getLaneChangeState(vehicleID, 1);
    jsonNode["LaneID"] = traci.getLaneID(vehicleID);
    jsonNode["LaneIndex"] = traci.getLaneIndex(vehicleID);
    jsonNode["Leader"] = traci.getLeader(vehicleID, 10.0);
    jsonNode["3DPos X-Coordiante"] = traci.getPosition3D(vehicleID).x;
    jsonNode["3DPos Y-Coordiante"] = traci.getPosition3D(vehicleID).y;
    jsonNode["3DPos Z-Coordiante"] = traci.getPosition3D(vehicleID).z;
    //j["PersonCapacity"] = traci.getPersonCapacity(vehicleID);
    //j["BestLane"] = traci.getBestLanes(vehicleID);

    //if(simTime() > 30){
        //j["Follower (5m)"] = traci.getFollower(vehicleID, 5);
        //j["Follower (10m)"] = traci.getFollower(vehicleID, 10.0);
        //j["Follower (25m)"] = traci.getFollower(vehicleID, 25.0);
        //j["Follower (50m)"] = traci.getFollower(vehicleID, 50.0);
    //}

    //Converts JSON Object to JSON file
    ConvertToJSONFile(jsonNode);

    //send JSON
    std::cout << "SimTime: " << simTime() << std::endl;
    std::thread sendThread(SimSocket::sendJSON, jsonNode);
    sendThread.detach();
}

void V2XConnection::writeToMap(std::string path, std::string vehicleID, TraCIAPI::VehicleScope traci) {

    //Check if file is already open
    //if not -> open the file

    if (!myfile.is_open()){
        std::cout << "Drinnen \n";
        openFile(path);
    }

    //write data into map
    data["Speed"] = traci.getSpeed(vehicleID);
    data["Acceleration"] = traci.getAcceleration(vehicleID);
    data["Angle"] = traci.getAngle(vehicleID);
    data["Distance"] = traci.getDistance(vehicleID);
    data["Height"] = traci.getHeight(vehicleID);
    data["Length"] = traci.getLength(vehicleID);
    data["Width"] = traci.getWidth(vehicleID);
    data["LanePosition"] = traci.getLanePosition(vehicleID);
    data["Signals"] = traci.getSignals(vehicleID);


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
    //myfile << "\n \n";
    //std::cout << "\n \n";


    //extract signals
    auto signalsNode = traci.getSignals(vehicleID);
    //traci.setSignals("flowNorthSouth.0", 255);

    //Convert 8-bit set to string
    std::string binary = std::bitset<8>(signalsNode).to_string(); //to binary
    //std::cout << "SignalBinary: " << binary << "\n";

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
}

// EOF