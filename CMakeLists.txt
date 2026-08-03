cmake_minimum_required(VERSION 3.28)
project(QTScanner_network_1)

set(CMAKE_PREFIX_PATH "C:/msys64/ucrt64/lib/cmake")
set(CMAKE_CXX_STANDARD 20)

find_package(Qt6 COMPONENTS
        Core
        Gui
        Widgets
        Network
        REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

add_executable(QTScanner_network_1
        main.cpp
        src/gui/MainWindow.h
        src/gui/ui_mainwindow.h
        src/gui/MainWindow.cpp
        src/network/IPAddressGenerator.h
        src/network/IPAddressGenerator.cpp
        src/scanner/NetworkScannerWorker.cpp
        src/scanner/DatabaseFingerprinter.h
        src/scanner/DatabaseFingerprinter.cpp
        src/scanner/NetworkScannerWorker.h
        src/scanner/SocketUtils.h
        src/scanner/SocketUtils.cpp

)

target_link_libraries(QTScanner_network_1 PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::Network
)
# Проверяем, что это Windows и мы собираем финальное приложение
if(WIN32)
    # Находим, где лежит стандартный инструмент windeployqt
    find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${CMAKE_PREFIX_PATH}/bin")

    if(WINDEPLOYQT_EXECUTABLE)
        # Приказываем автоматически скопировать все DLL в папку сборки сразу после линковки .exe
        add_custom_command(TARGET QTScanner_network_1 POST_BUILD
                COMMAND ${WINDEPLOYQT_EXECUTABLE}
                --no-compiler-runtime # Не тащим лишние системные библиотеки компилятора
                --verbose 1
                "$<TARGET_FILE:QTScanner_network_1>"
                COMMENT "Профессиональный деплой: сборка зависимостей Qt для переноса на другой ПК..."
        )
    endif()
endif()
