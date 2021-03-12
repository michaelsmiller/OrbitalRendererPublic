CFLAGS = -std=c++17 -O2
LDFLAGS = -lglfw -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi

SHADERS = hardcoded.vert hardcoded.frag
SHADEROBJ = $(patsubst %,obj/%.spv,$(SHADERS))

default: clean test

obj/%.spv: shaders/%
	glslc $< -o $@

# Does not link the shader objects. That is done at runtime
TriangleTest: main.cpp $(SHADEROBJ)
	g++ $(CFLAGS) -o $@ $< $(LDFLAGS)

test: TriangleTest
	./TriangleTest

clean:
	rm -f TriangleTest
	rm -f obj/*
