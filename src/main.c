/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: djelacik <djelacik@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/30 16:02:56 by djelacik          #+#    #+#             */
/*   Updated: 2025/01/04 18:56:34 by djelacik         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "cub3D.h"

void	free_textures(t_textures *textures)
{
	if (textures->north)
		mlx_delete_texture(textures->north);
	if (textures->south)
		mlx_delete_texture(textures->south);
	if (textures->west)
		mlx_delete_texture(textures->west);
	if (textures->east)
		mlx_delete_texture(textures->east);
	if (textures->door)
		mlx_delete_texture(textures->door);
}

//try many doors
//use argv[1] as cubfile
bool	initializer(t_data *data, char *filename, bool strict)
{
	int	monitor_width;
	int	monitor_height;
	int	status;

	status = 0;
	ft_memset(data, 0, sizeof(t_data));
	data->strict = strict;
	data->player.speed = 0.025;
	data->mlx = mlx_init(MIN_WIDTH, MIN_HEIGHT, "Cub3D Ray-Casting", true);
	data->window_time = mlx_get_time();
	if (!data->mlx)
	{
		data->error_msg = "Failed to initialize MLX";
		return (EXIT_FAILURE);
	}
	data->textures = gc_alloc(sizeof(t_textures));
	if (!data->textures)
	{
		//printf("Error: Failed to allocate memory for textures\n");
		mlx_terminate(data->mlx);
		data->error_msg = "Failed to allocate memory for textures";
		return (EXIT_FAILURE);
	}
	ft_memset(data->textures, 0, sizeof(t_textures));
	//data->textures->door = mlx_load_png("textures/pics/eagle.png"); //check error
	data->num_sprites = 3;
	data->sprites = gc_alloc(sizeof(t_sprite) * data->num_sprites);
	data->sprites[0].texture = 0;
	status = parse_cubfile(filename, data);
	if (status)
	{
		//printf("Parsing error (possibly, location shown with an X)\n");
		mlx_terminate(data->mlx);
		if (data->textures)
			free_textures(data->textures);
		return (EXIT_FAILURE);
	}
	mlx_get_monitor_size(0, &monitor_width, &monitor_height);
	data->width = monitor_width * 0.5;
	data->height = monitor_height * 0.5;
	mlx_set_window_size(data->mlx, data->width, data->height);
	mlx_set_window_limit(data->mlx, MIN_WIDTH, MIN_HEIGHT, monitor_width, monitor_height);
	mlx_set_window_pos(data->mlx, (monitor_width - data->width) / 2, (monitor_height - data->height) / 2);
	data->camera.x = data->width / 2;
	data->camera.y = data->height / 2;
	data->image = mlx_new_image(data->mlx, data->width, data->height);
	if (!data->image)
	{
		mlx_terminate(data->mlx);
		free_textures(data->textures);
		data->error_msg = "Failed to create image";
		return (EXIT_FAILURE);
	}
	/* sprite hardcoded try */
	// data->num_sprites = 3;
	// data->sprites = gc_alloc(sizeof(t_sprite) * data->num_sprites);
	// data->sprites[0].x = data->player.x - 0.5;
	// data->sprites[0].y = data->player.y - 0.5;
	// data->sprites[0].texture = 0;
	data->sprites[1].x = 5.5;
	data->sprites[1].y = 5.5;
	data->sprites[1].texture = 1;
	data->sprites[2].x = 2.5;
	data->sprites[2].y = 5.5;
	data->sprites[2].texture = 2;
	data->zBuffer = gc_alloc(sizeof(double) * data->width);
	data->sprite_textures = gc_alloc(sizeof(mlx_texture_t *) * 3);
	data->sprite_textures[0] = mlx_load_png("textures/pics/enemyfinal.png");
	data->sprite_textures[1] = mlx_load_png("textures/pics/greenlight.png");
	data->sprite_textures[2] = mlx_load_png("textures/pics/barrel.png");
	/* sprite hardcoded try */

	/* enemy */
	data->enemy = gc_alloc(sizeof(mlx_texture_t *) * 3);

	/* ai hands */
	data->ai_hands = gc_alloc(sizeof(mlx_texture_t *) * 4);

	/* hands */
	data->hud_hands = gc_alloc(sizeof(mlx_texture_t *) * 4);
	ft_memset(data->hud_hands, 0, sizeof(mlx_texture_t *) * 4);
	data->hud_hands[0] = mlx_load_png("textures/hand/hand111.png");
	data->hud_hands[1] = mlx_load_png("textures/hand/hand222.png");
	data->hud_hands[2] = mlx_load_png("textures/hand/hand333.png");
	data->hud_hands[3] = mlx_load_png("textures/hand/hand444.png");
	// data->hud_hands[4] = mlx_load_png("textures/hand/hand555.png");
	data->hud_frame_count = 4;
	/* hands */
	if (is_wall(data, data->player.x, data->player.y))
	{
		printf("Error: Player starts inside a wall\n");
		mlx_terminate(data->mlx);
		free_textures(data->textures);
		return (EXIT_FAILURE);
	}
	return (EXIT_SUCCESS);
}

bool	has_cub_extension(char *filename)
{
	int len;

	if (!filename)
		return (false);
	len = ft_strlen(filename);
	if (len < 4)
		return (false);
	return (ft_strncmp(filename + len - 4, ".cub", 4) == 0);
}

int	main(int argc, char **argv)
{
	t_data	data;
	int		error;
	bool	strict;
	
	strict = false;
	if (argc < 2 || !has_cub_extension(argv[1]) || argc > 3 || (argc == 3 && ft_strncmp(argv[2], "--strict", 9) != 0))
	{
		printf("Usage: %s <.cub file> [--strict]\n", argv[0]);
		return (EXIT_FAILURE);
	}
	if (argc == 3)
		strict = true;
	error = initializer(&data, argv[1], strict);
	if (error)
		error_exit(data.error_msg);
		//return (EXIT_FAILURE);
	mlx_image_to_window(data.mlx, data.image, 0, 0);
	data.game_state = STATE_PLAYING;
	mlx_loop_hook(data.mlx, loop_hook, &data);
	mlx_loop(data.mlx);
	mlx_terminate(data.mlx);
	free_textures(data.textures);
	mlx_delete_texture(data.sprite_textures[0]);
	mlx_delete_texture(data.sprite_textures[1]);
	mlx_delete_texture(data.sprite_textures[2]);
	mlx_delete_texture(data.hud_hands[0]);
	mlx_delete_texture(data.hud_hands[1]);
	mlx_delete_texture(data.hud_hands[2]);
	mlx_delete_texture(data.hud_hands[3]);
	// mlx_delete_texture(data.hud_hands[4]);
	//gc_free(data.textures);
	gc_free_all();
	return (EXIT_SUCCESS);
}
