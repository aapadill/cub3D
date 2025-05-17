#include "cub3D.h"
#include "string.h"

#define CHAR_WIDTH 8
#define CHAR_HEIGHT 16

void entry_screen(t_data *data)
{
	if (data->game_state == STATE_MENU)
	{
		// Animate background
		if (data->bg_img)
		{
			// Clear background to black with slight alpha for "trail" effect (approximation)
			for (int y = 0; y < data->height; y++)
			{
				for (int x = 0; x < data->width; x++)
				{
					// Instead of fading, just clear fully or semi-clear
					// To slow fade effect you might try a darker color but this won't blend
					mlx_put_pixel(data->bg_img, x, y, 0x0A0A0AFF); // very dark gray
				}
			}

			// Add bright dots randomly (fireflies)
			for (int i = 0; i < 80; i++)
			{
				int x = (rand() + data->frame * i) % data->width;
				int y = (rand() + data->frame * (i + 1)) % data->height;
				uint32_t color = 0xFFFFFFFF; // White
				mlx_put_pixel(data->bg_img, x, y, color);
			}

			data->frame++;
		}

		int win_w = data->mlx->width;
		int win_h = data->mlx->height;

		// Draw title once
		if (!data->menu_text)
		{
			const char *title = "Welcome to cubXd";
			int title_len = strlen(title);
			int title_x = (win_w - title_len * CHAR_WIDTH) / 2;
			int title_y = win_h / 2 - 40; // Adjust vertically above the name input
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
				usleep(100000); // crude debounce
			}
		}
		// Handle backspace
		if (mlx_is_key_down(data->mlx, MLX_KEY_BACKSPACE) && data->name_length > 0)
		{
			data->name_length--;
			data->player_name[data->name_length] = '\0';
			usleep(100000); // debounce
		}

		// Show current input
		if (data->input_text)
			mlx_delete_image(data->mlx, data->input_text);

		char buf[64];
		snprintf(buf, sizeof(buf), "Name: %s", data->player_name);
		int input_len = strlen(buf);
		int input_x = (win_w - input_len * CHAR_WIDTH) / 2;
		int input_y = win_h / 2;

		data->input_text = mlx_put_string(data->mlx, buf, input_x, input_y);

		// Press ENTER to continue
		if (mlx_is_key_down(data->mlx, MLX_KEY_ENTER) && data->name_length > 0)
		{
			data->game_state = STATE_PLAYING;
			mlx_delete_image(data->mlx, data->menu_text);
			mlx_delete_image(data->mlx, data->input_text);
			data->menu_text = NULL;
			data->input_text = NULL;
		}
	}
}
