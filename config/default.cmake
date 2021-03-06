# MacBook
if(USEMPI) 
  set(ENV{CC}  mpicc ) # C compiler for parallel build
  set(ENV{CXX} mpicxx) # C++ compiler for parallel build
else()
  set(ENV{CC}  gcc) # C compiler for serial build
  set(ENV{CXX} g++) # C++ compiler for serial build
endif()

set(GNU_SED "gsed")

set(USER_CXX_FLAGS "-std=c++11")
#set(USER_CXX_FLAGS "-stdlib=libstdc++ -std=c++11")
set(USER_CXX_FLAGS_RELEASE " ")
set(USER_CXX_FLAGS_DEBUG "-O0 -g -Wall -Wno-unknown-pragmas -Weverything")

set(FFTW_INCLUDE_DIR   "/usr/local/include")
set(FFTW_LIB           "/usr/local/lib/libfftw3.a")
set(NETCDF_INCLUDE_DIR "/Users/astrideichhorn/Documents/utilities/netcdf/include")
set(NETCDF_LIB_C       "/Users/astrideichhorn/Documents/utilities/netcdf/lib/libnetcdf.a")
set(NETCDF_LIB_CPP     "/Users/astrideichhorn/Documents/utilities/netcdf/lib/libnetcdf_c++4.a")
set(HDF5_LIB_1         "/Users/astrideichhorn/Documents/utilities/netcdf/lib/libhdf5.a")
set(HDF5_LIB_2         "/Users/astrideichhorn/Documents/utilities/netcdf/lib/libhdf5_hl.a")
set(SZIP_LIB           "")
set(LIBS ${FFTW_LIB} ${NETCDF_LIB_CPP} ${NETCDF_LIB_C} ${HDF5_LIB_2} ${HDF5_LIB_1} ${SZIP_LIB} m z curl)

#set(INCLUDE_DIRS ${FFTW_INCLUDE_DIR} ${NETCDF_INCLUDE_DIR})

if(USECUDA)
  set(CUDA_PROPAGATE_HOST_FLAGS OFF)
  set(LIBS ${LIBS} -rdynamic /usr/local/cuda/lib/libcufft.dylib)
  set(USER_CUDA_NVCC_FLAGS "-arch=sm_20")
endif()

add_definitions(-DRESTRICTKEYWORD=__restrict__)
