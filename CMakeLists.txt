set(FNC2ADIOS_SRC_DIR src)
set(FNC2ADIOS_INCLUDE_DIR include)

# FNC2ADIOS headers
include_directories(${FNC2ADIOS_INCLUDE_DIR})

# Target
#file(GLOB FNC2ADIOS_SRC_FILES "${FNC2ADIOS_SRC_DIR}/*.cpp")
set(FNC2ADIOS_SRC_FILES ${FNC2ADIOS_SRC_FILES} ${FNC2ADIOS_SRC_DIR}/Main.cpp)
set(FNC2ADIOS_SRC_FILES ${FNC2ADIOS_SRC_FILES} ${FNC2ADIOS_SRC_DIR}/Group.cpp)
set(FNC2ADIOS_SRC_FILES ${FNC2ADIOS_SRC_FILES} ${FNC2ADIOS_SRC_DIR}/Prepare.cpp)
set(FNC2ADIOS_SRC_FILES ${FNC2ADIOS_SRC_FILES} ${FNC2ADIOS_SRC_DIR}/GroupUtils.cpp)
set(FNC2ADIOS_SRC_FILES ${FNC2ADIOS_SRC_FILES} ${FNC2ADIOS_SRC_DIR}/BuildAdFunc.cpp)
set(FNC2ADIOS_SRC_FILES ${FNC2ADIOS_SRC_FILES} ${FNC2ADIOS_SRC_DIR}/Worker.cpp)
set(FNC2ADIOS_SRC_FILES ${FNC2ADIOS_SRC_FILES} ${FNC2ADIOS_SRC_DIR}/MPIUtils.cpp)
set(FNC2ADIOS_SRC_FILES ${FNC2ADIOS_SRC_FILES} ${FNC2ADIOS_SRC_DIR}/RoseHelperAdv.cpp)

add_executable(FNC2ADIOS ${FNC2ADIOS_SRC_FILES})

# Link
target_link_libraries(FNC2ADIOS ${EXTRA_LIBS})
