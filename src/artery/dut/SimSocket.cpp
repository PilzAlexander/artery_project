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
#include "artery/dut/DUTOtaInterfaceConnection.h"
#include "artery/dut/SimEventFromInterfaceVisitor.h"
#include "artery/dut/XML/pugixml.hpp"

#include <iostream>
#include <zmq.hpp>
#include <algorithm>
#include <array>
#include <fstream>
/********************************************************************************
 * Function declarations
 ********************************************************************************/
using namespace std;
using namespace omnetpp;

namespace artery {

    Define_Module(SimSocket)

    void SimSocket::initialize() {

        cModule *traci = getModuleByPath(par("traciModule"));

        if (traci) {
            traci->subscribe(traci::BasicNodeManager::updateNodeSignal, this);
        } else {
            throw cRuntimeError("No TraCI module found for signal subscription");
        }

        pugi::xml_node root = openXML();

        context_ = zmq::context_t(1);
        pubPortName_ = getValueFromXML(root,"pubPortName_","pubPortName");
        subPortName_ = getValueFromXML(root,"subPortName_","subPortName");
        portNameConfig_ = getValueFromXML(root,"portNameConfig_","portNameConfig");
        configXMLPath_ = getValueFromXML(root,"configXMLPath_","configXMLPath");
        publisherSocket_ = zmq::socket_t(context_, zmq::socket_type::pub);
        subscriberSocket_ = zmq::socket_t(context_, zmq::socket_type::sub);
        publisherSocketConfig_ = zmq::socket_t(context_, zmq::socket_type::pub);
        subscriberSocket_.set(zmq::sockopt::subscribe, "");
        connect(subPortName_, subscriberSocket_);
        bind(pubPortName_, publisherSocket_);
        bind(portNameConfig_, publisherSocketConfig_ );
    }

    void SimSocket::finish() {
    }

    SimSocket::SimSocket() {}

    SimSocket::~SimSocket() {
        unbind(getLastEndpoint(publisherSocket_), publisherSocket_);
        unbind(getLastEndpoint(publisherSocketConfig_), publisherSocketConfig_);
        disconnect(getLastEndpoint(subscriberSocket_), subscriberSocket_);
        close(publisherSocket_);
        close(subscriberSocket_ );
        close(publisherSocketConfig_);
    }

    std::string SimSocket::getLastEndpoint(zmq::socket_t& socketName) const {
        string lastEndpoint =
                socketName.get(zmq::sockopt::last_endpoint);
        return lastEndpoint;
    }

    void SimSocket::close(zmq::socket_t &socketName) {
        socketName.close();
    }

    void SimSocket::connect(const PortName &portName, zmq::socket_t& socketName) {
        try {
            socketName.connect(portName);
            connections_.push_back(portName);
            EV_INFO << "Connected to port: " << portName  << endl;
        } catch (zmq::error_t &cantConnect) {
            cerr << "Socket can't connect to port: " << cantConnect.what() << endl;
            close(socketName);
            return;
        }
    }

    void SimSocket::disconnect(const PortName &portName, zmq::socket_t& socketName) {
        auto connectionIterator = std::find(connections_.begin(), connections_.end(), portName);
        if (connectionIterator == connections_.end()) {
            cerr << "Socket: " << socketName << " failed to disconnect from port: " << portName << endl;
            return;
        }
        socketName.disconnect(portName);
        connections_.erase(connectionIterator);
    }

    void SimSocket::bind(const PortName &portName, zmq::socket_t& socketName) {
        try {
            socketName.bind(portName);
            bindings_.push_back(portName);
            EV_INFO << "Bound to port: " << portName  << endl;
        } catch (zmq::error_t &cantBind) {
            cerr << "Socket can't connect to port: " << cantBind.what() << endl;
            close(socketName);
            return;
        }
    }

