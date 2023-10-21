cfgsampler: cfgsampler.cpp gcache.h gdef.h
	g++ -g -o cfgsampler cfgsampler.cpp

gcache.h: build-grammar-cache.py grammar.py
	python3 build-grammar-cache.py 10 > gcache.h


gdef.h: compile-grammar.py grammar.py
	python3 compile-grammar.py > gdef.h


run-random-sample: depth=5
run-random-sample: sample=10
run-random-sample: rand=200
run-random-sample:
	./cfgsampler $(depth) -$(sample) $(rand)

run: depth=5
run:
	./cfgsampler $(depth)

clean:
	rm cfgsampler gcache.h gdef.h
