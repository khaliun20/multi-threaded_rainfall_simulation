CXX = g++
CXXFLAGS = -std=c++11

SOURCE = seq_rainfall.cpp
EXECUTABLE = seq_rainfall

# Default target
all: $(EXECUTABLE)

# Rule to build the executable
$(EXECUTABLE): $(SOURCE)
	$(CXX) $(CXXFLAGS) -o $@ $<

clean:
	rm -f $(EXECUTABLE) rainfall_table.txt
