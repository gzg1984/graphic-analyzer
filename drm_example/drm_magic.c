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
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KEND  "\e[0m"

void failtip()
{
	printf(KRED"[Failed]"KEND);
}
void successtip()
{
	printf(KGRN"[Success]"KEND);
}

void test_magic_auth_feature (int drm_fd)
{
	drm_magic_t magic;

	if (drmGetMagic(drm_fd , &magic)) 
	{
		failtip();
		printf("failed to get DRM magic,%m\n");
		return ;
	}
	successtip();
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
		successtip();
		printf("DRM_IOCTL_I915_GETPARAM I915_PARAM_CHIPSET_ID success, device id %d, 0x%x\n",device_id,device_id);
	}
	else
	{
		failtip();
		printf("DRM_IOCTL_I915_GETPARAM I915_PARAM_CHIPSET_ID failed, ret %d, %m\n",ret);
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
		successtip();
		printf("DRM_IOCTL_I915_GEM_CONTEXT_CREATE with original arg success,ctx_id %d, 0x%x\n",
				ctx_create.ctx_id,ctx_create.ctx_id);
	}
	else
	{
		failtip();
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
		successtip();
		printf("DRM_IOCTL_I915_GEM_CONTEXT_CREATE with ext arg success,ctx_id %d, 0x%x\n",
				ctx_create_ext.ctx_id,ctx_create_ext.ctx_id);
	}
	else
	{
		failtip();
		printf("DRM_IOCTL_I915_GEM_CONTEXT_CREATE with ext arg failed ret %d, %m\n",ret);
	}
}

void test_softpin(int drm_fd)
{
	/* On gfx8+, we require softpin */
	int has_softpin;
	struct drm_i915_getparam getparam;
	getparam.param = I915_PARAM_HAS_EXEC_SOFTPIN;
	getparam.value = &has_softpin;
	int ret = drmIoctl(drm_fd, DRM_IOCTL_I915_GETPARAM, (void *)&getparam);
	if(ret == 0)
	{
		successtip();
		printf("DRM_IOCTL_I915_GETPARAM I915_PARAM_HAS_EXEC_SOFTPIN %d, 0x%x\n",has_softpin,has_softpin);
	}
	else
	{
		failtip();
		printf("DRM_IOCTL_I915_GETPARAM I915_PARAM_HAS_EXEC_SOFTPIN ret %d, %m\n",ret);
	}
}

// 1 MB of batch should be enough for anyone, right?
#define BATCH_BO_SIZE (256 * 4096)
#define DATA_BO_SIZE 4096

void test_create_gem(int drm_fd)
{
	// Create the batch buffer
	struct drm_i915_gem_create gem_create;
	gem_create.size = BATCH_BO_SIZE;
	int ret = drmIoctl(drm_fd, DRM_IOCTL_I915_GEM_CREATE, 
			(void *)&gem_create);
	if(ret == 0)
	{
		successtip();
		printf("DRM_IOCTL_I915_GEM_CREATE %d\n",gem_create.handle);
	}
	else
	{
		failtip();
		printf("DRM_IOCTL_I915_GEM_CREATE ret %d, %m\n",ret);
	}
	/*
	batch_bo_handle = gem_create.handle;
#if GFX_VER >= 8
	batch_bo_addr = 0xffffffffdff70000ULL;
#endif
*/
}


void test_master_feature(int drm_fd)
{
       //Become the "master" of the DRI device
        int result = drmIoctl(drm_fd, DRM_IOCTL_SET_MASTER, 0);
        if ( result == 0 )
        {
		successtip();
                printf("DRM_IOCTL_SET_MASTER success\n");
        }
        else
        {
		failtip();
                printf("DRM_IOCTL_SET_MASTER failed : %d, %m \n",result);
        }

}


