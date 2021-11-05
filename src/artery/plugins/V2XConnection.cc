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
#include "json.hpp"
#include "SimSocket.h"

// Alexander Pilz
#include <thread>
#include <zmq.hpp>

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>

#include "traci/API.h"
#include "traci/BasicNodeManager.h"
#include "traci/CheckTimeSync.h"
#include "traci/Core.h"
#include "traci/ModuleMapper.h"
#include "traci/PersonSink.h"
#include "traci/VariableCache.h"
#include "traci/VehicleSink.h"
/********************************************************************************
 * Declarations
 ********************************************************************************/
// for convenience
using json = nlohmann::json;

static std::ofstream myfile;

static std::map<std::string, double> data;
static std::map<std::string, double> tmp_data;

const std::string filename = "emp.dat";
/******************************************************************************** */
//BOOST_IS_BITWISE_SERIALIZABLE(A)
/********************************************************************************
 * Program Code
 ********************************************************************************/
// Constructor without args
V2XConnection::V2XConnection(){
};

// Deconstructor
V2XConnection::~V2XConnection() = default;

template<typename Archive>
void serialize(Archive &ar, V2XConnection &a, const unsigned int version) {
    ar & a.speed_;
    ar & a.acc_;
    ar & a.angle_;
    ar & a.distance_;
    ar & a.height_;
    ar & a.length_;
    ar & a.width_;
    ar & a.lanePos_;
    ar & a.angle_;
    ar & a.distance_;
    ar & a.height_;
    ar & a.length_;
    ar & a.width_;
    ar & a.lanePos_;
    ar & a.signals_;
    ar & a.posX_;
    ar & a.posY_;
    ar & a.posZ_;
    ar & a.decel_;
    ar & a.roadID_;
    ar & a.roadIndex_;
    ar & a.laneID_;
    ar & a.laneIndex_;
    ar & a.end_;
    /*ar & a.pos3DX_;
    ar & a.pos3DY_;
    ar & a.pos3DZ_;*/
}

void save(std::map<std::string, double> data) {
    std::ofstream dataStream_txt{"/home/vagrant/Desktop/fork_repo/#Text#.txt"};
    std::ofstream dataStream_bin{"/home/vagrant/Desktop/fork_repo/#Bin#.txt"};

    boost::archive::text_oarchive oa_txt{dataStream_txt};
    boost::archive::binary_oarchive oa_bin{dataStream_bin};

    /*
    V2XConnection a{25, 12, 232,13,243,45,6745,23,
                    4354.4,232,23.34,2.3434,232.2,0.076,345.4,45,
                    34,45,23,34.554, "\n"};*/

    V2XConnection a{data["Speed"],data["Acceleration"],data["Angle"],data["Distance"],data["Height"],data["Length"],data["Width"],
                    data["LanePosition"],data["Signals"],data["Position_X-Coordinate"],data["Position_Y-Coordinate"],
                    data["Position_Z-Coordinate"],data["Decel"],data["RoadID"],data["RouteIndex"],data["LaneID"],data["LaneIndex"],
                    "\n"};

    oa_txt << a;
    oa_bin << a;
}

void load() {
    std::ifstream in_dataStream_txt("/home/vagrant/Desktop/fork_repo/#Text#.txt");
    std::ifstream in_dataStream_bin("/home/vagrant/Desktop/fork_repo/#Bin#.txt");

    // create and open an archive for input
    boost::archive::text_iarchive ia_txt(in_dataStream_txt);
    boost::archive::binary_iarchive ia_bin(in_dataStream_bin);

    V2XConnection a;
    //ia_txt >> a;
    ia_bin >> a;

    /*
    std::cout << a.Speed() << std::endl;
    std::cout << a.Acc() << std::endl;
    std::cout << a.Angle() << std::endl;
    std::cout << a.Distance() << std::endl;
    std::cout << a.Height() << std::endl;
    std::cout << a.Length() << std::endl;
    std::cout << a.Width() << std::endl;
    std::cout << a.LanePos() << std::endl;
    std::cout << a.Signals() << std::endl;
    std::cout << a.PosX() << std::endl;
    std::cout << a.PosY() << std::endl;
    std::cout << a.PosZ() << std::endl;
    std::cout << a.Decel() << std::endl;
    std::cout << a.RoadID() << std::endl;
    std::cout << a.RoadIndex() << std::endl;
    std::cout << a.LaneID() << std::endl;
    std::cout << a.LaneIndex() << std::endl;
     */

    /*std::cout << a.Pos3DX() << std::endl;
    std::cout << a.Pos3DY() << std::endl;
    std::cout << a.Pos3DZ() << std::endl;*/
    /*
    std::cout << a.End() << std::endl;
    */
}

//Close txt File
void V2XConnection::closeFile(const std::string path) {
    //std::cout << "Close Path \n";
    //std::cout << path << "\n";
    myfile.close();
}

