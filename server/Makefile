#Makefile template by Djal Bertrand (za goat)

#Not implemented yet : COMPILED_PATH = compiled/

OUTPUT_FILE = output.exe
COMPILE_COMMAND = clang++ -std=c++23
AMD_COMPILE_COMMAND = clang++-18 -pthread -std=c++23

SOURCES := $(wildcard lib/*.cpp) # here, we find every nested .cpp (* matches anything)
SOURCES := $(filter-out main.cpp, $(SOURCES)) # we filter out main.cpp
EXISTING_COMPILED := $(wildcard $(COMPILED_PATH)*.o)

COMPILED := $(patsubst %.cpp,$(COMPILED_PATH)%.o, $(SOURCES)) # we transform the .cpp strings to .o strings (% gets the name)

COMMAND_LINE_ARGS = 80 ./observed

all: build run

rock: $(SOURCES) main.cpp
	$(AMD_COMPILE_COMMAND) $^ -o rock_AMD_server.exe 

full_build: $(SOURCES) main.cpp
	$(COMPILE_COMMAND) $^ -o $(OUTPUT_FILE)
	./$(OUTPUT_FILE) $(COMMAND_LINE_ARGS)


build: main.cpp $(COMPILED)
	$(COMPILE_COMMAND) $^ -o $(OUTPUT_FILE) 


run: $(OUTPUT_FILE)
	./$^ $(COMMAND_LINE_ARGS)

clean: $(EXISTING_COMPILED)
ifneq ($(EXISTING_COMPILED), $(nullstring))
	$(info deleting $^)
	rm $^
endif

$(COMPILED): %.o: %.cpp
	$(info making $@)
	$(COMPILE_COMMAND) -c $? -o $@
