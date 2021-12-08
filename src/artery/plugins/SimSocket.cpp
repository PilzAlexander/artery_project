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
#include "artery/traci/VehicleController.h"
#include "artery/application/VehicleKinematics.h"
#include "artery/application/Middleware.h"
#include "artery/application/StationType.h"
#include <vanetza/geonet/serialization.hpp>
#include <vanetza/net/mac_address.hpp>
#include <vanetza/common/byte_view.hpp>
#include <vanetza/common/archives.hpp>
#include <vanetza/asn1/packet_visitor.hpp>
#include <vanetza/common/serialization_buffer.hpp>

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
        //unbind(portName_);
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
        publisherSocket_.unbind(portName_);
        connections_.erase(bindingIterator);
    }

// publish data
    void SimSocket::publish() {
        //Enter_Method("publish()");
        //serialize map
        std::ostringstream ss;
        boost::archive::text_oarchive archive(ss);

        std::string outbound_data = ss.str();
        // create buffer size for message
        zmq::message_t msgToSend(outbound_data);

        try {
            publisherSocket_.send(msgToSend, zmq::send_flags::none);

        } catch (zmq::error_t cantSend) {
            cerr << "Socket can't send: " << cantSend.what() << endl;
            unbind(portName_);
        }
    }

    void SimSocket::publishSimMsg(const vanetza::MacAddress &MacSource, const vanetza::MacAddress &MacDest,
                                  const vanetza::byte_view_range &byteViewRange) {

        SimTime sendingTime = simTime();
        std::string sendingTime_str = sendingTime.str();

        //serialize map
        std::ostringstream ss;
        vanetza::OutputArchive archive(ss);
        vanetza::serialize(archive, MacSource);
        vanetza::serialize(archive, MacDest);
        archive << byteViewRange.size();

        for (int i = 0; i < byteViewRange.size(); i++) {
            archive << byteViewRange.operator[](i);
            //std::cout << "Byte at place:[" << i << "]" << byteViewRange.operator[](i) << std::endl;
        }

        std::string outbound_data = ss.str();
        // create buffer size for message
        zmq::message_t msgToSend(outbound_data);

        try {
            //std::cout << "Message: " << msgToSend << endl;
            publisherSocket_.send(msgToSend, zmq::send_flags::none);
            //std::cout << "SimMsgToSend:" << msgToSend.data() << std::endl;
        } catch (zmq::error_t cantSend) {
            cerr << "Socket can't send: " << cantSend.what() << endl;
            unbind(portName_);
        }
    }

// subscribe to incoming data
    void SimSocket::subscribe() {
        zmq::message_t messageToReceive;

        try {
            if (messageToReceive.empty()) {
                //std::cout << "No message to receive..." << endl;
                return;
            } else {
                subscriberSocket_.recv(&messageToReceive, ZMQ_NOBLOCK);
            }
        } catch (zmq::error_t cantReceive) {
            cerr << "Socket can't receive: " << cantReceive.what() << endl;
            // TODO unbind
        }
        const char *buf = static_cast<const char *>(messageToReceive.data());
        std::string input_data_(buf, messageToReceive.size());
        std::istringstream archive_stream(input_data_);
        boost::archive::text_iarchive archive(archive_stream);

        try {
            //archive >> inputDataMap_;
        } catch (boost::archive::archive_exception &ex) {
            std::cout << "Archive Exception during deserializing:" << std::endl;
            std::cout << ex.what() << std::endl;
        } catch (int e) {
            std::cout << "EXCEPTION " << e << std::endl;
        }
    }

