#include <libdrm/drm.h>
#include <xf86drm.h>
#include <xf86drmMode.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <libdrm/i915_drm.h>



#include <stdio.h>


#include <unistd.h>

#include <string.h>
#include <errno.h>

const char pathlist[4][32]={
	"/dev/dri/card0",
	"/dev/dri/card1",
	"/dev/dri/renderD128",
	"/dev/dri/renderD129"
};

void test_magic_auth_feature (int drm_fd)
{
	drm_magic_t magic;

	if (drmGetMagic(drm_fd , &magic)) 
	{
		printf("failed to get DRM magic,%m\n");
		return ;
	}
	printf("Got magic: 0x%X\n",magic);
	printf("TODO: send magic to DRI2 connection to auth...\n");
}

void test_getparam_feature (int drm_fd)
{
	int device_id;
	struct drm_i915_getparam getparam ;
	getparam.param = I915_PARAM_CHIPSET_ID;
	getparam.value = &device_id;
	int ret = drmIoctl(drm_fd, DRM_IOCTL_I915_GETPARAM, (void *)&getparam);
	if(ret == 0)
	{
		printf("DRM_IOCTL_I915_GETPARAM success, device id %d, 0x%x\n",device_id,device_id);
	}
	else
	{
		printf("DRM_IOCTL_I915_GETPARAM failed, ret %d, %m\n",ret);
	}
}

void test_context_create(int drm_fd)
{
	/* test original create */
	struct   drm_i915_gem_context_create ctx_create ;
	ctx_create.ctx_id=0;
	ctx_create.pad=0;
	int ret = drmIoctl(drm_fd, DRM_IOCTL_I915_GEM_CONTEXT_CREATE, (void *)&ctx_create);
	if(ret == 0)
	{
		printf("DRM_IOCTL_I915_GEM_CONTEXT_CREATE with original arg success,ctx_id %d, 0x%x\n",ctx_create.ctx_id,ctx_create.ctx_id);
	}
	else
	{
		printf("DRM_IOCTL_I915_GEM_CONTEXT_CREATE with original arg failed ret %d, %m\n",ret);
	}
	/* test ext create */
	struct   drm_i915_gem_context_create_ext ctx_create_ext ;
	ctx_create_ext.ctx_id=0;
	ctx_create_ext.flags=0;
	ctx_create_ext.extensions=0;
	ret = drmIoctl(drm_fd, DRM_IOCTL_I915_GEM_CONTEXT_CREATE, (void *)&ctx_create_ext);
	if(ret == 0)
	{
		printf("DRM_IOCTL_I915_GEM_CONTEXT_CREATE with ext arg success,ctx_id %d, 0x%x\n",ctx_create.ctx_id,ctx_create.ctx_id);
	}
	else
	{
		printf("DRM_IOCTL_I915_GEM_CONTEXT_CREATE with ext arg failed ret %d, %m\n",ret);
	}
}

int main(int argc,char** argv)
{
	int ret = 0;

	for (int i = 0; i<4 ; i++)
	{
		const char* path=pathlist[i];
		int const drm_fd = open(path, O_RDWR | O_CLOEXEC);

		if (drm_fd < 0) {
			printf("\n===== Could not open %s: %m ===== \n",path);
			continue;
		}
		printf("\n===== open %s success =====\n",path);

		test_magic_auth_feature (drm_fd);
		test_getparam_feature (drm_fd);
		test_context_create (drm_fd);

		close(drm_fd);
	}

	return 0;










}

