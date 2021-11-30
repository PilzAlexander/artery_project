add_artery_feature(plugins
        GpsdServer.cc
        DUTOtaIndicationQueue.cc
        DUTOtaInterfaceLayer.cc
        DUTOtaInterfaceStub.cc
        DutContention.cc
        DutRadio.cc
        SimSocket.cpp
        DutScheduler.cc
        )

target_link_libraries(plugins PUBLIC INET)

find_package(SEA_V2X CONFIG)
mark_as_advanced(SEA_V2X_DIR)
if (SEA_V2X_FOUND)
    target_sources(plugins PUBLIC
            OtaInterfaceUsrp.cc
            UsrpConnection.cc
            )
    target_link_libraries(plugins PUBLIC SEA_V2X::sea_v2x)
    message(STATUS "Building Plugins with SEA API")
else()
    message(STATUS "Building Plugins without SEA API")
endif()
