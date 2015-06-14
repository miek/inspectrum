all:
	g++ -g -o inspectrum main.cpp `wx-config --cxxflags --libs` -lfftw3f

clean:
	rm inspectrum
