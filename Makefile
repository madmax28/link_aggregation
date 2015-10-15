BUILD_DIR=build
APP_NAME=aggregator
SOURCES=$(wildcard *.cpp)
HEADERS=$(wildcard *.hpp)

TARGET=$(addprefix $(BUILD_DIR)/, $(APP_NAME))


GXX=g++

.PHONY: default all clean

default: all

all: $(BUILD_DIR) $(TARGET)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(TARGET): $(SOURCES) $(HEADERS)
	$(GXX) -o $(TARGET) $(SOURCES)

clean:
	rm -rf $(BUILD_DIR)
