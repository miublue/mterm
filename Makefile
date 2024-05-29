OUT = mterm
LIB = `pkg-config --libs --cflags gtk+-3.0 vte-2.91 gdk-x11-3.0 x11`

# it seems i can't compile gtk3 programs with tcc blegh
all:
	gcc -o $(OUT) src/*.c $(LIB)

