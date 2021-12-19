add_artery_feature(dut
        DUTGpsdServer.cc
        DUTOtaIndicationQueue.cc
        DUTOtaInterfaceLayer.cc
        DUTOtaInterfaceConnection.cc
        DutContention.cc
        DutRadio.cc
        SimSocket.cpp
        DutScheduler.cc
        )

target_link_libraries(dut PUBLIC INET)

find_package(SEA_V2X CONFIG)
mark_as_advanced(SEA_V2X_DIR)
if (SEA_V2X_FOUND)
    target_sources(dut PUBLIC
            OtaInterfaceUsrp.cc
            UsrpConnection.cc
            )
    target_link_libraries(dut PUBLIC SEA_V2X::sea_v2x)
    message(STATUS "Building Plugins with SEA API")
else()
    message(STATUS "Building Plugins without SEA API")
endif()
