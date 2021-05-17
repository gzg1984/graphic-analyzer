WAYLAND=`pkg-config wayland-client --cflags --libs`
CFLAGS?=-std=c11 -Wall -Werror -O3 -fvisibility=hidden

hello_wayland: hello_wayland.c helpers.c helpers.h images.bin
	$(CC) -o hello_wayland *.c $(WAYLAND) -lrt

images.bin: images/convert.py images/window.png images/fish.png
	images/convert.py
	cat window.bin fish.bin > images.bin

clean:
	$(RM) fish.bin window.bin hello_wayland


run:
	# if weston is started by root
	# this program only work in root
	# ./hello_wayland
	# or
	# sudo XDG_RUNTIME_DIR=/run/user/0  ./hello_wayland
	# if weston is starated by normal user
	# this program work with the same normal user

