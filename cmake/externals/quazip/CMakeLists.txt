set(EXTERNAL_NAME quazip)
string(TOUPPER ${EXTERNAL_NAME} EXTERNAL_NAME_UPPER)

include(ExternalProject)

set(QUAZIP_CMAKE_ARGS 
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} 
    -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR> 
    -DCMAKE_PREFIX_PATH=${QT_CMAKE_PREFIX_PATH} 
    -DCMAKE_INSTALL_NAME_DIR:PATH=<INSTALL_DIR>/lib 
    -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
    -DZLIB_ROOT=${VCPKG_INSTALL_ROOT}
    -DCMAKE_POSITION_INDEPENDENT_CODE=ON)

if (NOT APPLE)
  set(QUAZIP_CMAKE_ARGS ${QUAZIP_CMAKE_ARGS} -DCMAKE_CXX_STANDARD=11)
endif ()

ExternalProject_Add(
  ${EXTERNAL_NAME}
  URL "${EXTERNAL_BUILD_ASSETS}/dependencies/quazip-0.7.3.zip"
  URL_MD5 ed03754d39b9da1775771819b8001d45
  BINARY_DIR ${EXTERNAL_PROJECT_PREFIX}/build
  CMAKE_ARGS ${QUAZIP_CMAKE_ARGS}
  LOG_DOWNLOAD 1
  LOG_CONFIGURE 1
  LOG_BUILD 1
)

# Hide this external target (for ide users)
set_target_properties(${EXTERNAL_NAME} PROPERTIES
    FOLDER "hidden/externals"
    INSTALL_NAME_DIR ${INSTALL_DIR}/lib
    BUILD_WITH_INSTALL_RPATH True)

ExternalProject_Get_Property(${EXTERNAL_NAME} INSTALL_DIR)
set(${EXTERNAL_NAME_UPPER}_INCLUDE_DIR ${INSTALL_DIR}/include CACHE PATH "List of QuaZip include directories")
set(${EXTERNAL_NAME_UPPER}_INCLUDE_DIRS ${${EXTERNAL_NAME_UPPER}_INCLUDE_DIR} CACHE PATH "List of QuaZip include directories")
set(${EXTERNAL_NAME_UPPER}_DLL_PATH ${INSTALL_DIR}/lib CACHE FILEPATH "Location of QuaZip DLL")

if (APPLE)
  set(${EXTERNAL_NAME_UPPER}_LIBRARY_RELEASE ${INSTALL_DIR}/lib/libquazip5.1.0.0.dylib CACHE FILEPATH "Location of QuaZip release library")
  set(${EXTERNAL_NAME_UPPER}_LIBRARY_DEBUG ${INSTALL_DIR}/lib/libquazip5d.1.0.0.dylib CACHE FILEPATH "Location of QuaZip release library")
elseif (WIN32)
  set(${EXTERNAL_NAME_UPPER}_LIBRARY_RELEASE ${INSTALL_DIR}/lib/quazip5.lib CACHE FILEPATH "Location of QuaZip release library")
  set(${EXTERNAL_NAME_UPPER}_LIBRARY_DEBUG ${INSTALL_DIR}/lib/quazip5d.lib CACHE FILEPATH "Location of QuaZip release library")
elseif (CMAKE_SYSTEM_NAME MATCHES "Linux")
  set(${EXTERNAL_NAME_UPPER}_LIBRARY_RELEASE ${INSTALL_DIR}/lib/libquazip5.so CACHE FILEPATH "Location of QuaZip release library")
  set(${EXTERNAL_NAME_UPPER}_LIBRARY_DEBUG ${INSTALL_DIR}/lib/libquazip5d.so CACHE FILEPATH "Location of QuaZip release library")
else ()
  set(${EXTERNAL_NAME_UPPER}_LIBRARY_RELEASE ${INSTALL_DIR}/lib/libquazip5.so CACHE FILEPATH "Location of QuaZip release library")
  set(${EXTERNAL_NAME_UPPER}_LIBRARY_DEBUG ${INSTALL_DIR}/lib/libquazip5.so CACHE FILEPATH "Location of QuaZip release library")
endif ()

include(SelectLibraryConfigurations)
select_library_configurations(${EXTERNAL_NAME_UPPER})

# Force selected libraries into the cache
set(${EXTERNAL_NAME_UPPER}_LIBRARY ${${EXTERNAL_NAME_UPPER}_LIBRARY} CACHE FILEPATH "Location of QuaZip libraries")
set(${EXTERNAL_NAME_UPPER}_LIBRARIES ${${EXTERNAL_NAME_UPPER}_LIBRARIES} CACHE FILEPATH "Location of QuaZip libraries")
