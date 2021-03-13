CFLAGS = -std=c++17 -O2
LDFLAGS = -lglfw -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi

SHADERS = hardcoded.vert hardcoded.frag
SHADEROBJ = $(patsubst %,obj/%.spv,$(SHADERS))

SOURCE_FILES=main.cpp molecule_kernel.cpp molecule_reader.cpp
OBJ_FILES = $(patsubst %,obj/%,$(SOURCE_FILES:.cpp=.o))

default: clean test

obj/%.o: src/%.cpp
	$(CXX) $(CFLAGS) -o $@ -c $<

obj/%.spv: shaders/%
	glslc $< -o $@

# Does not link the shader objects. That is done at runtime
MoleculeRenderer: $(OBJ_FILES) $(SHADEROBJ)
	$(CXX) $(CFLAGS) -o $@ $(OBJ_FILES) $(LDFLAGS)

test: MoleculeRenderer
	./MoleculeRenderer

clean:
	rm -f MoleculeRenderer
	rm -f obj/*
