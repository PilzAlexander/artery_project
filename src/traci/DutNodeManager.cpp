/********************************************************************************
  \project  INFM_HIL_Interface
  \file     DutNodeManager.cpp
  \brief    Node manager for the dut simulation, also the vehicle data are gotten from here and are being set here
  \author   Alexander Pilz
  \author   Johannes Winter
  \version  1.0.0
  \date     19.12.2021
 ********************************************************************************/
/********************************************************************************
 * Includes
 *********************************************************************************/
#include "DutNodeManager.h"
#include "artery/dut/SimSocket.h"
#include "omnetpp.h"
#include "iostream"
#include "traci/Angle.h"
#include "traci/Position.h"
#include "traci/SubscriptionManager.h"
#include <memory>
#include <string>
#include "traci/ModuleMapper.h"
#include "traci/PersonSink.h"
#include "traci/VariableCache.h"
#include "traci/VehicleSink.h"
#include <ostream>
#include <any>
#include "boost/variant/get.hpp"

#include "artery/traci/VehicleController.h"
#include "artery/application/StationType.h"
#include "artery/storyboard/Vehicle.h"
/********************************************************************************
 * Function declarations
 ********************************************************************************/
using namespace omnetpp;
using namespace vanetza;

namespace traci {

    namespace {
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

        class VehicleObjectImpl : public DutNodeManager::VehicleObject {
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

    void DutNodeManager::initialize() {
        m_twinId = par("twinId").stringValue();
        m_twinName = par("twinName").stringValue();
        EV_INFO << "m_twinId: " << m_twinId << std::endl;
        EV_INFO << "m_twinName: " << m_twinName << std::endl;

        BasicNodeManager::initialize();
    }

    cModule *DutNodeManager::createModule(const std::string &id, omnetpp::cModuleType *type) {
        if (id == m_twinId) {
            return type->create(m_twinName.c_str(), getSystemModule());
        } else {
            return BasicNodeManager::createModule(id, type);
        }
    }

    void DutNodeManager::updateVehicle(const std::string &id, VehicleSink *sink) {
        auto vehicle = m_subscriptions->getVehicleCache(id);
        auto &traci = m_api->vehicle;
        std::string dutName = par("twinId");

        // get vehicle data to send
        if (id == dutName) {
            // create module pointer to SimSocket with ID = 6
            cModule *mod = getSimulation()->getModule(6);
            auto *m_target = check_and_cast<artery::SimSocket *>(mod);
            m_target->getVehicleData(id, traci);
            setVehicleData(id, m_target->getInputDataMap());
        }

        VehicleObjectImpl update(vehicle);
        emit(updateVehicleSignal, id.c_str(), &update);
        if (sink) {
            sink->updateVehicle(vehicle->get<libsumo::VAR_POSITION>(),
                                TraCIAngle{vehicle->get<libsumo::VAR_ANGLE>()},
                                vehicle->get<libsumo::VAR_SPEED>());
        }
    }

    void DutNodeManager::setVehicleData(const std::string &id, artery::SimSocket::DataMap inputDataMap) {
        auto vehicle = m_subscriptions->getVehicleCache(id);
        auto &traci = m_api->vehicle;

        if (inputDataMap["Operation"] == boost::variant<int, double, std::string>("Speed_DUT")) {

            std::string speedStr = boost::get<std::string>(inputDataMap["Value"]);
            double speed = atof(speedStr.c_str());
            if (traci.getMaxSpeed(id) >= speed) {
                traci.setSpeed(id, speed);
            } else {
                EV_INFO << "Received speed exceeds simulated max speed of dut" << endl;
            }
        }

        if (inputDataMap["Operation"] == boost::variant<int, double, std::string>("Signals_DUT")) {

            std::string signalsStr = boost::get<std::string>(inputDataMap["Value"]);
            int signals = atoi(signalsStr.c_str());
            traci.setSignals(id, signals);
        }

        if (inputDataMap["Operation"] == boost::variant<int, double, std::string>("SpeedMode_DUT")) {

            std::string speedModeStr = boost::get<std::string>(inputDataMap["Value"]);
            int speedMode = atoi(speedModeStr.c_str());
            traci.setSpeedMode(id, speedMode);
        }

        if (inputDataMap["Operation"] == boost::variant<int, double, std::string>("MaxSpeed_DUT")) {

            std::string maxSpeedStr = boost::get<std::string>(inputDataMap["Value"]);
            double maxSpeed = atof(maxSpeedStr.c_str());
            traci.setMaxSpeed(id, maxSpeed);
        }

        if (inputDataMap["Operation"] == boost::variant<int, double, std::string>("SpeedFactor_DUT")) {

            std::string speedFactorStr = boost::get<std::string>(inputDataMap["Value"]);
            double speedFactor = atof(speedFactorStr.c_str());
            traci.setSpeedFactor(id, speedFactor);
        }

        /*
         * Insert more "setter blocks" for desired values
         */
    }
}// namespace traci
/********************************************************************************
 * EOF
 ********************************************************************************/