PHONY = default tests libgtest.a tests clean list

default:
	@make all

list:
	echo $(PHONY)

tests: libgtest.a moving_avg_ticker_test.cc
	@clang++ -I../lib/gtest-1.7.0/include -pthread moving_avg_ticker_test.cc ../src/moving_avg_ticker.c libgtest.a -o test_executable

libgtest.a:
	clang++ -I../lib/gtest-1.7.0/include -I../lib/gtest-1.7.0 -c ../lib/gtest-1.7.0/src/gtest-all.cc -lpthread
	ar -rv libgtest.a gtest-all.o

run: tests
	./test_executable

clean:
	rm libgtest.a gtest-all.o test_executable
