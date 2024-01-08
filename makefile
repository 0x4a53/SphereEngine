# Compiler settings
CC = g++
CFLAGS = -Wall -Wextra -std=c++11
LDFLAGS = -lglfw -lGLEW -lGL

# Project files
SRCS = main.cpp # Add your .cpp source files here
OBJS = $(SRCS:.cpp=.o)
DEPS = $(SRCS:.cpp=.d) # Dependency files
EXE = myOpenGLProgram

# Targets
all: $(EXE)

$(EXE): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

# Include the dependency files
-include $(DEPS)

# Rule to generate a file of dependencies
%.d: %.cpp
	$(CC) $(CFLAGS) -MM -MT $(@:.d=.o) $< > $@

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(EXE) $(DEPS)

.PHONY: all clean

