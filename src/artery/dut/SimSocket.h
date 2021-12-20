/********************************************************************************
  \project  INFM_HIL_Interface
  \file     SimSocket.h
  \brief    Provides the class for setting up a socket to send data from the simulation to the interface component and
            also to receive the incoming messages and data from the device under test (DuT)
  \author   Alexander Pilz
  \author   Johannes Winter
  \author   Fabian Genes
  \author   Thanaanncheyan Thavapalan
  \version  1.0.0
  \date     19.12.2021
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
#include "artery/dut/XML/pugixml.hpp"

#include <zmq.hpp>
/********************************************************************************
 * Class declaration
 ********************************************************************************/
// forward declaration
namespace traci { class API; }

class VehicleCache;

namespace artery {

    /**
     * Module for collecting data and messages from the simulation and sending it to the interface component
     * Also receives data and messages from the interface component and puts it into the simulation
     */
    class SimSocket : public traci::Listener, public omnetpp::cSimpleModule {
    public:

        using PortName = std::string;
        using PortContext = zmq::context_t;
        using DataMap = std::map<std::string, boost::variant<int, double, std::string>>;

        SimSocket();
        ~SimSocket();

        /**
         * Closes socket
         */
        void close(zmq::socket_t &socketName);

        /**
         * Connects a subscriber socket to a port
         *
         * @param portName
         */
        void connect(const PortName &portName, zmq::socket_t& socketName);

        /**
         * Disconnects a connected subscriber socket from port
         *
         * @param portName
         */
        void disconnect(const PortName &portName, zmq::socket_t& socketName);

        /**
         * Binds a publisher socket to a port
         * @param portName
         * @param socketName
         */
        void bind(const PortName &portName, zmq::socket_t& socketName);
        /**
         * Unbinds a bound publisher socket from port
         * @param portName
         * @param socketName
         */
        void unbind(const PortName &portName, zmq::socket_t& socketName);
        /**
         * Gets the last endpoint a socket was connected/bound to
         * @param socketName
         * @return
         */
        std::string getLastEndpoint(zmq::socket_t& socketName) const;

        /**
         * Publishes vehicle data, such as speed, dynamics, ...
         */
        void publish();

        /**
         * Publishes simulated (V2X) messages addressed to the dut
         * @param byteViewRange
         */
        void publishSimMsg(const vanetza::MacAddress &macSource, const vanetza::MacAddress &macDest,
                           const vanetza::byte_view_range &byteViewRange);

        /**
         * Subscribes to the publisher socket of the interface component and receive vehicle data
         */
        void subscribe();

        /**
         * Sends the connectorConfig XML to the interface component
         * !Absolute path for config file "connectorsConfig.xml has to be changed!
         * @param stringFilePath
         */
        void sendConfigString(const std::string& stringFilePath) ;

        /**
         * Collects simulated vehicle data from the DutNodeManager
         * @param vehicleID
         * @param traci
         */
        void getVehicleData(std::string vehicleID, TraCIAPI::VehicleScope traci);

        /**
         * Collects the simulated vehicle dynamics
         * @param dynamics
         */
        void getVehicleDynamics(VehicleKinematics dynamics);

        /**
         * Serializes the Mac Addresses and the payload for sending it to the interface component
         * @param macSource source address
         * @param macDest destination address
         * @param byteViewRange payload of message
         * @return returns a serialized string of the simulated message
         */
        std::string serializeSimMsg(const vanetza::MacAddress &macSource, const vanetza::MacAddress &macDest,
                                    const vanetza::byte_view_range &byteViewRange) const;

        /**
         * Serializes the vehicle data for sending it to the interface component
         * @return returns a serialized string of the vehicle data
         */
        std::string serializeVehicleData() const;

        /**
         * Gets the data member inputDataMap
         * @return return dataMap
         */
        const DataMap &getInputDataMap() const;
        traci::SubscriptionManager *getSubscriptions() { return subscriptions_; }

        /**
         * Wait-methode: wait until the interfaace receives the config
         */
        void waitConfigRecive( );

    protected:
        void initialize() override;
        void finish() override;
        traci::SubscriptionManager *subscriptions_{};

    private:
        PortName pubPortName_;
        PortName subPortName_;
        PortName portNameConfig_;
        PortName portNameConfigSub_;
        zmq::socket_t publisherSocket_;
        zmq::socket_t subscriberSocket_;
        zmq::socket_t publisherSocketConfig_;
        zmq::socket_t subscribeSocketConfig_;
        zmq::context_t context_;
        std::vector<PortName> connections_;
        std::vector<PortName> bindings_;
        DataMap vehicleDataMap_;
        DataMap diffVehicleDataMap_;
        DataMap inputDataMap_;
        int count = 0;
        std::string configXMLPath_;
        std::string directoryPath_;
        // DUT has always the same Mac in the simulation
        std::array<unsigned char, 6> macSource_ = {0x0a, 0xaa, 0x00, 0x00, 0x00, 0x01};
        std::array<unsigned char, 6> macDest_{};
        std::vector<unsigned char> payload_;
        const traci::VehicleController *mVehicleController = nullptr;

        void receiveSignal(cComponent *, simsignal_t signal, unsigned long, cObject *) override;

        /**
         * Converts a received String to a byte array
         * @param mac macAddress as string
         * @param bytes byte array to store converted string
         */
        void convertStringToByteArray(std::string &mac, std::array<unsigned char, 6> &bytes);

        std::stringstream getStringstreamOfXML(const std::string &stringFilePath) const;

        /**
         * Opens XML file
         *
         * @return root of xml
         */
        pugi::xml_node openXML() const;

        /**
         * Returns value for set up connection
         * !Absolute path for config file "connectionConfig.xml has to be changed!
         * @param root
         * @param entryName name of child
         * @param attributeName name of attribute of child
         * @return
         */
        std::string getValueFromXML(pugi::xml_node root, std::string entryName, const char *attributeName);
    };

} //namespace artery

#endif //ARTERY_SIMSOCKET_H
/********************************************************************************
 * EOF
 ********************************************************************************/