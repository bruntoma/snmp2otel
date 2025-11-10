CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -pedantic -g -O0
LDFLAGS := 

SRC_DIR := src
INCLUDE_DIR := include
BUILD_DIR := build
BIN_DIR := bin

TARGET := $(BIN_DIR)/program

SOURCES := $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS := $(SOURCES:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)

INCLUDES := -I$(INCLUDE_DIR)

all: $(TARGET)

$(TARGET): $(OBJECTS) | $(BIN_DIR)
	$(CXX) $(OBJECTS) -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

$(BIN_DIR):
	@mkdir -p $(BIN_DIR)

run: $(TARGET)
	@./$(TARGET)

debug: CXXFLAGS += -DDEBUG -g3
debug: $(TARGET)

release: CXXFLAGS := -std=c++17 -Wall -Wextra -pedantic -O3 -DNDEBUG
release: clean $(TARGET)

clean:
	@rm -rf $(BUILD_DIR)
	@rm -rf $(BIN_DIR)

rebuild: clean all

.PHONY: all run debug release clean rebuild info help

-include $(OBJECTS:.o=.d)

$(BUILD_DIR)/%.d: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	@$(CXX) $(CXXFLAGS) $(INCLUDES) -MM -MT $(BUILD_DIR)/$*.o $< > $@