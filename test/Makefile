PHONY = default tests libgtest.a tests clean list

default:
	@make all

list:
	echo $(PHONY)

# TODO: use variables and stuff, this is super messy.

tests: moving_avg_ticker_test accel_test

moving_avg_ticker_test: libgtest.a moving_avg_ticker_test.cc ../src/moving_avg_ticker.h ../src/moving_avg_ticker.c
	@clang++ -DIS_NOT_PEBBLE -I../lib/gtest-1.7.0/include -pthread moving_avg_ticker_test.cc ../src/moving_avg_ticker.c libgtest.a -o moving_avg_ticker_test -Wall

accel_test: libgtest.a accel_test.cc ../src
	@clang++ -DIS_NOT_PEBBLE -I../lib/gtest-1.7.0/include -pthread accel_test.cc ../src/accel.c ../src/moving_avg_ticker.c libgtest.a -o accel_test -Wall

libgtest.a:
	clang++ -I../lib/gtest-1.7.0/include -I../lib/gtest-1.7.0 -c ../lib/gtest-1.7.0/src/gtest-all.cc -lpthread
	ar -rv libgtest.a gtest-all.o

run: moving_avg_ticker_test accel_test
	./moving_avg_ticker_test
	./accel_test

clean:
	rm -f libgtest.a gtest-all.o moving_avg_ticker_test accel_test
