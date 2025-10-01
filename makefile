CXX = g++
CXXFLAGS = -Wall -Werror -std=c++11

TARGETS = firstfit bestfit

FIRSTFIT_SRC = first-fit.cpp
BESTFIT_SRC = best-fit.cpp

all: $(TARGETS)

firstfit: $(FIRSTFIT_SRC)
	$(CXX) $(CXXFLAGS) -o firstfit $(FIRSTFIT_SRC)

bestfit: $(BESTFIT_SRC)
	$(CXX) $(CXXFLAGS) -o bestfit $(BESTFIT_SRC)

clean:
	rm -f $(TARGETS) *.o
