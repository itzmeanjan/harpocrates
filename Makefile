CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -pedantic
OPTFLAGS = -O3
IFLAGS = -I ./include

all: test_harpocrates

test/a.out: test/main.cpp include/*.hpp
	$(CXX) $(CXXFLAGS) $(OPTFLAGS) $(IFLAGS) $< -o $@

test_harpocrates: test/a.out
	./$<

clean:
	find . -name '*.out' -o -name '*.o' -o -name '*.so' -o -name '*.gch' | xargs rm -rf

format:
	find . -name '*.cpp' -o -name '*.hpp' | xargs clang-format -i --style=Mozilla

bench/a.out: bench/main.cpp include/*.hpp
	$(CXX) $(CXXFLAGS) $(OPTFLAGS) $(IFLAGS) $< -lbenchmark -o $@

benchmark: bench/a.out
	./$<
