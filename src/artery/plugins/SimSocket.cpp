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
#include <iostream>
#include <utility>
#include <zmq.hpp>

#include "artery/traci/Cast.h"
#include "inet/common/INETMath.h"
#include "traci/CheckTimeSync.h"
#include "artery/plugins/MessageContext.h"
#include "traci/BasicNodeManager.h"

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <sstream>

#include "traci/Angle.h"

#include "traci/API.h"
#include <boost/geometry/strategies/transform/matrix_transformers.hpp>
#include <omnetpp/checkandcast.h>
#include <algorithm>
#include <array>
#include <omnetpp/clistener.h>
#include <omnetpp/csimplemodule.h>


//Alles mögliche
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
 * Function declarations
 ********************************************************************************/
using namespace std;
using namespace omnetpp;
//using namespace traci;
namespace artery {


Define_Module(SimSocket)

//Define signal
//omnetpp::simsignal_t SimSocket::dataStateChanged = omnetpp::cComponent::registerSignal("dataStateChanged");


const simsignal_t SimSocket::dataStateChanged = cComponent::registerSignal("dataStateChanged");

void SimSocket::initialize() {

    //get rtaci from ModulePath
    cModule* traci = getModuleByPath(par("traciModule_2"));

    //Subscribe signal to actual Traci
    if (traci) {
        traci->subscribe(traci::BasicNodeManager::updateSendStatus, this);
        std::cout << "*************************" << std::endl;
    } else {
        throw cRuntimeError("No TraCI module found for signal subscription");
    }

    context_ = zmq::context_t(1);
    //portName_ = "tcp://127.0.0.1:7777";
    portName_ = "tcp://*:7777";
    auto *msgPtr = new SimMessage();

    /*auto * msgPtr = new SimMessage(traci.getSpeed(vehicleID)
            ,traci.getAcceleration(vehicleID)
            ,traci.getAngle(vehicleID)
            ,traci.getDistance(vehicleID)
            ,traci.getHeight(vehicleID)
            ,traci.getLength(vehicleID)
            ,traci.getWidth(vehicleID)
            ,traci.getLanePosition(vehicleID)
            ,traci.getSignals(vehicleID)
            ,traci.getPosition(vehicleID).x
            ,traci.getPosition(vehicleID).y
            ,traci.getPosition(vehicleID).z
            ,traci.getDecel(vehicleID)
            ,traci.getRoadID(vehicleID)
            ,traci.getRouteIndex(vehicleID)
            ,traci.getLaneID(vehicleID)
            ,traci.getLaneIndex(vehicleID)
            ,"\n");*/

    dataSim_ = msgPtr;
    socketSim_ = zmq::socket_t(context_, zmq::socket_type::pub);
    bind(portName_);

    EV_INFO << "portName_ " << portName_ << endl;
    EV_INFO << "context_ " << context_<< endl;
    EV_INFO << "dataSim_" << dataSim_<< endl;
    EV_INFO << "socketSim_" << socketSim_<< endl;

    cout << "Initialisierung erfolgt" << endl;

    //std::thread pubThread();

    //std::thread pubThread([&]{socketPtr->publish("tcp://127.0.0.1:7777", socketPtr->getDataSim());});
    // pubThread.detach();
}

void SimSocket::finish() {
    unbind(portName_);
    cSimpleModule::finish();
}


// constructor without args
SimSocket::SimSocket() {}

// constructor with args
SimSocket::SimSocket(SimSocket::PortName portName, SimSocket::DataSim dataSim, PortContext &context)
        : portName_(std::move(portName)), dataSim_(&dataSim), socketSim_(context, zmq::socket_type::pub),
          subscriber_(context, zmq::socket_type::sub)
//nullMessage_(0)
{

    //socketSim_.set(zmq::sockopt::immediate, false);
    bind(portName_);
    // DEBUG
    cout << "Bound to port Address: " << portName_ << endl;
}

// destructor
SimSocket::~SimSocket() {
    close();
}

void SimSocket::close() {
    socketSim_.close();
}

// connect the socket to a port
void SimSocket::connect(const PortName &portName) {
    try {
        socketSim_.connect(portName_);
        connections_.push_back(portName);
        // DEBUG
        cout << "Connected to Socket: " << portName << endl;
    } catch (zmq::error_t cantConnect) {
        cerr << "Socket can't connect to port: " << cantConnect.what() << endl;
        close();
        return;
    }
}

// disconnect the socket from a port
void SimSocket::disconnect(const PortName &portName) {
    auto connectionIterator = std::find(connections_.begin(), connections_.end(), portName);
    if (connectionIterator == connections_.end()) {
        cerr << "SimSocket::" << portName << "failed to disconnect from SimSocket::" << portName << endl;
        return;
    }

    socketSim_.disconnect(portName_);
    connections_.erase(connectionIterator);
}

// bind the socket to a port
void SimSocket::bind(const PortName &portName) {
    socketSim_.bind(portName_);
    bindings_.push_back(portName);
}

// unbind the socket from a port
void SimSocket::unbind(const PortName &portName) {

    auto bindingIterator = std::find(bindings_.begin(), bindings_.end(), portName);
    if (bindingIterator == bindings_.end()) {
        cerr << "SimSocket::" << portName << "failed to unbind from SimSocket::" << portName << endl;
        return;
    }

    socketSim_.disconnect(portName_);
    connections_.erase(bindingIterator);

    socketSim_.unbind(portName_);
}

void SimSocket::sendMessageZMQ(std::string data_zmq, std::string port, zmq::socket_t socket, zmq::context_t context) {

    try {
        // construct a REQ (request) socket and connect to interface
        zmq::socket_t socketZMQ{context, zmq::socket_type::req};
        // socket.connect(port);
        socket.connect(port);

        for (;;) {
            // set up some static data to send
            socketZMQ.send(zmq::buffer(data_zmq), zmq::send_flags::none);
        }
        socketZMQ.close();
    }
    catch (zmq::error_t &e) {
        cerr << "Error " << e.what() << endl;
    }
}

void SimSocket::publishNew() {

    //std::stringstream ss(std::ios_base::binary| std::ios_base::out| std::ios_base::in);
    //boost::archive::binary_oarchive oa(ss, boost::archive::no_header);

    //std::ofstream ofstream("/home/vagrant/Desktop/#testtesttest1#.txt");
    std::ostringstream archive_stream;
    boost::archive::text_oarchive archive(archive_stream);
    archive << dataSim_;

    std::cout << "######################################### \n";
    //emit(dataStateChanged, );

    //for (;;) {
        std::string outbound_data = archive_stream.str();
        // create buffer size for message
        //zmq::message_t msgToSend(outbound_data.length());
    zmq::message_t msgToSend("TEST Nachricht");

        //zmq::message_t msgToSend(sizeof(dataSim));
        // copy the data string into the message data

        /*memcpy(msgToSend.data(), outbound_data.data(),outbound_data.length());

        if((memcpy(msgToSend.data(), outbound_data.data(),outbound_data.length())) != 0) {
            cerr << "error memcpy" << endl;
        }*/

        try {
            //publish the data
            socketSim_.send(msgToSend, zmq::send_flags::none);
            // testausgabe
            //std::cout << "Accelleration after Thread created: " << dataSim.getAcc() << endl;
            //std::cout << "TestMessage nach .send: " << msgToSend.to_string() << std::endl;
        } catch (zmq::error_t cantSend) {
            cerr << "Socket can't send: " << cantSend.what() << endl;
            unbind(portName_);
            // break;
        }
    //} // loop

}


void SimSocket::publish(const SimSocket::PortName &portName, SimSocket::DataSim dataSim_) {

    MessageContext messageContext;
    messageContext.AddContext("pubContext", 1);
    //zmq::socket_t socketSim_(messageContext.GetContext("pubContext")
    //                       , zmq::socket_type::pub);

    //std::stringstream ss(std::ios_base::binary| std::ios_base::out| std::ios_base::in);
    //boost::archive::binary_oarchive oa(ss, boost::archive::no_header);

    //std::ofstream ofstream("/home/vagrant/Desktop/#testtesttest1#.txt");
    std::ostringstream archive_stream;
    boost::archive::text_oarchive archive(archive_stream);
    archive << dataSim_;

    for (;;) {
        std::string outbound_data = archive_stream.str();
        // create buffer size for message
        zmq::message_t msgToSend(outbound_data.length());
        //zmq::message_t msgToSend(sizeof(dataSim));
        // copy the data string into the message data

        /*memcpy(msgToSend.data(), outbound_data.data(),outbound_data.length());

        if((memcpy(msgToSend.data(), outbound_data.data(),outbound_data.length())) != 0) {
            cerr << "error memcpy" << endl;
        }*/

        try {
            //publish the data
            socketSim_.send(msgToSend, zmq::send_flags::none);
            // testausgabe
            //std::cout << "Accelleration after Thread created: " << dataSim.getAcc() << endl;
            //std::cout << "TestMessage nach .send: " << msgToSend.to_string() << std::endl;
        } catch (zmq::error_t cantSend) {
            cerr << "Socket can't send: " << cantSend.what() << endl;
            unbind(portName);
            // break;
        }
    } // loop
}

// function for sending data to the interface
/*void SimSocket::sendToInterface(const SimSocket::PortName & portName
void SimSocket::sendToInterface(const SimSocket::PortName & portName
        , SimSocket::DataSim dataSim
        , zmq::send_flags flags
        , zmq::context_t &context) {

    zmq::context_t contextPub{1};
    zmq::socket_t socketSim_(contextPub, zmq::socket_type::pub);

    for(;;){
        // create buffer size for message
        zmq::message_t data(dataSim.length());
        // copy the data string into the message data
        memcpy(data.data(), dataSim.c_str(), data.size());

        try {
            //publish the data
            socketSim_.send(data, zmq::send_flags::none);
            // testausgabe
            std::cout << "SimTime: " << simTime() << std::endl;
        } catch (zmq::error_t cantSend) {
            cerr << "Socket can't send: " <<  cantSend.what() << endl;
            unbind(portName);
            break;
        }
    } // loop
}*/

void SimSocket::sendMessage(std::string messageNachricht) {

    /*

      const string endpoint = "tcp://localhost:5555";

      // initialize the 0MQ context
      zmqpp::context context;

      // generate a push socket
        zmqpp::socket_type type = zmqpp::socket_type::req;
      zmqpp::socket socket (context, type);
      // open the connection
      cout << "Connecting to hello world server…" << endl;

      socket.connect(endpoint);

      cout << "F-Dog";
          cout << socketPointer<< endl << endl;
         // socket.send(messageNachricht);
      pthread_t t1 ,t2;
      int i1,i2;
      i1 = pthread_create(&t1, NULL, reinterpret_cast<void *(*)(void *)>( socketPointer->send(messageNachricht)), (void*) "t1");
      pthread_join(t1,NULL);
          socket.close();

      int request_nbr;
      for (request_nbr = 0; request_nbr != 10; request_nbr++) {
          // send a message
          cout << "Sending Hello " << request_nbr <<"…" << endl;
          zmqpp::message message;
          // compose a message from a string and a number
          message << messageNachricht;
         pthread_t t1 ,t2;
           int i1,i2;
         i1 = pthread_create(&t1, NULL, reinterpret_cast<void *(*)(void *)>( socketPointer->send(message)), (void*) "t1");
         // pthread_join(t1,NULL);
         cout << &socketPointer;
         socket.send(message);

         // socketPointer->receive(buffer);
         // socket.receive(buffer);
           char buf [256];
          int nbytes = zmq_recv(socketPointer, buf, 256, 0);
          cout << "F-Dog"<< endl;
          cout << "Received World " << request_nbr << endl;
      }
      */

}

// function to send a json string via zeromq
void SimSocket::sendJSON(nlohmann::basic_json<> json) {
    try {
        // initialize the zmq context with a single IO thread
        // TODO this initialisations and connections have to happen somewhere else (just once)
        zmq::context_t context{1};

        // construct a REQ (request) socket and connect to interface
        zmq::socket_t socketZMQ{context, zmq::socket_type::req};
        socketZMQ.connect("tcp://localhost:5555");

        // json string manipulation stuff
        // mimic python json.dump();
        std::string json_str = json.dump();
        // create buffer size for message
        zmq::message_t query(json_str.length());
        // copy the json string into the message data
        memcpy(query.data(), (json_str.c_str()), (json_str.size()));

        // send the data
        socketZMQ.send(query, zmq::send_flags::none);
        // close the socket TODO close in deconstructor or somewhere else.
        socketZMQ.close();
    }
    catch (zmq::error_t &e) {
        cerr << "Error " << e.what() << endl;
    }
}

//recieve NodeUpdate Signal from BasicNodeManager
void SimSocket::receiveSignal(cComponent*, simsignal_t signal, unsigned long, cObject*)
{
    if (signal == traci::BasicNodeManager::updateSendStatus) {
        publishNew();
        std::cout << "Zeit: " << simTime() << std::endl;
    }
}

/********************************************************************************
 * Getter and Setter
 ********************************************************************************/

const SimSocket::PortName &SimSocket::getPortName() const {
    return portName_;
}

const SimSocket::DataSim &SimSocket::getDataSim() const {
    return *dataSim_;
}

const zmq::socket_t &SimSocket::getSocketSim() const {
    return socketSim_;
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