DEBUG_FLAG=-DDEBUG
all: clean instruction_counter test

instruction_counter: instruction.cpp
#	g++ -g -std=c++11  -o instruction_counter instruction.cpp -L/home/jinyuyang/workspace/build/dyninst-10.1.0/lib -I/home/jinyuyang/workspace/build/dyninst-10.1.0/include -ldyninstAPI -ldyninstAPI_RT -linstructionAPI -lboost_system -lsymLite -ldynDwarf -ldynElf -lcommon -lelf -ldw
	g++ -g -std=c++11 $(DEBUG_FLAG) -o instruction_psg_counter instruction_psg.cpp -L/home/jinyuyang/workspace/build/dyninst-10.1.0/lib -I/home/jinyuyang/workspace/build/dyninst-10.1.0/include -ldyninstAPI -ldyninstAPI_RT -linstructionAPI -lboost_system -lsymLite -ldynDwarf -ldynElf -lcommon -lelf -ldw
#	g++ -g -std=c++11  -o PSG-instruction_counter PSG-addr.cpp -L/home/jinyuyang/workspace/build/dyninst-10.1.0/lib -I/home/jinyuyang/workspace/build/dyninst-10.1.0/include -ldyninstAPI -ldyninstAPI_RT -linstructionAPI -lboost_system -lsymLite -ldynDwarf -ldynElf -lcommon -lelf -ldw
#counter1:
#	g++ -g -std=c++11 -o counter1 counter.cpp -L/home/jinyuyang/workspace/build/dyninst-10.1.0/lib -I/home/jinyuyang/workspace/build/dyninst-10.1.0/include -ldyninstAPI -ldyninstAPI_RT -lboost_system -lsymLite -ldynDwarf -ldynElf -lcommon -lelf -ldw
test:
	gcc -O0 -g -o test test.cpp 
# export LD_LIBRARY_PATH=/home/wzby/workspace/build/dyninst-10.1.0/lib:$LD_LIBRARY_PATH
# export PATH=/home/wzby/workspace/build/dyninst-10.1.0/lib:$PATH
# export DYNINSTAPI_RT_LIB=/home/wzby/workspace/build/dyninst-10.1.0/lib:$DYNINSTAPI_RT_LIB
clean:
	rm -f instruction_counter test
