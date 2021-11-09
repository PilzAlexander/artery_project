/********************************************************************************
  \project  INFM_HIL_Interface
  \file     V2XConnection.h
  \brief    Provides the class for collecting the live data from the node
  \author   Johannes Winter
  \version  1.0.0
  \date     18.10.2021
 ********************************************************************************/

#ifndef ARTERY_V2XCONNECTION_H
#define ARTERY_V2XCONNECTION_H

/********************************************************************************
 * Includes
 *********************************************************************************/
#include <boost/serialization/access.hpp>
#include <utils/traci/TraCIAPI.h>
#include "../../../../../../../../usr/include/c++/8/fstream"
#include "../../../../../../../../usr/include/c++/8/iostream"
#include "json.hpp"
/********************************************************************************
 * Class declaration
 ********************************************************************************/
class V2XConnection {
public:
    ~V2XConnection();
    V2XConnection();
    V2XConnection(double speed, double acc, double angle, double distance, double height, double length,
                  double width, double lanePos, double signals, double posX, double posY, double posZ, double decel,
                  double roadID, double roadIndex, double laneID, double laneIndex/*, double pos3DX, double pos3DY, double pos3DZ*/, std::string end) :
                  speed_{speed}, acc_{acc}, angle_{angle}, distance_{distance}, height_{height}, length_{length},
                  width_{width}, lanePos_{lanePos}, signals_{signals}, posX_{posX}, posY_{posY}, posZ_{posZ}, decel_{decel},
                  roadID_{roadID}, roadIndex_{roadIndex}, laneID_{laneID}, laneIndex_{laneIndex}
                  /*, pos3Dx_{pos3DX}, pos3DY_{pos3DY}, pos3DZ_{pos3DZ}*/, end_{end}{}
                  double Speed() const {return speed_;}
                  double Acc() const {return acc_;}
                  double Angle() const {return angle_;}
                  double Distance() const {return distance_;}
                  double Height() const {return height_;}
                  double Length() const {return length_;}
                  double Width() const {return width_;}
                  double LanePos() const {return lanePos_;}
                  double Signals() const {return signals_;}
                  double PosX() const {return posX_;}
                  double PosY() const {return posY_;}
                  double PosZ() const {return posZ_;}
                  double Decel() const {return decel_;}
                  double RoadID() const {return roadID_;}
                  double RoadIndex() const {return roadIndex_;}
                  double LaneID() const {return laneID_;}
                  double LaneIndex() const {return laneIndex_;}
                  std::string End() const {return end_;}
                  /*
                  double Pos3DX() const {return pos3DX_;}
                  double Pos3DY() const {return pos3DY_;}
                  double Pos3DZ() const {return pos3DZ_;}*/

    static void writeToJSON(const std::string vehicleID, TraCIAPI::VehicleScope traci);
    static void writeToMap(std::string path, std::string vehicleID, TraCIAPI::VehicleScope traci);
    static void closeFile(const std::string path);
    static void openFile(const std::string path);
    static void ConvertToJSONFile(nlohmann::json JSON);
    static void initializeMap();
    static void prepareSerializing();

    //void save();
    //void load();

private:
    friend class boost::serialization::access;

    template <typename Archive>
    friend void serialize(Archive &ar, V2XConnection &a, const unsigned int version);

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

#endif /* ARTERY_V2XCONNECTION_H */

//EOF