cmake_minimum_required(VERSION 3.21)

project(CinemaManagement VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

add_library(cms_core STATIC
    src/DataStructures/HashTable.cpp
    src/DataStructures/Matrix2D.cpp
    src/controller/AuthController.cpp
    src/controller/FilmController.cpp
    src/controller/AuditoriumController.cpp
    src/controller/OperationsController.cpp
    src/repository/ProjectDataEngine.cpp
    src/repository/txt/TxtDatabase.cpp
    src/service/AuthService.cpp
    src/service/FilmService.cpp
    src/service/AuditoriumService.cpp
    src/service/ScreeningService.cpp
    src/service/SaleService.cpp
    src/service/ReportService.cpp
    src/service/ServiceSupport.cpp
)

target_include_directories(cms_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/DataStructures
)

find_library(CMS_CRYPT_LIBRARY crypt REQUIRED)
target_link_libraries(cms_core PUBLIC ${CMS_CRYPT_LIBRARY})

target_compile_options(cms_core PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic>
)

add_executable(cms_core_check
    tests/CoreSmokeTest.cpp
)
target_link_libraries(cms_core_check PRIVATE cms_core)

add_executable(cms_storage_check
    tests/TxtDatabaseTest.cpp
)
target_link_libraries(cms_storage_check PRIVATE cms_core)

add_executable(cms_data_engine_check
    tests/ProjectDataEngineTest.cpp
)
target_link_libraries(cms_data_engine_check PRIVATE cms_core)

add_executable(cms_services_check
    tests/MinimumServicesTest.cpp
)
target_link_libraries(cms_services_check PRIVATE cms_core)

enable_testing()
add_test(NAME cms_core_check COMMAND cms_core_check)
add_test(NAME cms_storage_check COMMAND cms_storage_check)
add_test(NAME cms_data_engine_check COMMAND cms_data_engine_check)
add_test(NAME cms_services_check COMMAND cms_services_check)

add_executable(cms_auth_controller_check tests/AuthControllerTest.cpp)
target_link_libraries(cms_auth_controller_check PRIVATE cms_core)
add_test(NAME cms_auth_controller_check COMMAND cms_auth_controller_check)

add_executable(cms_film_controller_check tests/FilmControllerTest.cpp)
target_link_libraries(cms_film_controller_check PRIVATE cms_core)
add_test(NAME cms_film_controller_check COMMAND cms_film_controller_check)
add_executable(cms_integrity_check tests/IntegrityTest.cpp)
target_link_libraries(cms_integrity_check PRIVATE cms_core)
add_test(NAME cms_integrity_check COMMAND cms_integrity_check)
add_executable(cms_performance_check tests/PerformanceTest.cpp)
target_link_libraries(cms_performance_check PRIVATE cms_core)
add_test(NAME cms_performance_check COMMAND cms_performance_check)
set_tests_properties(cms_performance_check PROPERTIES TIMEOUT 30)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets QUIET)
if (QT_FOUND)
    find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

    add_library(cms_qt STATIC
        src/ui/dialog/LoginDialog.cpp
        src/ui/dialog/LoginDialog.h
        src/ui/widget/FilmPage.cpp
        src/ui/widget/FilmPage.h
        src/ui/widget/AccountPage.cpp
        src/ui/widget/AccountPage.h
        src/ui/widget/AuditoriumPage.cpp
        src/ui/widget/AuditoriumPage.h
        src/ui/widget/OperationsPages.cpp
        src/ui/widget/OperationsPages.h
        src/ui/mainwindow/MainWindow.cpp
        src/ui/mainwindow/MainWindow.h
    )
    target_link_libraries(cms_qt PUBLIC cms_core Qt${QT_VERSION_MAJOR}::Widgets)

    add_executable(cinema_app src/CinemaApplication.cpp)
    target_link_libraries(cinema_app PRIVATE cms_qt)
    target_compile_definitions(cinema_app PRIVATE CMS_DEMO_SOURCE="${CMAKE_CURRENT_SOURCE_DIR}/data/demo")
    add_executable(cms_qt_check tests/QtLoginTest.cpp)
    target_link_libraries(cms_qt_check PRIVATE cms_qt)
    add_test(NAME cms_qt_check COMMAND cms_qt_check)
    set_tests_properties(cms_qt_check PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
    add_executable(cms_qt_film_check tests/QtFilmTest.cpp)
    target_link_libraries(cms_qt_film_check PRIVATE cms_qt)
    add_test(NAME cms_qt_film_check COMMAND cms_qt_film_check)
    set_tests_properties(cms_qt_film_check PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen" TIMEOUT 20)
    add_executable(cms_qt_account_check tests/QtAccountTest.cpp)
    target_link_libraries(cms_qt_account_check PRIVATE cms_qt)
    add_test(NAME cms_qt_account_check COMMAND cms_qt_account_check)
    set_tests_properties(cms_qt_account_check PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen" TIMEOUT 20)
    add_test(NAME cms_qt_account_hidpi_check COMMAND cms_qt_account_check)
    set_tests_properties(cms_qt_account_hidpi_check PROPERTIES
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_SCALE_FACTOR=1.5;QT_FONT_DPI=144" TIMEOUT 20)
    add_executable(cms_qt_auditorium_check tests/QtAuditoriumTest.cpp)
    target_link_libraries(cms_qt_auditorium_check PRIVATE cms_qt)
    target_compile_definitions(cms_qt_auditorium_check PRIVATE CMS_DEMO_PATH="${CMAKE_CURRENT_SOURCE_DIR}/data/demo")
    add_test(NAME cms_qt_auditorium_check COMMAND cms_qt_auditorium_check)
    set_tests_properties(cms_qt_auditorium_check PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen" TIMEOUT 20)
    add_executable(cms_qt_operations_check tests/QtOperationsTest.cpp)
    target_link_libraries(cms_qt_operations_check PRIVATE cms_qt)
    add_test(NAME cms_qt_operations_check COMMAND cms_qt_operations_check)
    set_tests_properties(cms_qt_operations_check PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen" TIMEOUT 30)
    add_test(NAME cms_qt_operations_hidpi_check COMMAND cms_qt_operations_check)
    set_tests_properties(cms_qt_operations_hidpi_check PROPERTIES
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_SCALE_FACTOR=1.5;QT_FONT_DPI=144" TIMEOUT 30)
    add_executable(cms_qt_deployment_check tests/QtDeploymentTest.cpp)
    target_link_libraries(cms_qt_deployment_check PRIVATE cms_qt)
    add_test(NAME cms_qt_deployment_check COMMAND cms_qt_deployment_check $<TARGET_FILE:cinema_app>)
    set_tests_properties(cms_qt_deployment_check PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen" TIMEOUT 40)
else()
    message(STATUS "Qt Widgets not found: cinema_app is skipped; cms_core_check remains available.")
endif()

get_property(CMS_TARGETS DIRECTORY PROPERTY BUILDSYSTEM_TARGETS)
foreach(CMS_TARGET IN LISTS CMS_TARGETS)
    if(CMS_TARGET MATCHES "^cms_.*check$")
        target_compile_options(${CMS_TARGET} PRIVATE $<$<CXX_COMPILER_ID:GNU,Clang>:-UNDEBUG>)
    endif()
endforeach()
