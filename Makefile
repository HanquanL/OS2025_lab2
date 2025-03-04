linker: sched.cpp
		g++ -o sched sched.cpp -std=c++11 -pthread
clean:
		rm -f sched *~