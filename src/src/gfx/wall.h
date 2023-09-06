#ifndef wall_include_file
#define wall_include_file

#ifdef __cplusplus
extern "C" {
#endif

#define wall_width 16
#define wall_height 16
#define wall_size 258
#define wall ((gfx_sprite_t*)wall_data)
extern unsigned char wall_data[258];

#ifdef __cplusplus
}
#endif

#endif
