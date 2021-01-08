#!/bin/bash

CXX := g++
MPICXX := mpicxx
CXXFLAGS := -std=c++11 -fno-rtti -UNDEBUG -g -O2 -fno-omit-frame-pointer -fstack-protector-all
#MPI_FLAGS := -lmpichf90 -lmpich -lopa -lmpl -lrt -lpthread -lm
MPI_FLAGS := -lmpi -lmpi_mpifh
PLUGIN_CXXFLAGS := -fpic
PLUGIN_LDFLAGS := -shared
AR := ar
ARTAG := rcs

MPI_INCLUDE := -I/opt/openmpi-3.0.0/include
MPI_LIB := -L/opt/openmpi-3.0.0/lib
#MPI_INCLUDE := -I/mnt/home/jinyuyang/JASMIN/jasmin/thirdparty/mpich2-1.5/include
#MPI_LIB := -L/mnt/home/jinyuyang/JASMIN/jasmin/thirdparty/mpich2-1.5/lib

all: psg asm libdynco libcommdep

psg:
	#g++ -g -std=c++11 -o PSG PSG.cpp -L/home/jinyuyang/workspace/build/dyninst-10.1.0/lib -I/home/jinyuyang/workspace/build/dyninst-10.1.0/include -lparseAPI -linstructionAPI -lsymtabAPI -lsymLite -ldynDwarf -ldynElf -lboost_system -lcommon -lelf -ldwarf
	g++ -g -std=c++11 -o $(BAGUA_DIR)/bin/psg-addr $(BAGUA_DIR)/src/staticAnalysis/psg-addr.cpp -L${DYNINST_DIR}/lib -I${DYNINST_DIR}/include -lparseAPI -linstructionAPI -lsymtabAPI -lsymLite -ldynDwarf -ldynElf -lboost_system -lcommon -lelf -ldwarf
	g++ -g -std=c++11 -o $(BAGUA_DIR)/bin/psg-so-addr $(BAGUA_DIR)/src/staticAnalysis/psg-so-addr.cpp -L${DYNINST_DIR}/lib -I${DYNINST_DIR}/include -lparseAPI -linstructionAPI -lsymtabAPI -lsymLite -ldynDwarf -ldynElf -lboost_system -lcommon -lelf -ldwarf
	

libdynco:
	python $(BAGUA_DIR)/src/dynamicAnalysis/sampling/wrap/wrap.py -f $(BAGUA_DIR)/src/dynamicAnalysis/sampling/wrap/commData.w -o $(BAGUA_DIR)/src/dynamicAnalysis/sampling/commData.cpp
	#$(MPICXX) $(CXXFLAGS) $(MPI_INCLUDE) -ldl -Wl,--no-undefined -o $(BAGUA_DIR)/bin/$@-mpich2-1.5.so $(BAGUA_DIR)/src/dynamicAnalysis/sampling/dyncollect.cpp $(BAGUA_DIR)/src/dynamicAnalysis/sampling/commData-mpich2-1.5.cpp $(MPI_LIB) -shared -fPIC $(MPI_FLAGS) -lpapi -lunwind -lm
	$(MPICXX) $(CXXFLAGS) $(MPI_INCLUDE) -ldl -Wl,--no-undefined -o $(BAGUA_DIR)/bin/$@.so $(BAGUA_DIR)/src/dynamicAnalysis/sampling/dyncollect.cpp $(BAGUA_DIR)/src/dynamicAnalysis/sampling/commData.cpp $(MPI_LIB) -shared -fPIC $(MPI_FLAGS) -lpapi -lunwind
	$(MPICXX) $(CXXFLAGS) $(MPI_INCLUDE) -ldl -Wl,--no-undefined -o $(BAGUA_DIR)/bin/$@_multiPMU.so $(BAGUA_DIR)/src/dynamicAnalysis/sampling/dyncollect_multiPMU.cpp $(BAGUA_DIR)/src/dynamicAnalysis/sampling/commData.cpp $(MPI_LIB) -shared -fPIC $(MPI_FLAGS) -lpapi -lunwind
	$(MPICXX) $(CXXFLAGS) $(MPI_INCLUDE) -ldl -Wl,--no-undefined -o $(BAGUA_DIR)/bin/$@_multiPMU_2.so $(BAGUA_DIR)/src/dynamicAnalysis/sampling/dyncollect_multiPMU_2.cpp $(BAGUA_DIR)/src/dynamicAnalysis/sampling/commData.cpp $(MPI_LIB) -shared -fPIC $(MPI_FLAGS) -lpapi -lunwind
	$(MPICXX) $(CXXFLAGS) $(MPI_INCLUDE) -ldl -Wl,--no-undefined -o $(BAGUA_DIR)/bin/$@_multiPMU_lib.so $(BAGUA_DIR)/src/dynamicAnalysis/sampling/dyncollect_multiPMU_lib.cpp $(BAGUA_DIR)/src/dynamicAnalysis/sampling/commData.cpp $(MPI_LIB) -shared -fPIC $(MPI_FLAGS) -lpapi -lunwind

libcommdep:
	python $(BAGUA_DIR)/src/dynamicAnalysis/sampling/wrap/wrap.py -f $(BAGUA_DIR)/src/dynamicAnalysis/commdependence/commDependence_v1.1.w -o $(BAGUA_DIR)/src/dynamicAnalysis/commdependence/commDependence.cpp
	$(MPICXX) $(CXXFLAGS) $(MPI_INCLUDE) -ldl -Wl,--no-undefined -o $(BAGUA_DIR)/bin/$@.so $(BAGUA_DIR)/src/dynamicAnalysis/commdependence/commDependence.cpp $(MPI_LIB) -shared -fPIC $(MPI_FLAGS) -lunwind
	g++ -g -std=c++11 $(BAGUA_DIR)/src/dynamicAnalysis/commdependence/commDepDetect.cpp -o $(BAGUA_DIR)/bin/commDepDetect
	g++ -g -std=c++11 $(BAGUA_DIR)/src/dynamicAnalysis/commdependence/commDepDetectApproxi.cpp -o $(BAGUA_DIR)/bin/commDepDetectApproxi
	
asm:
	g++ -g -std=c++11  -o $(BAGUA_DIR)/bin/instruction_psg_counter $(BAGUA_DIR)/src/staticAnalysis/instruction_counter/instruction_psg.cpp -L${DYNINST_DIR}/lib -I${DYNINST_DIR}/include -ldyninstAPI -ldyninstAPI_RT -linstructionAPI -lboost_system -lsymLite -ldynDwarf -ldynElf -lcommon -lelf -ldw
