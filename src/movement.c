/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   movement.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: djelacik <djelacik@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/31 15:40:09 by djelacik          #+#    #+#             */
/*   Updated: 2025/04/14 10:20:25 by aapadill         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "cub3D.h"

static int can_move_to(t_data *data, double new_x, double new_y)
{
	// Check four diagonal directions
	double diag_offset;

	diag_offset = COLLISION_LIMIT / sqrt(2);
	// Check four cardinal directions
	if (is_wall(data, new_x + COLLISION_LIMIT, new_y)) // Right
		return 0;
	if (is_wall(data, new_x - COLLISION_LIMIT, new_y)) // Left
		return 0;
	if (is_wall(data, new_x, new_y + COLLISION_LIMIT)) // Down
		return 0;
	if (is_wall(data, new_x, new_y - COLLISION_LIMIT)) // Up
		return 0;
	if (is_wall(data, new_x + diag_offset, new_y + diag_offset)) // Bottom-right
		return 0;
	if (is_wall(data, new_x - diag_offset, new_y + diag_offset)) // Bottom-left
		return 0;
	if (is_wall(data, new_x + diag_offset, new_y - diag_offset)) // Top-right
		return 0;
	if (is_wall(data, new_x - diag_offset, new_y - diag_offset)) // Top-left
		return 0;
	return 1; // No collisions, movement allowed
}

//normalize diagonal movement
static void handle_movement(t_data *data)
{
	double orig_x;
	double orig_y;
	double new_x;
	double new_y;

	orig_x = data->player.x;
	orig_y = data->player.y;
	if (mlx_is_key_down(data->mlx, MLX_KEY_W))
	{
		new_x = orig_x + cos(data->player.angle) * data->player.speed;
		new_y = orig_y + sin(data->player.angle) * data->player.speed;
	}
	else if (mlx_is_key_down(data->mlx, MLX_KEY_S))
	{
		new_x = orig_x - cos(data->player.angle) * data->player.speed;
		new_y = orig_y - sin(data->player.angle) * data->player.speed;
	}
	else if (mlx_is_key_down(data->mlx, MLX_KEY_D))
	{
		new_x = orig_x - sin(data->player.angle) * data->player.speed;
		new_y = orig_y + cos(data->player.angle) * data->player.speed;
	}
	else if (mlx_is_key_down(data->mlx, MLX_KEY_A))
	{
		new_x = orig_x + sin(data->player.angle) * data->player.speed;
		new_y = orig_y - cos(data->player.angle) * data->player.speed;
	}
	else
	{
		data->is_player_moving = false;
		return; // No forward/backward movement
	}

	// Try moving diagonally first
	if (can_move_to(data, new_x, new_y))
	{
		data->player.x = new_x;
		data->player.y = new_y;
		data->is_player_moving = true;
	}
	else
	{
		// If blocked, try moving along X only
		if (can_move_to(data, new_x, orig_y))
		{
			data->player.x = new_x;
			data->is_player_moving = true;
		}
		// And try moving along Y only
		if (can_move_to(data, orig_x, new_y))
		{
			data->player.y = new_y;
			data->is_player_moving = true;
		}
	}
}

static void handle_shake(t_data *data)
{
	if (data->is_player_moving)
	{
		data->camera.shake_time += SEC_PER_FRAME;
		data->camera.shake_offset = sin(data->camera.shake_time * SHAKE_VEL_WALK) * PIX_WALK;
		if (data->is_player_shooting)
			data->camera.shake_offset += sin(data->camera.shake_time * SHAKE_VEL_RECO) * PIX_RECOIL;
		//printf("Shake time: %f\n", data->camera.shake_time);
	}
	else if (data->is_player_shooting)
	{
		data->camera.shake_time += SEC_PER_FRAME;
		data->camera.shake_offset = sin(data->camera.shake_time * SHAKE_VEL_RECO) * PIX_RECOIL;
		//printf("Shake time: %f\n", data->camera.shake_time);
	}
	else
	{
		data->camera.shake_time = 0;
		data->camera.shake_offset = 0;
	}
}

static void	handle_new_gun(t_data *data)
{
	if (mlx_is_key_down(data->mlx, MLX_KEY_2))
	{
		printf("New gun coming...\n");
		data->calling_new_gun = true;
		data->is_gun_ready = false;
	}
}

static void	handle_shooting(t_data *data)
{
	if (mlx_is_key_down(data->mlx, MLX_KEY_SPACE))
		data->is_player_shooting = true;
	if (mlx_is_mouse_down(data->mlx, MLX_MOUSE_BUTTON_LEFT))
		data->is_player_shooting = true;
}

static void	handle_rotation(t_data *data)
{
	if (mlx_is_key_down(data->mlx, MLX_KEY_LEFT))
		data->player.angle -= data->player.speed;
	if (mlx_is_key_down(data->mlx, MLX_KEY_RIGHT))
		data->player.angle += data->player.speed;
}

static void handle_mouse_rotation(t_data *data)
{
	int32_t aux_x = 0;
	int32_t aux_y = 0;
	int32_t dx = 0;
	//int32_t dy = 0;
	
	// if (!data->flag)
	// {
	// 	mlx_set_cursor_mode(data->mlx, MLX_MOUSE_DISABLED);
	// 	return ;
	// }
	if (mlx_is_key_down(data->mlx, MLX_KEY_TAB))
		data->camera.toggle = 0;
	if (!data->camera.toggle && mlx_is_mouse_down(data->mlx, MLX_MOUSE_BUTTON_LEFT))
		data->camera.toggle = 1;
	if (data->camera.toggle == 0)
	{
		mlx_set_cursor_mode(data->mlx, MLX_MOUSE_NORMAL);
		return ;
	}
	mlx_get_mouse_pos(data->mlx, &aux_x, &aux_y);
	dx = aux_x - data->camera.x;
	//dy = aux_y - data->camera.y;
	if (dx)
		data->player.angle += dx * MOUSE_SENSITIVITY;
	mlx_set_cursor_mode(data->mlx, MLX_MOUSE_HIDDEN);
	data->camera.x = data->width / 2;
	data->camera.y = data->height / 2;
	mlx_set_mouse_pos(data->mlx, data->camera.x, data->camera.y);
}

void my_resize_callback(int width, int height, void* param)
{
	t_data *data = (t_data *)param;

	if (width <= MIN_WIDTH || height <= MIN_HEIGHT)
	{
		if (width == MIN_WIDTH && height == MIN_HEIGHT)
		{
			printf("Reached smallest resolution: %i x %i\n", width, height);
			return ;
		}
		if (width <= MIN_WIDTH)
			data->new_width = MIN_WIDTH;
		else
			data->new_width = width;
		if (height <= MIN_HEIGHT)
			data->new_height = MIN_HEIGHT;
		else
			data->new_height = height;
	}
	else
	{
		data->new_width = width;
		data->new_height = height;
	}
	data->resize_pending = true;
	printf("Resize Callback: new dimensions set to %i x %i\n", data->new_width, data->new_height);
}

static void	render(t_data *data)
{
	// mlx_delete_image(data->mlx, data->image);
	mlx_resize_hook(data->mlx, &my_resize_callback, (void *)data);
	// data->image = mlx_new_image(data->mlx, data->width, data->height);
	// mlx_image_to_window(data->mlx, data->image, 0, 0);
	if (data->resize_pending)
	{
		data->width = data->new_width;
		data->height = data->new_height;
		gc_free(data->zBuffer);
		//free(data->zBuffer);
		data->zBuffer = gc_alloc(sizeof(double) * data->width);
		if (!data->zBuffer)
		{
			printf("Error: Failed to allocate memory for zBuffer\n");
			return; //exit if possible
		}
		//if (data->image)
		mlx_delete_image(data->mlx, data->image);
		data->image = mlx_new_image(data->mlx, data->new_width, data->new_height);
		mlx_image_to_window(data->mlx, data->image, 0, 0);
		//if (!data->image)
		data->player.speed = (double)data->height * PLAYER_SPEED;
		data->camera.x = data->width / 2;
		data->camera.y = data->height / 2;
		printf("Render will now happen with %dx%d\n", data->width, data->height);
		data->resize_pending = false;
	}
	draw_floor_and_ceiling(data);
	//draw_walls(data);
	//draw_sprites(data);
	draw_walls_and_sprites(data);
	draw_mini_map(data);
	draw_mini_player(data);
	draw_mini_rays(data);
	// Draw HUD hands
	draw_hud_hands(data);
    // Hud animation
	shooting_animation(data);
}

void	loop_hook(void *param)
{
	t_data	*data;
	data = (t_data *)param;

	handle_movement(data);
	handle_shake(data);
	handle_new_gun(data);
	handle_shooting(data);
	handle_mouse_rotation(data);
	handle_rotation(data);
	render(data);
	update_doors(data);
	if (mlx_is_key_down(data->mlx, MLX_KEY_ESCAPE))
		mlx_close_window(data->mlx);
}
