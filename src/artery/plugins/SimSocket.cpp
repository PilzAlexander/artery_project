/********************************************************************************
  \project  INFM_HIL_Interface
  \file     SimSocket.h
  \brief    Provides the functions for setting up a socket to send data from the simulation to the interface component
  \author   Alexander Pilz
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
#include <pthread.h>
#include <zmq.hpp>

/********************************************************************************
 * Function declaration
 ********************************************************************************/

using namespace std::chrono_literals;
using namespace std;

// Constructor without args
SimSocket::SimSocket() {}

// Constructor with args
SimSocket::SimSocket(const std::string &port, const std::string &dataZmq) : port(port), data_zmq(dataZmq) {}

// Destructor
SimSocket::~SimSocket() {}

int SimSocket::createSocket(std::string port, std::string data_zmq) {
   const string endpoint = "tcp://localhost:5555";

    // initialize the 0MQ context
   // zmqpp::context context;

    // generate a push socket
  //  zmqpp::socket_type type = zmqpp::socket_type::req;
   //zmqpp::socket socket (context, type);
     //   void *socket =  zmq_socket(context, ZMQ_PAIR);
   // assert(socket);
    // open the connection
    cout << "Connecting to hello world server…" << endl;
   // int rc = zmq_connect(socket, "tcp://localhost:5555");
  // socket.connect(endpoint);
  // socketPointer = &socket;
    data_zmq = "tt";
   // zmqpp::socket& socketPointer = socket;

   // socketPointer = &socket;

    // compose a message from a string and a number


  //  sendTest(socket, "FICKT EUCH ALLE");
    cout << "T-Dog " ;
  //  cout << &socket << endl << endl;

}

// Function for creating the communication Socket
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

void SimSocket::sendMessageZMQ(std::string messageNachricht){
    try {
        // initialize the zmq context with a single IO thread
        zmq::context_t context{1};

        // construct a REQ (request) socket and connect to interface
        zmq::socket_t socketZMQ{context, zmq::socket_type::req};
        socketZMQ.connect("tcp://localhost:5555");

        // set up some static data to send
        socketZMQ.send(zmq::buffer(messageNachricht), zmq::send_flags::none);
        socketZMQ.close();

    }
    catch (zmq::error_t & e){
        cerr << "Error " << e.what() << endl;
    }
}

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
    catch (zmq::error_t & e){
        cerr << "Error " << e.what() << endl;
    }
}

/********************************************************************************
 * Getter and Setter
 ********************************************************************************/

const std::string &SimSocket::getPort() const {
    return port;
}

void SimSocket::setPort(const std::string &port) {
    SimSocket::port = port;
}

const std::string &SimSocket::getDataZmq() const {
    return data_zmq;
}

void SimSocket::setDataZmq(const std::string &dataZmq) {
    data_zmq = dataZmq;
}

/********************************************************************************
 * EOF
 ********************************************************************************/
