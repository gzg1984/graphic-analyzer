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

unsigned int global_ctx_id;
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
		global_ctx_id=ctx_create.ctx_id;
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

static inline uintptr_t
ALIGN(uintptr_t value, int32_t alignment)
{
   //assert(util_is_power_of_two_nonzero(alignment));
   return (((value) + (alignment) - 1) & ~((alignment) - 1));
}


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
		printf("DRM_IOCTL_I915_GEM_CREATE success , handle %d\n",gem_create.handle);
	}
	else
	{
		failtip();
		printf("DRM_IOCTL_I915_GEM_CREATE ret %d, %m\n",ret);
	}

	uint32_t batch_bo_handle = gem_create.handle;
	/*
#if GFX_VER >= 8
#endif
*/
uint64_t	batch_bo_addr = 0xffffffffdff70000ULL;

	struct drm_i915_gem_caching gem_caching ;
	gem_caching.handle = batch_bo_handle;
	gem_caching.caching = I915_CACHING_CACHED;
	ret = drmIoctl(drm_fd, DRM_IOCTL_I915_GEM_SET_CACHING, (void *)&gem_caching);
	if(ret == 0)
	{
		successtip();
		printf("DRM_IOCTL_I915_GEM_SET_CACHING success \n");
	}
	else
	{
		failtip();
		printf("DRM_IOCTL_I915_GEM_SET_CACHING ret %d, %m\n",ret);
	}


	struct drm_i915_gem_mmap gem_mmap ;
	gem_mmap.handle = batch_bo_handle;
	gem_mmap.offset = 0;
	gem_mmap.size = BATCH_BO_SIZE;
	gem_mmap.flags = 0;
	ret = drmIoctl(drm_fd, DRM_IOCTL_I915_GEM_MMAP, (void *)&gem_mmap);
	//batch_map = (void *)(uintptr_t)gem_mmap.addr_ptr;
	if(ret == 0)
	{
		successtip();
		printf("DRM_IOCTL_I915_GEM_MMAP success ,map addr is %p\n",(void *)gem_mmap.addr_ptr);
	}
	else
	{
		failtip();
		printf("DRM_IOCTL_I915_GEM_MMAP ret %d, %m\n",ret);
	}


	 /* The requirement for using I915_EXEC_NO_RELOC are:
       *
       *   The addresses written in the objects must match the corresponding
       *   reloc.gtt_offset which in turn must match the corresponding
       *   execobject.offset.
       *
       *   Any render targets written to in the batch must be flagged with
       *   EXEC_OBJECT_WRITE.
       *
       *   To avoid stalling, execobject.offset should match the current
       *   address of that object within the active context.
       */
	/*
      int flags = I915_EXEC_NO_RELOC | I915_EXEC_RENDER;

      if (batch->needs_sol_reset)
         flags |= I915_EXEC_GEN7_SOL_RESET;

      / * Set statebuffer relocations * /
      const unsigned state_index = batch->state.bo->index;
      if (state_index < batch->exec_count &&
          batch->exec_bos[state_index] == batch->state.bo) {
         struct drm_i915_gem_exec_object2 *entry =
            &batch->validation_list[state_index];
         assert(entry->handle == batch->state.bo->gem_handle);
         entry->relocation_count = batch->state_relocs.reloc_count;
         entry->relocs_ptr = (uintptr_t) batch->state_relocs.relocs;
      }

      / * Set batchbuffer relocations * /
      struct drm_i915_gem_exec_object2 *entry = &batch->validation_list[0];
      assert(entry->handle == batch->batch.bo->gem_handle);
      entry->relocation_count = batch->batch_relocs.reloc_count;
      entry->relocs_ptr = (uintptr_t) batch->batch_relocs.relocs;

      if (batch->use_batch_first) {
         flags |= I915_EXEC_BATCH_FIRST | I915_EXEC_HANDLE_LUT;
      } else {
         / * Move the batch to the end of the validation list * /
         struct drm_i915_gem_exec_object2 tmp;
         struct brw_bo *tmp_bo;
         const unsigned index = batch->exec_count - 1;

         tmp = *entry;
         *entry = batch->validation_list[index];
         batch->validation_list[index] = tmp;

         tmp_bo = batch->exec_bos[0];
         batch->exec_bos[0] = batch->exec_bos[index];
         batch->exec_bos[index] = tmp_bo;
      }

      ret = execbuffer(brw->screen->fd, batch, brw->hw_ctx,
                       4 * USED_BATCH(*batch),
                       in_fence_fd, out_fence_fd, flags);

      throttle(brw);
      */
      struct drm_i915_gem_exec_object2 entry;
      memset(&entry, 0, sizeof(entry));
      entry.handle = batch_bo_handle;
      entry.relocation_count=0;
      entry.relocs_ptr=0;
 //     entry.flags = EXEC_OBJECT_SUPPORTS_48B_ADDRESS |
                      EXEC_OBJECT_PINNED |
                      EXEC_OBJECT_WRITE;
      entry.flags = EXEC_OBJECT_SUPPORTS_48B_ADDRESS |
                      EXEC_OBJECT_PINNED ;
      //entry.offset = data_bo_addr;
