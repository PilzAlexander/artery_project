//
// Created by vagrant on 10/21/21.
//

#ifndef ARTERY_INTERFACECONNECTION_H
#define ARTERY_INTERFACECONNECTION_H
#include "fstream"
#include "iostream"

namespace artery
{

    class InterfaceConnection {

    public:
        ~InterfaceConnection();
        InterfaceConnection();
        void writeToFile(const std::string &);
        void closeFile();
        void openFile();

    };

} //namespace artery

#endif /* ARTERY_INTERFACECONNECTION_H */
