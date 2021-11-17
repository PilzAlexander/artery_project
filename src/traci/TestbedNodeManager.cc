#include <thread>
#include "TestbedNodeManager.h"
#include "omnetpp.h"
#include "iostream"

using namespace omnetpp;

namespace traci
{

Define_Module(TestbedNodeManager)

void TestbedNodeManager::initialize()
{
    m_twinId = par("twinId").stringValue();
    m_twinName = par("twinName").stringValue();

    std::cout << "DUT ID: " << m_twinId << std::endl;

    BasicNodeManager::initialize();
}

cModule* TestbedNodeManager::createModule(const std::string& id, omnetpp::cModuleType* type)
{
    if (id == m_twinId) {
        return type->create(m_twinName.c_str(), getSystemModule());

    } else {
        return BasicNodeManager::createModule(id, type);
    }
}

} /* namespace traci */
