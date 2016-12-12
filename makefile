CXX += -std=c++11 -stdlib=libc++ -O3 -g

EXECUTABLE = RunLaplacian

SRC = lib
INCLUDE = include
OBJDIR = build

CFLAGS += -I$(INCLUDE) -Wimplicit-function-declaration -Wall -Wextra -pedantic
VIPS_FLAGS = `pkg-config vips-cpp --cflags --libs`

all: $(EXECUTABLE)

RunLaplacian: $(OBJDIR)/laplacian.o
	$(CXX) $(CFLAGS) $(CPPFLAGS) $(CXXFLAGS) $(VIPS_FLAGS) $(OBJDIR)/laplacian.o -o RunLaplacian

$(OBJDIR)/%.o: $(SRC)/%.c++
	$(CXX) -c $(CFLAGS) $(CPPFLAGS) $(CXXFLAGS) $(VIPS_FLAGS) $< -o $@

clean:
	rm -f $(OBJDIR)/*.o $(EXECUTABLE)