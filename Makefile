all:
	g++ -g -o inspectrum inputsource.cpp main.cpp `wx-config --cxxflags --libs` -lfftw3f

clean:
	rm inspectrum
