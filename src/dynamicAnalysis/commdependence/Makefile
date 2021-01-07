MPI_INCLUDE := -I/opt/openmpi-3.0.0/include
MPI_LIB := -L/opt/openmpi-3.0.0/lib
MPI_FLAGS := -lmpi -lmpi_mpifh

all: parse lib test

lib:
	python ./wrap/wrap.py -f ./commDependence_v1.1.w -o ./commDependence.cpp
	mpicxx -std=c++11 -fno-rtti -UNDEBUG -g -O2 -fno-omit-frame-pointer -fstack-protector-all -ldl -Wl,--no-undefined -o libcommDep.so ./commDependence.cpp $(MPI_LIB) -shared -fPIC $(MPI_FLAGS) -lunwind

test:
	mpicxx commSplitTest.cpp -o commSplitTest
	python ./wrap/wrap.py -f ./wrap/mpi_request_lib.w -o ./mpi_request_lib.cpp 
	mpicxx -std=c++11 -fno-rtti -UNDEBUG -g -O2 -fno-omit-frame-pointer -fstack-protector-all -ldl -Wl,--no-undefined -o libreq.so ./mpi_request_lib.cpp $(MPI_LIB) -shared -fPIC $(MPI_FLAGS) -lunwind
	mpicxx ./mpi_wait_test.cpp -o mpi_wait_test 

parse:
	g++ -g -std=c++11 ./commDepDetect.cpp -o commDepDetect
