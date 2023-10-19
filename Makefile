r: r.cpp d.h defs.h
	g++ -g -o r r.cpp

d.h: build-grammar-cache.py
	python3 build-grammar-cache.py > d.h


defs.h: compile-grammar.py
	python3 compile-grammar.py > defs.h



