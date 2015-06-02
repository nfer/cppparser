
# Directories:
SRC_DIR          = ..
GTEST_DIR        = .
CUR_DIR          = .

# Sources:
CUR_SRC         = $(wildcard $(CUR_DIR)/*.cpp) ../split.cpp

### set variables before include make.rules
# debug ?= 1
# quiet ?= 1

out_executable  = cppparser_gtestcase
include_dirs    = $(CUR_DIR) $(GTEST_DIR) $(SRC_DIR)
cpp_sources     = $(CUR_SRC)

CXXFLAGS		+= -g -Wall -Wextra
LDFLAGS         += -lpthread $(GTEST_DIR)/gtest/lib/gtest_main.a
include make.rules
