#ifndef ARTERY_GPSDSERVER_H_1ONTYCQA
#define ARTERY_GPSDSERVER_H_1ONTYCQA
/********************************************************************************
 * Includes
 *********************************************************************************/
#include "artery/application/Timer.h"
#include <boost/asio.hpp>
#include <string>
/********************************************************************************
 * Class declaration
 ********************************************************************************/
namespace artery {

    class DUTOtaInterfaceLayer;

    class DUTGpsdServer {
    public:
        /**
         * Construct gpsd server providing NMEA0183 sentences
         * \param timebase for simulation time (YYYY-MM-DD HH:mm:ss)
         * \param port TCP port where server accepts connection
         */
        DUTGpsdServer(const std::string &timebase, unsigned short port);

        ~DUTGpsdServer();

        void sendPositionFix(DUTOtaInterfaceLayer &);

    private:
        void write(const std::string &sentence);

        void waitForListener(unsigned short port);

        boost::asio::io_service mIoService;
        boost::asio::ip::tcp::socket mSocket;
        Timer mTimer;
    };

} // namespace artery

#endif /* ARTERY_GPSDSERVER_H_1ONTYCQA */

