find_package(Boost 1.40 COMPONENTS system REQUIRED)
find_package(PkgConfig)
pkg_check_modules(GAZEBO gazebo)
pkg_check_modules(SDF sdformat)
find_package(gazebo REQUIRED)
include_directories(
  ${PROJECT_SOURCE_DIR}
  ${GAZEBO_INCLUDE_DIRS}
  ${SDF_INCLUDE_DIRS}
  ${PROJECT_SOURCE_DIR}/../include)

link_directories(
  ${GAZEBO_LIBRARY_DIRS}
  ${ARMDILLO_LIBRARY_DIRS})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GAZEBO_CXX_FLAGS} -pthread")

add_library(gz_core SHARED gz_core.cc)
target_link_libraries(gz_core
  ${GAZEBO_LIBRARIES}
  ${SDF_LIBRARIES})

add_library(gz_rccar SHARED gz_rccar.cc)
target_link_libraries(gz_rccar
  ${GAZEBO_LIBRARIES}
  ${SDF_LIBRARIES})

add_library(rccar_plugin SHARED rccar_plugin.cc)
target_link_libraries(rccar_plugin
  ${GAZEBO_LIBRARIES}
  ${SDF_LIBRARIES}
  ${Boost_LIBRARIES})

add_library(gz_d435i SHARED gz_d435i.cc)
target_link_libraries(gz_d435i
  ${GAZEBO_LIBRARIES}
  ${SDF_LIBRARIES})

add_library(d435i_depth_plugin SHARED d435i_depth_plugin.cc)
target_link_libraries(d435i_depth_plugin
  ${GAZEBO_LIBRARIES}
  ${SDF_LIBRARIES}
  ${Boost_LIBRARIES}
  DepthCameraPlugin)

add_library(d435i_color_plugin SHARED d435i_color_plugin.cc)
target_link_libraries(d435i_color_plugin
  ${GAZEBO_LIBRARIES}
  ${SDF_LIBRARIES}
  ${Boost_LIBRARIES}
  CameraPlugin)
