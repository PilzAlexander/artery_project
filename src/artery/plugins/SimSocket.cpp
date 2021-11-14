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
#include <iostream>
#include <utility>
#include <zmq.hpp>
#include <thread>
#include "traci/CheckTimeSync.h"
#include "traci/BasicNodeManager.h"
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/variant.hpp>

#include <algorithm>
#include <array>
#include <iostream>
#include <utility>
#include <zmq.hpp>
/********************************************************************************
 * Function declarations
 ********************************************************************************/
using namespace std;
using namespace omnetpp;
namespace artery {

    Define_Module(SimSocket)

void SimSocket::initialize() {

    //get traci from ModulePath
    cModule* traci = getModuleByPath(par("traciModule_2"));

    //Subscribe signal to actual Traci
    if (traci) {
        traci->subscribe(traci::BasicNodeManager::updateSendStatus, this);
    } else {
        throw cRuntimeError("No TraCI module found for signal subscription");
    }

    // set up zmq socket and stuff
    context_ = zmq::context_t(1);
    portName_ = "tcp://*:7777";
    subPortName_ = "tcp://localhost:7778";
    publisherSocket_ = zmq::socket_t(context_, zmq::socket_type::pub);
    subscriberSocket_ = zmq::socket_t(context_, zmq::socket_type::sub);
    subscriberSocket_.setsockopt(ZMQ_SUBSCRIBE, "", 0);
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

    publisherSocket_.disconnect(portName_);
    connections_.erase(bindingIterator);
    publisherSocket_.unbind(portName_);
}

// publish data
void SimSocket::publish() {

    //serialize map
    std::ostringstream ss;
    boost::archive::text_oarchive archive(ss);
    archive << vehicleDataMap;
    std::string outbound_data = ss.str();
    // create buffer size for message
    zmq::message_t msgToSend(outbound_data);

    try {
        std::cout << "Message: " << msgToSend << endl;
        publisherSocket_.send(msgToSend, zmq::send_flags::none);

    } catch (zmq::error_t cantSend) {
        cerr << "Socket can't send: " << cantSend.what() << endl;
        unbind(portName_);
    }
}

// subscribe to incoming data
void SimSocket::subscribe() {

    zmq::message_t reply;

    try {
        std::cout << "Receiving... " << endl;
        subscriberSocket_.recv(&reply);

    } catch (zmq::error_t cantReceive) {
        cerr << "Socket can't receive: " << cantReceive.what() << endl;
        // TODO unbind
    }

        const char *buf = static_cast<const char*>(reply.data());
            std::cout << "CHAR [" << buf << "]" << std::endl;

            std::string input_data_( buf, reply.size() );
            std::istringstream archive_stream(input_data_);
            boost::archive::text_iarchive archive(archive_stream);

            std::map<std::string , boost::variant<int, double, std::string>> receiveMap;
            // With time  not working
       // std::map<std::string , boost::variant<int, double, std::string, std::time_t>> simEventMap;

            try
            {
                archive >> receiveMap;
            } catch (boost::archive::archive_exception& ex) {
                std::cout << "Archive Exception during deserializing:" << std::endl;
                std::cout << ex.what() << std::endl;
            } catch (int e) {
                std::cout << "EXCEPTION " << e << std::endl;
            }


            std::vector<std::string> keyVektor;
            std::vector<boost::variant<int, double, std::string>> valueVektor;
            for (auto const& element: receiveMap) {
                keyVektor.push_back(element.first);
                valueVektor.push_back(element.second);
                std::string keyAsString = element.first;

                auto valueAsAny =   element.second;
                std::stringstream stringStreamValue ;
                stringStreamValue <<  valueAsAny;
                std::cout << "Key: " << element.first << std::endl;
                std::cout << "value: " << stringStreamValue.str() << std::endl;

            }
}

// call in basic node manager to get data and write to a global map
void SimSocket::getVehicleData(std::string vehicleID, TraCIAPI::VehicleScope traci) {

    DataMap map;

    SimTime sendingTime = simTime();
    std::string sendingTime_str = sendingTime.str();

    map.insert(std::pair<std::string, std::string>("origin", "Origin"));
    map.insert(std::pair<std::string, std::string>("current", sendingTime_str));
    map.insert(std::pair<std::string, std::string>("vehicleID", vehicleID));
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

    vehicleDataMap = map;
}
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
    if (signal == traci::BasicNodeManager::updateSendStatus) {
        //getVehicleData("flowNorthSouth.1");
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