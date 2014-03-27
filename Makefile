# TODO: update this.
.PHONY = default echo/objects echo/sources list tests run bin clean

SOURCES_C               = $(wildcard src/*.c)
TEST_CC                 = $(wildcard test/*.cc)

_SOURCES_OBJECTS_TMP    = ${SOURCES_C:.c=.o}
_TEST_OBJECTS_TMP       = ${TEST_CC:.cc=.o}
SOURCES_OBJECTS         = $(subst src,bin,$(_SOURCES_OBJECTS_TMP))
TEST_OBJECTS            = $(subst test,bin,$(_TEST_OBJECTS_TMP))

EXEC = tests

# removed: -Wl,-z,relro -Wl,-z,now
C_ARGS = -ansi -DIS_NOT_PEBBLE -DRELEASE -ffunction-sections -fno-exceptions -fPIC -fPIE -fstack-protector-all -fstrict-overflow -fvisibility=hidden -g -m64 -Os -pedantic-errors -pipe -std=c99 -W -Wall -Wbad-function-cast -Wc++-compat -Wcast-qual -Wcomment -Wconversion -Wdeprecated -Werror -Wextra -Wfloat-equal -Wformat -Wformat-nonliteral -Wformat-security -Wformat-y2k -Winit-self -Wmissing-include-dirs -Wmultichar -Wnested-externs -Wold-style-definition -Wpacked -Wpointer-arith -Wredundant-decls -Wreturn-type -Wsign-compare -Wstack-protector -Wstrict-prototypes -Wswitch-default -Wswitch-enum -Wtrigraphs -Wundef -Wuninitialized -Wuninitialized -Wunused-function -Wunused-label -Wunused-macros -Wunused-parameter -Wvariadic-macros -Wwrite-strings -Wlong-long -Wincompatible-pointer-types -Wtype-limits -Werror=type-limits
# removed: -lpthread, -Wl,-z,relro -Wl,-z,now -Wsign-conversion
CXX_ARGS = -DIS_NOT_PEBBLE -fno-exceptions -fPIC -fstack-protector -fvisibility=hidden -g -lpthread -m64 -O3 -pipe -W -Wall -Wextra -Wextra-tokens -Wconversion -Wformat -Wformat-nonliteral -Wformat-security -Winit-self -Wmultichar -Wno-deprecated -Wno-unused-function -Wno-unused-label -Wno-unused-parameter -Wpointer-arith -Wreturn-type -Wsign-compare -Wuninitialized -Wcast-qual -Wsign-compare -Wno-sign-conversion -Wno-conversion

default:
	@make all

echo/objects:
	@echo ${SOURCES_OBJECTS}
	@echo ${TEST_OBJECTS}

echo/sources:
	@echo ${SOURCES_C}
	@echo ${TEST_CC}

list:
	@echo $(.PHONY)

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
