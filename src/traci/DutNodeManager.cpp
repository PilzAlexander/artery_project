/********************************************************************************
  \project  INFM_HIL_Interface
  \file     DutNodeManager.cpp
  \brief    Node manager for the dut simulation
  \author   Alexander Pilz
  \author   Johannes Winter
  \version  1.0.0
  \date     17.11.2021
 ********************************************************************************/

/********************************************************************************
 * Includes
 *********************************************************************************/
#include "DutNodeManager.h"
#include "artery/plugins/SimSocket.h"
#include "omnetpp.h"
#include "iostream"

/********************************************************************************
 * Function declarations
 ********************************************************************************/
using namespace omnetpp;

namespace traci {

    Define_Module(DutNodeManager)

    void DutNodeManager::initialize() {
        m_twinId = par("twinId").stringValue();
        m_twinName = par("twinName").stringValue();

        std::cout << "DUT ID: " << m_twinId << std::endl;

        BasicNodeManager::initialize();



        // kopie aus dem basicnodemanager aus der update vehicle
        //auto& vehicleID = vehicle->getId();
        //auto& traci = m_api->vehicle;

        // get vehicle data to send
        //artery::SimSocket::getVehicleData("flowNorthSouth.0", traci);

    }

    cModule *DutNodeManager::createModule(const std::string &id, omnetpp::cModuleType *type) {
        if (id == m_twinId) {
            return type->create(m_twinName.c_str(), getSystemModule());

        } else {
            return BasicNodeManager::createModule(id, type);
        }
    }






}// namespace traci

/********************************************************************************
 * EOF
 ********************************************************************************/