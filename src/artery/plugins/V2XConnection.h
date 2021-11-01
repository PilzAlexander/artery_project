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

    static void writeToJSON(const std::string vehicleID, TraCIAPI::VehicleScope traci);
    static void writeToMap(std::string path, std::string vehicleID, TraCIAPI::VehicleScope traci);
    static void closeFile(const std::string path);
    static void openFile(const std::string path);
    static void ConvertToJSONFile(nlohmann::json JSON);
};


#endif /* ARTERY_V2XCONNECTION_H */

//EOF