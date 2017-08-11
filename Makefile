all: sample2D

sample2D: Sample_GL3_2D.cpp
	g++ -o  sample2D Sample_GL3_2D.cpp -lGL -lGLU -lGLEW -lglut  -g -O0 
clean:
	rm sample2D

