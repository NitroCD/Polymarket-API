CXX = g++
CXXFLAGS = -Wall -Iinclude
LDFLAGS = -lcurl

SRC_DIR = src
SOURCES = main.cpp $(SRC_DIR)/event.cpp $(SRC_DIR)/market.cpp
OBJECTS = $(SOURCES:.cpp=.o)
TARGET = main

$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET).exe