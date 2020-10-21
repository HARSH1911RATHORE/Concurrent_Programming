
all: counter mysort

counter: main_counter.cpp
	g++ main_counter.cpp -pthread  -g -o counter

mysort: bucket_main.cpp
	g++ bucket_main.cpp -pthread -g -o mysort
.PHONY : clean
clean:
	rm counter
	rm mysort

