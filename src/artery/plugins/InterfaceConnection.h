//
// Created by vagrant on 10/21/21.
//



#ifndef ARTERY_INTERFACECONNECTION_H
#define ARTERY_INTERFACECONNECTION_H
#include "../../../../../../../../usr/include/c++/8/fstream"
#include "../../../../../../../../usr/include/c++/8/iostream"

namespace artery
{

    class InterfaceConnection {

    public:
        ~InterfaceConnection();
        InterfaceConnection();
        void static writeToFile(const std::string);
        void closeFile();
        void openFile();

    };

} //namespace artery

#endif /* ARTERY_INTERFACECONNECTION_H */
