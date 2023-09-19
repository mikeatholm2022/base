

CXX   = g++


DEBUG = 0


INC_DIRS = -I"." -I"$(BOOST_ROOT)/include" 
LIB_DIRS = -L"$(BOOST_ROOT)/lib"


# Is this a debug build
ifeq ($(DEBUG),1)
   CXXFLAGS = -O0 -ggdb3 -fPIC -DBOOST_BIND_GLOBAL_PLACEHOLDERS
else
   CXXFLAGS = -O3 -pthread -fPIC -fomit-frame-pointer -march=native -Wno-write-strings -ffloat-store -ffast-math -fno-math-errno -DBOOST_BIND_GLOBAL_PLACEHOLDERS
endif


# For debugging with the most verbose errors and warnings
#CXXFLAGS = -O0 -pthread -g -fPIC -Wall -DBOOST_BIND_GLOBAL_PLACEHOLDERS


LDFLAGS = -shared -pthread -lboost_thread
#LDFLAGS = -shared -pthread -lrt -ldl -lboost_thread -lboost_filesystem -lboost_date_time
#LDFLAGS = -shared -fPIC -pedantic -Wall -Wextra -march=native -ggdb3


target  = ../bin/libchaos_base.so
csrc = $(wildcard *.cpp)
objs = $(csrc:.cpp=.o)


.cpp.o:
	$(CXX) -c $(CXXFLAGS) $(INC_DIRS) $< -o $@


$(target): $(objs)
	$(CXX) $(LIB_DIRS) -o $@ $^ $(LDFLAGS)


.PHONY: clean
clean:
	rm -f ../bin/$(target) $(objs)

