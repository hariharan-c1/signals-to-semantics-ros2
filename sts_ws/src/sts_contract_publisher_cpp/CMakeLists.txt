cmake_minimum_required(VERSION 3.10)
project(sts_contract_publisher_cpp)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(sts_interfaces REQUIRED)

add_executable(ego_state_test_publisher
  src/ego_state_test_publisher.cpp
)
target_link_libraries(ego_state_test_publisher
  rclcpp::rclcpp
  sts_interfaces::sts_interfaces__rosidl_typesupport_cpp
)
if(APPLE)
  target_link_options(ego_state_test_publisher PRIVATE
    "LINKER:-dead_strip_dylibs"
  )
endif()

install(TARGETS
  ego_state_test_publisher
  DESTINATION lib/${PROJECT_NAME}
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
