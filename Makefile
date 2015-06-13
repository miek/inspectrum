all:
	g++ -g -o merry-supernova main.cpp `wx-config --cxxflags --libs` -lfftw3f

clean:
	rm merry-supernova
