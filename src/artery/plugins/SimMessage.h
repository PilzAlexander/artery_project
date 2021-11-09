/********************************************************************************
  \project  INFM_HIL_Interface
  \file     SimSocket.h
  \brief    Provides the class for the messages we want to send
  \author   Alexander Pilz
  \author   Johannes Winter
  \version  1.0.0
  \date     09.11.2021
 ********************************************************************************/

/********************************************************************************
 * Includes
 *********************************************************************************/
#include <iostream>
#include <string>
#include <sstream>
#include <boost/serialization/serialization.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include "V2XConnection.h"

#ifndef ARTERY_SIMMESSAGE_H
#define ARTERY_SIMMESSAGE_H

/********************************************************************************
 * Class declaration
 ********************************************************************************/

template <class T>
class SimMessage {

public:
    SimMessage();
    ~SimMessage() = default;
    SimMessage(double speed, double acc, double angle, double distance, double height, double length,
               double width, double lanePos, double signals, double posX, double posY, double posZ, double decel,
               double roadID, double roadIndex, double laneID, double laneIndex/*, double pos3DX, double pos3DY, double pos3DZ*/, std::string end) :
            speed_{speed}, acc_{acc}, angle_{angle}, distance_{distance}, height_{height}, length_{length},
            width_{width}, lanePos_{lanePos}, signals_{signals}, posX_{posX}, posY_{posY}, posZ_{posZ}, decel_{decel},
            roadID_{roadID}, roadIndex_{roadIndex}, laneID_{laneID}, laneIndex_{laneIndex}
            /*, pos3Dx_{pos3DX}, pos3DY_{pos3DY}, pos3DZ_{pos3DZ}*/, end_{end}{}

   // T getData();
   // std::string toString();

private:
    friend class boost::serialization::access;

    template <class Archive>
    void serialize(Archive &ar, V2XConnection &a, const unsigned int version)
    {
        {
            ar & a.speed_;
            ar & a.acc_;
            ar & a.angle_;
            ar & a.distance_;
            ar & a.height_;
            ar & a.length_;
            ar & a.width_;
            ar & a.lanePos_;
            ar & a.angle_;
            ar & a.distance_;
            ar & a.height_;
            ar & a.length_;
            ar & a.width_;
            ar & a.lanePos_;
            ar & a.signals_;
            ar & a.posX_;
            ar & a.posY_;
            ar & a.posZ_;
            ar & a.decel_;
            ar & a.roadID_;
            ar & a.roadIndex_;
            ar & a.laneID_;
            ar & a.laneIndex_;
            ar & a.end_;
            /*ar & a.pos3DX_;
            ar & a.pos3DY_;
            ar & a.pos3DZ_;*/
        }


    }

    double speed_;
    double acc_;
    double angle_;
    double distance_;
    double height_;
    double length_;
    double width_;
    double lanePos_;
    double signals_;
    double posX_;
    double posY_;
    double posZ_;
    double decel_;
    double roadID_;
    double roadIndex_;
    double laneID_;
    double laneIndex_;
    std::string end_;
    /*double pos3DX_;
    double pos3DY_;
    double pos3DZ_;*/
};



#endif //ARTERY_SIMMESSAGE_H

/********************************************************************************
 * EOF
 ********************************************************************************/