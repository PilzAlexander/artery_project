/********************************************************************************
  \project  INFM_HIL_Interface
  \file     SimSocket.h
  \brief    Provides the functions for setting up a socket to send data from the simulation to the interface component
  \author   Alexander Pilz
  \author   Fabian Genes
  \author   Johannes Winter
  \version  1.0.0
  \date     31.10.2021
 ********************************************************************************/

/********************************************************************************
 * Includes
 *********************************************************************************/
#include "SimSocket.h"
#include <omnetpp/clistener.h>
#include <omnetpp/csimplemodule.h>
#include "traci/CheckTimeSync.h"
#include "traci/BasicNodeManager.h"
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/variant.hpp>
#include "artery/application/VehicleDataProvider.h"
#include "artery/traci/VehicleController.h"
#include "artery/application/VehicleKinematics.h"
#include "artery/application/Middleware.h"
#include "artery/application/StationType.h"

#include <iostream>
#include <utility>
#include <zmq.hpp>
#include <algorithm>
#include <array>
/********************************************************************************
 * Function declarations
 ********************************************************************************/
using namespace std;
using namespace omnetpp;

namespace artery {

Define_Module(SimSocket)

void SimSocket::initialize() {
    //get traci from ModulePath
    cModule *traci = getModuleByPath(par("traciModule"));

    //Subscribe signal to actual Traci
    if (traci) {
        traci->subscribe(traci::BasicNodeManager::updateNodeSignal, this);
    } else {
        throw cRuntimeError("No TraCI module found for signal subscription");
    }

    // set up zmq socket and stuff
    context_ = zmq::context_t(1);
    portName_ = "tcp://*:7777";
    subPortName_ = "tcp://localhost:7778";
    publisherSocket_ = zmq::socket_t(context_, zmq::socket_type::pub);
    subscriberSocket_ = zmq::socket_t(context_, zmq::socket_type::sub);
    //subscriberSocket_.setsockopt(ZMQ_SUBSCRIBE, "", 0);
    subscriberSocket_.connect(subPortName_); // TODO anderer Port als publisher
    bind(portName_);
}

void SimSocket::finish() {
    unbind(portName_);
    cSimpleModule::finish();
}

// constructor without args
SimSocket::SimSocket() {}

// destructor
SimSocket::~SimSocket() {
    close();
}

void SimSocket::close() {
    publisherSocket_.close();
}

// connect the publisher socket to a port
void SimSocket::connect(const PortName &portName) {
    try {
        publisherSocket_.connect(portName_);
        connections_.push_back(portName);
        // DEBUG
        cout << "Connected to Socket: " << portName << endl;
    } catch (zmq::error_t cantConnect) {
        cerr << "Socket can't connect to port: " << cantConnect.what() << endl;
        close();
        return;
    }
}

// disconnect the publisher socket from a port
void SimSocket::disconnect(const PortName &portName) {
    auto connectionIterator = std::find(connections_.begin(), connections_.end(), portName);
    if (connectionIterator == connections_.end()) {
        cerr << "SimSocket::" << portName << "failed to disconnect from SimSocket::" << portName << endl;
        return;
    }
    publisherSocket_.disconnect(portName_);
    connections_.erase(connectionIterator);
}

// bind the publisher socket to a port
void SimSocket::bind(const PortName &portName) {
    publisherSocket_.bind(portName_);
    bindings_.push_back(portName);
}

// unbind the socket from a port
void SimSocket::unbind(const PortName &portName) {

    auto bindingIterator = std::find(bindings_.begin(), bindings_.end(), portName);
    if (bindingIterator == bindings_.end()) {
        cerr << "SimSocket::" << portName << "failed to unbind from SimSocket::" << portName << endl;
        return;
    }

    connections_.erase(bindingIterator);
    publisherSocket_.unbind(portName_);
}

// publish data
void SimSocket::publish() {
    //serialize map
    std::ostringstream ss;
    boost::archive::text_oarchive archive(ss);
    archive << vehicleDataMap;
    archive << vehicleDynamicsMap;
    archive << EventMap;
    std::string outbound_data = ss.str();
    // create buffer size for message
    zmq::message_t msgToSend(outbound_data);

    try {
        //std::cout << "Message: " << msgToSend << endl;
        publisherSocket_.send(msgToSend, zmq::send_flags::none);

    } catch (zmq::error_t cantSend) {
        cerr << "Socket can't send: " << cantSend.what() << endl;
        unbind(portName_);
    }
}

// subscribe to incoming data
void SimSocket::subscribe() {
    zmq::message_t messageToReceive;

    try {
        if(messageToReceive.empty()){
            //std::cout << "No message to receive..." << endl;
            return;
        }else{
            subscriberSocket_.recv(&messageToReceive, ZMQ_NOBLOCK);
        }
    } catch (zmq::error_t cantReceive) {
        cerr << "Socket can't receive: " << cantReceive.what() << endl;
        // TODO unbind
    }
        const char *buf = static_cast<const char*>(messageToReceive.data());
        std::string input_data_( buf, messageToReceive.size() );
        std::istringstream archive_stream(input_data_);
        boost::archive::text_iarchive archive(archive_stream);

        try
        {
            archive >> inputDataMap;
        } catch (boost::archive::archive_exception& ex) {
            std::cout << "Archive Exception during deserializing:" << std::endl;
            std::cout << ex.what() << std::endl;
        } catch (int e) {
            std::cout << "EXCEPTION " << e << std::endl;
        }

        /*
        std::vector<std::string> keyVektor;
        std::vector<boost::variant<int, double, std::string>> valueVektor;
        for (auto const& element: inputDataMap) {
            keyVektor.push_back(element.first);
            valueVektor.push_back(element.second);
            std::string keyAsString = element.first;
            auto valueAsAny =   element.second;
            std::stringstream stringStreamValue ;
            stringStreamValue <<  valueAsAny;
            std::cout << "Key: " << element.first << std::endl;
            std::cout << "value: " << stringStreamValue.str() << std::endl;
        }*/
}

// call in basic node manager to get data and write to a global map
void SimSocket::getVehicleData(std::string vehicleID, TraCIAPI::VehicleScope traci) {
    DataMap map;

    SimTime sendingTime = simTime();
    std::string sendingTime_str = sendingTime.str();

    map.insert(std::pair<std::string, std::string>("origin", "Origin"));
    map.insert(std::pair<std::string, std::string>("current", sendingTime_str));
    map.insert(std::pair<std::string, std::string>("vehicleID_DUT", vehicleID));
    map.insert(std::pair<std::string, double>("Speed_DUT", traci.getSpeed(vehicleID)));
    map.insert(std::pair<std::string, double>("Acceleration_DUT", traci.getAcceleration(vehicleID)));
    map.insert(std::pair<std::string, double>("Angle_DUT", traci.getAngle(vehicleID)));
    map.insert(std::pair<std::string, double>("Distance_DUT", traci.getDistance(vehicleID)));
    map.insert(std::pair<std::string, double>("Height_DUT", traci.getHeight(vehicleID)));
    map.insert(std::pair<std::string, double>("Length_DUT", traci.getLength(vehicleID)));
    map.insert(std::pair<std::string, double>("Width_DUT", traci.getWidth(vehicleID)));
    map.insert(std::pair<std::string, double>("LanePosition_DUT", traci.getLanePosition(vehicleID)));
    map.insert(std::pair<std::string, int>("Signals_DUT", traci.getSignals(vehicleID)));
    map.insert(std::pair<std::string, double>("Position_X-Coordinate_DUT", traci.getPosition(vehicleID).x));
    map.insert(std::pair<std::string, double>("Position_Y-Coordinate_DUT", traci.getPosition(vehicleID).y));
    map.insert(std::pair<std::string, double>("Position_Z-Coordinate_DUT", traci.getPosition(vehicleID).z));
    map.insert(std::pair<std::string, double>("Decel_DUT", traci.getDecel(vehicleID)));
    map.insert(std::pair<std::string, std::string>("RoadID_DUT", traci.getRoadID(vehicleID)));
    map.insert(std::pair<std::string, double>("RouteIndex_DUT", traci.getRouteIndex(vehicleID)));
    map.insert(std::pair<std::string, std::string>("LaneID_DUT", traci.getLaneID(vehicleID)));
    map.insert(std::pair<std::string, double>("LaneIndex_DUT", traci.getLaneIndex(vehicleID)));

/*
    for(const auto& elem : map)
    {
        std::cout << elem.first << " " << elem.second << " " << "\n";
    }
    std::cout << "*****************************************************" << endl;
*/
    vehicleDataMap = map;
}

void SimSocket::getVehicleDynamics(VehicleKinematics dynamics){
    DataMap map;

    if(!isnan(dynamics.speed.value())){
        map.insert(std::pair<std::string, double>("Speed_Dynamics", dynamics.speed.value()));
    }
    if(!isnan(dynamics.yaw_rate.value())){
        map.insert(std::pair<std::string, double>("YawRate_Dynamics", dynamics.yaw_rate.value()));
    }
    if(!isnan(dynamics.acceleration.value())){
        map.insert(std::pair<std::string, double>("Acceleration_Dynamics", dynamics.acceleration.value()));
    }
    if(!isnan(dynamics.heading.value())){
        map.insert(std::pair<std::string, double>("Heading_Dynamics", dynamics.heading.value()));
    }
    if(!isnan(dynamics.geo_position.latitude.value())){
        map.insert(std::pair<std::string, double>("Latitude_Dynamics", dynamics.geo_position.latitude.value()));
    }
    if(!isnan(dynamics.geo_position.longitude.value())){
        map.insert(std::pair<std::string, double>("Longitude_Dynamics", dynamics.geo_position.longitude.value()));
    }
    if(!isnan(dynamics.position.x.value())){
        map.insert(std::pair<std::string, double>("PosX_Dynamics", dynamics.position.x.value()));
    }
    if(!isnan(dynamics.position.y.value())){
        map.insert(std::pair<std::string, double>("PosY_Dynamics", dynamics.position.y.value()));
    }

/*
    for(const auto& elem : map)
    {
        std::cout << elem.first << " " << elem.second << " " << "\n";
    }
    std::cout << "*****************************************************" << endl;
*/
    vehicleDynamicsMap = map;
}

void SimSocket::getOtaInterfaceStub(vanetza::MacAddress& MacSource, vanetza::MacAddress& MacDest, vanetza::byte_view_range& byteViewRange) {
    //std::cout << "SOURCE: " << MacSource << endl;
}

/*
void SimSocket::getEvent(omnetpp::cEvent* event){
    DataMap map;

    map.insert(std::pair<std::string, string>("Event: ",  event->str()));
    map.insert(std::pair<std::string, string>("Name: ",  event->getName()));
    map.insert(std::pair<std::string, string>("TargetObject: ",  event->getTargetObject()->str()));
    map.insert(std::pair<std::string, string>("Owner: ",  event->getOwner()->str()));
    map.insert(std::pair<std::string, string>("Descriptor: ",  event->getDescriptor()->str()));
    map.insert(std::pair<std::string, string>("ArrivalTime: ",  event->getArrivalTime().str()));
    map.insert(std::pair<std::string, int>("PreviousEventNumber: ",  event->getPreviousEventNumber()));
    map.insert(std::pair<std::string, int>("NamePooling: ",  event->getNamePooling()));

    EventMap = map;

    /*
    for(const auto& elem : map)
    {
        std::cout << elem.first << " " << elem.second << " " << "\n";
    }
    std::cout << "*****************************************************" << endl;
*/

