all:
	windres icon.rc -O coff -o icon.res
	clang++ main.cpp icon.res src/GameState.cpp src/collision/*.cpp src/common/*.cpp src/dynamics/*.cpp src/rope/*.cpp -o a.exe -g3 -I include -L lib -w -lSDL2main -lSDL2 -lSDL2_image -lSDL2_mixer -lSDL2_ttf
static:
	windres icon.rc -O coff -o icon.res
	g++ main.cpp icon.res src/GameState.cpp src/collision/*.cpp src/common/*.cpp src/dynamics/*.cpp src/rope/*.cpp -o a.exe -g3 -static -I include -DSDL_STATIC -L lib -w -lSDL2main -lSDL2 -lSDL2_image -lSDL2_mixer -lSDL2_ttf -lopengl32 -lgdi32 -lwinmm -limm32 -lole32 -loleaut32 -lshell32 -lsetupapi -lversion -luuid