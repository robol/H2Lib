# --- Linux, as many warnings as possible
LD=gcc
AR = ar
ARFLAGS = cru
RM = rm
CC = gcc
GCC = gcc

# Select the path of your Matlab installation
MATLAB_HOME=/path/to/matlab

# Select your current Matlab architecture
MATLAB_ARCH=glnxa64

CFLAGS = -fPIC -Wall -Wextra -pedantic -ansi -O3 -march=native -funroll-loops -funswitch-loops -DUSE_BLAS \
    -I$(MATLAB_HOME)/extern/include -DLAPACK_USE_LONGS

LDFLAGS = -L$(MATLAB_HOME)/bin/$(MATLAB_ARCH) -Wl,-rpath=$(MATLAB_HOME)/bin/$(MATLAB_ARCH)

LIBS = -lm -lmwlapack -lmwblas
