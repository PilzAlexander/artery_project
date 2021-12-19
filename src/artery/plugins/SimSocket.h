/********************************************************************************
  \project  INFM_HIL_Interface
  \file     SimSocket.h
  \brief    Provides the class for setting up a socket to send data from the simulation to the interface component
  \author   Alexander Pilz
  \author   Johannes Winter
  \author   Fabian Genes
  \author   Thanaanncheyan Thavapalan
  \version  1.0.0
  \date     31.10.2021
 ********************************************************************************/
#ifndef ARTERY_SIMSOCKET_H
#define ARTERY_SIMSOCKET_H
/********************************************************************************
 * Includes
 *********************************************************************************/
#include "artery/application/VehicleKinematics.h"
#include "DUTOtaInterfaceConnection.h"
#include <inet/common/ModuleAccess.h>
#include "traci/Boundary.h"
#include "traci/Listener.h"
#include "traci/SubscriptionManager.h"

#include <zmq.hpp>
/********************************************************************************
 * Class declaration
 ********************************************************************************/
// forward declaration
namespace traci { class API; }

class VehicleCache;

namespace artery {

    /**
     * Module for collecting data from the simulation and sending it to the interface hardware
     */
    class SimSocket : public traci::Listener, public omnetpp::cSimpleModule {
    public:

        using PortName = std::string;
        using PortContext = zmq::context_t;
        using DataMap = std::map<std::string, boost::variant<int, double, std::string>>;

        /**
         * Constructor of SimSocket
         */
        SimSocket();

        /**
         * Deconstructor of SimSocket
         */
        ~SimSocket();

        /**
         * close socket
         */
        void close(zmq::socket_t &socketName);

        /**
         * connect to port
         *
         * @param portName
         */
        void connect(const PortName &portName, zmq::socket_t& socketName);

        /**
         * disconnect from port
         *
         * @param portName
         */
        void disconnect(const PortName &portName, zmq::socket_t& socketName);
        void bind(const PortName &portName, zmq::socket_t& socketName);
        void unbind(const PortName &portName, zmq::socket_t& socketName);
        std::string getLastEndpoint(zmq::socket_t& socketName) const;

        /**
         * Method for publishing vehicle data, such as speed, dynamics, ...
         */
        void publish();

        /**
         * Method for publishing simulated messages addressed to the dut
         * @param byteViewRange
         */
        void publishSimMsg(const vanetza::MacAddress &macSource, const vanetza::MacAddress &macDest,
                           const vanetza::byte_view_range &byteViewRange);

        /**
         * Method for subscribing to the interface component and receiving vehicle data
         */
        void subscribe();

        /**
       * Method for sending  the connectorConfig  XML to the Interface
       * @param stringFilePath
       */
        void sendConfigString(std::string stringFilePath) ;

        /**
         * Method for collecting simulated vehicle data from the DutNodeManager
         * @param vehicleID
         * @param traci
         */
        void getVehicleData(std::string vehicleID, TraCIAPI::VehicleScope traci);

        /**
         * Method for collecting the simulated vehicle dynamics
         * @param dynamics
         */
        void getVehicleDynamics(VehicleKinematics dynamics);

        /**
         * Method for serializing the Mac Addresses and the payload for sending it to the interface component
         * @param macSource
         * @param macDest
         * @param byteViewRange
         * @return
         */
        std::string serializeSimMsg(const vanetza::MacAddress &macSource, const vanetza::MacAddress &macDest,
                                    const vanetza::byte_view_range &byteViewRange) const;

        /**
         * Method for serializing the vehicle dataa for sending it to the interface component
         * @return
         */
        std::string serializeVehicleData() const;

        const DataMap &getInputDataMap() const;
        traci::SubscriptionManager *getSubscriptions() { return subscriptions_; }

    protected:
        void initialize() override;
        void finish() override;
        traci::SubscriptionManager *subscriptions_{};

    private:
        PortName pubPortName_;
        PortName subPortName_;
        PortName portNameConfig_;
        zmq::socket_t publisherSocket_;
        zmq::socket_t subscriberSocket_;
        zmq::socket_t publisherSocketConfig_;
        zmq::context_t context_;
        std::vector<PortName> connections_;
        std::vector<PortName> bindings_;
        DataMap vehicleDataMap_;
        DataMap diffVehicleDataMap_;
        DataMap inputDataMap_;
        int count = 0;
        std::string configXMLPath_;
        // DUT has always the same Mac in the simulation
        std::array<unsigned char, 6> macSource_ = {0x0a, 0xaa, 0x00, 0x00, 0x00, 0x01};
        std::array<unsigned char, 6> macDest_{};
        std::vector<unsigned char> payload_;
        const traci::VehicleController *mVehicleController = nullptr;

        void receiveSignal(cComponent *, simsignal_t signal, unsigned long, cObject *) override;

        /**
         * Method for converting a received String to a byte array
         * @param mac
         * @param bytes
         */
        void convertStringToByteArray(std::string &mac, std::array<unsigned char, 6> &bytes);

    };

} //namespace artery

#endif //ARTERY_SIMSOCKET_H
/********************************************************************************
 * EOF
 ********************************************************************************/