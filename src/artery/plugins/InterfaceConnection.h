/********************************************************************************
  \project  INFM_HIL_Interface
  \file     InterfaceConnection.h
  \brief    Provides the class for collecting the live data from the node
  \author   Johannes Winter
  \version  1.0.0
  \date     18.10.2021
 ********************************************************************************/

#ifndef ARTERY_INTERFACECONNECTION_H
#define ARTERY_INTERFACECONNECTION_H

/********************************************************************************
 * Includes
 *********************************************************************************/
#include "../../../../../../../../usr/include/c++/8/fstream"
#include "../../../../../../../../usr/include/c++/8/iostream"
#include "json.hpp"

/********************************************************************************
 * Class declaration
 ********************************************************************************/
class InterfaceConnection {
    
public:

    ~InterfaceConnection();
    InterfaceConnection();

    static void writeToFile(const std::string path, const std::string vehicleID, TraCIAPI::VehicleScope traci);
    static void closeFile(const std::string path);
    static void openFile(const std::string path);



};


#endif /* ARTERY_INTERFACECONNECTION_H */

//EOF