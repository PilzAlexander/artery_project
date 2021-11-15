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

class SimMessage {

public:
    SimMessage();
    // constructor with args
    SimMessage(double speed, double acc, double angle, double distance, double height, double length, double width,
               double lanePos, int signals, double posX, double posY, double posZ, double decel, std::string roadID,
               int roadIndex, std::string laneID, int laneIndex, const char end[2]);

    ~SimMessage();

    double getSpeed() const;

    double getAcc() const;

    double getAngle() const;

    double getDistance() const;

    double getHeight() const;

    double getLength() const;

    double getWidth() const;

    double getLanePos() const;

    int getSignals() const;

    double getPosX() const;

    double getPosY() const;

    double getPosZ() const;

    double getDecel() const;

    const std::string &getRoadId() const;

    int getRoadIndex() const;

    const std::string &getLaneId() const;

    int getLaneIndex() const;

    const std::string &getEnd() const;
    // T getData();
   // std::string toString();

private:
    friend class boost::serialization::access;

    template <class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {
            ar & speed_;
            ar & acc_;
            ar & angle_;
            ar & distance_;
            ar & height_;
            ar & length_;
            ar & width_;
            ar & lanePos_;
            ar & angle_;
            ar & distance_;
            ar & height_;
            ar & length_;
            ar & width_;
            ar & lanePos_;
            ar & signals_;
            ar & posX_;
            ar & posY_;
            ar & posZ_;
            ar & decel_;
            ar & roadID_;
            ar & roadIndex_;
            ar & laneID_;
            ar & laneIndex_;
            ar & end_;
            /*ar & a.pos3DX_;
            ar & a.pos3DY_;
            ar & a.pos3DZ_;*/
    }

    double speed_{};
    double acc_{};
    double angle_{};
    double distance_{};
    double height_{};
    double length_{};
    double width_{};
    double lanePos_{};
    int signals_{};
    double posX_{};
    double posY_{};
    double posZ_{};
    double decel_{};
    std::string roadID_{};
    int roadIndex_{};
    std::string laneID_{};
    int laneIndex_{};
    std::string end_;
    /*double pos3DX_;
    double pos3DY_;
    double pos3DZ_;*/
};

#endif //ARTERY_SIMMESSAGE_H

/********************************************************************************
 * EOF
 ********************************************************************************/