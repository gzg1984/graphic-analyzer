
all:drm-prime-dumb-kms modeset dri_master drm_magic

drm_magic:drm_magic.c
	gcc -g -o $@ $^  `pkg-config libdrm --cflags --libs`

drm-prime-dumb-kms:drm-prime-dumb-kms.c
	gcc -g -o $@ $^   `pkg-config libdrm --cflags --libs`

modeset:modeset.c
	gcc -g -o modeset modeset.c  `pkg-config libdrm --cflags --libs`

dri_master:dri_master.c
	gcc -o  $@ $^ 


kms:kms-pageflip.c
	gcc -g -o kms kms-pageflip.c  `pkg-config libdrm --cflags --libs`


clean:
	rm -f drm-prime-dumb-kms
	rm -f modeset
	rm -f dri_master
	rm -f drm_magic

