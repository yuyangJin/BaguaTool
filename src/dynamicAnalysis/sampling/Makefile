SRC_ROOT := .

CXX := g++
CXXFLAGS := -std=c++11 -fno-rtti -UNDEBUG -g -O2 -fno-omit-frame-pointer -fstack-protector-all 
PLUGIN_CXXFLAGS := -fpic

PLUGIN_LDFLAGS := -shared

AR := ar
ARTAG := rcs

#MPI_INCLUDE := -I/opt/openmpi-3.0.0/include
MPI_INCLUDE := -I/mnt/home/jinyuyang/JASMIN/jasmin/thirdparty/mpich2-1.5/include

all: libdynco # libdyncoPMU


#libsampler: $(SRC_ROOT)/sampler.cpp 
#	mpicxx $(CXXFLAGS) $(MPI_INCLUDE) -ldl -Wl,--no-undefined  -lunwind -shared -fPIC -lmpi -lpapi -o $@.so $< mpiinfo.cpp
	#$(CXX) $(CXXFLAGS) $(MPI_INCLUDE) -ldl -Wl,--no-undefined -lunwind -shared -fPIC -lmpi -lpapi -o $@.so $<

libdynco: $(SRC_ROOT)/dyncollect.cpp 
	#mpicxx $(CXXFLAGS) $(MPI_INCLUDE) -ldl -Wl,--no-undefined -L/usr/lib/x86_64-linux-gnu/libpapi.so -L/home/jinyuyang/workspace/build/libunwind-1.3.1/lib  -o $@.so $<  -shared -fPIC -lmpi -lpapi -lunwind
	#mpicxx $(CXXFLAGS) $(MPI_INCLUDE) -ldl -Wl,--no-undefined -o $@.so $< commData.cpp  -shared -fPIC -lmpi -lmpi_mpifh -lpapi -lunwind 
	mpicxx $(CXXFLAGS) $(MPI_INCLUDE) -ldl -Wl,--no-undefined -o $@-mpich2-1.5.so $< commData-mpich2-1.5.cpp -L/mnt/home/jinyuyang/JASMIN/jasmin/thirdparty/mpich2-1.5/lib -shared -fPIC -lmpichf90 -lmpich -lopa -lmpl -lrt -lpthread -lpapi -lunwind -fopenmp -lm 

libdyncoPMU: $(SRC_ROOT)/dyncollect_multiPMU.cpp
	#mpicxx $(CXXFLAGS) $(MPI_INCLUDE) -ldl -Wl,--no-undefined -o $@.so $< commData.cpp  -shared -fPIC -lmpi -lmpi_mpifh -lpapi -lunwind 
	mpicxx $(CXXFLAGS) $(MPI_INCLUDE)  -ldl -Wl,--no-undefined -o $@-mpich2-1.5-new.so $< commData-mpich2-1.5.cpp -L/mnt/home/jinyuyang/JASMIN/jasmin/thirdparty/mpich2-1.5/lib -shared -fPIC -lmpichf90 -lmpich -lopa -lmpl -lrt -lpthread -lpapi -lunwind -fopenmp -lm 

libcomm: $(SRC_ROOT)/commData.cpp
	#mpicxx $(CXXFLAGS) $(MPI_INCLUDE) -ldl -Wl,--no-undefined  -o $@.so $< -shared -fPIC -lmpi -lmpi_mpifh 
	mpicxx $(CXXFLAGS) $(MPI_INCLUDE)  -ldl -Wl,--no-undefined -o $@-mpich2-1.5-new.so  commData-mpich2-1.5-1.cpp -L/mnt/home/jinyuyang/JASMIN/jasmin/thirdparty/mpich2-1.5/lib -shared -fPIC -lmpichf90 -lmpich -lopa -lmpl -lrt -lpthread -lunwind -fopenmp -lm 
clean:
	rm -rf *.o libdynco.so
