# invoke with make build
build:
	clang -o main main.c libs/draw.c libs/physics.c libs/structures.c -lallegro -lallegro_primitives -lallegro_font -lallegro_font -lallegro_image -lm

# invoke with make debug_build LEVEL=XXX
debug_build:
	clang -o main main.c libs/draw.c libs/physics.c libs/structures.c -lallegro -lallegro_primitives -lallegro_font -lallegro_font -lallegro_image -lm -DDEBUG=1 -DS_LEVEL=$(LEVEL)

