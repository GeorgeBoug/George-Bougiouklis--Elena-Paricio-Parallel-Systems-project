# EASY SETTINGS (MODIFY HERE)
FILE = com-Orkut.mtx
THREADS = 4


# COMPILERS
CXX = g++
CXXFLAGS = -O3 -std=c++17 -march=native
OMPFLAG = -fopenmp
PTHREADFLAG = -pthread

CILKCXX = clang++
CILKFLAGS = -O3 -std=c++17 -fopencilk

COMMON = conversion_graph.cpp


# TARGETS to create

all: cc_sequential cc_pthreads cc_openmp cc_opencilk


cc_sequential: cc_sequential.cpp $(COMMON)
	$(CXX) $(CXXFLAGS) cc_sequential.cpp $(COMMON) -o cc_sequential
	./cc_sequential $(FILE)


cc_pthreads: cc_pthreads.cpp $(COMMON)
	$(CXX) $(CXXFLAGS) $(PTHREADFLAG) cc_pthreads.cpp $(COMMON) -o cc_pthreads
	./cc_pthreads $(FILE) $(THREADS)


cc_openmp: cc_openmp.cpp $(COMMON)
	$(CXX) $(CXXFLAGS) $(OMPFLAG) cc_openmp.cpp $(COMMON) -o cc_openmp
	./cc_openmp $(FILE) $(THREADS)


cc_opencilk: cc_opencilk.cpp $(COMMON)
	$(CILKCXX) $(CILKFLAGS) cc_opencilk.cpp $(COMMON) -o cc_opencilk
	./cc_opencilk $(FILE) $(THREADS)


clean:

	rm -f cc_sequential cc_pthreads cc_openmp cc_cilk *.o