    /*
    std::cout << "*****************************" << std::endl;
    std::cout << "String:    " << event->str() << std::endl;
    std::cout << "Name:    " << event->getName() << std::endl;
    std::cout << "Target:    " << event->getTargetObject() << std::endl;
    std::cout << "Owner:    " << event->getOwner() << std::endl;
    std::cout << "Descriptor:    " << event->getDescriptor() << std::endl;
    std::cout << "*****************************" << std::endl;
*/
//}

/*
void SimSocket::setVehicleData(TraCIAPI::VehicleScope traci, DataMap map) {

    // TODO set incoming vehicle data
    auto vehicle = subscriptions_->getVehicleCache("12");

    //TraCIAPI::VehicleScope::setSpeed("flow",1.0);
    map.insert(std::pair<std::string, double>("Speed", traci.getSpeed(vehicleID)));
    map.insert(std::pair<std::string, double>("Acceleration", traci.getAcceleration(vehicleID)));
    map.insert(std::pair<std::string, double>("Angle", traci.getAngle(vehicleID)));
    map.insert(std::pair<std::string, double>("Distance", traci.getDistance(vehicleID)));
    map.insert(std::pair<std::string, double>("Height", traci.getHeight(vehicleID)));
    map.insert(std::pair<std::string, double>("Length", traci.getLength(vehicleID)));
    map.insert(std::pair<std::string, double>("Width", traci.getWidth(vehicleID)));
    map.insert(std::pair<std::string, double>("LanePosition", traci.getLanePosition(vehicleID)));
    map.insert(std::pair<std::string, int>("Signals", traci.getSignals(vehicleID)));
    map.insert(std::pair<std::string, double>("Position_X-Coordinate", traci.getPosition(vehicleID).x));
    map.insert(std::pair<std::string, double>("Position_Y-Coordinate", traci.getPosition(vehicleID).y));
    map.insert(std::pair<std::string, double>("Position_Z-Coordinate", traci.getPosition(vehicleID).z));
    map.insert(std::pair<std::string, double>("Decel", traci.getDecel(vehicleID)));
    map.insert(std::pair<std::string, std::string>("RoadID", traci.getRoadID(vehicleID)));
    map.insert(std::pair<std::string, double>("RouteIndex", traci.getRouteIndex(vehicleID)));
    map.insert(std::pair<std::string, std::string>("LaneID", traci.getLaneID(vehicleID)));
    map.insert(std::pair<std::string, double>("LaneIndex", traci.getLaneIndex(vehicleID)));
}*/

//receive NodeUpdate Signal from BasicNodeManager
void SimSocket::receiveSignal(cComponent*, simsignal_t signal, unsigned long, cObject*)
{
    if (signal == traci::BasicNodeManager::updateNodeSignal) {
        publish();
        subscribe();
    }
}
/********************************************************************************
 * Getter and Setter
 ********************************************************************************/
const SimSocket::PortName &SimSocket::getPortName() const {
    return portName_;
}

const zmq::socket_t &SimSocket::getSocketSim() const {
    return publisherSocket_;
}

const zmq::message_t &SimSocket::getNullMessage() const {
    return nullMessage_;
}

const vector<SimSocket::PortName> &SimSocket::getConnections() const {
    return connections_;
}

const vector<SimSocket::PortName> &SimSocket::getBindings() const {
    return bindings_;
}

const zmq::context_t &SimSocket::getContext() const {
    return context_;
}

}// namespace artery
/********************************************************************************
 * EOF
 ********************************************************************************/