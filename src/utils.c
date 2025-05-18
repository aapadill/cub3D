/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: djelacik <djelacik@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/04 16:34:31 by djelacik          #+#    #+#             */
/*   Updated: 2025/03/04 14:44:23 by aapadill         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "cub3D.h"
#define CHAR_WIDTH 8
#define CHAR_HEIGHT 16

double	lookup_door_progress(t_data *data, int x, int y)
{
	int i = 0;
	while (i < data->door_count)
	{
		if (data->doors[i].x == x && data->doors[i].y == y)
			return data->doors[i].progress;
		i++;
	}
	return 0;
}

//make smoother, currently it's buggy if moving in a distance == 2
void update_doors(t_data *data)
{
	int	i = 0;
	while (i < data->door_count)
	{
		t_door *door = &data->doors[i];
		double dx = door->x + 0.5 - data->player.x;
		double dy = door->y + 0.5 - data->player.y;
		double distance = sqrt(dx * dx + dy * dy);

		// If player is close enough and roughly facing the door, trigger it
		if (distance < 3) /* adjust threshold */
		{
			// Check the angle between player direction and the door
			// (For simplicity, you might compare the player's angle with the angle to the door)
			// If conditions are met, start opening:
			if (door->state == CLOSED)
			{
				door->state = OPENING;
			}
		}
		
		// Animate door opening/closing
		if (door->state == OPENING)
		{
			door->progress += DOOR_OPENING_SPEED; // adjust speed as needed
			if (door->progress > 1.0)
			{
				door->progress = 1.01;
				door->state = OPEN;
			}
		}
		else if (door->state == OPEN)
		{
			// Optionally, if the player moves away, start closing the door
			if (distance > 3)
				door->state = CLOSING;
		}
		else if (door->state == CLOSING)
		{
			door->progress -= DOOR_OPENING_SPEED;
			if (door->progress <= 0.0)
			{
				door->progress = 0.0;
				door->state = CLOSED;
			}
		}
		i++;
	}
}

static char	get_map_cell(t_data *data, double x, double y)
{
	int	xi;
	int	yi;

	xi = (int)x;
	yi = (int)y;
	if (yi < 0 || yi >= data->map.height)
		return ('1');
	if (xi < 0 || xi >= (int)ft_strlen(data->map.grid[yi]))
		return ('1');
	return (data->map.grid[yi][xi]);
}

bool	is_wall(t_data *data, double x, double y)
{
	char	cell;

	cell = get_map_cell(data, x, y);
	if (cell == '1')
		return (1);
	else if (cell == 'D')
	{
		int i = 0;
		while (i < data->door_count)
		{
			if (data->doors[i].x == (int)x && data->doors[i].y == (int)y)
			{
				// If door is closed or only partially open, consider it a wall
				if (data->doors[i].state == CLOSED || data->doors[i].progress < 0.8)
					return (true);
				else
					return (false);
			}
			i++;
		}
		return (true);
	}
	return (false);
}

void	draw_floor_and_ceiling(t_data *data)
{
	int	x;
	int	y;

	y = 0;
	while (y < data->height)
	{
		x = 0;
		while (x < data->width)
		{
			if (y < data->height / 2)
				mlx_put_pixel(data->image, x, y, data->ceiling); // Katon väri
			else
				mlx_put_pixel(data->image, x, y, data->floor); // Lattian väri
			x++;
		}
		y++;
	}
}

// void entry_screen(t_data *data)
// {
// 	if (data->game_state == STATE_MENU)
// 	{
// 		// // Animate background
// 		// if (data->bg_img)
// 		// {
// 		// 	// Clear background to black with slight alpha for "trail" effect (approximation)
// 		// 	for (int y = 0; y < data->height; y++)
// 		// 	{
// 		// 		for (int x = 0; x < data->width; x++)
// 		// 		{
// 		// 			// Instead of fading, just clear fully or semi-clear
// 		// 			// To slow fade effect you might try a darker color but this won't blend
// 		// 			mlx_put_pixel(data->bg_img, x, y, 0x0A0A0AFF); // very dark gray
// 		// 		}
// 		// 	}

// 		// 	// Add bright dots randomly (fireflies)
// 		// 	for (int i = 0; i < 80; i++)
// 		// 	{
// 		// 		int x = (rand() + data->frame * i) % data->width;
// 		// 		int y = (rand() + data->frame * (i + 1)) % data->height;
// 		// 		uint32_t color = 0xFFFFFFFF; // White
// 		// 		mlx_put_pixel(data->bg_img, x, y, color);
// 		// 	}

// 		// 	data->frame++;
// 		// }

// 		int win_w = data->mlx->width;
// 		int win_h = data->mlx->height;

// 		// Draw title once
// 		if (!data->menu_text)
// 		{
// 			const char *title = "What object do you want as a gun?";
// 			int title_len = ft_strlen(title);
// 			int title_x = (win_w - title_len * CHAR_WIDTH) / 2;
// 			int title_y = win_h / 2 - 40; // Adjust vertically above the name input
// 			data->menu_text = mlx_put_string(data->mlx, title, title_x, title_y);
// 		}

