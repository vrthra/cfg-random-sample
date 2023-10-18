r: r.cpp d.h
	g++ -g -o r r.cpp

d.h: build-grammar-cache.py
	python3 build-grammar-cache.py > d.h


