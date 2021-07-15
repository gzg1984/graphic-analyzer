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

#include <sys/mman.h>

#include <stdlib.h>


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
__u32 test_create_framebuffer(int drm_fd,const struct drm_mode_create_dumb create_dumb)
{
	struct drm_mode_fb_cmd cmd_dumb={0};
	cmd_dumb.width=create_dumb.width;
	cmd_dumb.height=create_dumb.height;
	cmd_dumb.bpp=create_dumb.bpp;
	cmd_dumb.pitch=create_dumb.pitch;
	cmd_dumb.depth=24;
	cmd_dumb.handle=create_dumb.handle;
	int result = drmIoctl(drm_fd,DRM_IOCTL_MODE_ADDFB,&cmd_dumb);
	if ( result == 0 )
	{
		successtip();
		printf("DRM_IOCTL_MODE_ADDFB success\n");
	}
	else
	{
		failtip();
		printf("DRM_IOCTL_MODE_ADDFB failed : %d, %m \n",result);
	}
	return cmd_dumb.fb_id;
}
void test_map_dumb(int drm_fd,const struct drm_mode_create_dumb create_dumb)
{
	void *fb_base;
	long fb_w;
	long fb_h;
	struct drm_mode_map_dumb map_dumb={0};
	map_dumb.handle=create_dumb.handle;
	int result = drmIoctl(drm_fd,DRM_IOCTL_MODE_MAP_DUMB,&map_dumb);
	if ( result == 0 )
	{
		successtip();
		printf("DRM_IOCTL_MODE_MAP_DUMB success\n");
	}
	else
	{
		failtip();
		printf("DRM_IOCTL_MODE_MAP_DUMB failed : %d, %m \n",result);
		return;
	}


	printf("Trying put random color\n");

	fb_base = mmap(0, create_dumb.size, PROT_READ | PROT_WRITE, MAP_SHARED, drm_fd, map_dumb.offset);
	fb_w=create_dumb.width;
	fb_h=create_dumb.height;

	int col=(rand()%0x00ffffff)&0x00ff00ff;

	int x,y;
	for (y=0;y<fb_h;y++)
		for (x=0;x<fb_w;x++)
		{
			int location=y*(fb_w) + x;
			*(((uint32_t*)fb_base)+location)=col;
		}
	printf("End put random color\n");
}
__u32 test_create_dump_buf(int drm_fd , __u32 width, __u32 height)
{
	//------------------------------------------------------------------------------
	//Creating a dumb buffer
	//------------------------------------------------------------------------------
	struct drm_mode_create_dumb create_dumb={0};

	//If we create the buffer later, we can get the size of the screen first.
	//This must be a valid mode, so it's probably best to do this after we find
	//a valid crtc with modes.
	create_dumb.width = width;
	create_dumb.height = height;
	create_dumb.bpp = 32;
	create_dumb.flags = 0;
	create_dumb.pitch = 0;
	create_dumb.size = 0;
	create_dumb.handle = 0;
	printf("Creating Dump Buffer width %d, height %d\n",width,height);
	int result = drmIoctl(drm_fd, DRM_IOCTL_MODE_CREATE_DUMB, &create_dumb);
	if ( result == 0 )
	{
		successtip();
		printf("DRM_IOCTL_MODE_CREATE_DUMB success\n");
		test_map_dumb(drm_fd,create_dumb);
		return test_create_framebuffer(drm_fd,create_dumb);
	}
	else
	{
		failtip();
		printf("DRM_IOCTL_MODE_CREATE_DUMB failed : %d, %m \n",result);
	}
}

struct drm_mode_crtc test_crtc_feature(int drm_fd, __u32 crtc_id)
{
	struct drm_mode_crtc crtc={0};

	crtc.crtc_id=crtc_id;
	int result =	drmIoctl(drm_fd, DRM_IOCTL_MODE_GETCRTC, &crtc);
	if ( result == 0 )
	{
		successtip();
		printf("DRM_IOCTL_MODE_GETCRTC [%d] success\n", crtc_id);
	}
	else
	{
		failtip();
		printf("DRM_IOCTL_MODE_GETCRTC failed : %d, %m \n",result);
	}
	return crtc;
}


