# Project Name

# TRAILING SPACES GET ADDED TO THE NAME, and will result in " .map, --cref" error
TARGET = PatchFemine

# Sources
CPP_SOURCES = Femine.cpp

#DEBUG=1

# Library Locations
LIBDAISY_DIR = ./libDaisy
DAISYSP_DIR = ./DaisySP

# add libfmsynth to sources
# C_SOURCES = ./libfmsynth/include

# Core location, and generic Makefile.
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile

