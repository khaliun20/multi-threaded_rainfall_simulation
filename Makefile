
CXX = g++
CXXFLAGS = -std=c++11 -O3 
SOURCE_SEQ = rainfall_seq.cpp
SOURCE_PARALLEL = rainfall_pt.cpp

EXECUTABLE_SEQ = rainfall_seq
EXECUTABLE_PARALLEL = rainfall_pt

all: $(EXECUTABLE_SEQ) $(EXECUTABLE_PARALLEL)

# Rule to build the sequential executable
$(EXECUTABLE_SEQ): $(SOURCE_SEQ)
	$(CXX) $(CXXFLAGS) -o $@ $<

# Rule to build the parallel executable
$(EXECUTABLE_PARALLEL): $(SOURCE_PARALLEL)
	$(CXX) $(CXXFLAGS) -o $@ $< -pthread 

clean:
	rm -f $(EXECUTABLE_SEQ) $(EXECUTABLE_PARALLEL)
