#!/bin/bsh

CXX := g++
MPICXX := mpicxx
CXXFLAGS := -std=c++11 -fno-rtti -UNDEBUG -g -O2 -fno-omit-frame-pointer -fstack-protector-all

# For mpich
#MPI_FLAGS := -lmpichf90 -lmpich -lopa -lmpl -lrt -lpthread -lm
#MPI_INCLUDE := -I/mnt/home/jinyuyang/JASMIN/jasmin/thirdparty/mpich2-1.5/include
#MPI_LIB := -L/mnt/home/jinyuyang/JASMIN/jasmin/thirdparty/mpich2-1.5/lib
# For openmpi
MPI_FLAGS := -lmpi -lmpi_mpifh
MPI_INCLUDE := -I/opt/openmpi-3.0.0/include
MPI_LIB := -L/opt/openmpi-3.0.0/lib

IGRAPH_INCLUDE :=  -I/mnt/home/jinyuyang/build/igraph-0.9.0/include/igraph/
IGRAPH_LIB := -L/mnt/home/jinyuyang/build/igraph-0.9.0/lib -ligraph

DYNINST_INCLUDE := -I${DYNINST_DIR}/include 
DYNINST_LIB := -L${DYNINST_DIR}/lib -lparseAPI -linstructionAPI -lsymtabAPI -lsymLite -ldynDwarf -ldynElf -lboost_system -lcommon -lelf -ldwarf

PLUGIN_CXXFLAGS := -fpic
PLUGIN_LDFLAGS := -shared
AR := ar
ARTAG := rcs

all: graphsd_dyninst graphperf_preprocess

graphsd_dyninst: static_analysis igraph

igraph:
	$(CXX) $(BAGUA_DIR)/src/GraphSD/dyninst/igraph.cpp $(IGRAPH_INCLUDE) $(IGRAPH_LIB) -o $(BAGUA_DIR)/bin/igraph_test

static_analysis: $(BAGUA_DIR)/src/pag.cpp $(BAGUA_DIR)/src/pag.h $(BAGUA_DIR)/src/GraphSD/dyninst/static_analysis.cpp $(BAGUA_DIR)/src/GraphSD/dyninst/static_analysis.h  $(BAGUA_DIR)/src/GraphSD/dyninst/main.cpp 
	$(CXX) -g -std=c++11 -o $(BAGUA_DIR)/bin/$@ $^ $(IGRAPH_INCLUDE) $(IGRAPH_LIB) $(DYNINST_INCLUDE) $(DYNINST_LIB) 

graphperf_preprocess: $(BAGUA_DIR)/src/GraphPerf/preprocessing/main.cpp $(BAGUA_DIR)/src/pag.cpp $(BAGUA_DIR)/src/pag.h $(BAGUA_DIR)/src/utils.cpp $(BAGUA_DIR)/src/GraphPerf/preprocessing/preprocess.cpp $(BAGUA_DIR)/src/GraphPerf/preprocessing/preprocess.h 
	$(CXX) -g -std=c++11 -o $(BAGUA_DIR)/bin/$@ $^ $(IGRAPH_INCLUDE) $(IGRAPH_LIB)