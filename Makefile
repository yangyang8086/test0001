# Makefile for SMTP Parser

# 1. Compiler and Flags
CXX = g++
CXXFLAGS = -std=c++11 -I$(PPCAPPLUSPLUS_HOME)/Dist/include
LDFLAGS = -L$(PPCAPPLUSPLUS_HOME)/Dist

# 2. PcapPlusPlus Configuration
# -----------------------------------------------------------------------------
# IMPORTANT: You might need to change this path to where you installed PcapPlusPlus
#            或者，您可以在运行 make 时指定它，例如：
#            make PPCAPPLUSPLUS_HOME=/path/to/pcapplusplus
#
# IMPORTANT: You may need to change this path to where you installed PcapPlusPlus
#            Alternatively, you can specify it when running make, e.g.:
#            make PPCAPPLUSPLUS_HOME=/path/to/pcapplusplus
PPCAPPLUSPLUS_HOME ?= /usr/local

# PcapPlusPlus libraries to link with
# The order is important!
LIBS = -lPcapPlusPlus -lPacket -lCommon -lpcap

# 3. Project Files
# -----------------------------------------------------------------------------
TARGET = smtp_parser
SOURCES = main.cpp
OBJECTS = $(SOURCES:.cpp=.o)

# 4. Rules
# -----------------------------------------------------------------------------
.PHONY: all
all: $(TARGET)

$(TARGET): $(OBJECTS)
	@echo "Linking..."
	$(CXX) $(OBJECTS) $(LDFLAGS) $(LIBS) -o $(TARGET)

%.o: %.cpp
	@echo "Compiling $<..."
	$(CXX) $(CXXFLAGS) -c $< -o $@

.PHONY: clean
clean:
	@echo "Cleaning up..."
	rm -f $(OBJECTS) $(TARGET)

.PHONY: help
help:
	@echo "Usage: make [PPCAPPLUSPLUS_HOME=/path/to/pcapplusplus]"
	@echo "  Compiles the project."
	@echo "  You may need to provide the path to your PcapPlusPlus installation."
	@echo ""
	@echo "Targets:"
	@echo "  all       (default) Compile the project."
	@echo "  clean     Remove compiled files."