    void SimSocket::unbind(const PortName &portName, zmq::socket_t& socketName) {
        auto bindingIterator = std::find(bindings_.begin(), bindings_.end(), portName);
        if (bindingIterator == bindings_.end()) {
            cerr << "Socket: " << socketName << " failed to unbind from port: " << portName << endl;
            return;
        }
        socketName.unbind(portName);
        bindings_.erase(bindingIterator);
    }

    void SimSocket::publish() {
        string outboundVehicleData = serializeVehicleData();
        zmq::message_t msgToSend(outboundVehicleData);

        try {
            publisherSocket_.send(msgToSend, zmq::send_flags::none);

        } catch (zmq::error_t &cantSend) {
            cerr << "Socket can't send: " << cantSend.what() << endl;
            unbind(pubPortName_, publisherSocket_);
        }
    }

    void SimSocket::sendConfigString(const std::string& stringFilePath) {
        std::ifstream xmlFile(stringFilePath);
        std::stringstream ss;
        ss << xmlFile.rdbuf();
        std::istringstream iss(ss.str());
        try {
            EV_INFO << "Config sent to simulation" << endl;
            publisherSocketConfig_.send(zmq::buffer(ss.str()), zmq::send_flags::none);
        } catch (zmq::error_t &cantSend) {
            cerr << "Socket can't send: " << cantSend.what() << endl;
            unbind(portNameConfig_, publisherSocketConfig_);
        }
    }

    string SimSocket::serializeVehicleData() const {
        ostringstream vehicleDataStream;
        boost::archive::text_oarchive archive(vehicleDataStream);

        archive << diffVehicleDataMap_;
        string outboundVehicleData = vehicleDataStream.str();
        return outboundVehicleData;
    }

    void SimSocket::publishSimMsg(const vanetza::MacAddress &macSource, const vanetza::MacAddress &macDest,
                                  const vanetza::byte_view_range &byteViewRange) {
        string outboundData = serializeSimMsg(macSource, macDest, byteViewRange);
        zmq::message_t msgToSend(outboundData);

        try {
            publisherSocket_.send(msgToSend, zmq::send_flags::none);
        } catch (zmq::error_t &cantSend) {
            cerr << "Socket can't send: " << cantSend.what() << endl;
            unbind(pubPortName_, publisherSocket_);
        }
    }

    string SimSocket::serializeSimMsg(const vanetza::MacAddress &macSource, const vanetza::MacAddress &macDest,
                                      const vanetza::byte_view_range &byteViewRange) const {
        ostringstream stringsStream;
        boost::archive::text_oarchive archive(stringsStream);
        vanetza::operator<<(stringsStream, macSource);
        vanetza::operator<<(stringsStream, macDest);
        archive << byteViewRange.size();

        for (int i = 0; i < byteViewRange.size(); i++) {
            archive << byteViewRange.operator[](i);
        }

        string outboundData = stringsStream.str();
        return outboundData;
    }

    void SimSocket::subscribe() {
        zmq::message_t messageToReceive;

        try {
            subscriberSocket_.recv(messageToReceive, zmq::recv_flags::dontwait);
        } catch (zmq::error_t &cantReceive) {
            cerr << "Socket can't receive: " << cantReceive.what() << endl;
            disconnect(subPortName_, subscriberSocket_);
        }

        if (!messageToReceive.empty()) {
            const char *buf = static_cast<const char *>(messageToReceive.data());
            std::string inputData(buf, messageToReceive.size());

            std::istringstream archiveStream(inputData);
            boost::archive::text_iarchive archive(archiveStream);

            try {
                archive >> inputDataMap_;

                if ("V2X" == boost::apply_visitor(SimEventFromInterfaceVisitor(), inputDataMap_["Operation"])) {

                    int payloadLength;
                    unsigned char byte;
                    std::string macDest;
                    std::string macSource;

                    std::istringstream archiveV2XStream(
                            boost::apply_visitor(SimEventFromInterfaceVisitor(), inputDataMap_["Value"]));
                    boost::archive::text_iarchive archiveV2X(archiveV2XStream);

                    archiveV2X >> macSource;
                    archiveV2X >> macDest;
                    convertStringToByteArray(macDest, macDest_);

                    archiveV2X >> payloadLength;
                    for (int i = 0; i < payloadLength; i++) {
                        archiveV2X >> byte;
                        payload_.push_back(byte);
                    }

                    cModule *mod = getSimulation()->getModule(5);
                    auto *mTarget = check_and_cast<artery::DUTOtaInterfaceConnection *>(mod);
                    mTarget->placeGeoNetPacketInQueue(macSource_, macDest_, payload_);
                }

            } catch (boost::archive::archive_exception &ex) {
                EV_INFO << "Archive Exception during deserializing: " << ex.what() << endl;
            } catch (int e) {
                EV_INFO << "EXCEPTION " << e << endl;
            }
        }
    }