//Create txt File and initialize map
void V2XConnection::openFile(const std::string path) {
    //open txt file
    myfile.open(path);

    /*
    //Initialize Map
    tmp_data["Speed"] = 0;
    tmp_data["Acceleration"] =  0;
    tmp_data["Angle"] =  0;
    tmp_data["Distance"] =  0;
    tmp_data["Height"] =  0;
    tmp_data["Length"] =  0;
    tmp_data["Width"] =  0;
    tmp_data["LanePosition"] =  0;
    tmp_data["Signals"] =  0;
    tmp_data["Position X-Coordinate"] = 0;
    tmp_data["Position Y-Coordinate"] = 0;
    tmp_data["Position Z-Coordinate"] = 0;
    //data["Route"] = traci.getRoute(vehicleID);
    tmp_data["Decel"] = 0;
    tmp_data["RoadID"] = 0;
    tmp_data["RouteIndex"] = 0;
    //data["LaneChangeState"] = traci.getLaneChangeState(vehicleID, 1);
    tmp_data["LaneID"] = 0;
    tmp_data["LaneIndex"] = 0;
    //data["Leader"] = traci.getLeader(vehicleID, 10.0);
    tmp_data["3DPos X-Coordiante"] = 0;
    tmp_data["3DPos Y-Coordiante"] = 0;
    tmp_data["3DPos Z-Coordiante"] = 0;*/
}

void V2XConnection::initializeMap(){
    //Initialize Map
    data["Speed"] = 0;
    data["Acceleration"] =  0;
    data["Angle"] =  0;
    data["Distance"] =  0;
    data["Height"] =  0;
    data["Length"] =  0;
    data["Width"] =  0;
    data["LanePosition"] =  0;
    data["Signals"] =  0;
    data["Position_X-Coordinate"] = 0;
    data["Position_Y-Coordinate"] = 0;
    data["Position_Z-Coordinate"] = 0;
    //data["Route"] = traci.getRoute(vehicleID);
    data["Decel"] = 0;
    data["RoadID"] = 0;
    data["RouteIndex"] = 0;
    //data["LaneChangeState"] = traci.getLaneChangeState(vehicleID, 1);
    data["LaneID"] = 0;
    data["LaneIndex"] = 0;
    //data["Leader"] = traci.getLeader(vehicleID, 10.0);
    data["3DPos_X-Coordiante"] = 0;
    data["3DPos_Y-Coordiante"] = 0;
    data["3DPos_Z-Coordiante"] = 0;
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

    // create an empty structure (null)
    json jsonNode;

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

    //std::thread sendThread(SimSocket::sendJSON, jsonNode);
   // sendThread.detach();

}

void V2XConnection::writeToMap(std::string path, std::string vehicleID, TraCIAPI::VehicleScope traci) {

    //Check if file is already open
    //if not -> open the file
    if (!myfile.is_open()){
        openFile(path);
    }
    initializeMap();

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
    data["Position_X-Coordinate"] = traci.getPosition(vehicleID).x;
    data["Position_Y-Coordinate"] = traci.getPosition(vehicleID).y;
    data["Position_Z-Coordinate"] = traci.getPosition(vehicleID).z;
    data["Decel"] = traci.getDecel(vehicleID);
    data["RouteIndex"] = traci.getRouteIndex(vehicleID);
    data["LaneIndex"] = traci.getLaneIndex(vehicleID);
    data["3DPos_X-Coordiante"] = traci.getPosition3D(vehicleID).x;
    data["3DPos_Y-Coordiante"] = traci.getPosition3D(vehicleID).y;
    data["3DPos_Z-Coordiante"] = traci.getPosition3D(vehicleID).z;
    //data["Route"] = traci.getRoute(vehicleID);
    //data["Leader"] = traci.getLeader(vehicleID, 10.0);
    //data["LaneChangeState"] = traci.getLaneChangeState(vehicleID, 1);
    //data["LaneID"] = traci.getLaneID(vehicleID);
    //data["RoadID"] = traci.getRoadID(vehicleID);

    //transform data from map into .txt file
    myfile << "Speed: " << data["Speed"] << "\n";
    myfile << "Acceleration: " <<  data["Acceleration"] << "\n";
    myfile << "Angle: " << data["Angle"] << "\n";
    myfile << "Distance: " << data["Distance"] << "\n";
    myfile << "Height: " << data["Height"] << "\n";
    myfile << "Length: " << data["Length"] << "\n";
    myfile << "Width: " << data["Width"] << "\n";
    myfile << "LanePosition: " << data["LanePosition"] << "\n";
    myfile << "Position X-Coordinate: " << data["Position_X-Coordinate"] << "\n";
    myfile << "Position Y-Coordinate: " << data["Position_Y-Coordinate"] << "\n";
    myfile << "Position Z-Coordinate: " << data["Position_Z-Coordinate"] << "\n";
    myfile << "Decel: " << data["Decel"] << "\n";
    myfile << "RouteIndex: " << data["RouteIndex"] << "\n";
    myfile << "RoadID:" << traci.getRoadID(vehicleID);
    myfile << "LaneIndex: " << data["LaneIndex"] << "\n";
    myfile << "LaneID: " << traci.getLaneID(vehicleID);
    myfile << "3DPosition X-Coordinate: " << data["3DPos_X-Coordiante"] << "\n";
    myfile << "3DPosition Y-Coordinate: " << data["3DPos_y-Coordiante"] << "\n";
    myfile << "3DPosition Z-Coordinate: " << data["3DPos_z-Coordiante"] << "\n";
    myfile << "Signals: " << data["Signals"] << "\n";

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

    save(data);
    load();

    //closeFile(path);
}

// EOF