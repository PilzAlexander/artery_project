#include "artery/traci/Cast.h"
#include "artery/traci/VehicleMobility.h"
#include <omnetpp/cwatch.h>

#include <iostream>
#include <fstream>
#include <zmq.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>

using namespace traci;

namespace artery
{
    std::ofstream myfile;

    /*
    //  Socket to talk to clients
    void *context = zmq_ctx_new ();
    void *responder = zmq_socket (context, ZMQ_REP);
    int rc = zmq_bind (responder, "tcp://*:5555");
    //InterfaceConnection* Test = new InterfaceConnection();
     */


void VehicleMobility::initializeSink(std::shared_ptr<API> api, std::shared_ptr<VehicleCache> cache, const Boundary& boundary)
{
    ASSERT(api);
    ASSERT(cache);
    mTraci = api;
    mVehicleId = cache->getId();
    mNetBoundary = boundary;
    mController.reset(new VehicleController(api, cache));
    //Test->openFile();
    myfile.open("/home/vagrant/Desktop/fork_repo/artery_project/Test1.txt");

}



void VehicleMobility::initializeVehicle(const TraCIPosition& traci_pos, TraCIAngle traci_heading, double traci_speed)
{
    const auto opp_pos = position_cast(mNetBoundary, traci_pos);
    const auto opp_angle = angle_cast(traci_heading);
    initialize(opp_pos, opp_angle, traci_speed);
}

void VehicleMobility::updateVehicle(const TraCIPosition& traci_pos, TraCIAngle traci_heading, double traci_speed)
{
    const auto opp_pos = position_cast(mNetBoundary, traci_pos);
    const auto opp_angle = angle_cast(traci_heading);
    update(opp_pos, opp_angle, traci_speed);

    const auto vehicleID = getVehicleController()->getVehicleId();

    //std::cout << vehicleID;

    //Test->writeToFile(vehicleID);

    /*
    myfile << "Signals1: " << int(getVehicleController()->getTraCI()->vehicle.getSignals("flow0.0")) << "\n";
    myfile << "Distance: " << int(getVehicleController()->getTraCI()->vehicle.getDistance("flow0.0")) << "\n";
    myfile << "Acceleration: " << int(getVehicleController()->getTraCI()->vehicle.getAcceleration("flow0.0"))
           << "\n";
    myfile << "Speed: " << int(getVehicleController()->getTraCI()->vehicle.getSpeed("flow0.0")) << "\n";
    myfile << "Max Speed: " << int(getVehicleController()->getTraCI()->vehicle.getMaxSpeed("flow0.0")) << "\n";
    myfile << "Speed Mode: " << int(getVehicleController()->getTraCI()->vehicle.getSpeedMode("flow0.0")) << "\n";
    myfile << "Speed Deviation: " << getVehicleController()->getTraCI()->vehicle.getSpeedDeviation("flow0.0")
           << "\n";
    myfile << "CO2 Emission: " << int(getVehicleController()->getTraCI()->vehicle.getCO2Emission("flow0.0"))
           << "\n";
    //myfile << "Emergency Decel: " << int(getVehicleController()->getTraCI()->vehicle.getEmergencyDecel("flow0.0"))
    //    << "\n";
    myfile << "Height: " << int(getVehicleController()->getTraCI()->vehicle.getHeight("flow0.0")) << "\n";
    myfile << "Length: " << int(getVehicleController()->getTraCI()->vehicle.getLength("flow0.0")) << "\n";
    myfile << "Width: " << getVehicleController()->getTraCI()->vehicle.getWidth("flow0.0") << "\n";
    myfile << "Line: " << getVehicleController()->getTraCI()->vehicle.getLine("flow0.0") << "\n";
    myfile << "Vehicle Class: " << getVehicleController()->getTraCI()->vehicle.getVehicleClass("flow0.0") << "\n";
    myfile << "\n\n";

     */
    /*
    assert (rc == 0);
    //while (1) {
    char buffer [10];
    zmq_recv (responder, buffer, 10, 0);
    printf ("Received Hello\n");
    sleep (1);          //  Do some 'work'
    zmq_send (responder, "Feinstes Schweeeeeiiiiiiinnn", 5, 0);
     */
}

VehicleController* VehicleMobility::getVehicleController()
{
ASSERT(mController);
return mController.get();
}

} // namespace artery
