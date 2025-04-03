# Does some automation for the qt projects
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 COMPONENTS Core Widgets SerialPort REQUIRED)

if (NOT Qt6_FOUND)
    message(WARNING "QT binaries not found")
    set(QT_FOUND 0)
endif ()

set(QT_FOUND 1)