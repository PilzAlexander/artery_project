/********************************************************************************
  \project  INFM_HIL_Interface
  \file     SimSocket.h
  \brief    Provides the class for setting up a socket to send data from the simulation to the interface component
  \author   Alexander Pilz
  \author   Johannes Winter
  \author   Fabian Genes
  \version  1.0.0
  \date     31.10.2021
 ********************************************************************************/
#ifndef ARTERY_SIMSOCKET_H
#define ARTERY_SIMSOCKET_H
/********************************************************************************
 * Includes
 *********************************************************************************/
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/variant/variant.hpp>
#include "traci/Angle.h"
#include "traci/Boundary.h"
#include "traci/NodeManager.h"
#include "traci/Listener.h"
#include "traci/Position.h"
#include "traci/SubscriptionManager.h"
#include <omnetpp/ccomponent.h>
#include "artery/inet/gemv2/VehicleIndex.h"
#include "artery/inet/gemv2/Visualizer.h"
#include "artery/traci/Cast.h"
#include "traci/BasicNodeManager.h"
#include "traci/API.h"
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/register/linestring.hpp>
#include <boost/geometry/strategies/transform/matrix_transformers.hpp>
#include <boost/range/adaptor/indexed.hpp>
#include <boost/range/adaptor/transformed.hpp>
#include <boost/units/cmath.hpp>
#include <inet/common/ModuleAccess.h>
#include <omnetpp/checkandcast.h>
#include "artery/utility/Geometry.h"
#include <boost/geometry/index/rtree.hpp>
#include <omnetpp/clistener.h>
#include <omnetpp/csimplemodule.h>
#include "artery/application/VehicleKinematics.h"
#include "DUTOtaInterfaceStub.h"

#include <iostream>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <iostream>
#include <bitset>
#include <zmq.hpp>
#include <set>
#include <vector>
#include <algorithm>
#include <array>
/********************************************************************************
 * Class declaration
 ********************************************************************************/
// forward declaration
namespace traci { class API;}

//class API;
class VehicleCache;

namespace artery {

    /**
     * Module for collecting data from the simulation and sending it to the interface hardware
     */
    class SimSocket : public traci::Listener, public omnetpp::cSimpleModule
    {
    public:

        using PortName = std::string; // port address
        using PortContext = zmq::context_t; // context
        using DataMap = std::map <std::string, boost::variant<int, double, std::string>>;
        static const omnetpp::simsignal_t dataStateChanged;

        /**
         * Constructor of SimSocket
         */
        SimSocket();
        /**
         * Deconstructor of SimSocket
         */
        ~SimSocket();

        // socket functions
        void close();
        void connect(const PortName &portName);
        void disconnect(const PortName &portName);
        void bind(const PortName &portName);
        void unbind(const PortName &portName);

        // send and receive functions
        /**
         * Method for publishing vehicle data, such as speed, dynamics, ...
         */
        void publish();
        /**
         * Method for publishing simulated messages addressed to the dut
         * @param byteViewRange
         */
        void publishSimMsg(const vanetza::MacAddress& MacSource, const vanetza::MacAddress& MacDest, const vanetza::byte_view_range& byteViewRange);
        /**
         * Method for subscribing to the interface hardware and receiving vehicle data
         */
        void subscribe();

        void subscribeDutMsg(std::unique_ptr<GeoNetPacket> packet);
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
        //static void setVehicleData(TraCIAPI::VehicleScope traci, DataMap map);

        //getter
        traci::SubscriptionManager *getSubscriptions() { return subscriptions_; }

    protected:
        void initialize() override;
        void finish() override;
        traci::SubscriptionManager *subscriptions_{};

    private:
        PortName portName_;
        PortName subPortName_;
        zmq::socket_t publisherSocket_;
        zmq::socket_t subscriberSocket_;
        zmq::context_t context_;
        std::vector<PortName> connections_;
        std::vector<PortName> bindings_;
        DataMap vehicleDataMap_;
        DataMap tmpVehicleDataMap_;
        DataMap diffVehicleDataMap_;
        DataMap inputDataMap_;
        const traci::VehicleController* mVehicleController = nullptr;

    void receiveSignal(cComponent *, simsignal_t signal, unsigned long, cObject *) override;

    };

} //namespace artery

#endif //ARTERY_SIMSOCKET_H
/********************************************************************************
 * EOF
 ********************************************************************************/