# TODO: update this.
PHONY = default echo/objects echo/sources list tests run bin clean

SOURCES_C               = $(wildcard src/*.c)
TEST_CC                 = $(wildcard test/*.cc)

_SOURCES_OBJECTS_TMP    = ${SOURCES_C:.c=.o}
_TEST_OBJECTS_TMP       = ${TEST_CC:.cc=.o}
SOURCES_OBJECTS         = $(subst src,bin,$(_SOURCES_OBJECTS_TMP))
TEST_OBJECTS            = $(subst test,bin,$(_TEST_OBJECTS_TMP))

EXEC = tests

# Pebble args: -std=c99 -fdata-sections -g -Wall -Wextra -Werror -Wno-unused-parameter -Wno-error=unused-function -Wno-error=unused-variable -fPIE -DRELEASE -Os
# removed: -Wl,-z,relro -Wl,-z,now
C_ARGS = -DIS_NOT_PEBBLE -DRELEASE -pipe -m64 -ansi -fPIC -fPIE -g -Os -ffunction-sections -fno-exceptions -fstack-protector-all -fvisibility=hidden -W -Wall -Wextra -Wunused-parameter -Wunused-function -Wunused-label -Wpointer-arith -Wformat -Wreturn-type -Wsign-compare -Wmultichar -Wformat-nonliteral -Winit-self -Wuninitialized -Wdeprecated -Wformat-security -Werror -Wcomment -Wtrigraphs -Wundef -Wunused-macros -pedantic-errors -std=c99
# removed: -lpthread, -Wl,-z,relro -Wl,-z,now
CXX_ARGS = -DIS_NOT_PEBBLE -lpthread -pipe -m64 -ansi -fPIC -g -O3 -fno-exceptions -fstack-protector -fvisibility=hidden -W -Wall -Wno-unused-parameter -Wno-unused-function -Wno-unused-label -Wpointer-arith -Wformat -Wreturn-type -Wsign-compare -Wmultichar -Wformat-nonliteral -Winit-self -Wuninitialized -Wno-deprecated -Wformat-security -Wall

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

bin/tests: ${SOURCES_OBJECTS} ${TEST_OBJECTS} bin/libgtest.a bin
	clang++ -Ilib/gtest-1.7.0/include ${SOURCES_OBJECTS} ${TEST_OBJECTS} bin/libgtest.a -lpthread -o bin/tests

run: tests
	./bin/tests

bin/%.o : src/%.c bin
	clang ${C_ARGS} -c $< -o $@

bin/%.o : test/%.cc bin
	clang++ -Ilib/gtest-1.7.0/include ${CXX_ARGS} -c $< -o $@

bin/libgtest.a: bin
	clang++ -Ilib/gtest-1.7.0/include -Ilib/gtest-1.7.0 -c lib/gtest-1.7.0/src/gtest-all.cc -o bin/libgtest.a

bin:
	mkdir -p bin

clean:
	rm -rf bin
