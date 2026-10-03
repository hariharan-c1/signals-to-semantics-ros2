cmake_minimum_required(VERSION 3.16)
project(sts_ego_state_cpp)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rcl_interfaces REQUIRED)
find_package(nav_msgs REQUIRED)
find_package(sts_interfaces REQUIRED)

add_library(ego_kinematics_estimator STATIC
  src/ego_kinematics_estimator.cpp
)
target_compile_features(ego_kinematics_estimator PUBLIC cxx_std_17)
set_target_properties(ego_kinematics_estimator PROPERTIES CXX_EXTENSIONS OFF)
target_include_directories(ego_kinematics_estimator PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

add_executable(ego_state_node src/ego_state_node.cpp)
target_link_libraries(ego_state_node
  ego_kinematics_estimator
  rclcpp::rclcpp
  rcl_interfaces::rcl_interfaces__rosidl_typesupport_cpp
  nav_msgs::nav_msgs__rosidl_typesupport_cpp
  sts_interfaces::sts_interfaces__rosidl_typesupport_cpp
)
if(APPLE)
  # As in M1, omit unused RoboStack Python generator dylibs from this C++ node.
  target_link_options(ego_state_node PRIVATE "LINKER:-dead_strip_dylibs")
endif()

install(TARGETS ego_kinematics_estimator
  EXPORT export_ego_kinematics_estimator
  ARCHIVE DESTINATION lib
)
install(DIRECTORY include/ DESTINATION include)
install(TARGETS ego_state_node DESTINATION lib/${PROJECT_NAME})
ament_export_targets(export_ego_kinematics_estimator HAS_LIBRARY_TARGET)

if(BUILD_TESTING)
  find_package(ament_cmake_gtest REQUIRED)
  ament_add_gtest(test_ego_kinematics_estimator
    test/test_ego_kinematics_estimator.cpp
  )
  if(APPLE AND GTEST_FROM_SOURCE_FOUND)
    # RoboStack Clang injects prefix/include as a regular include directory, which
    # takes precedence over ament's SYSTEM vendor headers. Keep sources/headers
    # from the same GoogleTest version, including in consumers of the test target.
    target_compile_options(gtest PUBLIC "-iquote${GTEST_FROM_SOURCE_INCLUDE_DIRS}")
  endif()
  target_link_libraries(test_ego_kinematics_estimator ego_kinematics_estimator)

  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
