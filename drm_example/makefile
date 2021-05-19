
all:drm modeset dri

drm:drm-prime-dumb-kms.c
	gcc -g -o drm drm-prime-dumb-kms.c  `pkg-config libdrm --cflags --libs`

modeset:modeset.c
	gcc -g -o modeset modeset.c  `pkg-config libdrm --cflags --libs`

dri:dri_master.c
	gcc -o dri dri_master.c

clean:
	rm -f drm
	rm -f modeset
	rm -f dri