//      entry.offset = batch_bo_addr;
//         batch_bo_addr = 0xffffffffdff70000ULL;
//   data_bo_addr = 0xffffffffefff0000ULL;

entry.offset = 0xffffffffefff0000ULL;

      /*
       *    / * Set batchbuffer relocations * /
   struct drm_i915_gem_exec_object2 *entry = &batch->validation_list[0];
   assert(entry->handle == batch->command.bo->gem_handle);
   entry->relocation_count = batch->command.relocs.reloc_count;
   entry->relocs_ptr = (uintptr_t)batch->command.relocs.relocs;
   */

   struct drm_i915_gem_execbuffer2 execbuf = {
      //.buffers_ptr = (uintptr_t)batch->validation_list,
      .buffers_ptr = (uintptr_t)&entry,
      //.buffer_count = batch->exec_count,
      .buffer_count = 1,
      .batch_start_offset = 0,
      /* This must be QWord aligned. */
      .batch_len = ALIGN(sizeof(entry), 8),
      .flags = I915_EXEC_RENDER |
               I915_EXEC_NO_RELOC |
               I915_EXEC_BATCH_FIRST |
               I915_EXEC_HANDLE_LUT,
      //.rsvd1 = batch->hw_ctx_id, /* rsvd1 is actually the context ID */
      .rsvd1 = global_ctx_id, /* rsvd1 is actually the context ID */
   };

   /*
   if (num_fences(batch)) {
      execbuf.flags |= I915_EXEC_FENCE_ARRAY;
      execbuf.num_cliprects = num_fences(batch);
      execbuf.cliprects_ptr =
         (uintptr_t)util_dynarray_begin(&batch->exec_fences);
   }
   */

