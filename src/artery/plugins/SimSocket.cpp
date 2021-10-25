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
#include <zmq.hpp>
#include "zmq_addon.hpp"

/********************************************************************************
 * Function declaration
 ********************************************************************************/

using namespace std::chrono_literals;

// Constructor without args
SimSocket::SimSocket() {}

// Constructor with args
SimSocket::SimSocket(const std::string &port, const std::string &dataZmq) : port(port), data_zmq(dataZmq) {}

// Destructor
SimSocket::~SimSocket() {

}




// Function for creating the communication Socket
int SimSocket::createSocket(std::string port, std::string data_zmq) {

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
