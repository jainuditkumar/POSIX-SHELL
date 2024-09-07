# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -Wall -g

# Source files
SRCS = shell.cpp search.cpp pwd.cpp pinfo.cpp ls.cpp io_redirection.cpp echo.cpp cd.cpp history.cpp

# Object files
OBJS = $(SRCS:.cpp=.o)

# Output executable
TARGET = shell

# Default target
all: $(TARGET)

# Build the executable
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

# Build object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up build files
clean:
	rm -f $(OBJS) $(TARGET)
