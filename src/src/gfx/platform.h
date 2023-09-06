#ifndef platform_include_file
#define platform_include_file

#ifdef __cplusplus
extern "C" {
#endif

#define platform_width 16
#define platform_height 16
#define platform_size 258
#define platform ((gfx_sprite_t*)platform_data)
extern unsigned char platform_data[258];

#ifdef __cplusplus
}
#endif

#endif