void test_version_feature(int drm_fd)
{
	char name[20] = "";
	drm_version_t version = {
		.name = name,
		.name_len = sizeof(name) - 1,
	};
	int result = drmIoctl(drm_fd, DRM_IOCTL_VERSION, &version);
	if ( result == 0 )
	{
		successtip();
		printf("DRM_IOCTL_VERSION success, name is %s\n",name);
	}
	else
	{
		failtip();
		printf("DRM_IOCTL_SET_MASTER failed : %d, %m \n",result);
	}
}
void test_getconnection_info(int drm_fd,uint32_t connector_id)
{

	/*
	void *fb_base[10];
	long fb_w[10];
	long fb_h[10];

	//Loop though all available connectors
	int i;
	for (i=0;i<res.count_connectors;i++)
	{
	*/
		struct drm_mode_modeinfo conn_mode_buf[20]={0};
		uint64_t	conn_prop_buf[20]={0},
					conn_propval_buf[20]={0},
					conn_enc_buf[20]={0};

		struct drm_mode_get_connector conn={0};

		conn.connector_id=connector_id;

		drmIoctl(drm_fd, DRM_IOCTL_MODE_GETCONNECTOR, &conn);	//get connector resource counts
		//Check if the connector is OK to use (connected to something)
		if (conn.count_encoders<1 || conn.count_modes<1 || !conn.encoder_id || !conn.connection)
		{
			printf("[Connect ID 0x%x]Not connected\n",connector_id);
			return;
		}
		successtip();
		printf("[Connect ID 0x%x]is connected,count_modes is %d\n",connector_id,conn.count_modes);

		conn.modes_ptr=(uint64_t)conn_mode_buf;
		conn.props_ptr=(uint64_t)conn_prop_buf;
		conn.prop_values_ptr=(uint64_t)conn_propval_buf;
		conn.encoders_ptr=(uint64_t)conn_enc_buf;
		drmIoctl(drm_fd, DRM_IOCTL_MODE_GETCONNECTOR, &conn);	//get connector resources



		/*
//------------------------------------------------------------------------------
//Creating a dumb buffer
//------------------------------------------------------------------------------
		struct drm_mode_create_dumb create_dumb={0};
		struct drm_mode_map_dumb map_dumb={0};
		struct drm_mode_fb_cmd cmd_dumb={0};

		//If we create the buffer later, we can get the size of the screen first.
		//This must be a valid mode, so it's probably best to do this after we find
		//a valid crtc with modes.
		create_dumb.width = conn_mode_buf[0].hdisplay;
		create_dumb.height = conn_mode_buf[0].vdisplay;
		create_dumb.bpp = 32;
		create_dumb.flags = 0;
		create_dumb.pitch = 0;
		create_dumb.size = 0;
		create_dumb.handle = 0;
		ioctl(dri_fd, DRM_IOCTL_MODE_CREATE_DUMB, &create_dumb);

		cmd_dumb.width=create_dumb.width;
		cmd_dumb.height=create_dumb.height;
		cmd_dumb.bpp=create_dumb.bpp;
		cmd_dumb.pitch=create_dumb.pitch;
		cmd_dumb.depth=24;
		cmd_dumb.handle=create_dumb.handle;
		ioctl(dri_fd,DRM_IOCTL_MODE_ADDFB,&cmd_dumb);

		map_dumb.handle=create_dumb.handle;
		ioctl(dri_fd,DRM_IOCTL_MODE_MAP_DUMB,&map_dumb);

		fb_base[i] = mmap(0, create_dumb.size, PROT_READ | PROT_WRITE, MAP_SHARED, dri_fd, map_dumb.offset);
		fb_w[i]=create_dumb.width;
		fb_h[i]=create_dumb.height;

//------------------------------------------------------------------------------
//Kernel Mode Setting (KMS)
//------------------------------------------------------------------------------

		printf("%d : mode: %d, prop: %d, enc: %d\n",conn.connection,conn.count_modes,conn.count_props,conn.count_encoders);
		printf("modes: %dx%d FB: %p\n",conn_mode_buf[0].hdisplay,conn_mode_buf[0].vdisplay,fb_base[i]);

		struct drm_mode_get_encoder enc={0};

		enc.encoder_id=conn.encoder_id;
		ioctl(dri_fd, DRM_IOCTL_MODE_GETENCODER, &enc);	//get encoder

		struct drm_mode_crtc crtc={0};

		crtc.crtc_id=enc.crtc_id;
		ioctl(dri_fd, DRM_IOCTL_MODE_GETCRTC, &crtc);

		crtc.fb_id=cmd_dumb.fb_id;
		crtc.set_connectors_ptr=(uint64_t)&res_conn_buf[i];
		crtc.count_connectors=1;
		crtc.mode=conn_mode_buf[0];
		crtc.mode_valid=1;
		ioctl(dri_fd, DRM_IOCTL_MODE_SETCRTC, &crtc);
	}
*/

}

void test_getresources(int drm_fd)
{
        uint32_t res_fb_buf[10]={0},
                        res_crtc_buf[10]={0},
                        res_conn_buf[10]={0},
                        res_enc_buf[10]={0};
        struct drm_mode_card_res res={0};
        res.fb_id_ptr=(uint64_t)res_fb_buf;
        res.crtc_id_ptr=(uint64_t)res_crtc_buf;
        res.connector_id_ptr=(uint64_t)res_conn_buf;
        res.encoder_id_ptr=(uint64_t)res_enc_buf;
        //Get resource IDs
        int ret = drmIoctl(drm_fd, DRM_IOCTL_MODE_GETRESOURCES, &res);
	if ( ret == 0 )
	{
		successtip();
		printf("DRM_IOCTL_MODE_GETRESOURCES fb: %d, crtc: %d, conn: %d, enc: %d\n",
				res.count_fbs,res.count_crtcs,res.count_connectors,res.count_encoders);
		printf("Before Double Resource Get\n");
		int i;
		for (i=0;i<res.count_connectors;i++)
		{
			test_getconnection_info(drm_fd,res_conn_buf[i]);
		}

		printf("After Double Resource Get\n");
        	drmIoctl(drm_fd, DRM_IOCTL_MODE_GETRESOURCES, &res);
		for (i=0;i<res.count_connectors;i++)
		{
			test_getconnection_info(drm_fd,res_conn_buf[i]);
		}
	}
	else
	{
		failtip();
		printf("DRM_IOCTL_MODE_GETRESOURCES failed : %d, %m \n",ret);
	}

}



int main(int argc,char** argv)
{
	int ret = 0;

	//printf("\e[1;34mThis is blue text\e[0m\n");

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
		test_softpin (drm_fd);
		test_create_gem(drm_fd);
		test_master_feature(drm_fd);
		test_version_feature(drm_fd);
		test_getresources(drm_fd);
		close(drm_fd);
	}

	return 0;
}

