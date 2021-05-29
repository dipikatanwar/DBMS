sampleobjects = buffer_manager.o file_manager.o

all : 
	make linearsearch
	make binarysearch
	make deletion
	make join1
	make join2

linearsearch : $(sampleobjects) linearsearch.o
	    g++ -std=c++11 -o linearsearch $(sampleobjects) linearsearch.o

binarysearch : $(sampleobjects) binarysearch.o
	     g++ -std=c++11 -o binarysearch $(sampleobjects) binarysearch.o

deletion : $(sampleobjects) deletion.o
	     g++ -std=c++11 -o deletion $(sampleobjects) deletion.o

join1 : $(sampleobjects) join1.o
		g++ -std=c++11 -o join1 $(sampleobjects) join1.o

join2 : $(sampleobjects) join2.o
		g++ -std=c++11 -o join2 $(sampleobjects) join2.o

sample_run.o : sample_run.cpp
	g++ -std=c++11 -c sample_run.cpp

buffer_manager.o : buffer_manager.cpp
	g++ -std=c++11 -c buffer_manager.cpp

file_manager.o : file_manager.cpp
	g++ -std=c++11 -c file_manager.cpp

linearsearch.o : linearsearch.cpp
	g++ -std=c++11 -c linearsearch.cpp

binarysearch.o : binarysearch.cpp
	g++ -std=c++11 -c binarysearch.cpp

deletion.o : deletion.cpp
	g++ -std=c++11 -c deletion.cpp

join1.o : join1.cpp
	g++ -std=c++11 -c join1.cpp

join2.o : join2.cpp
	g++ -std=c++11 -c join2.cpp

clean :
	rm -f *.o
	rm -f sample_run
	rm -f linearsearch
	rm -f binarysearch
	rm -f deletion
	rm -f join1
	rm -f join2
