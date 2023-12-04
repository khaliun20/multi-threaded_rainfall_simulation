CXX = g++
CXXFLAGS = -std=c++11

SOURCE = rainfall_simulation_sequential.cpp
EXECUTABLE = rainfall_simulation_sequential

# Default target
all: $(EXECUTABLE)

# Rule to build the executable
$(EXECUTABLE): $(SOURCE)
	$(CXX) $(CXXFLAGS) -o $@ $<

clean:
	rm -f $(EXECUTABLE)
