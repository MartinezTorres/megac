CFLAGS += -Wall -Wextra -pedantic
CFLAGS += -Wcast-qual -Wcast-align -Wstrict-aliasing=1 -Wswitch-enum -Wundef -Wfatal-errors -Wshadow
CFLAGS += -Winvalid-pch -Winit-self -Wno-inline -Wpacked -Wpointer-arith -Wlarger-than-65500 -Wmissing-declarations -Wmissing-format-attribute -Wmissing-noreturn -Wredundant-decls -Wsign-compare -Wswitch-enum -Wundef -Wunreachable-code -Wwrite-strings

CFLAGS += -Werror


CFLAGS += -I./src

CFLAGS += -O3 -g

CFLAGS += -std=gnu++2a

LFLAGS += -lboost_system -lboost_program_options -lboost_iostreams

CXX = g++
#CXX = clang++

CC_FILES  := $(wildcard ./src/*.cc)
OBJ_FILES := $(patsubst ./src/%.cc, ./tmp/%.o, $(CC_FILES))

all: ./bin/megac

.PHONY: clean

./tmp/%.o: ./src/%.cc ./src/*.h
	@echo "CREATING $@"
	@mkdir -p $(@D)
	@$(CXX) -c -o $@ $< $(CFLAGS)

./bin/megac: $(OBJ_FILES)
	@echo "CREATING $@" 
	@mkdir -p $(@D)
	@$(CXX) -o $@ $+ $(CFLAGS) $(LFLAGS)

clean:
	rm -rf ./bin ./tmp

