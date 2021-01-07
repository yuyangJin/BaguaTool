all: clean func_instrument hello lib

func_instrument:
	#g++ -g -std=c++11 -o func_instrument patch.cpp -L/home/jinyuyang/workspace/build/dyninst-10.1.0/lib -I/home/jinyuyang/workspace/build/dyninst-10.1.0/include -ldyninstAPI -ldyninstAPI_RT -lboost_system -lsymLite -ldynDwarf -ldynElf -lcommon -lelf -ldw
	g++ -g -std=c++11 -o loop_instrument patch_psg.cpp -L/home/jinyuyang/workspace/build/dyninst-10.1.0/lib -I/home/jinyuyang/workspace/build/dyninst-10.1.0/include -ldyninstAPI -ldyninstAPI_RT -lboost_system -lsymLite -ldynDwarf -ldynElf -lcommon -lelf -ldw

hello:
	g++ -g -O0 hello.cpp -o hello

lib:
	g++ -g -O0 -std=c++11 lib.c -o liblib.so -fPIC -shared -lmpich -lpapi #-lmpi # -lmpich # -lpapi

clean:
	rm -f func_instrument hello liblib.so
