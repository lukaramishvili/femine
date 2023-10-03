# Project Name

# TRAILING SPACES GET ADDED TO THE NAME, and will result in " .map, --cref" error
TARGET = PatchFemine

# Sources
CPP_SOURCES = Femine.cpp

#DEBUG=1

# Library Locations
LIBDAISY_DIR = ./libDaisy
DAISYSP_DIR = ./DaisySP

# add libfmsynth to libraries (free bonus: libfmsynth will be included before others)
OBJECTS = ./libfmsynth/obj/src/fmsynth.o
# OBJECTS = ./libfmsynth/obj/src/arm/fmsynth_neon.o

# LIBS += -l ./libfmsynth/obj/src/fmsynth.o
# LIBDIR = -L ./libfmsynth/obj/src

# Core location, and generic Makefile.
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile

