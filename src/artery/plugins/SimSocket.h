/********************************************************************************
  \project  INFM_HIL_Interface
  \file     SimSocket.h
  \brief    Provides the class for setting up a socket to send data from the simulation to the interface component
  \author   Alexander Pilz
  \version  1.0.0
  \date     31.10.2021
 ********************************************************************************/

#ifndef ARTERY_SIMSOCKET_H
#define ARTERY_SIMSOCKET_H

/********************************************************************************
 * Includes
 *********************************************************************************/
#include <zmq.hpp>
#include <iostream>

/********************************************************************************
 * Class declaration
 ********************************************************************************/
class SimSocket {

private:
    std::string port;
    std::string data_zmq;

 public:

    SimSocket(const std::string &port, const std::string &dataZmq);

    ~SimSocket();

    static int createSocket(std::string port, std::string data_zmq);
    const std::string &getPort() const;
    static void sendMessage(std::string message);
    static void sendMessageZMQ(std::string message);
    void setPort(const std::string &port);

    const std::string &getDataZmq() const;

    void setDataZmq(const std::string &dataZmq);


};





#endif //ARTERY_SIMSOCKET_H

/********************************************************************************
 * EOF
 ********************************************************************************/
