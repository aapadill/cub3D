/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cub3D.h                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: djelacik <djelacik@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/30 16:01:21 by djelacik          #+#    #+#             */
/*   Updated: 2025/02/21 14:55:18 by aapadill         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CUB3D_H
# define CUB3D_H



# include <math.h>
# include <stdlib.h>
# include <stdio.h>
# include <limits.h>
# include <float.h> //erase
# include <assert.h> //erase
# include <fcntl.h> //file open

# include "libft.h"
# include "vec.h"
# include "gc_alloc.h"

#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>

# include <MLX42/MLX42.h>

#define GRAY_COLOR 0x444444FF
#define LIGHT_GRAY_COLOR 0xBBBBBBFF
#define RED_COLOR 0xFF0000FF
#define GREEN_COLOR 0x00FF00FF
#define BLUE_COLOR 0x0000FFFF
#define BLUE_COLOR_2 0x00AAFFFF
#define YELLOW_COLOR 0xFFFF00FF
#define BLACK_COLOR 0x000000FF
#define WHITE_COLOR 0xFFFFFFFF
#define SKY_COLOR 0x87CEEBFF
#define FLOOR_COLOR 0x8B4513FF

#define MIN_WIDTH 384
#define MIN_HEIGHT 216
#define MINIMAP_SCALE 0.15
#define TILE_SIZE 64
#define STEP_SIZE 0.5
#define FOV M_PI / 3

#define COLLISION_LIMIT 0.15
#define MOUSE_SENSITIVITY 0.001
#define DOOR_OPENING_SPEED 0.05
#define PLAYER_SPEED 0.00005
#define SEC_PER_FRAME 5
#define PIX_RECOIL 5
#define PIX_WALK 3
#define SHAKE_VEL_WALK 10
#define SHAKE_VEL_RECO 50

#define VEC_INIT_SIZE 4

#define ENEMY_SPEED  0.005
# define MAX_NAME_LEN 20

// #define IMAGE_BASE_64 "iVBORw0KGgoAAAANSUhEUgAAAMAAAADACAMAAABlApw1AAAAIGNIUk0AAHomAACAhAAA+gAAAIDoAAB1MAAA6mAAADqYAAAXcJy6UTwAAAByUExURQAAAJgAAPwAAOwAAPycnPxUVPxcXPz8VKioqHx8fIyMjNDQ0LS0tEhISCwsLJiYmGRkZFRUVHBwcAAAACAgIKhoQPCUXLRwRPywgFQ8HPykcPzEpPzYxMh8THRMKIBQLOiMWNyIVPy4kPycYKBkPP///6i6Sn8AAAABdFJOUwBA5thmAAAAAWJLR0QlwwHJDwAAAAd0SU1FB+kDEQgvDTAkv6gAAAI4SURBVHja7dpbb6pQFEXhejsqgorXem29/P/f2BlW2Nk1lmL6sCBnfA+E0D7MkYDHU317AwAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAADgf9LpdLwnEOA94W/ru91uixsIcKLRdtLr9fr9vo4P15uOgAYYDAb/CgqwE13xHkWA96hX2N0vw+FQR60PT0I7EOBK//RqsaaPRqPxeKyjztvUQID3env11O4kSSaTiY46t9fTFryxI6ABtFK3u+7+NE0VoKPOdaUd6wlojCzLQoDOvecQ4D3nReOCdoejeI8iwHtUbdPpVHNns9m0NJ/PdSXPc+9pBHhPeyVAb+AWi4VOdLSA1ryYEuAtBMxKFqDr3tMI8J5WO0C3u3aPSllhuVx6TyPAe1rtgNVqZa+bYbqu6Lher73XEeC97jebzcYC9NIZ3szFAdvt1nsjAd4bK9e/F54G7HY7Agiott/vD4fD8Xg8nU7LyKpwPp+3Je+lBHgv/TlAD8CxoIaPkhZ/Fi6Xi64TQEDF+hBg9Ehcr1dd1E9DAwEEVARoYtwQB9xut/AYeI8lwHvszwF2r79/Z+sJIKBa+M+AbY3XE0BAHeGP6fb3UK3cfBd+oaEfGxPgvd526yRJEvtgb1243+86j3/BGnTRezUB3qufrbdxFpDneZZl9gmfffky/uSvQQ0ENGx9HCAhwB4PEz4/9v8KCwHeAbb44e43cYAdn76YOn+LhQDXgDRN6zwD5uHdXhzg1kDAXwO+AI+nxgq+cWFMAAAAJXRFWHRkYXRlOmNyZWF0ZQAyMDI1LTAzLTE3VDA4OjQ3OjA4KzAwOjAwpfXRXAAAACV0RVh0ZGF0ZTptb2RpZnkAMjAyNS0wMy0xN1QwODo0NzowOCswMDowMNSoaeAAAAAodEVYdGRhdGU6dGltZXN0YW1wADIwMjUtMDMtMTdUMDg6NDc6MTMrMDA6MDBNEBxbAAAAJ3RFWHR3ZWJwOm11eC1ibGVuZABBdG9wQmFja2dyb3VuZEFscGhhQmxlbmSzunTVAAAAAElFTkSuQmCC"

//# define DEBUG
#ifdef DEBUG 
# define DBG_PRINT(...) fprintf(stderr, __VA_ARGS__)
#else
# define DBG_PRINT(...) ((void)0)
#endif

typedef enum e_door_state{
	CLOSED,
	OPENING,
	OPEN,
	CLOSING
} t_door_state;

typedef struct s_player {
	double		x;
	double		y;
	double		angle;
	double		speed;
}	t_player;

typedef struct s_view {
	int32_t		x;
    int32_t		y;
	int8_t		toggle;
	double		shake_offset;
	double		shake_time;
}	t_view;

// typedef struct s_sprite {
// 	double		x;
// 	double		y;
// 	double		distance;
// 	double		angle;
// 	mlx_texture_t	*texture;
// }	t_sprite;

typedef struct s_textures {
	mlx_texture_t *north;
	mlx_texture_t *south;
	mlx_texture_t *west;
	mlx_texture_t *east;
	mlx_texture_t *door;
	//mlx_texture_t *barrel;
	//mlx_texture_t **sprite; //array of sprites
}	t_textures;

typedef struct s_door {
	int				x;
	int				y;
	t_door_state	state;
	double			progress;
}	t_door;

typedef struct s_sprite {
	double	x;
	double	y;
	int		texture;
}	t_sprite;

typedef struct s_map {
	char	**grid;
	int		width;
	int		height;
}	t_map;

typedef struct s_ray {
	double			distance;
	double			hit_x;
	double			hit_y;
	int				side;
	double			wall_x;
	mlx_texture_t	*texture;
	double			dir_x;
	double			dir_y;
	bool			is_door;
	double			door_progress;
}	t_ray;

//rich
typedef enum e_game_state {
	STATE_MENU,
	STATE_PLAYING
}	t_game_state;

typedef struct s_data {
	bool			resize_pending;
    int				new_width;
    int				new_height;

	int				width; //window width
	int				height; //window height
	t_map			map;

	bool 			f_color_found;
	bool			c_color_found;

	mlx_t			*mlx;
	mlx_image_t		*image;
	t_ray			*ray;
	t_player		player;
	t_view			camera;
	t_textures		*textures;
	uint32_t		floor;
	uint32_t		ceiling;

	t_door			*doors;
	int				door_count;

	t_sprite		*sprites;
	int				num_sprites;
	double			*zBuffer;
	mlx_texture_t	**sprite_textures;

	mlx_texture_t **enemy;

	mlx_texture_t **hud_hands;
	mlx_texture_t **ai_hands;
	int hud_frame;
	int hud_frame_count;
	int hud_frame_timer;

	bool			is_player_moving;
	bool			is_player_shooting;

	bool			strict;
	char			*error_msg;
	bool 			calling_new_gun;
	bool			is_gun_ready;
	
    double			window_time;
	double			time_one;
	double			time_two;
	char 			*gun_description;//for api

	char player_name[20 + 1];
	int name_length;
	mlx_image_t *menu_text;
	mlx_image_t *input_text;
	int game_state;

	mlx_image_t *bg_img;
	int frame;
}	t_data;

/* 

					CLEAN PROTOTYPES, SOME OF THIS DONT EVEN EXIST ANYMORE 

*/

