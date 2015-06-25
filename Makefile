all:
	g++ -O3 -std=c++11 -g -o inspectrum inputsource.cpp wximagepanel.cpp main.cpp `wx-config --cxxflags --libs` -lfftw3f

clean:
	rm inspectrum
