
all:gl3example gl3 glx_xcb

gl3example:gl3example.cpp
	g++ gl3example.cpp -o gl3example -lX11 -lGL

gl3:gl3.cxx
	cc -o gl3 gl3.cxx -lGL -lX11

glx_xcb:glx_xcb.cpp
	g++ glx_xcb.cpp -o glx_xcb -lxcb -lGL -lX11  -lX11-xcb

