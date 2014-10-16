CPP_FILES := $(wildcard context/*.cpp)
OBJ_FILES := $(addprefix obj/,$(notdir $(CPP_FILES:.cpp=.o)))
LD_FLAGS := -lfastbit 
CC_FLAGS := -std=c++11 -w

main: $(OBJ_FILES)
	g++ $(LD_FLAGS) -o $@ $^ -lfastbit

obj/%.o: context/%.cpp
	g++ $(CC_FLAGS) -c -o $@ $<