//   int ret = 0;
   /*
      if (!batch->screen->no_hw &&
      intel_ioctl(batch->screen->fd, DRM_IOCTL_I915_GEM_EXECBUFFER2, &execbuf))
      ret = -errno;
      */
   ret = drmIoctl(drm_fd, DRM_IOCTL_I915_GEM_EXECBUFFER2, &execbuf);
   if(ret == 0)
   {
	   successtip();
	   printf("DRM_IOCTL_I915_GEM_EXECBUFFER2 success \n");

	   struct   drm_i915_gem_wait gem_wait ;
	   gem_wait.bo_handle = batch_bo_handle;
	   gem_wait.timeout_ns = INT64_MAX;
	   ret = drmIoctl(drm_fd, DRM_IOCTL_I915_GEM_WAIT, (void *)&gem_wait);
	   if(ret == 0)
	   {
		   successtip();
		   printf("DRM_IOCTL_I915_GEM_WAIT success \n");
	   }
	   else
	   {
		   failtip();
		   printf("DRM_IOCTL_I915_GEM_WAIT ret %d, %m\n",ret);
	   }


	   int i = 0;
	   for(i=0 ;i <100;i++)
	   {
		   memset((void*)gem_mmap.addr_ptr,1+100*i,BATCH_BO_SIZE);
		   usleep(10000);
	   }
   }
   else
   {
	   failtip();
	   printf("DRM_IOCTL_I915_GEM_EXECBUFFER2 ret %d, %m\n",ret);
   }



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
__u32 test_create_dump_buf(int drm_fd , __u32 width, __u32 height,struct drm_mode_create_dumb* pcreate_dumb)
{
	//------------------------------------------------------------------------------
	//Creating a dumb buffer
	//------------------------------------------------------------------------------

	//If we create the buffer later, we can get the size of the screen first.
	//This must be a valid mode, so it's probably best to do this after we find
	//a valid crtc with modes.
	pcreate_dumb->width = width;
	pcreate_dumb->height = height;
	pcreate_dumb->bpp = 32;
	pcreate_dumb->flags = 0;
	pcreate_dumb->pitch = 0;
	pcreate_dumb->size = 0;
	pcreate_dumb->handle = 0;
	printf("Creating Dump Buffer width %d, height %d\n",width,height);
	int result = drmIoctl(drm_fd, DRM_IOCTL_MODE_CREATE_DUMB, pcreate_dumb);
	if ( result == 0 )
	{
		successtip();
		printf("DRM_IOCTL_MODE_CREATE_DUMB success, handle is %d\n",pcreate_dumb->handle);
		test_map_dumb(drm_fd,*pcreate_dumb);
		return test_create_framebuffer(drm_fd,*pcreate_dumb);
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
	int ret = drmIoctl(drm_fd, DRM_IOCTL_MODE_SETCRTC, &crtc);
	if ( ret == 0 )
	{
		successtip();
		printf("DRM_IOCTL_MODE_SETCRTC success\n");
	}
	else
	{
		failtip();
		printf("DRM_IOCTL_MODE_SETCRTC failed : %d, %m \n",ret);
	}
}
void test_one_encoder_and_crtc(int drm_fd, __u32 encoder_id,__u32 fb_id,uint32_t connector_id,struct drm_mode_modeinfo conn_mode)
{
	struct drm_mode_get_encoder enc={0};
	enc.encoder_id=encoder_id;
	int result = drmIoctl(drm_fd, DRM_IOCTL_MODE_GETENCODER, &enc);	//get encoder
	if ( result == 0 )
	{
		successtip();
		printf("DRM_IOCTL_MODE_GETENCODER [%d(0x%x)] success: crtc ID [%d]\n",
				encoder_id,encoder_id,enc.crtc_id);
		if ( enc.crtc_id > 0 )
		{
			struct drm_mode_crtc crtc = test_crtc_feature(drm_fd,enc.crtc_id);
			printf("Displaying Frame Buffer [%d(0x%x)] in dumb-buff to Crtc [%d] via connector [%d(0x%x)]\n",
					fb_id,fb_id,crtc.crtc_id,connector_id,connector_id);
			display_dumbbuf_to_crtc(drm_fd,crtc,fb_id,connector_id,conn_mode);
		}
	}
	else
	{
		failtip();
		printf("DRM_IOCTL_MODE_GETENCODER %d failed : %d, %m \n",encoder_id,result);
	}
}
void test_encoder_feature(int drm_fd, __u32 encoder_id, __u32 count_encoders,uint32_t* conn_encoders,__u32 fb_id,uint32_t connector_id,struct drm_mode_modeinfo conn_mode)
{

	int i = 0;
	for(i = 0; i < count_encoders ; i++)
	{
		printf("Found Encoder [%d(0x%x)] \n", 
				conn_encoders[i],conn_encoders[i]);
		test_one_encoder_and_crtc(drm_fd,conn_encoders[i],fb_id,connector_id,conn_mode);
	}

	test_one_encoder_and_crtc(drm_fd,encoder_id,fb_id,connector_id,conn_mode);

	/*
	struct drm_mode_get_encoder enc={0};
	enc.encoder_id=encoder_id;
	int result = drmIoctl(drm_fd, DRM_IOCTL_MODE_GETENCODER, &enc);	//get encoder
	if ( result == 0 )
	{
		successtip();
		printf("DRM_IOCTL_MODE_GETENCODER [%d] success:[%d]\n",
				encoder_id,enc.crtc_id);
		struct drm_mode_crtc crtc = test_crtc_feature(drm_fd,enc.crtc_id);
		if ( crtc.crtc_id != -1 )
		{
			printf("Displaying Frame Buffer [%d(0x%x)] in dumb-buff to Crtc [%d] via connector [%d(0x%x)]\n",
					fb_id,fb_id,crtc.crtc_id,connector_id,connector_id);
			display_dumbbuf_to_crtc(drm_fd,crtc,fb_id,connector_id,conn_mode);
		}
	}
	else
	{
		failtip();
		printf("DRM_IOCTL_MODE_GETENCODER %d failed : %d, %m \n",encoder_id,result);
	}
	*/

}


void test_fill_colors( uint8_t * primed_framebuffer,uint_fast64_t const size_in_pixels ,uint32_t const width_pixel,
		uint32_t const diff_between_width_and_stride)
{
        /* The colors table */
        uint32_t const red   = (0xff<<16);
        uint32_t const green = (0xff<<8);
        uint32_t const blue  = (0xff);
        uint32_t const colors[] = {red, green, blue};
        uint_fast64_t pixel = 0;
	/* While we didn't get a 'q' + Enter or reached the bottom of the
         * screen... */
        while ( pixel < size_in_pixels) {
                /* Choose a random color. 3 being the size of the colors table. */
                uint32_t current_color = colors[rand()%3];

                /* Color every pixel of the row.
                 * Now, the framebuffer is linear. Meaning that the first pixel of
                 * the first row should be at index 0, but the first pixel of the
                 * second row should be at index (stride+0) and the first pixel of
                 * the n-th row should be at (n*stride+0).
                 *
                 * Instead of computing the value, we'll just increment the "pixel"
                 * index and accumulate the padding once done with the current row,
                 * in order to be ready to start for the next row.
                 */
                for (uint_fast32_t p = 0; p < width_pixel; p++)
                        ((uint32_t *) primed_framebuffer)[pixel++] = current_color;
                pixel += diff_between_width_and_stride;
                //LOG("pixel : %lu, size : %lu\n", pixel, size_in_pixels);
		usleep(10000);
        }

}


void test_map_prime_fd(int drm_fd,int dma_buf_fd,__u64 dumb_size,__u32 dumb_pitch ,__u32 dumb_width,__u32 dumb_height )
{
	 /* Map the exported buffer, using the PRIME File descriptor */
        /* That ONLY works if the DRM driver implements gem_prime_mmap.
         * This function is not implemented in most of the DRM drivers for
         * GPU with discrete memory. Meaning that it will surely fail with
         * Radeon, AMDGPU and Nouveau drivers for desktop cards ! */
	/*
        uint8_t * primed_framebuffer = mmap(
                0, create_request.size, PROT_READ | PROT_WRITE, MAP_SHARED,
                dma_buf_fd, 0);
		*/
        uint8_t * primed_framebuffer = mmap(
                0, dumb_size, PROT_READ | PROT_WRITE, MAP_SHARED,
                dma_buf_fd, 0);
        int ret = errno;

        /* Bail out if we could not map the framebuffer using this method */
        if (primed_framebuffer == NULL || primed_framebuffer == MAP_FAILED) {
                printf( "Could not map buffer exported through PRIME : %s (%d)\n"
                        "Buffer : %p\n",
                        strerror(ret), ret,
                        primed_framebuffer);
                return;
        }

        printf("Buffer mapped !\n");

        /* The fun begins ! At last !
         * We'll do something simple :
         * We'll lit a row of pixel, on the screen, starting from the top,
         * down to the bottom of screen, using either Red, Blue or Green
         * randomly, each time we press Enter.
         * If we press 'q' and then Enter, the process will stop.
         * The process will also stop once we've reached the bottom of the
         * screen.
         */
        uint32_t const bytes_per_pixel = 4;
        uint_fast64_t size = dumb_size;

        /* Cleanup the framebuffer */
        memset(primed_framebuffer, 0, size);


        /* Pitch is the stride in bytes.
         * However, for our purpose we'd like to know the stride in pixels.
         * So we'll divide the pitch (in bytes) by the number of bytes
         * composing a pixel to get that information.
         */
        uint32_t const stride_pixel = dumb_pitch / bytes_per_pixel;
        uint32_t const width_pixel  = dumb_width;

	/* The width is padded so that each row starts with a specific
         * alignment. That means that we have useless pixels that we could
         * avoid dealing with in the first place.
         * Now, it might be faster to just lit these useless pixels and get
         * done with it. */
        uint32_t const diff_between_width_and_stride =
                stride_pixel - width_pixel;
        uint_fast64_t const size_in_pixels =
                dumb_height * stride_pixel;



	test_fill_colors(primed_framebuffer,size_in_pixels,width_pixel,diff_between_width_and_stride );

        munmap(primed_framebuffer, dumb_size);

}
void test_prime_feaute(int drm_fd,__u32 handle,__u64 size ,__u32 pitch,__u32 width,__u32 height)
{
	/* For this test only : Export our dumb buffer using PRIME */
	/* This will provide us a PRIME File Descriptor that we'll use to
	 * map the represented buffer. This could be also be used to reimport
	 * the GEM buffer into another GPU */
	struct drm_prime_handle prime_request = {
		//.handle = create_request.handle,
		.handle = handle,
		.flags  = DRM_CLOEXEC | DRM_RDWR,
		.fd     = -1
	};

	int ret = drmIoctl(drm_fd, DRM_IOCTL_PRIME_HANDLE_TO_FD, &prime_request);
	int const dma_buf_fd = prime_request.fd;
	if ( ret == 0 )
	{
		successtip();
		printf("DRM_IOCTL_PRIME_HANDLE_TO_FD [%d] success:[%d]\n",
				handle,prime_request.fd);
		test_map_prime_fd(drm_fd,dma_buf_fd,size, pitch,width,height);
	}
	else
	{
		/* If we could not export the buffer, bail out since that's the
		 * purpose of our test */
		failtip();
		printf("DRM_IOCTL_PRIME_HANDLE_TO_FD %d failed : %d, %m \n",handle,ret);
		printf("Could not export buffer : %s (%d) - FD : %d\n",
				strerror(ret), ret,
				dma_buf_fd);
	}


}
void test_getconnection_info(int drm_fd,uint32_t connector_id)
{
	struct drm_mode_modeinfo conn_mode_buf[20]={0};
	uint32_t	conn_prop_buf[20]={0},
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
	int ret = drmIoctl(drm_fd, DRM_IOCTL_MODE_GETCONNECTOR, &conn);	//get connector resources

	if ( ret == 0 )
	{
		successtip();
		printf("DRM_IOCTL_MODE_GETCONNECTOR [%d] success\n", connector_id);

		printf("Creating Dump Buffer for Buf0\n");
		struct drm_mode_create_dumb create_dumb={0};
		__u32 fb_id = test_create_dump_buf(drm_fd, conn_mode_buf[0].hdisplay, conn_mode_buf[0].vdisplay,&create_dumb);

		test_encoder_feature(drm_fd ,conn.encoder_id, conn.count_encoders, conn_enc_buf,fb_id,connector_id,conn_mode_buf[0]);
		test_prime_feaute(drm_fd,create_dumb.handle,create_dumb.size,create_dumb.pitch,create_dumb.width,create_dumb.height);

	}
	else
	{
		failtip();
		printf("DRM_IOCTL_MODE_GETCONNECTOR [%d] failed : %d, %m \n",connector_id,ret);
	}
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

void test_drm_gem(int drm_fd)
{
	struct drm_gem_flink gf;
	memset(&gf,0,sizeof(gf));
	gf.handle=1;
	int ret = drmIoctl(drm_fd,DRM_IOCTL_GEM_FLINK,&gf);
	if ( ret == 0)
	{
		successtip();
		printf("DRM_IOCTL_GEM_FLINK success, got name %d\n",gf.name);
		struct drm_gem_open go;
		memset(&go,0,sizeof(go));
		go.name=gf.name;
		ret = drmIoctl(drm_fd,DRM_IOCTL_GEM_OPEN,&open);
		if ( ret == 0)
		{
			successtip();
			printf("DRM_IOCTL_GEM_OPEN success\n");
		}
		else
		{
			failtip();
			printf("DRM_IOCTL_GEM_OPEN %d failed : %d, %m\n",go.name,ret);

		}
	}
	else
	{
		failtip();
		printf("DRM_IOCTL_GEM_FLINK %d failed : %d, %m\n",gf.handle,ret);

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
		test_drm_gem(drm_fd);
		close(drm_fd);
	}

	return 0;
}

