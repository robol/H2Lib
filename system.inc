
# --- System configuration file for Linux with gcc

AR = ar
ARFLAGS = cru
RM = rm
CC = gcc
GCC = gcc

CFLAGS = -Wall -fPIC -g -O0
LDFLAGS = -Wl,-L,.,-R,.
LIBS =

#
# Debug settings
#
ifdef DEBUG
  CFLAGS += -g
endif

#
# Optimization settings
#
ifdef OPT
  CFLAGS += -O3 -march=native -funroll-loops -funswitch-loops
endif

#
# Complex or real matrices and vectors?
#
ifdef USE_COMPLEX
  CFLAGS += -DUSE_COMPLEX
endif

#
# BLAS/LAPACK
#
ifdef USE_BLAS
  CFLAGS += -DUSE_BLAS
  LIBS += -llapack -lblas -lgfortran
endif

ifdef USE_MATLAB_BLAS
  CFLAGS += -DUSE_BLAS -DLAPACK_USE_LONGS
  LIBS += -lmwlapack -lmwblas -lgfortran

  # Select the path of your Matlab installation
  MATLAB_HOME=/home/robol/Software/matlab

  # Select your current Matlab architecture
  MATLAB_ARCH=glnxa64

  LDFLAGS += -L$(MATLAB_HOME)/bin/$(MATLAB_ARCH) -Wl,-rpath=$(MATLAB_HOME)/bin/$(MATLAB_ARCH)
endif 

#
# Cairo graphics
#
ifdef USE_CAIRO
  CFLAGS += -DUSE_CAIRO $(shell pkg-config --cflags cairo)
  LIBS += $(shell pkg-config --libs cairo)
endif

#
# OpenMP parallelization
#
ifdef USE_OPENMP
  CFLAGS += -DUSE_OPENMP -fopenmp
  LDFLAGS += -fopenmp
endif

#
# OpenCL heterogeneous programming
#
ifdef USE_OPENCL
  CFLAGS += -DUSE_OPENCL -I/usr/local/cuda/include
  LIBS += -lOpenCL
endif

#
# NetCDF import/export
#
ifdef USE_NETCDF
  CFLAGS += -DUSE_NETCDF $(shell pkg-config --cflags netcdf)
  LIBS += $(shell pkg-config --libs netcdf)
endif

#
# zlib compression
#
ifdef USE_ZLIB
  CFLAGS += -DUSE_ZLIB
  LIBS += -lz
endif

#
# GTK+ 3.x visualization
#
ifdef USE_GTK3
  CFLAGS += -DUSE_GTK3 $(shell pkg-config --cflags gtk+-3.0)
  LIBS += $(shell pkg-config --libs gtk+-3.0)
endif

#
# FreeGLUT visualization
#
ifdef USE_FREEGLUT
  CFLAGS += -DUSE_FREEGLUT
  LIBS += -lglut -lGLU -lGL
endif

#
# Mathematics library
#
LIBS += -lm
