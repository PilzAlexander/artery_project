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
#include "traci/Angle.h"
#include "traci/Listener.h"
#include "traci/Position.h"
#include "traci/SubscriptionManager.h"
#include <omnetpp/csimplemodule.h>
#include <memory>
#include <string>
#include "traci/ModuleMapper.h"
#include "traci/PersonSink.h"
#include "traci/VariableCache.h"
#include "traci/VehicleSink.h"
#include <ostream>

/********************************************************************************
 * Function declarations
 ********************************************************************************/
using namespace omnetpp;

namespace traci {

namespace
{
    static const std::set<int> sPersonVariables{
            libsumo::VAR_POSITION, libsumo::VAR_SPEED, libsumo::VAR_ANGLE
    };
    static const std::set<int> sVehicleVariables{
            libsumo::VAR_POSITION, libsumo::VAR_SPEED, libsumo::VAR_ANGLE
    };
    static const std::set<int> sSimulationVariables{
            libsumo::VAR_DEPARTED_VEHICLES_IDS, libsumo::VAR_ARRIVED_VEHICLES_IDS,
            libsumo::VAR_TELEPORT_STARTING_VEHICLES_IDS,
            libsumo::VAR_TIME
    };

class VehicleObjectImpl : public DutNodeManager::VehicleObject
{
public:
    VehicleObjectImpl(std::shared_ptr<VehicleCache> cache) : m_cache(cache) {}
    std::shared_ptr<VehicleCache> getCache() const override { return m_cache; }
    const TraCIPosition &getPosition() const override { return m_cache->get<libsumo::VAR_POSITION>(); }
    TraCIAngle getHeading() const override { return TraCIAngle{m_cache->get<libsumo::VAR_ANGLE>()}; }
    double getSpeed() const override { return m_cache->get<libsumo::VAR_SPEED>(); }

private:
    std::shared_ptr<VehicleCache> m_cache;
};

} // namespace

Define_Module(DutNodeManager)

void DutNodeManager::initialize()
{
    m_twinId = par("twinId").stringValue();
    m_twinName = par("twinName").stringValue();

    std::cout << "m_twinId: " << m_twinId << std::endl;
    std::cout << "m_twinName: " << m_twinName << std::endl;
    BasicNodeManager::initialize();
}

cModule *DutNodeManager::createModule(const std::string &id, omnetpp::cModuleType *type)
{
    if (id == m_twinId) {

        return type->create(m_twinName.c_str(), getSystemModule());

    } else {
        return BasicNodeManager::createModule(id, type);
    }
}

void DutNodeManager::updateVehicle(const std::string & id, VehicleSink * sink)
{
    auto vehicle = m_subscriptions->getVehicleCache(id);

    // kopie aus dem basicnodemanager aus der update vehicle
    auto& traci = m_api->vehicle;

    std::cout << "DUT ID: " << id << std::endl;

    // get vehicle data to send
    artery::SimSocket::getVehicleData("flowNorthSouth.0", traci);

    VehicleObjectImpl update(vehicle);
    emit(updateVehicleSignal, id.c_str(), &update);
    if (sink) {
        sink->updateVehicle(vehicle->get<libsumo::VAR_POSITION>(),
                            TraCIAngle { vehicle->get<libsumo::VAR_ANGLE>() },
                            vehicle->get<libsumo::VAR_SPEED>());
    }
}

}// namespace traci

/********************************************************************************
 * EOF
 ********************************************************************************/