//
// Created by vagrant on 10/21/21.
//


#include "artery/Plugins/InterfaceConnection.h"
#include "artery/traci/VehicleMobility.h"
#include "artery/traci/VehicleController.h"
#include "artery/traci/Cast.h"
#include "artery/traci/VehicleMobility.h"
#include <omnetpp/cwatch.h>

namespace artery {
    std::ofstream myfile;

    InterfaceConnection::InterfaceConnection() {
        std::cout<<" was geht ab im constructor";


    }

    InterfaceConnection::~InterfaceConnection() {
        myfile.close();
    }

    void InterfaceConnection::closeFile() {
        myfile.close();
    }

    void InterfaceConnection::openFile() {
        myfile.open("/home/vagrant/Test1.txt");
    }

    void InterfaceConnection::writeToFile(const std::string &vehicleId) {

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
        //   << "\n";
        myfile << "Height: " << int(getVehicleController()->getTraCI()->vehicle.getHeight("flow0.0")) << "\n";
        myfile << "Length: " << int(getVehicleController()->getTraCI()->vehicle.getLength("flow0.0")) << "\n";
        myfile << "Width: " << getVehicleController()->getTraCI()->vehicle.getWidth("flow0.0") << "\n";
        myfile << "Line: " << getVehicleController()->getTraCI()->vehicle.getLine("flow0.0") << "\n";
        myfile << "Vehicle Class: " << getVehicleController()->getTraCI()->vehicle.getVehicleClass("flow0.0") << "\n";

    }
}