// call in basic node manager to get data and write to a global map
    void SimSocket::getVehicleData(std::string vehicleID, TraCIAPI::VehicleScope traci) {

        SimTime sendingTime = simTime();
        std::string sendingTime_str = sendingTime.str();

        if (vehicleDataMap_["Speed_DUT"] != boost::variant<int, double, std::string>(traci.getSpeed(vehicleID))) {
            vehicleDataMap_.insert_or_assign("Speed_DUT", traci.getSpeed(vehicleID));
            diffVehicleDataMap_.insert_or_assign("Speed_DUT", traci.getSpeed(vehicleID));
        } else {
            diffVehicleDataMap_.erase("Speed_DUT");
        }

        if (vehicleDataMap_["Acceleration_DUT"] !=
            boost::variant<int, double, std::string>(traci.getAcceleration(vehicleID))) {
            vehicleDataMap_.insert_or_assign("Acceleration_DUT", traci.getAcceleration(vehicleID));
            diffVehicleDataMap_.insert_or_assign("Acceleration_DUT", traci.getAcceleration(vehicleID));
        } else {
            diffVehicleDataMap_.erase("Acceleration_DUT");
        }

        if (vehicleDataMap_["Distance_DUT"] != boost::variant<int, double, std::string>(traci.getDistance(vehicleID))) {
            vehicleDataMap_.insert_or_assign("Distance_DUT", traci.getDistance(vehicleID));
            diffVehicleDataMap_.insert_or_assign("Distance_DUT", traci.getDistance(vehicleID));
        } else {
            diffVehicleDataMap_.erase("Distance_DUT");
        }

        if (vehicleDataMap_["Height_DUT"] != boost::variant<int, double, std::string>(traci.getHeight(vehicleID))) {
            vehicleDataMap_.insert_or_assign("Height_DUT", traci.getHeight(vehicleID));
            diffVehicleDataMap_.insert_or_assign("Height_DUT", traci.getHeight(vehicleID));
        } else {
            diffVehicleDataMap_.erase("Height_DUT");
        }

        if (vehicleDataMap_["Angle_DUT"] != boost::variant<int, double, std::string>(traci.getAngle(vehicleID))) {
            vehicleDataMap_.insert_or_assign("Angle_DUT", traci.getAngle(vehicleID));
            diffVehicleDataMap_.insert_or_assign("Angle_DUT", traci.getAngle(vehicleID));
        } else {
            diffVehicleDataMap_.erase("Angle_DUT");
        }

        if (vehicleDataMap_["Length_DUT"] != boost::variant<int, double, std::string>(traci.getLength(vehicleID))) {
            vehicleDataMap_.insert_or_assign("Length_DUT", traci.getLength(vehicleID));
            diffVehicleDataMap_.insert_or_assign("Length_DUT", traci.getLength(vehicleID));
        } else {
            diffVehicleDataMap_.erase("Length_DUT");
        }

        if (vehicleDataMap_["Width_DUT"] != boost::variant<int, double, std::string>(traci.getWidth(vehicleID))) {
            vehicleDataMap_.insert_or_assign("Width_DUT", traci.getWidth(vehicleID));
            diffVehicleDataMap_.insert_or_assign("Width_DUT", traci.getWidth(vehicleID));
        } else {
            diffVehicleDataMap_.erase("Width_DUT");
        }

        if (vehicleDataMap_["LanePosition_DUT"] !=
            boost::variant<int, double, std::string>(traci.getLanePosition(vehicleID))) {
            vehicleDataMap_.insert_or_assign("LanePosition_DUT", traci.getLanePosition(vehicleID));
            diffVehicleDataMap_.insert_or_assign("LanePosition_DUT", traci.getLanePosition(vehicleID));
        } else {
            diffVehicleDataMap_.erase("LanePosition_DUT");
        }

        if (vehicleDataMap_["Signals_DUT"] != boost::variant<int, double, std::string>(traci.getSignals(vehicleID))) {
            vehicleDataMap_.insert_or_assign("Signals_DUT", traci.getSignals(vehicleID));
            diffVehicleDataMap_.insert_or_assign("Signals_DUT", traci.getSignals(vehicleID));
        } else {
            diffVehicleDataMap_.erase("Signals_DUT");
        }

        if (vehicleDataMap_["LanePosition_DUT"] !=
            boost::variant<int, double, std::string>(traci.getLanePosition(vehicleID))) {
            vehicleDataMap_.insert_or_assign("LanePosition_DUT", traci.getLanePosition(vehicleID));
            diffVehicleDataMap_.insert_or_assign("LanePosition_DUT", traci.getLanePosition(vehicleID));
        } else {
            diffVehicleDataMap_.erase("LanePosition_DUT");
        }

        if (vehicleDataMap_["Position_X-Coordinate_DUT"] !=
            boost::variant<int, double, std::string>(traci.getPosition(vehicleID).x)) {
            vehicleDataMap_.insert_or_assign("Position_X-Coordinate_DUT", traci.getPosition(vehicleID).x);
            diffVehicleDataMap_.insert_or_assign("Position_X-Coordinate_DUT", traci.getPosition(vehicleID).x);
        } else {
            diffVehicleDataMap_.erase("Position_X-Coordinate_DUT");
        }

        if (vehicleDataMap_["Position_y-Coordinate_DUT"] !=
            boost::variant<int, double, std::string>(traci.getPosition(vehicleID).y)) {
            vehicleDataMap_.insert_or_assign("Position_y-Coordinate_DUT", traci.getPosition(vehicleID).y);
            diffVehicleDataMap_.insert_or_assign("Position_y-Coordinate_DUT", traci.getPosition(vehicleID).y);
        } else {
            diffVehicleDataMap_.erase("Position_y-Coordinate_DUT");
        }

        if (vehicleDataMap_["Position_z-Coordinate_DUT"] !=
            boost::variant<int, double, std::string>(traci.getPosition(vehicleID).z)) {
            vehicleDataMap_.insert_or_assign("Position_z-Coordinate_DUT", traci.getPosition(vehicleID).z);
            diffVehicleDataMap_.insert_or_assign("Position_z-Coordinate_DUT", traci.getPosition(vehicleID).z);
        } else {
            diffVehicleDataMap_.erase("Position_z-Coordinate_DUT");
        }

        if (vehicleDataMap_["Decel_DUT"] != boost::variant<int, double, std::string>(traci.getDecel(vehicleID))) {
            vehicleDataMap_.insert_or_assign("Decel_DUT", traci.getDecel(vehicleID));
            diffVehicleDataMap_.insert_or_assign("Decel_DUT", traci.getDecel(vehicleID));
        } else {
            diffVehicleDataMap_.erase("Decel_DUT");
        }

        if (vehicleDataMap_["RoadID_DUT"] != boost::variant<int, double, std::string>(traci.getRoadID(vehicleID))) {
            vehicleDataMap_.insert_or_assign("RoadID_DUT", traci.getRoadID(vehicleID));
            diffVehicleDataMap_.insert_or_assign("RoadID_DUT", traci.getRoadID(vehicleID));
        } else {
            diffVehicleDataMap_.erase("RoadID_DUT");
        }

        if (vehicleDataMap_["RouteIndex_DUT"] !=
            boost::variant<int, double, std::string>(traci.getRouteIndex(vehicleID))) {
            vehicleDataMap_.insert_or_assign("RouteIndex_DUT", traci.getRouteIndex(vehicleID));
            diffVehicleDataMap_.insert_or_assign("RouteIndex_DUT", traci.getRouteIndex(vehicleID));
        } else {
            diffVehicleDataMap_.erase("RouteIndex_DUT");
        }

        if (vehicleDataMap_["LaneID_DUT"] != boost::variant<int, double, std::string>(traci.getLaneID(vehicleID))) {
            vehicleDataMap_.insert_or_assign("LaneID_DUT", traci.getLaneID(vehicleID));
            diffVehicleDataMap_.insert_or_assign("LaneID_DUT", traci.getLaneID(vehicleID));
        } else {
            diffVehicleDataMap_.erase("LaneID_DUT");
        }

        if (vehicleDataMap_["LaneIndex_DUT"] !=
            boost::variant<int, double, std::string>(traci.getLaneIndex(vehicleID))) {
            vehicleDataMap_.insert_or_assign("LaneIndex_DUT", traci.getLaneIndex(vehicleID));
            diffVehicleDataMap_.insert_or_assign("LaneIndex_DUT", traci.getLaneIndex(vehicleID));
        } else {
            diffVehicleDataMap_.erase("LaneIndex_DUT");
        }

        if (vehicleDataMap_["AccumulatedWaitingTime_DUT"] !=
            boost::variant<int, double, std::string>(traci.getAccumulatedWaitingTime(vehicleID))) {
            vehicleDataMap_.insert_or_assign("AccumulatedWaitingTime_DUT", traci.getAccumulatedWaitingTime(vehicleID));
            diffVehicleDataMap_.insert_or_assign("AccumulatedWaitingTime_DUT",
                                                 traci.getAccumulatedWaitingTime(vehicleID));
        } else {
            diffVehicleDataMap_.erase("AccumulatedWaitingTime_DUT");
        }

        if (vehicleDataMap_["AllowedSpeed_DUT"] !=
            boost::variant<int, double, std::string>(traci.getAllowedSpeed(vehicleID))) {
            vehicleDataMap_.insert_or_assign("AllowedSpeed_DUT", traci.getAllowedSpeed(vehicleID));
            diffVehicleDataMap_.insert_or_assign("AllowedSpeed_DUT", traci.getAllowedSpeed(vehicleID));
        } else {
            diffVehicleDataMap_.erase("AllowedSpeed_DUT");
        }

        if (vehicleDataMap_["Co2Emission_DUT"] !=
            boost::variant<int, double, std::string>(traci.getCO2Emission(vehicleID))) {
            vehicleDataMap_.insert_or_assign("Co2Emission_DUT", traci.getCO2Emission(vehicleID));
            diffVehicleDataMap_.insert_or_assign("Co2Emission_DUT", traci.getCO2Emission(vehicleID));
        } else {
            diffVehicleDataMap_.erase("Co2Emission_DUT");
        }

        if (vehicleDataMap_["COEmission_DUT"] !=
            boost::variant<int, double, std::string>(traci.getCOEmission(vehicleID))) {
            vehicleDataMap_.insert_or_assign("COEmission_DUT", traci.getCOEmission(vehicleID));
            diffVehicleDataMap_.insert_or_assign("COEmission_DUT", traci.getCOEmission(vehicleID));
        } else {
            diffVehicleDataMap_.erase("COEmission_DUT");
        }

        if (vehicleDataMap_["ElectricityConsumption_DUT"] !=
            boost::variant<int, double, std::string>(traci.getElectricityConsumption(vehicleID))) {
            vehicleDataMap_.insert_or_assign("ElectricityConsumption_DUT", traci.getElectricityConsumption(vehicleID));
            diffVehicleDataMap_.insert_or_assign("ElectricityConsumption_DUT",
                                                 traci.getElectricityConsumption(vehicleID));
        } else {
            diffVehicleDataMap_.erase("ElectricityConsumption_DUT");
        }

        if (vehicleDataMap_["FuelConsumption_DUT"] !=
            boost::variant<int, double, std::string>(traci.getFuelConsumption(vehicleID))) {
            vehicleDataMap_.insert_or_assign("FuelConsumption_DUT", traci.getFuelConsumption(vehicleID));
            diffVehicleDataMap_.insert_or_assign("FuelConsumption_DUT", traci.getFuelConsumption(vehicleID));
        } else {
            diffVehicleDataMap_.erase("FuelConsumption_DUT");
        }

        if (vehicleDataMap_["HCEmission_DUT"] !=
            boost::variant<int, double, std::string>(traci.getHCEmission(vehicleID))) {
            vehicleDataMap_.insert_or_assign("HCEmission_DUT", traci.getHCEmission(vehicleID));
            diffVehicleDataMap_.insert_or_assign("HCEmission_DUT", traci.getHCEmission(vehicleID));
        } else {
            diffVehicleDataMap_.erase("HCEmission_DUT");
        }

        if (vehicleDataMap_["Imperfection_DUT"] !=
            boost::variant<int, double, std::string>(traci.getImperfection(vehicleID))) {
            vehicleDataMap_.insert_or_assign("Imperfection_DUT", traci.getImperfection(vehicleID));
            diffVehicleDataMap_.insert_or_assign("Imperfection_DUT", traci.getImperfection(vehicleID));
        } else {
            diffVehicleDataMap_.erase("Imperfection_DUT");
        }

        if (vehicleDataMap_["LateralLanePosition_DUT"] !=
            boost::variant<int, double, std::string>(traci.getLateralLanePosition(vehicleID))) {
            vehicleDataMap_.insert_or_assign("LateralLanePosition_DUT", traci.getLateralLanePosition(vehicleID));
            diffVehicleDataMap_.insert_or_assign("LateralLanePosition_DUT", traci.getLateralLanePosition(vehicleID));
        } else {
            diffVehicleDataMap_.erase("LateralLanePosition_DUT");
        }

        if (vehicleDataMap_["MaxSpeed_DUT"] != boost::variant<int, double, std::string>(traci.getMaxSpeed(vehicleID))) {
            vehicleDataMap_.insert_or_assign("MaxSpeed_DUT", traci.getMaxSpeed(vehicleID));
            diffVehicleDataMap_.insert_or_assign("MaxSpeed_DUT", traci.getMaxSpeed(vehicleID));
        } else {
            diffVehicleDataMap_.erase("MaxSpeed_DUT");
        }

        if (vehicleDataMap_["MaxSpeedLat_DUT"] !=
            boost::variant<int, double, std::string>(traci.getMaxSpeedLat(vehicleID))) {
            vehicleDataMap_.insert_or_assign("MaxSpeedLat_DUT", traci.getMaxSpeedLat(vehicleID));
            diffVehicleDataMap_.insert_or_assign("MaxSpeedLat_DUT", traci.getMaxSpeedLat(vehicleID));
        } else {
            diffVehicleDataMap_.erase("MaxSpeedLat_DUT");
        }

        if (vehicleDataMap_["MinGap_DUT"] != boost::variant<int, double, std::string>(traci.getMinGap(vehicleID))) {
            vehicleDataMap_.insert_or_assign("MinGap_DUT", traci.getMinGap(vehicleID));
            diffVehicleDataMap_.insert_or_assign("MinGap_DUT", traci.getMinGap(vehicleID));
        } else {
            diffVehicleDataMap_.erase("MinGap_DUT");
        }

        if (vehicleDataMap_["MinGapLat_DUT"] !=
            boost::variant<int, double, std::string>(traci.getMinGapLat(vehicleID))) {
            vehicleDataMap_.insert_or_assign("MinGapLat_DUT", traci.getMinGapLat(vehicleID));
            diffVehicleDataMap_.insert_or_assign("MinGapLat_DUT", traci.getMinGapLat(vehicleID));
        } else {
            diffVehicleDataMap_.erase("MinGapLat_DUT");
        }

        if (vehicleDataMap_["NOxEmission_DUT"] !=
            boost::variant<int, double, std::string>(traci.getNOxEmission(vehicleID))) {
            vehicleDataMap_.insert_or_assign("NOxEmission_DUT", traci.getNOxEmission(vehicleID));
            diffVehicleDataMap_.insert_or_assign("NOxEmission_DUT", traci.getNOxEmission(vehicleID));
        } else {
            diffVehicleDataMap_.erase("NOxEmission_DUT");
        }

        if (vehicleDataMap_["NoiseEmission_DUT"] !=
            boost::variant<int, double, std::string>(traci.getNoiseEmission(vehicleID))) {
            vehicleDataMap_.insert_or_assign("NoiseEmission_DUT", traci.getNoiseEmission(vehicleID));
            diffVehicleDataMap_.insert_or_assign("NoiseEmission_DUT", traci.getNoiseEmission(vehicleID));
        } else {
            diffVehicleDataMap_.erase("NoiseEmission_DUT");
        }

        if (vehicleDataMap_["PMxEmission_DUT"] !=
            boost::variant<int, double, std::string>(traci.getPMxEmission(vehicleID))) {
            vehicleDataMap_.insert_or_assign("PMxEmission_DUT", traci.getPMxEmission(vehicleID));
            diffVehicleDataMap_.insert_or_assign("PMxEmission_DUT", traci.getPMxEmission(vehicleID));
        } else {
            diffVehicleDataMap_.erase("PMxEmission_DUT");
        }

        if (vehicleDataMap_["Slope_DUT"] != boost::variant<int, double, std::string>(traci.getSlope(vehicleID))) {
            vehicleDataMap_.insert_or_assign("Slope_DUT", traci.getSlope(vehicleID));
            diffVehicleDataMap_.insert_or_assign("Slope_DUT", traci.getSlope(vehicleID));
        } else {
            diffVehicleDataMap_.erase("Slope_DUT");
        }

        if (vehicleDataMap_["SpeedDeviation_DUT"] !=
            boost::variant<int, double, std::string>(traci.getSpeedDeviation(vehicleID))) {
            vehicleDataMap_.insert_or_assign("SpeedDeviation_DUT", traci.getSpeedDeviation(vehicleID));
            diffVehicleDataMap_.insert_or_assign("SpeedDeviation_DUT", traci.getSpeedDeviation(vehicleID));
        } else {
            diffVehicleDataMap_.erase("SpeedDeviation_DUT");
        }

        if (vehicleDataMap_["Imperfection_DUT"] !=
            boost::variant<int, double, std::string>(traci.getImperfection(vehicleID))) {
            vehicleDataMap_.insert_or_assign("Imperfection_DUT", traci.getImperfection(vehicleID));
            diffVehicleDataMap_.insert_or_assign("Imperfection_DUT", traci.getImperfection(vehicleID));
        } else {
            diffVehicleDataMap_.erase("Imperfection_DUT");
        }

        if (vehicleDataMap_["SpeedFactor_DUT"] !=
            boost::variant<int, double, std::string>(traci.getSpeedFactor(vehicleID))) {
            vehicleDataMap_.insert_or_assign("SpeedFactor_DUT", traci.getSpeedFactor(vehicleID));
            diffVehicleDataMap_.insert_or_assign("SpeedFactor_DUT", traci.getSpeedFactor(vehicleID));
        } else {
            diffVehicleDataMap_.erase("SpeedFactor_DUT");
        }

        if (vehicleDataMap_["SpeedWithoutTraCI_DUT"] !=
            boost::variant<int, double, std::string>(traci.getSpeedWithoutTraCI(vehicleID))) {
            vehicleDataMap_.insert_or_assign("SpeedWithoutTraCI_DUT", traci.getSpeedWithoutTraCI(vehicleID));
            diffVehicleDataMap_.insert_or_assign("SpeedWithoutTraCI_DUT", traci.getSpeedWithoutTraCI(vehicleID));
        } else {
            diffVehicleDataMap_.erase("SpeedWithoutTraCI_DUT");
        }

        if (vehicleDataMap_["Tau_DUT"] != boost::variant<int, double, std::string>(traci.getTau(vehicleID))) {
            vehicleDataMap_.insert_or_assign("Tau_DUT", traci.getTau(vehicleID));
            diffVehicleDataMap_.insert_or_assign("Tau_DUT", traci.getTau(vehicleID));
        } else {
            diffVehicleDataMap_.erase("Tau_DUT");
        }

        if (vehicleDataMap_["EmissionClass_DUT"] !=
            boost::variant<int, double, std::string>(traci.getEmissionClass(vehicleID))) {
            vehicleDataMap_.insert_or_assign("EmissionClass_DUT", traci.getEmissionClass(vehicleID));
            diffVehicleDataMap_.insert_or_assign("EmissionClass_DUT", traci.getEmissionClass(vehicleID));
        } else {
            diffVehicleDataMap_.erase("EmissionClass_DUT");
        }

        if (vehicleDataMap_["WaitingTime_DUT"] !=
            boost::variant<int, double, std::string>(traci.getWaitingTime(vehicleID))) {
            vehicleDataMap_.insert_or_assign("WaitingTime_DUT", traci.getWaitingTime(vehicleID));
            diffVehicleDataMap_.insert_or_assign("WaitingTime_DUT", traci.getWaitingTime(vehicleID));
        } else {
            diffVehicleDataMap_.erase("WaitingTime_DUT");
        }

        if (vehicleDataMap_["LaneChangeMode_DUT"] !=
            boost::variant<int, double, std::string>(traci.getLaneChangeMode(vehicleID))) {
            vehicleDataMap_.insert_or_assign("LaneChangeMode_DUT", traci.getLaneChangeMode(vehicleID));
            diffVehicleDataMap_.insert_or_assign("LaneChangeMode_DUT", traci.getLaneChangeMode(vehicleID));
        } else {
            diffVehicleDataMap_.erase("LaneChangeMode_DUT");
        }

        if (vehicleDataMap_["LateralAlignment_DUT"] !=
            boost::variant<int, double, std::string>(traci.getLateralAlignment(vehicleID))) {
            vehicleDataMap_.insert_or_assign("LateralAlignment_DUT", traci.getLateralAlignment(vehicleID));
            diffVehicleDataMap_.insert_or_assign("LateralAlignment_DUT", traci.getLateralAlignment(vehicleID));
        } else {
            diffVehicleDataMap_.erase("LateralAlignment_DUT");
        }

        if (vehicleDataMap_["SpeedMode_DUT"] !=
            boost::variant<int, double, std::string>(traci.getSpeedMode(vehicleID))) {
            vehicleDataMap_.insert_or_assign("SpeedMode_DUT", traci.getSpeedMode(vehicleID));
            diffVehicleDataMap_.insert_or_assign("SpeedMode_DUT", traci.getSpeedMode(vehicleID));
        } else {
            diffVehicleDataMap_.erase("SpeedMode_DUT");
        }

        if (vehicleDataMap_["StopState_DUT"] !=
            boost::variant<int, double, std::string>(traci.getStopState(vehicleID))) {
            vehicleDataMap_.insert_or_assign("StopState_DUT", traci.getStopState(vehicleID));
            diffVehicleDataMap_.insert_or_assign("StopState_DUT", traci.getStopState(vehicleID));
        } else {
            diffVehicleDataMap_.erase("StopState_DUT");
        }

        if (vehicleDataMap_["RoutingMode_DUT"] !=
            boost::variant<int, double, std::string>(traci.getRoutingMode(vehicleID))) {
            vehicleDataMap_.insert_or_assign("RoutingMode_DUT", traci.getRoutingMode(vehicleID));
            diffVehicleDataMap_.insert_or_assign("RoutingMode_DUT", traci.getRoutingMode(vehicleID));
        } else {
            diffVehicleDataMap_.erase("RoutingMode_DUT");
        }

        if (vehicleDataMap_["ShapeClass_DUT"] !=
            boost::variant<int, double, std::string>(traci.getShapeClass(vehicleID))) {
            vehicleDataMap_.insert_or_assign("ShapeClass_DUT", traci.getShapeClass(vehicleID));
            diffVehicleDataMap_.insert_or_assign("ShapeClass_DUT", traci.getShapeClass(vehicleID));
        } else {
            diffVehicleDataMap_.erase("ShapeClass_DUT");
        }

        /*
    if(vehicleDataMap_["ApparentDecel_DUT"] != boost::variant<int, double, std::string>(traci.getApparentDecel(vehicleID))){
        vehicleDataMap_.insert_or_assign("ApparentDecel_DUT", traci.getApparentDecel(vehicleID));
        diffVehicleDataMap_.insert_or_assign("ApparentDecel_DUT", traci.getApparentDecel(vehicleID));
    }else{
        diffVehicleDataMap_.erase("ApparentDecel_DUT");
    }

    if (vehicleDataMap_["StopArrivalDelay_DUT"] !=
        boost::variant<int, double, std::string>(traci.getStopArrivalDelay(vehicleID))) {
        vehicleDataMap_.insert_or_assign("StopArrivalDelay_DUT", traci.getStopArrivalDelay(vehicleID));
        diffVehicleDataMap_.insert_or_assign("StopArrivalDelay_DUT", traci.getStopArrivalDelay(vehicleID));
    } else {
        diffVehicleDataMap_.erase("StopArrivalDelay_DUT");
    }

    if (vehicleDataMap_["StopDelay_DUT"] !=
        boost::variant<int, double, std::string>(traci.getStopDelay(vehicleID))) {
        vehicleDataMap_.insert_or_assign("StopDelay_DUT", traci.getStopDelay(vehicleID));
        diffVehicleDataMap_.insert_or_assign("StopDelay_DUT", traci.getStopDelay(vehicleID));
    } else {
        diffVehicleDataMap_.erase("StopDelay_DUT");
    }
    if (vehicleDataMap_["PersonCapacity_DUT"] !=
        boost::variant<int, double, std::string>(traci.getPersonCapacity(vehicleID))) {
        vehicleDataMap_.insert_or_assign("PersonCapacity_DUT", traci.getPersonCapacity(vehicleID));
        diffVehicleDataMap_.insert_or_assign("PersonCapacity_DUT", traci.getPersonCapacity(vehicleID));
    } else {
        diffVehicleDataMap_.erase("PersonCapacity_DUT");
    }*/

//vehicleDataMap_.insert(std::pair<std::string, double>("EmergencyDecel_DUT", traci.getEmergencyDecel(vehicleID)));
//vehicleDataMap_.insert(std::pair<std::string, std::pair<string,double>>("Follower_DUT", traci.getFollower(vehicleID, traci.getDistance(vehicleID))));
//vehicleDataMap_.insert(std::pair<std::string, std::pair<string,double>>("LaneChangeMode_DUT", traci.getLeader(vehicleID, traci.getDistance(vehicleID))));

        for (const auto &elem: diffVehicleDataMap_) {
            std::cout << elem.first << " " << elem.second << "\n";
        }
        std::cout << "\n";
    }

    void SimSocket::getVehicleDynamics(VehicleKinematics dynamics) {
/*
    if(!isnan(dynamics.speed.value())){
        vehicleDataMap_.insert(std::pair<std::string, double>("Speed_Dynamics", dynamics.speed.value()));
    }
    if(!isnan(dynamics.yaw_rate.value())){
        vehicleDataMap_.insert(std::pair<std::string, double>("YawRate_Dynamics", dynamics.yaw_rate.value()));
    }
    if(!isnan(dynamics.acceleration.value())){
        vehicleDataMap_.insert(std::pair<std::string, double>("Acceleration_Dynamics", dynamics.acceleration.value()));
    }
    if(!isnan(dynamics.heading.value())){
        vehicleDataMap_.insert(std::pair<std::string, double>("Heading_Dynamics", dynamics.heading.value()));
    }
    if(!isnan(dynamics.geo_position.latitude.value())){
        vehicleDataMap_.insert(std::pair<std::string, double>("Latitude_Dynamics", dynamics.geo_position.latitude.value()));
    }
    if(!isnan(dynamics.geo_position.longitude.value())){
        vehicleDataMap_.insert(std::pair<std::string, double>("Longitude_Dynamics", dynamics.geo_position.longitude.value()));
    }
    if(!isnan(dynamics.position.x.value())){
        vehicleDataMap_.insert(std::pair<std::string, double>("PosX_Dynamics", dynamics.position.x.value()));
    }
    if(!isnan(dynamics.position.y.value())){
        vehicleDataMap_.insert(std::pair<std::string, double>("PosY_Dynamics", dynamics.position.y.value()));
    }*/
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
    void SimSocket::receiveSignal(cComponent *, simsignal_t signal, unsigned long, cObject *) {
        if (signal == traci::BasicNodeManager::updateNodeSignal) {
            publish();
            subscribe();
        }
    }
}// namespace artery
/********************************************************************************
 * EOF
 ********************************************************************************/