BUILD_DIR=build
APP_NAME=aggregator
LIBRARIES=-lnetfilter_queue -lnfnetlink
SOURCES=$(wildcard src/*.cc)
HEADERS=$(wildcard src/*.hh)

TARGET=$(addprefix $(BUILD_DIR)/, $(APP_NAME))

GXX=g++

.PHONY: default all clean

default: all

all: $(BUILD_DIR) $(TARGET)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(TARGET): $(SOURCES) $(HEADERS)
	$(GXX) $(LIBRARIES) -o $(TARGET) $(SOURCES)

clean:
	rm -rf $(BUILD_DIR)
