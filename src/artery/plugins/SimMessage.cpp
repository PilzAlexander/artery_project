/********************************************************************************
  \project  INFM_HIL_Interface
  \file     SimSocket.h
  \brief    Provides the members for the messages we want to send
  \author   Alexander Pilz
  \author   Johannes Winter
  \version  1.0.0
  \date     09.11.2021
 ********************************************************************************/

/********************************************************************************
 * Includes
 *********************************************************************************/
#include "SimMessage.h"
#include <utility>
#include <string>
/********************************************************************************
 *
 *********************************************************************************/
SimMessage::SimMessage(double speed, double acc, double angle, double distance, double height, double length,
                       double width, double lanePos, int signals, double posX, double posY, double posZ, double decel,
                       std::string roadID, int roadIndex, std::string laneID, int laneIndex, const char *end)
                       : speed_{speed}
        , acc_{acc}
        , angle_{angle}
        , distance_{distance}
        , height_{height}
        , length_{length}
        , width_{width}
        , lanePos_{lanePos}
        , signals_{signals}
        , posX_{posX}
        , posY_{posY}
        , posZ_{posZ}
        , decel_{decel}
        , roadID_{roadID}
        , roadIndex_{roadIndex}
        , laneID_{laneID}
        , laneIndex_{laneIndex}
        /*, pos3Dx_{pos3DX}, pos3DY_{pos3DY}, pos3DZ_{pos3DZ}*/
        , end_{end}{}

// constructor with args
SimMessage::SimMessage() {

}

// getter
double SimMessage::getSpeed() const {
    return speed_;
}

double SimMessage::getAcc() const {
    return acc_;
}

double SimMessage::getAngle() const {
    return angle_;
}

double SimMessage::getDistance() const {
    return distance_;
}

double SimMessage::getHeight() const {
    return height_;
}

double SimMessage::getLength() const {
    return length_;
}

double SimMessage::getWidth() const {
    return width_;
}

double SimMessage::getLanePos() const {
    return lanePos_;
}

int SimMessage::getSignals() const {
    return signals_;
}

double SimMessage::getPosX() const {
    return posX_;
}

double SimMessage::getPosY() const {
    return posY_;
}

double SimMessage::getPosZ() const {
    return posZ_;
}

double SimMessage::getDecel() const {
    return decel_;
}

const std::string &SimMessage::getRoadId() const {
    return roadID_;
}

int SimMessage::getRoadIndex() const {
    return roadIndex_;
}

const std::string &SimMessage::getLaneId() const {
    return laneID_;
}

int SimMessage::getLaneIndex() const {
    return laneIndex_;
}

const std::string &SimMessage::getEnd() const {
    return end_;
}





SimMessage::~SimMessage() = default;

/********************************************************************************
 * EOF
 ********************************************************************************/