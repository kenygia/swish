# Copyright (C) 2015  Alexander Lamaison <swish@lammy.co.uk>
#
# This program is free software: you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free Software
# Foundation, either version 3 of the License, or (at your option) any later
# version.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
# details.
#
# You should have received a copy of the GNU General Public License along with
# this program.  If not, see <http://www.gnu.org/licenses/>.

find_package(Boost 1.40 REQUIRED COMPONENTS unit_test_framework)

add_subdirectory(common_boost)

set(TEST_RUNNER_ARGUMENTS
  --result_code=yes --build_info=yes --log_level=test_suite)

include(FindCygwin)

if(CYGWIN_INSTALL_PATH)
  set(TEST_RUNNER_ENVIRONMENT
    "PATH=${CYGWIN_INSTALL_PATH}/bin;${CYGWIN_INSTALL_PATH}/usr/sbin")
  string(REPLACE ";" "\\;" TEST_RUNNER_ENVIRONMENT
    "${TEST_RUNNER_ENVIRONMENT}")

  if(NOT EXISTS "${CYGWIN_INSTALL_PATH}/usr/sbin/sshd.exe")
    message(WARNING
      # Running sshd for the tests requires a version of sshd that matches
      # the version of chmod used to set the permissions on the hostkey
      # file.  sshd refuses to run if it thinks the key is world-readable,
      # and different implementations of chmod save those permissions
      # in different ways.  Cygwin uses the 'special' Unix subsystem area
      # of the Windows ACL so the version of sshd must also use that method.
      "Cygwin sshd not found: tests are likely to fail to run")
  endif()
else()
  message(WARNING
    "Cygwin not found: tests are likely to fail to run")
endif()

include(CMakeParseArguments)
# swish_test_suite(SUBJECT test-target [VARIANT suite-variant] SOURCES ...
#                  LIBRARIES ...)
function(SWISH_TEST_SUITE)
  set(options)
  set(oneValueArgs SUBJECT VARIANT)
  set(multiValueArgs SOURCES LIBRARIES)
  cmake_parse_arguments(SWISH_TEST_SUITE
    "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  if(SWISH_TEST_SUITE_VARIANT)
    set(_TEST_EXE_NAME
      "test-${SWISH_TEST_SUITE_SUBJECT}-${SWISH_TEST_SUITE_VARIANT}")
  else()
    set(_TEST_EXE_NAME "test-${SWISH_TEST_SUITE_SUBJECT}")
  endif()

  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/../module.cpp.in
    ${CMAKE_CURRENT_BINARY_DIR}/module.cpp @ONLY)

  add_executable(${_TEST_EXE_NAME}
    ${SWISH_TEST_SUITE_SOURCES}
    ${CMAKE_CURRENT_BINARY_DIR}/module.cpp)

  target_link_libraries(${_TEST_EXE_NAME}
    PRIVATE
    ${SWISH_TEST_SUITE_SUBJECT} test-common_boost ${SWISH_TEST_SUITE_LIBRARIES})

  add_test(
    NAME ${_TEST_EXE_NAME}
    COMMAND ${_TEST_EXE_NAME} ${TEST_RUNNER_ARGUMENTS}
    WORKING_DIRECTORY $<TARGET_FILE_DIR:${_TEST_EXE_NAME}>)

  if(TEST_RUNNER_ENVIRONMENT)
    set_tests_properties(${_TEST_EXE_NAME} PROPERTIES
      ENVIRONMENT "${TEST_RUNNER_ENVIRONMENT}")
  endif()
endfunction()

set(_FIXTURE_FILES test_zip_file.zip)
set(_FIXTURE_SSHD_ETC_FILES
  fixture_dsakey fixture_dsakey.pub
  fixture_hostkey fixture_hostkey.pub
  fixture_rsakey fixture_rsakey.pub)
set(_FIXTURE_FILES_DIR ${CMAKE_CURRENT_LIST_DIR}/common_boost)
set(_PERMISSION_SCRIPT_DIR ${CMAKE_CURRENT_LIST_DIR})

function(SWISH_COPY_FIXTURE_FILES TEST_TARGET)
  foreach(FILE ${_FIXTURE_FILES})
    add_custom_command(
      TARGET ${TEST_TARGET} POST_BUILD
      COMMAND ${CMAKE_COMMAND}
      -E copy_if_different
      ${_FIXTURE_FILES_DIR}/${FILE}
      $<TARGET_FILE_DIR:${TEST_TARGET}>/${FILE}
      VERBATIM)
  endforeach()

  foreach(FILE ${_FIXTURE_SSHD_ETC_FILES})
    add_custom_command(
      TARGET ${TEST_TARGET} POST_BUILD
      COMMAND ${CMAKE_COMMAND}
      -E copy_if_different
      ${_FIXTURE_FILES_DIR}/${FILE}
      $<TARGET_FILE_DIR:${TEST_TARGET}>/sshd-etc/${FILE}
      VERBATIM)
  endforeach()


  if(CYGWIN_INSTALL_PATH)
    add_custom_command(
      TARGET ${TEST_TARGET} POST_BUILD
      COMMAND
      cd $<TARGET_FILE_DIR:${TEST_TARGET}>/sshd-etc/
      VERBATIM)
    add_custom_command(
      TARGET ${TEST_TARGET} POST_BUILD
      COMMAND
      set PATH=${CYGWIN_INSTALL_PATH}/bin$<SEMICOLON>)
    add_custom_command(
      TARGET ${TEST_TARGET} POST_BUILD
      COMMAND
      bash ${_PERMISSION_SCRIPT_DIR}/fix_key_permissions.sh
      VERBATIM)
  endif()
endfunction()

include(CMakeDependentOption)

option(BUILD_TESTING_SLOW "Build tests that take some time" ON)

cmake_dependent_option(BUILD_TESTING_GUI ON
  "Build tests that show GUI elements on screen"
  "BUILD_TESTING_SLOW" OFF)

option(BUILD_TESTING_INTEGRATION "Build integration tests" ON)

cmake_dependent_option(BUILD_TESTING_NETWORK OFF
  "Build tests that require a remote test server"
  "BUILD_TESTING_INTEGRATION" OFF)

if(BUILD_TESTING_INTEGRATION)
  add_subdirectory(connection)
  add_subdirectory(drop_target)
  add_subdirectory(provider)
  add_subdirectory(ssh)
  add_subdirectory(remote_folder) # Only remote_commands_test.cpp
  add_subdirectory(shell_folder)
endif()

if(BUILD_TESTING_NETWORK)
  add_subdirectory(provider-network)
endif()

if(BUILD_TESTING_GUI)
  add_subdirectory(ezel)
  add_subdirectory(forms)
  add_subdirectory(shell_folder-dialogue)
endif()

add_subdirectory(host_folder)
add_subdirectory(nse)
add_subdirectory(versions)

# From http://stackoverflow.com/a/16163137/67013
if(CMAKE_CONFIGURATION_TYPES)
  add_custom_target(CHECK COMMAND ${CMAKE_CTEST_COMMAND}
    --force-new-ctest-process --output-on-failure
    --build-config "$<CONFIGURATION>")
else()
  add_custom_target(CHECK COMMAND ${CMAKE_CTEST_COMMAND}
    --force-new-ctest-process --output-on-failure)
endif()