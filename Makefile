# Makefile for compiling malloc.c
#
# - Variables can be overridden from the environment. Example:
#     make CC=clang CFLAGS="-O3 -g" TARGET=yourname.so
#   symbol `malloc`. You may still build a binary named `malloc` by
#   overriding `TARGET` on the make command line.

# C++ compiler and flags are overrideable (use ?= so environment can override)
CXX ?= 
CFLAGS ?= -std=c++26 -Wall -Wextra -O2 -g

# Position-independent code flags for building shared objects. Overrideable.
PICFLAGS ?= -fPIC

# Linker flags for producing a shared object. Overrideable.
LDFLAGS ?= -shared

# Default executable name (overrideable). Avoids using the name "malloc".
TARGET ?= malloc.so

# Source files and derived object files
SRCS := malloc.cpp
OBJS := $(SRCS:.c=.o)

.PHONY: all clean

# Default goal: build the executable
all: $(TARGET)

# Link step: $@ = target, $^ = all prerequisites (object files)
# Use $(LDFLAGS) so user can override linker flags from environment.
$(TARGET): $(OBJS)
	$(CXX) $(CFLAGS) $(LDFLAGS) -o $@ $^

# Pattern rule: how to compile a .cpp into a .o
# $< = first prerequisite (the .cpp file)
%.o: %.cpp
	$(CXX) $(CFLAGS) $(PICFLAGS) -c -o $@ $<

# Remove build artifacts
clean:
	rm -f $(TARGET) $(OBJS)
	