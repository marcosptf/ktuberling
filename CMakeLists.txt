project(ktuberling)

cmake_minimum_required (VERSION 2.8.12 FATAL_ERROR)
set (QT_MIN_VERSION "5.3.0")
set (KF5_MIN_VERSION "5.15.0")

find_package(ECM 1.7.0 REQUIRED CONFIG)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

find_package(Qt5 ${QT_MIN_VERSION} REQUIRED NO_MODULE COMPONENTS PrintSupport Svg Widgets Xml Multimedia)
find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS Config)

if(NOT ${CMAKE_SYSTEM_NAME} MATCHES "Android")
    find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS
        Completion
        ConfigWidgets
        CoreAddons
        Crash
        DBusAddons
        KIO
        DocTools
        I18n
        KDELibs4Support #TODO eventually remove kdelibs4support
        WidgetsAddons
        XmlGui
    )

    find_package(KF5KDEGames 4.9.0 REQUIRED)
endif()


include(FeatureSummary)
include(ECMAddAppIcon)
include(ECMInstallIcons)
include(KDEInstallDirs)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(KDECMakeSettings)

add_definitions(${QT_DEFINITIONS})
add_definitions(-DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS)

add_subdirectory(sounds)
add_subdirectory(pics)
if(KF5DocTools_FOUND)
    add_subdirectory(doc)
endif()

########### next target ###############

set(ktuberling_common_SRCS
   action.cpp
   playground.cpp
   todraw.cpp
   soundfactory.cpp
   filefactory.cpp
)

if(${CMAKE_SYSTEM_NAME} MATCHES "Android")
    set(ktuberling_mobile_SRCS
        ${ktuberling_common_SRCS}
        main_mobile.cpp
    )

    qt5_add_resources(ktuberling_mobile_SRCS android_data/resources.qrc)

    add_executable(ktuberling_mobile ${ktuberling_mobile_SRCS})

    target_link_libraries(ktuberling_mobile
        Qt5::Gui
        Qt5::Svg
        Qt5::Multimedia
        Qt5::Xml
        Qt5::Widgets
        KF5::ConfigCore )

    install(TARGETS ktuberling_mobile RUNTIME DESTINATION bin)

else()

    set(ktuberling_SRCS
        ${ktuberling_common_SRCS}
        main.cpp
        toplevel.cpp
        playgrounddelegate.cpp
    )

    file(GLOB ICONS_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/*-apps-ktuberling.png")
    ecm_add_app_icon(ktuberling_SRCS ICONS ${ICONS_SRCS})

    add_executable(ktuberling ${ktuberling_SRCS})

    target_link_libraries(ktuberling
        Qt5::PrintSupport
        Qt5::Svg
        Qt5::Multimedia
        KF5::Completion
        KF5::Crash
        KF5::DBusAddons
        KF5::KIOCore
        KF5::KDELibs4Support
        KF5::XmlGui
        KF5KDEGames
    )

    install(TARGETS ktuberling  ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

    install(PROGRAMS org.kde.ktuberling.desktop  DESTINATION  ${KDE_INSTALL_APPDIR})
    install(FILES ktuberlingui.rc  DESTINATION  ${KDE_INSTALL_KXMLGUI5DIR}/ktuberling)

    ecm_install_icons(ICONS
        128-apps-ktuberling.png
        16-apps-ktuberling.png
        22-apps-ktuberling.png
        32-apps-ktuberling.png
        48-apps-ktuberling.png
        64-apps-ktuberling.png
        128-mimetypes-application-x-tuberling.png
        16-mimetypes-application-x-tuberling.png
        22-mimetypes-application-x-tuberling.png
        32-mimetypes-application-x-tuberling.png
        48-mimetypes-application-x-tuberling.png
        64-mimetypes-application-x-tuberling.png
        DESTINATION ${KDE_INSTALL_ICONDIR} THEME hicolor
    )
endif()

install(FILES org.kde.ktuberling.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})
feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
