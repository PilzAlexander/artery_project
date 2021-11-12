/********************************************************************************
  \project  INFM_HIL_Interface
  \file     SimSocket.h
  \brief    Provides the class for setting up a socket to send data from the simulation to the interface component
  \author   Alexander Pilz
  \author   Fabian Genes
  \version  1.0.0
  \date     31.10.2021
 ********************************************************************************/

#ifndef ARTERY_SIMSOCKET_H
#define ARTERY_SIMSOCKET_H

/********************************************************************************
 * Includes
 *********************************************************************************/

#include <zmq.hpp>
#include "zmq_addon.hpp"
#include <iostream>
#include "json.hpp"
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include "artery/plugins/SimMessage.h"

// testzeug
#include "traci/Angle.h"
#include "traci/Boundary.h"
#include "traci/NodeManager.h"
#include "traci/Listener.h"
#include "traci/Position.h"
#include "traci/SubscriptionManager.h"
#include <omnetpp/ccomponent.h>
#include <omnetpp/csimplemodule.h>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <iostream>
#include <bitset>

#include "artery/utility/Geometry.h"
#include "traci/Angle.h"
#include "traci/Boundary.h"
#include "traci/Position.h"
#include <boost/geometry/index/rtree.hpp>
#include <omnetpp/clistener.h>
#include <omnetpp/csimplemodule.h>
#include <functional>
#include <set>
#include <vector>

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
#include <algorithm>
#include <array>
/********************************************************************************
 * Class declaration
 ********************************************************************************/
// forward declaration
namespace traci { class API; }

//class API;
class ModuleMapper;
class PersonSink;
class VehicleCache;
class VehicleSink;

namespace artery {

    inline SimMessage *VehicleData;

    //class SimSocket : public omnetpp::cSimpleModule, public omnetpp::cListener {

    class SimSocket : public traci::Listener, public omnetpp::cSimpleModule{
    public:



        // signals to define!!!!!!! see basicNodeManager

        using PortName = std::string; // port address
        using PortContext = zmq::context_t; // context
        //using DataSim = std::string; // data to send
        using DataSim = SimMessage;
        static const omnetpp::simsignal_t dataStateChanged;

        // constructors and deconstructor
        SimSocket();

        //SimSocket(PortName portName, DataSim dataSim, PortContext &context);

        ~SimSocket();

        // socket functions
        void close();

        void connect(const PortName &portName);

        void disconnect(const PortName &portName);

        void bind(const PortName &portName);

        void unbind(const PortName &portName);

        // send and receive functions
        void publish();

        static void getVehicleData(std::string vehicleID, TraCIAPI::VehicleScope traci);

        static void sendJSON(nlohmann::basic_json<> json);

        // getter
        const PortName &getPortName() const;

        const DataSim &getDataSim() const;

        const zmq::socket_t &getSocketSim() const;

        const zmq::message_t &getNullMessage() const;

        const std::vector<SimSocket::PortName> &getConnections() const;

        const std::vector<SimSocket::PortName> &getBindings() const;

        const zmq::context_t &getContext() const;

        traci::SubscriptionManager *getSubscriptions() { return subscriptions_; }

    protected:
        void initialize() override;

        void finish() override;

        traci::SubscriptionManager *subscriptions_{};

    private:
        DataSim *dataSim_ ;
        PortName portName_;
        zmq::socket_t socketSim_;
        zmq::socket_t subscriber_;
        zmq::context_t context_;
        zmq::message_t nullMessage_;
        std::vector<PortName> connections_;
        std::vector<PortName> bindings_;
        //static const simsignal_t dataStateChanged;

    void receiveSignal(cComponent *, simsignal_t signal, unsigned long, cObject *);
    };

}//namespace artery
#endif //ARTERY_SIMSOCKET_H

/********************************************************************************
 * EOF
 ********************************************************************************/
