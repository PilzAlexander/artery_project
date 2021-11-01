#include <thread>
#include "TestbedNodeManager.h"
#include "omnetpp.h"

#include "artery/plugins/SimSocket.h"
#include "iostream"

using namespace omnetpp;

namespace traci
{

Define_Module(TestbedNodeManager)

void TestbedNodeManager::initialize()
{
    m_twinId = par("twinId").stringValue();
    m_twinName = par("twinName").stringValue();
    BasicNodeManager::initialize();

    // open socket with initializing the TestbedNodeManager AP
    //SimSocket newSocket = SimSocket("tcp://127.0.0.1:5557", "Sack");

    // send data
    //SimSocket::createSocket(newSocket.getPort(), newSocket.getDataZmq());

}

cModule* TestbedNodeManager::createModule(const std::string& id, omnetpp::cModuleType* type)
{
    if (id == m_twinId) {
        std::cout << "TEEEEEEEEEEEEEEEEEEEEEEEST NODE" << std::endl;
        return type->create(m_twinName.c_str(), getSystemModule());

    } else {
        std::cout << "TEEEEEEEEEEEEEEEEEEEEEEEST ELSE" << std::endl;
        return BasicNodeManager::createModule(id, type);
    }
}

} /* namespace traci */
