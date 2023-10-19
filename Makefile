cfgsampler: cfgsampler.cpp gcache.h gdef.h
	g++ -g -o cfgsampler cfgsampler.cpp

gcache.h: build-grammar-cache.py grammar.py
	python3 build-grammar-cache.py > gcache.h


gdef.h: compile-grammar.py grammar.py
	python3 compile-grammar.py > gdef.h