//color_utils.c
uint8_t		get_r(uint32_t rgba);
uint8_t		get_g(uint32_t rgba);
uint8_t		get_b(uint32_t rgba);
uint8_t		get_a(uint32_t rgba);
uint32_t	get_rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a);

//shading.c
uint32_t	simple_shading(uint32_t color, double distance);

//texture.c
uint32_t		get_texture_color(mlx_texture_t *texture, int x, int y);
mlx_texture_t 	*get_wall_texture(t_data *data, t_ray *ray);
void			draw_wall_texture(t_data *data, t_ray *ray, int screen_x, int vis_start_y, int vis_end_y, int orig_start_y, int wall_height);

//ray.c
double	calculate_distance(t_data *data, double angle, t_ray *ray);
void	calculate_ray_data(t_data *data, double angle, t_ray *ray);

//rays.c
double	calculate_corrected_distance(double distance, double ray_angle, double player_angle);
void	draw_wall_column(t_data *data, double angle, int screen_x);
//void	draw_walls(t_data *data);
void	draw_walls_and_sprites(t_data *data);

//movement.c
void	loop_hook(void *param);

//minimap.c
void	draw_mini_map(t_data *data);
void	draw_mini_player(t_data *data);
void	draw_mini_rays(t_data *data);
void	draw_square(mlx_image_t *image, int x, int y, int size, int color);

//utils.c
double	lookup_door_progress(t_data *data, int x, int y);
void	update_doors(t_data *data);
bool	is_wall(t_data *data, double x, double y); //update
void	draw_floor_and_ceiling(t_data *data);

//parsing.c
int		parse_cubfile(char *filepath, t_data *data);

//free.c
void	free_and_exit(void);
void	free_and_exit_with(int exit_code);
void	error_exit(char *msg);

//sprites.c
//void	draw_sprites(t_data *data);
void 	draw_sprite_column(t_data *data, int x, double dir_x, double dir_y, double plane_x, double plane_y);
void	draw_hud_hands(t_data *data);
void	shooting_animation(t_data *data);
void call_chatgpt(char *prompt, t_data *data);
// void call_dalle(char *image_prompt, t_data *data);
//void	call_dalle_with_base64(const char *prompt, const char *base64_image);
// void	call_dalle_with_reference(const char *prompt_text, const char *image_path);
int generate_with_gpt_image(const char *prompt, const char *save_path);
void entry_screen(t_data *data);
void wrapper(void *param);
void *ai_worker(void *arg);

#endif