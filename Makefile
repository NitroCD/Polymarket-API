CXX = g++
CXXFLAGS = -Wall -Iinclude
LDFLAGS = -lcurl

SRC_DIR = src
BUILD_DIR = build
BIN_DIR = bin

# Automatically find every .cpp in src/, plus main.cpp at the root
SOURCES = main.cpp $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS = $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(notdir $(SOURCES)))
TARGET = $(BIN_DIR)/main

$(TARGET): $(OBJECTS) | $(BIN_DIR)
	$(CXX) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

$(BUILD_DIR)/%.o: %.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)

.PHONY: clean