    void SimSocket::convertStringToByteArray(string &mac, array<unsigned char, 6> &bytes) {
        replace(mac.begin(), mac.end(), ':', ' ');
        istringstream hexStream(mac);

        std::vector<unsigned char> byteVector{};
        unsigned int c;
        while (hexStream >> hex >> c) {
            byteVector.push_back(c);
        }
        std::copy_n(byteVector.begin(), 6, bytes.begin());
    }

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

        if (vehicleDataMap_["Position_x-Coordinate_DUT"] !=
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
    }

    void SimSocket::getVehicleDynamics(VehicleKinematics dynamics) {

        if (!isnan(dynamics.speed.value()) &&
            vehicleDataMap_["Speed_Dynamics"] != boost::variant<int, double, std::string>(
                    dynamics.speed.value())) {
            vehicleDataMap_.insert_or_assign("Speed_Dynamics", dynamics.speed.value());
            diffVehicleDataMap_.insert_or_assign("Speed_Dynamics", dynamics.speed.value());
        } else {
            diffVehicleDataMap_.erase("Speed_Dynamics");
        }

        if (!isnan(dynamics.yaw_rate.value()) &&
            vehicleDataMap_["YawRate_Dynamics"] != boost::variant<int, double, std::string>
                    (dynamics.speed.value())) {
            vehicleDataMap_.insert_or_assign("YawRate_Dynamics", dynamics.yaw_rate.value());
            diffVehicleDataMap_.insert_or_assign("YawRate_Dynamics", dynamics.yaw_rate.value());
        } else {
            diffVehicleDataMap_.erase("YawRate_Dynamics");
        }

        if (!isnan(dynamics.acceleration.value()) &&
            vehicleDataMap_["Acceleration_Dynamics"] != boost::variant<int, double, std::string>
                    (dynamics.speed.value())) {
            vehicleDataMap_.insert_or_assign(
                    "Acceleration_Dynamics", dynamics.acceleration.value());
            diffVehicleDataMap_.insert_or_assign("Acceleration_Dynamics", dynamics.acceleration.value());
        } else {
            diffVehicleDataMap_.erase("Acceleration_Dynamics");
        }

        if (!isnan(dynamics.heading.value()) &&
            vehicleDataMap_["Heading_Dynamics"] != boost::variant<int, double, std::string>
                    (dynamics.speed.value())) {
            vehicleDataMap_.insert_or_assign("Heading_Dynamics", dynamics.heading.value());
            diffVehicleDataMap_.insert_or_assign("Heading_Dynamics", dynamics.heading.value());
        } else {
            diffVehicleDataMap_.erase("Heading_Dynamics");
        }

        if (!isnan(dynamics.geo_position.latitude.value()) &&
            vehicleDataMap_["Latitude_Dynamics"] != boost::variant<int, double, std::string>
                    (dynamics.speed.value())) {
            vehicleDataMap_.insert_or_assign(
                    "Latitude_Dynamics", dynamics.geo_position.latitude.value());
            diffVehicleDataMap_.insert_or_assign("Latitude_Dynamics", dynamics.geo_position.latitude.value());
        } else {
            diffVehicleDataMap_.erase("Latitude_Dynamics");
        }

        if (!isnan(dynamics.geo_position.longitude.value()) &&
            vehicleDataMap_["Longitude_Dynamics"] != boost::variant<int, double, std::string>
                    (dynamics.speed.value())) {
            vehicleDataMap_.insert_or_assign(
                    "Longitude_Dynamics", dynamics.geo_position.longitude.value());
            diffVehicleDataMap_.insert_or_assign("Longitude_Dynamics", dynamics.geo_position.longitude.value());
        } else {
            diffVehicleDataMap_.erase("Longitude_Dynamics");
        }

        if (!isnan(dynamics.position.x.value()) &&
            vehicleDataMap_["PosX_Dynamics"] != boost::variant<int, double, std::string>
                    (dynamics.speed.value())) {
            vehicleDataMap_.insert_or_assign("PosX_Dynamics", dynamics.position.x.value());
            diffVehicleDataMap_.insert_or_assign("PosX_Dynamics", dynamics.position.x.value());
        } else {
            diffVehicleDataMap_.erase("PosX_Dynamics");
        }

        if (!isnan(dynamics.position.y.value()) &&
            vehicleDataMap_["PosY_Dynamics"] != boost::variant<int, double, std::string>
                    (dynamics.speed.value())) {
            vehicleDataMap_.insert_or_assign("PosY_Dynamics", dynamics.position.y.value());
            diffVehicleDataMap_.insert_or_assign("PosY_Dynamics", dynamics.position.y.value());
        } else {
            diffVehicleDataMap_.erase("PosY_Dynamics");
        }
    }

