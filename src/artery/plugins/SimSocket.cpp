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

#include <iostream>
#include <utility>
#include <zmq.hpp>
#include <algorithm>
#include <array>


//***********************
#include "artery/networking/GeoNetIndication.h"
#include "artery/networking/GeoNetPacket.h"
#include "artery/nic/RadioDriverBase.h"
#include "artery/plugins/OtaInterfaceLayer.h"
#include "artery/plugins/OtaInterface.h"
#include "artery/plugins/DutRadio.h"
#include "artery/traci/ControllableVehicle.h"
#include "artery/traci/VehicleController.h"
#include <inet/common/ModuleAccess.h>
#include <vanetza/net/packet_variant.hpp>
#include <inet/physicallayer/contract/packetlevel/IRadio.h>
//****************
/********************************************************************************
 * Function declarations
 ********************************************************************************/
using namespace std;
using namespace omnetpp;
namespace artery {

    Define_Module(SimSocket)

void SimSocket::initialize() {

    //get traci from ModulePath
    cModule* traci = getModuleByPath(par("traciModule"));

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

    //std::cout << "Speed Data: " << vehicleDataMap["Speed"] << endl;
    //std::cout << "Speed Data: " << vehicleDynamicsMap["Speed Dynamics"] << endl;

    //serialize map
    std::ostringstream ss;
    boost::archive::text_oarchive archive(ss);
    archive << vehicleDataMap;
    archive << vehicleDynamicsMap;
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
            std::cout << "No message to receive..." << endl;
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
    //VehicleKinematics dynamics = getKinematics(mVehicleController(traci));

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

void SimSocket::getVehicleDynamics(VehicleKinematics dynamics){
    DataMap map;

    map.insert(std::pair<std::string, double>("Speed Dynamics", dynamics.speed.value()));
    map.insert(std::pair<std::string, double>("Yaw Rate Dynamics", dynamics.yaw_rate.value()));
    map.insert(std::pair<std::string, double>("Acceleration Dynamics", dynamics.acceleration.value()));
    map.insert(std::pair<std::string, double>("Heading Dynamics", dynamics.heading.value()));
    map.insert(std::pair<std::string, double>("Latitude Dynamics", dynamics.geo_position.latitude.value()));
    map.insert(std::pair<std::string, double>("Longitude Dynamics", dynamics.geo_position.longitude.value()));
    map.insert(std::pair<std::string, double>("PosX Dynamics", dynamics.position.x.value()));
    map.insert(std::pair<std::string, double>("PoY Dynamics", dynamics.position.y.value()));

    for(const auto& elem : map)
    {
        std::cout << elem.first << " " << elem.second << " " << "\n";
    }

    vehicleDynamicsMap = map;
}

void SimSocket::getEvent(omnetpp::cEvent* event){
    std::map <std::string, std::string> dataMap;

    /*
    std::cout << "*****************************" << std::endl;
    std::cout << "EVENT:    " << event << std::endl;
    std::cout << "String:    " << event->str() << std::endl;
    std::cout << "Name:    " << event->getName() << std::endl;
    std::cout << "Target:    " << event->getTargetObject() << std::endl;
    std::cout << "Target String:    " << event->getTargetObject()->str() << std::endl;
    std::cout << "Owner:    " << event->getOwner() << std::endl;
    std::cout << "Owner String:    " << event->getOwner()->str() << std::endl;
    std::cout << "Descriptor:    " << event->getDescriptor() << std::endl;
    std::cout << "Descriptor String:    " << event->getDescriptor()->str() << std::endl;
    std::cout << "ArrivalTime:    " << event->getArrivalTime() << std::endl;
    std::cout << "PreviousEventNumber:    " << event->getPreviousEventNumber() << std::endl;
    std::cout << "NamePooling:    " << event->getNamePooling() << std::endl;
    std::cout << "*****************************" << std::endl;
     */
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