#Generated by VisualGDB (http://visualgdb.com)
#DO NOT EDIT THIS FILE MANUALLY UNLESS YOU ABSOLUTELY NEED TO
#USE VISUALGDB PROJECT PROPERTIES DIALOG INSTEAD

BINARYDIR := Debug

#Toolchain
CC := gcc
CXX := g++
LD := $(CXX)
AR := ar
OBJCOPY := objcopy

#Additional flags
PREPROCESSOR_MACROS := DEBUG
INCLUDE_DIRS := /usr/include/libmongoc-1.0 /usr/include/libbson-1.0 /root/mongo-client-install/include
LIBRARY_DIRS := 
LIBRARY_NAMES := pthread mongoc-1.0 bson-1.0
ADDITIONAL_LINKER_INPUTS := /usr/local/lib/libboost_system.a /usr/local/lib/libboost_thread.a /usr/local/lib/libboost_regex.a /root/mongo-client-install/lib/libmongoclient.a
MACOS_FRAMEWORKS := 
LINUX_PACKAGES := 

CFLAGS := -ggdb -ffunction-sections -O0
CXXFLAGS := -ggdb -ffunction-sections -std=c++11 -O0
ASFLAGS := 
LDFLAGS := -Wl,-gc-sections
COMMONFLAGS := 

START_GROUP := -Wl,--start-group
END_GROUP := -Wl,--end-group

#Additional options detected from testing the toolchain
IS_LINUX_PROJECT := 1
