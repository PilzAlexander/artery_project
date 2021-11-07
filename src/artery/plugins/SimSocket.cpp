/********************************************************************************
  \project  INFM_HIL_Interface
  \file     SimSocket.h
  \brief    Provides the functions for setting up a socket to send data from the simulation to the interface component
  \author   Alexander Pilz
  \author   Fabian Genes
  \version  1.0.0
  \date     31.10.2021
 ********************************************************************************/


/********************************************************************************
 * Includes
 *********************************************************************************/

#include "SimSocket.h"
#include <chrono>
#include <thread>
#include <iostream>
#include <zmq.hpp>

#include "artery/traci/Cast.h"
#include "inet/common/INETMath.h"
#include "traci/CheckTimeSync.h"
#include "traci/Core.h"
#include "artery/plugins/MessageContext.h"

/********************************************************************************
 * Function declarations
 ********************************************************************************/

using namespace std;

// constructor without args
SimSocket::SimSocket() {}

// constructor with args
SimSocket::SimSocket(SimSocket::PortName portName
                     , SimSocket::DataSim dataSim
                     ,PortContext &context)
        : portName_(portName)
        , dataSim_(dataSim)
        , socketSim_(context, zmq::socket_type::pub)
        , subscriber_ (context, zmq::socket_type::sub)
        //nullMessage_(0)
{

    //socketSim_.set(zmq::sockopt::immediate, false);

    bind(portName_);
    // DEBUG
    cout << "Bound to port Address: " << portName_ << endl;
}

// destructor
SimSocket::~SimSocket()
{
    //close();
}

void SimSocket::close()
{
    socketSim_.close();
}

// connect the socket to a port
void SimSocket::connect(const PortName & portName)
{
    try {

        socketSim_.connect(portName_);
        connections_.push_back(portName);
        // DEBUG
        cout << "Connected to Socket: " << portName << endl;
    } catch (zmq::error_t cantConnect) {
        cerr << "Socket can't connect to port: " <<  cantConnect.what() << endl;
        close();
        return;
    }
}

// disconnect the socket from a port
void SimSocket::disconnect(const PortName & portName)
{
    auto connectionIterator = std::find(connections_.begin()
            , connections_.end()
            , portName);
    if(connectionIterator == connections_.end()){
        cerr << "SimSocket::" << portName << "failed to disconnect from SimSocket::" << portName << endl;
        return;
    }

    socketSim_.disconnect(portName_);
    connections_.erase(connectionIterator);
}

// bind the socket to a port
void SimSocket::bind(const PortName & portName)
{
    socketSim_.bind(portName_);
    bindings_.push_back(portName);
}

// unbind the socket from a port
void SimSocket::unbind(const PortName & portName) {

    auto bindingIterator = std::find(bindings_.begin()
                                , bindings_.end()
                                , portName);
    if(bindingIterator == bindings_.end()){
        cerr << "SimSocket::" << portName << "failed to unbind from SimSocket::" << portName << endl;
        return;
    }

    socketSim_.disconnect(portName_);
    connections_.erase(bindingIterator);

    socketSim_.unbind(portName_);

}

// function for creating the communication Socket
/*int SimSocket::createSocket(std::string port, std::string data_zmq) {

    using namespace std::chrono_literals;

    // initialize the zmq context with a single IO thread
    zmq::context_t context{1};

    // construct a REP (reply) socket and bind to interface
    zmq::socket_t socket{context, zmq::socket_type::rep};
    socket.bind(port);

    for (int i = 0; i < 10; i ++)
    {
        zmq::message_t request;

        // receive a request from client
        try {
            socket.recv(request, zmq::recv_flags::none);
        } catch (zmq::error_t) {
            std::cout << "Fehler bei Receive" << std::endl;
        }

        std::cout << "Received " << request.to_string() << std::endl;

        // simulate work
        std::this_thread::sleep_for(1s);

        // send the reply to the client
        socket.send(zmq::buffer(data_zmq), zmq::send_flags::none);

    }

    socket.close();
    return 0;

}*/

void SimSocket::sendMessageZMQ(std::string data_zmq
                               , std::string port
                               , zmq::socket_t socket
                               , zmq::context_t context){

    try {

        // construct a REQ (request) socket and connect to interface
        zmq::socket_t socketZMQ{context, zmq::socket_type::req};
        // socket.connect(port);
        socket.connect(port);

        for(;;){

            // set up some static data to send
            socketZMQ.send(zmq::buffer(data_zmq), zmq::send_flags::none);

        }
        socketZMQ.close();
    }
    catch (zmq::error_t & e){
        cerr << "Error " << e.what() << endl;
    }
}

void SimSocket::publish(const SimSocket::PortName & portName
        , SimSocket::DataSim dataSim)
        {

    MessageContext messageContext;
    messageContext.AddContext("pubContext", 1);
    zmq::socket_t socketSim_(messageContext.GetContext("pubContext")
                             , zmq::socket_type::pub);
int i = 0;
    for (;;) {
        // create buffer size for message
        zmq::message_t data(dataSim.length());
        // copy the data string into the message data
        memcpy(data.data(), dataSim.c_str(), data.size());

        try {
            //publish the data
            socketSim_.send(data, zmq::send_flags::none);
            // testausgabe


            std::cout << i++ << std::endl;
            //std::cout << dataSim << std::endl;
            //std::cout << "SimTime: " << simTime() << std::endl;
            if(i > 100000) {
                break;
            }
        } catch (zmq::error_t cantSend) {
            cerr << "Socket can't send: " << cantSend.what() << endl;
            unbind(portName);
            break;
        }
    } // loop
}



// function for sending data to the interface
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
}




void SimSocket::sendMessage(std::string messageNachricht)
{

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


         *
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
void SimSocket::sendJSON(nlohmann::basic_json<> json)
{
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
    catch (zmq::error_t & e){
        cerr << "Error " << e.what() << endl;
    }
}



/********************************************************************************
 * Getter and Setter
 ********************************************************************************/

const SimSocket::PortName &SimSocket::getPortName() const {
    return portName_;
}

const SimSocket::DataSim &SimSocket::getDataSim() const {
    return dataSim_;
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

void SimSocket::setPortName(const SimSocket::PortName &portName) {
    portName_ = portName;
}

void SimSocket::setDataSim(const SimSocket::DataSim &dataSim) {
    dataSim_ = dataSim;
}















/********************************************************************************
 * EOF
 ********************************************************************************/
