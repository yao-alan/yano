make:
	g++ -std=c++17 -O2 -Wall -o yano *.cpp -lxcb -lxcb-image -lX11

PHONY: test clean

test:
	make && ./yano

clean:
	rm -f ./yano