    const SimSocket::DataMap &SimSocket::getInputDataMap() const {
        return inputDataMap_;
    }

    void SimSocket::receiveSignal(cComponent *, simsignal_t signal, unsigned long, cObject *) {
        if (signal == traci::BasicNodeManager::updateNodeSignal) {
            //Workaround because it's not possible to send in the Omnet++ Init methode
            if (count < 1) {
                //sending config once at the beginning
                sendConfigString(configXMLPath_);
                count = 1;
            }
            publish();
            subscribe();
        }
    }

    std::string SimSocket::getValueFromXML( pugi::xml_node root, std::string entryName, const char * attributeName) {
        std::string strValue;
        // Search for the first matching entry with the given hint attribute
        string searchStr = entryName;
        pugi::xpath_node xpathNode = root.select_single_node(searchStr.c_str());
        if (xpathNode) {
            pugi::xml_node selectedNode = xpathNode.node();
            strValue =  selectedNode.child(attributeName).attribute("path").as_string();
        }
        return strValue;
    }
	
	stringstream SimSocket::getStringstreamOfXML(const string &stringFilePath) const {
        std::ifstream xmlFile(stringFilePath);
        std::stringstream ss;
        ss << xmlFile.rdbuf();
        std::istringstream iss(ss.str());
        return ss;
    }

    pugi::xml_node SimSocket::openXML() const {// Create empty XML document within memory
        pugi::xml_document doc;
        // Load XML file into memory
        // Remark: to fully read declaration entries you have to specify
        // "pugi::parse_declaration"
        pugi::xml_parse_result result = doc.load_file(
                "/home/vagrant/artery_project/src/artery/dut/connectionConfig.xml",
                pugi::parse_default | pugi::parse_declaration);
        if (!result) {
            cout << "Parse error: " << result.description()
                 << ", character pos= " << result.offset;
        }
        // A valid XML document must have a single root node
        pugi::xml_node root = doc.document_element();
        return root;
    }
}// namespace artery
/********************************************************************************
 * EOF
 ********************************************************************************/