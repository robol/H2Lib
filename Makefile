
# ------------------------------------------------------------
# Components of the main library
# ------------------------------------------------------------

H2LIB_CORE0 = \
	Library/basic.c \
	Library/settings.c \
	Library/parameters.c

H2LIB_CORE1 = \
	Library/avector.c \
	Library/amatrix.c \
	Library/factorizations.c \
	Library/eigensolvers.c \
	Library/sparsematrix.c \
	Library/sparsepattern.c \
	Library/gaussquad.c \
	Library/krylov.c

H2LIB_CORE2 = \
	Library/cluster.c \
	Library/clustergeometry.c \
	Library/block.c \
	Library/clusterbasis.c \
	Library/clusteroperator.c \
	Library/uniform.c \
	Library/h2matrix.c \
	Library/rkmatrix.c \
	Library/hmatrix.c

H2LIB_CORE3 = \
	Library/truncation.c \
	Library/harith.c \
	Library/hcoarsen.c \
	Library/h2compression.c \
	Library/h2update.c \
	Library/h2arith.c \
	Library/aca.c

H2LIB_SIMPLE = 

H2LIB_BEM = \
	Library/curve2d.c \
	Library/singquad1d.c \
	Library/bem2d.c \
	Library/laplacebem2d.c \
	Library/surface3d.c \
	Library/macrosurface3d.c \
	Library/singquad2d.c \
	Library/bem3d.c \
	Library/laplacebem3d.c 

SOURCES_libh2 := \
	$(H2LIB_CORE0) \
	$(H2LIB_CORE1) \
	$(H2LIB_CORE2) \
	$(H2LIB_CORE3) \
	$(H2LIB_SIMPLE) \
	$(H2LIB_BEM)

HEADERS_libh2 := $(SOURCES_libh2:.c=.h)

OBJECTS_libh2 := $(SOURCES_libh2:.c=.o)

DEPENDENCIES_libh2 := $(SOURCES_libh2:.c=.d)

# ------------------------------------------------------------
# Test programs
# ------------------------------------------------------------

SOURCES_stable := \
	Tests/test_amatrix.c \
	Tests/test_eigen.c \
	Tests/test_hmatrix.c \
	Tests/test_h2matrix.c \
	Tests/test_laplacebem2d.c \
	Tests/test_laplacebem3d.c \
	Tests/test_h2compression.c

SOURCES_tests = $(SOURCES_stable)

OBJECTS_tests := \
	$(SOURCES_tests:.c=.o)

DEPENDENCIES_tests := \
	$(SOURCES_tests:.c=.d)

PROGRAMS_tests := \
	$(SOURCES_tests:.c=)

# ------------------------------------------------------------
# All files
# ------------------------------------------------------------

SOURCES := \
	$(SOURCES_libh2) \
	$(SOURCES_tests)

HEADERS := \
	$(HEADER_libh2)

OBJECTS := \
	$(OBJECTS_libh2) \
	$(OBJECTS_tests)

DEPENDENCIES := \
	$(DEPENDENCIES_libh2) \
	$(DEPENDENCIES_tests)

PROGRAMS := \
	$(PROGRAMS_tests)

# ------------------------------------------------------------
# Standard target
# ------------------------------------------------------------

all: programs

# ------------------------------------------------------------
# System-dependent parameters (e.g., name of compiler)
# ------------------------------------------------------------

Makefile: make.inc

include make.inc

# ------------------------------------------------------------
# Rules for test programs
# ------------------------------------------------------------

programs: $(PROGRAMS_tests)

$(PROGRAMS_tests): %: %.o
	$(CC) $(LDFLAGS) -Wl,-L,.,-R,. $< -o $@ -lh2 -lm $(LIBS) 

$(PROGRAMS_tests) $(PROGRAMS_tools): libh2.a libh2.so

$(OBJECTS_tests): %.o: %.c
	@$(GCC) -MT $@ -MM -I Library $< > $(<:%.c=%.d)
	$(CC) $(CFLAGS) -I Library -c $< -o $@

-include $(DEPENDENCIES_tests) $(DEPENDENCIES_tools)
$(OBJECTS_tests): Makefile

# ------------------------------------------------------------
# Rules for the Doxygen documentation
# ------------------------------------------------------------

doc:
	doxygen Doc/Doxyfile

# ------------------------------------------------------------
# Rules for the main library
# ------------------------------------------------------------

libh2.a: $(OBJECTS_libh2)
	$(AR) $(ARFLAGS) $@ $(OBJECTS_libh2)

libh2.so: $(OBJECTS_libh2)
	$(GCC) -shared -o libh2.so $(OBJECTS_libh2) $(LDFLAGS) $(LIBS)

$(OBJECTS_libh2): %.o: %.c
	@$(GCC) -MT $@ -MM $< > $(<:%.c=%.d)
	$(CC) $(CFLAGS) -c $< -o $@

-include $(DEPENDENCIES_libh2)
$(OBJECTS_libh2): Makefile

# ------------------------------------------------------------
# Useful additions
# ------------------------------------------------------------

.PHONY: clean cleandoc programs indent

clean:
	$(RM) -f $(OBJECTS) $(DEPENDENCIES) $(PROGRAMS) libh2.a libh2.so

cleandoc:
	$(RM) -rf Doc/html Doc/latex

indent:
	indent -bap -br -nce -cdw -npcs \
	  -di10 -nbc -brs -blf -i2 -lp \
	  -T amatrix -T pamatrix -T pcamatrix \
	  -T avector -T pavector -T pcavector \
	  -T cluster -T pcluster -T pccluster \
	  -T block -T pblock -T pcblock \
	  -T rkmatrix -T prkmatrix -T pcrkmatrix \
	  -T hmatrix -T phmatrix -T pchmatrix \
	  -T uniform -T puniform -T pcuniform \
	  -T h2matrix -T ph2matrix -T pch2matrix \
	  $(SOURCES)
