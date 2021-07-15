
all:drm modeset dri_master drm_magic

drm_magic:drm_magic.c
	gcc -g -o $@ $^  `pkg-config libdrm --cflags --libs`

drm:drm-prime-dumb-kms.c
	gcc -g -o drm drm-prime-dumb-kms.c  `pkg-config libdrm --cflags --libs`

modeset:modeset.c
	gcc -g -o modeset modeset.c  `pkg-config libdrm --cflags --libs`

dri_master:dri_master.c
	gcc -o  $@ $^ 


kms:kms-pageflip.c
	gcc -g -o kms kms-pageflip.c  `pkg-config libdrm --cflags --libs`


clean:
	rm -f drm
	rm -f modeset
	rm -f dri_master
	rm -f drm_magic