struct drm_mode_crtc test_encoder_feature(int drm_fd, __u32 encoder_id)
{
	struct drm_mode_get_encoder enc={0};
	enc.encoder_id=encoder_id;
	int result=	drmIoctl(drm_fd, DRM_IOCTL_MODE_GETENCODER, &enc);	//get encoder
	if ( result == 0 )
	{
		successtip();
		printf("DRM_IOCTL_MODE_GETENCODER [%d] success:[%d]\n",
				encoder_id,enc.crtc_id);
		return test_crtc_feature(drm_fd,enc.crtc_id);
	}
	else
	{
		failtip();
		printf("DRM_IOCTL_MODE_GETENCODER failed : %d, %m \n",result);
	}

}
void display_dumbbuf_to_crtc(int drm_fd,struct drm_mode_crtc crtc, __u32 fb_id ,uint64_t connector_id,struct drm_mode_modeinfo conn_mode)
{
	//------------------------------------------------------------------------------
	//Kernel Mode Setting (KMS)
	//------------------------------------------------------------------------------

	//		printf("%d : mode: %d, prop: %d, enc: %d\n",conn.connection,conn.count_modes,conn.count_props,conn.count_encoders);
	//		printf("modes: %dx%d FB: %p\n",conn_mode_buf[0].hdisplay,conn_mode_buf[0].vdisplay,fb_base[i]);

	//crtc.fb_id=cmd_dumb.fb_id;
	crtc.fb_id=fb_id;
	crtc.set_connectors_ptr=(uint64_t)&connector_id;
	crtc.count_connectors=1;
	crtc.mode=conn_mode;
	crtc.mode_valid=1;
	drmIoctl(drm_fd, DRM_IOCTL_MODE_SETCRTC, &crtc);
}
void test_getconnection_info(int drm_fd,uint32_t connector_id)
{
	struct drm_mode_modeinfo conn_mode_buf[20]={0};
	uint64_t	conn_prop_buf[20]={0},
			conn_propval_buf[20]={0},
			conn_enc_buf[20]={0};

	struct drm_mode_get_connector conn={0};

	conn.connector_id=connector_id;

	drmIoctl(drm_fd, DRM_IOCTL_MODE_GETCONNECTOR, &conn);	//get connector resource counts
	//Check if the connector is OK to use (connected to something)
	/*
	   if (conn.count_encoders<1 || conn.count_modes<1 || !conn.encoder_id || !conn.connection)
	   {
	   printf("[Connect ID 0x%x]Not connected, found conn.count_encoders %d,conn.count_modes %d, conn.encoder_id  flag is %d ,conn.connection flag is %d\n",
	   connector_id,conn.count_encoders,conn.count_modes,conn.encoder_id ,conn.connection );
	   return;
	   }
	   */
	if ( 1/* connector_status_connected*/ != conn.connection)
	{
		printf("[Connect ID 0x%x]Not connected conn.connection flag is %d\n",
				connector_id,conn.connection );
		return;
	}
	successtip();
	printf("[Connect ID 0x%x] is  connected, found conn.count_encoders %d,conn.count_modes %d, conn.encoder_id is %d ,conn.connection flag is %d\n",
			connector_id,conn.count_encoders,conn.count_modes,conn.encoder_id ,conn.connection );
	//printf("[Connect ID 0x%x]is connected,count_modes is %d\n",connector_id,conn.count_modes);

	conn.modes_ptr=(uint64_t)conn_mode_buf;
	conn.props_ptr=(uint64_t)conn_prop_buf;
	conn.prop_values_ptr=(uint64_t)conn_propval_buf;
	conn.encoders_ptr=(uint64_t)conn_enc_buf; //useless ? is it same as encoder_id ?
	drmIoctl(drm_fd, DRM_IOCTL_MODE_GETCONNECTOR, &conn);	//get connector resources

	struct drm_mode_crtc crtc = test_encoder_feature(drm_fd ,conn.encoder_id);

	printf("Creating Dump Buffer for Buf0\n");
	__u32 fb_id = test_create_dump_buf(drm_fd, conn_mode_buf[0].hdisplay, conn_mode_buf[0].vdisplay);

	printf("Displaying Frame Buffer [%d(0x%x)] in dumb-buff to Crtc [%d] via connector [%d(0x%x)]\n",
			fb_id,fb_id,crtc.crtc_id,connector_id,connector_id);
	display_dumbbuf_to_crtc(drm_fd,crtc,fb_id,connector_id,conn_mode_buf[0]);

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
		/*
		printf("Before Double Resource Get\n");
		int i;
		for (i=0;i<res.count_connectors;i++)
		{
			test_getconnection_info(drm_fd,res_conn_buf[i]);
		}

		printf("After Double Resource Get\n");
		*/
        	drmIoctl(drm_fd, DRM_IOCTL_MODE_GETRESOURCES, &res);
		int i = 0;
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

