
all:drm modeset

drm:drm-prime-dumb-kms.c
	gcc -g -o drm drm-prime-dumb-kms.c  `pkg-config libdrm --cflags --libs`


modeset:modeset.c
	gcc -g -o modeset modeset.c  `pkg-config libdrm --cflags --libs`

clean:
	rm drm
	rm modeset