// 		// Handle key input for name
// 		for (int key = MLX_KEY_A; key <= MLX_KEY_Z; key++)
// 		{
// 			if (mlx_is_key_down(data->mlx, key) && data->name_length < MAX_NAME_LEN)
// 			{
// 				char c = 'A' + (key - MLX_KEY_A);
// 				data->player_name[data->name_length++] = c;
// 				data->player_name[data->name_length] = '\0';
// 				usleep(100000); // crude debounce
// 			}
// 		}
// 		// Handle backspace
// 		if (mlx_is_key_down(data->mlx, MLX_KEY_BACKSPACE) && data->name_length > 0)
// 		{
// 			data->name_length--;
// 			data->player_name[data->name_length] = '\0';
// 			usleep(100000); // debounce
// 		}

// 		// Show current input
// 		if (data->input_text)
// 			mlx_delete_image(data->mlx, data->input_text);

// 		char buf[64];
// 		snprintf(buf, sizeof(buf), "Gun: %s", data->player_name);
// 		int input_len = ft_strlen(buf);
// 		int input_x = (win_w - input_len * CHAR_WIDTH) / 2;
// 		int input_y = win_h / 2;

// 		data->input_text = mlx_put_string(data->mlx, buf, input_x, input_y);

// 		// Press ENTER to continue
// 		if (mlx_is_key_down(data->mlx, MLX_KEY_ENTER) && data->name_length > 0)
// 		{
// 			// data->game_state = STATE_PLAYING;
// 			// mlx_delete_image(data->mlx, data->menu_text);
// 			// mlx_delete_image(data->mlx, data->input_text);
// 			// data->menu_text = NULL;
// 			// data->input_text = NULL;

// 			// Got a name — spawn the AI worker and resume
// 			printf("Type \"%s\"\n", data->player_name);
// 			data->calling_new_gun = true;
// 			data->is_gun_ready    = false;
// 			pthread_t tid;
// 			pthread_create(&tid, NULL, ai_worker, data);
// 			pthread_detach(tid);
// 			data->game_state = STATE_PLAYING;
// 			// clean up the menu text
// 			mlx_delete_image(data->mlx, data->menu_text);
// 			mlx_delete_image(data->mlx, data->input_text);
// 			data->menu_text  = NULL;
// 			data->input_text = NULL;
// 			// now the regular loop (try_load_hands → etc.) will kick in,
// 			// watching calling_new_gun/is_gun_ready for when the new
// 			// textures appear
// 		}
// 	}
// }

void entry_screen(t_data *data)
{
    if (data->game_state != STATE_MENU)
        return;

    int win_w = data->mlx->width;
    int win_h = data->mlx->height;

    // Draw title once
    if (!data->menu_text)
    {
        const char *title = "What object do you want as a gun?";
        int title_len = ft_strlen(title);
        int title_x   = (win_w - title_len * CHAR_WIDTH) / 2;
        int title_y   = win_h / 2 - 40;
        int box_x     = title_x - 4;
        int box_y     = title_y - 4;
        int box_w     = title_len * CHAR_WIDTH + 8;
        int box_h     = CHAR_HEIGHT + 8;

        // Draw black background box
        for (int y = box_y; y < box_y + box_h; y++)
            for (int x = box_x; x < box_x + box_w; x++)
                mlx_put_pixel(data->image, x, y, BLACK_COLOR);

        data->menu_text = mlx_put_string(data->mlx, title, title_x, title_y);
    }

    // Handle key input for name
    for (int key = MLX_KEY_A; key <= MLX_KEY_Z; key++)
    {
        if (mlx_is_key_down(data->mlx, key) && data->name_length < MAX_NAME_LEN)
        {
            char c = 'A' + (key - MLX_KEY_A);
            data->player_name[data->name_length++] = c;
            data->player_name[data->name_length] = '\0';
            usleep(100000);
        }
    }
    if (mlx_is_key_down(data->mlx, MLX_KEY_BACKSPACE) && data->name_length > 0)
    {
        data->name_length--;
        data->player_name[data->name_length] = '\0';
        usleep(100000);
    }

    // Draw input box background
    char buf[64];
    snprintf(buf, sizeof(buf), "> %s", data->player_name);
    int input_len = ft_strlen(buf);
    int input_x   = (win_w - input_len * CHAR_WIDTH) / 2;
    int input_y   = win_h / 2;
    int box_x2    = input_x - 4;
    int box_y2    = input_y - 4;
    int box_w2    = input_len * CHAR_WIDTH + 8;
    int box_h2    = CHAR_HEIGHT + 8;

    for (int y = box_y2; y < box_y2 + box_h2; y++)
        for (int x = box_x2; x < box_x2 + box_w2; x++)
            mlx_put_pixel(data->image, x, y, BLACK_COLOR);

    if (data->input_text)
        mlx_delete_image(data->mlx, data->input_text);
    data->input_text = mlx_put_string(data->mlx, buf, input_x, input_y);

    // Press ENTER to continue
    if (mlx_is_key_down(data->mlx, MLX_KEY_ENTER) && data->name_length > 0)
    {
        data->calling_new_gun = true;
        data->is_gun_ready    = false;
        pthread_t tid;
        pthread_create(&tid, NULL, ai_worker, data);
        pthread_detach(tid);
        data->game_state = STATE_PLAYING;

        mlx_delete_image(data->mlx, data->menu_text);
        mlx_delete_image(data->mlx, data->input_text);
        data->menu_text  = NULL;
        data->input_text = NULL;
    }
}