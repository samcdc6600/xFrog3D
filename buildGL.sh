#!/bin/sh
#OpenBSD
#freeglut must be installed! :)
#clang++ -std=c++11 -Wall main.cpp utility.cpp world.cpp -lGL -lGLU -lglut -L/usr/X11R6/lib -I/usr/X11R6/include -L/usr/local/lib/ -I/usr/local/include/
#Ubuntu
#freeglut3-dev must be installed! :)
clang++ -Wall -std=c++14 main.cpp utility.cpp world.cpp Camera.cpp Frogject.cpp Tesselation.cpp normals.cpp -lglut -lGLU -lGL ./libSOIL.a
#g++ -Wall -std=c++14 main.cpp utility.cpp world.cpp Camera.cpp Frogject.cpp Tesselation.cpp normals.cpp -lglut -lGLU -lGL ./libSOIL.a
rm *~
rm *.gch
rm *#
