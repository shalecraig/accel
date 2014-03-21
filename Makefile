# TODO: update this.
PHONY = default echo/objects echo/sources list tests run bin clean

SOURCES_C               = $(wildcard src/*.c)
TEST_CC                 = $(wildcard test/*.cc)

_SOURCES_OBJECTS_TMP    = ${SOURCES_C:.c=.o}
_TEST_OBJECTS_TMP       = ${TEST_CC:.cc=.o}
SOURCES_OBJECTS         = $(subst src,bin,$(_SOURCES_OBJECTS_TMP))
TEST_OBJECTS            = $(subst test,bin,$(_TEST_OBJECTS_TMP))

EXEC = tests

default:
	@make all

echo/objects:
	@echo ${SOURCES_OBJECTS}
	@echo ${TEST_OBJECTS}

echo/sources:
	@echo ${SOURCES_C}
	@echo ${TEST_CC}

list:
	@echo $(PHONY)

tests: bin/tests

bin/tests: ${SOURCES_OBJECTS} ${TEST_OBJECTS} bin/libgtest.a
	clang++ -Ilib/gtest-1.7.0/include ${SOURCES_OBJECTS} ${TEST_OBJECTS} bin/libgtest.a -o bin/tests

run: tests
	./bin/tests

bin/%.o : src/%.c
	clang -DIS_NOT_PEBBLE -c $< -o $@

bin/%.o : test/%.cc
	clang++ -DIS_NOT_PEBBLE -Ilib/gtest-1.7.0/include -c $< -o $@

bin/libgtest.a: bin
	clang++ -Ilib/gtest-1.7.0/include -Ilib/gtest-1.7.0 -c lib/gtest-1.7.0/src/gtest-all.cc -lpthread -o bin/libgtest.a

bin:
	mkdir -p bin

clean:
	rm -rf bin
