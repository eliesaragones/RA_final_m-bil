cmake_minimum_required(VERSION 3.0.2)
project(mwc_taxibot)

add_compile_options(-std=c++17)

find_package(catkin REQUIRED COMPONENTS
  actionlib
  geometry_msgs
  message_generation
  move_base_msgs
  roscpp
  sensor_msgs
  std_msgs
  tf2
)

add_message_files(
  FILES
  TaxiStatus.msg
)

add_service_files(
  FILES
  RequestTaxi.srv
)

generate_messages(
  DEPENDENCIES
  std_msgs
)

catkin_package(
  INCLUDE_DIRS include
  LIBRARIES taxi_core
  CATKIN_DEPENDS
    actionlib
    geometry_msgs
    message_runtime
    move_base_msgs
    roscpp
    sensor_msgs
    std_msgs
    tf2
)

include_directories(
  include
  ${catkin_INCLUDE_DIRS}
)

add_library(taxi_core
  src/taxi_core.cpp
)
add_dependencies(taxi_core
  ${${PROJECT_NAME}_EXPORTED_TARGETS}
  ${catkin_EXPORTED_TARGETS}
)
target_link_libraries(taxi_core
  ${catkin_LIBRARIES}
)

add_executable(taxi_manager_node
  src/taxi_manager_node.cpp
)
add_dependencies(taxi_manager_node
  ${${PROJECT_NAME}_EXPORTED_TARGETS}
  ${catkin_EXPORTED_TARGETS}
)
target_link_libraries(taxi_manager_node
  taxi_core
  ${catkin_LIBRARIES}
)

add_executable(taxi_request_client
  src/taxi_request_client.cpp
)
add_dependencies(taxi_request_client
  ${${PROJECT_NAME}_EXPORTED_TARGETS}
  ${catkin_EXPORTED_TARGETS}
)
target_link_libraries(taxi_request_client
  ${catkin_LIBRARIES}
)

add_executable(vision_perception_node
  src/vision_perception_node.cpp
)
add_dependencies(vision_perception_node
  ${${PROJECT_NAME}_EXPORTED_TARGETS}
  ${catkin_EXPORTED_TARGETS}
)
target_link_libraries(vision_perception_node
  ${catkin_LIBRARIES}
)

if(CATKIN_ENABLE_TESTING)
  catkin_add_gtest(taxi_core_test
    test/taxi_core_test.cpp
  )
  if(TARGET taxi_core_test)
    target_link_libraries(taxi_core_test
      taxi_core
      ${catkin_LIBRARIES}
    )
  endif()
endif()

install(TARGETS taxi_core taxi_manager_node taxi_request_client vision_perception_node
  ARCHIVE DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
  LIBRARY DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
  RUNTIME DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION}
)

install(DIRECTORY include/${PROJECT_NAME}/
  DESTINATION ${CATKIN_PACKAGE_INCLUDE_DESTINATION}
)

install(DIRECTORY config launch maps worlds
  DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION}
)
