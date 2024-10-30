all:
	g++ -Isrc/include -Lsrc/lib -o cassebrique main.cpp -lmingw32 -lSDL2main -lSDL2 -lSDL2_ttf -lm