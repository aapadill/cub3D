/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   hands.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aapadill <aapadill@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/17 10:26:53 by aapadill          #+#    #+#             */
/*   Updated: 2025/03/17 10:27:12 by aapadill         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "cub3D.h"

void shooting_animation(t_data *data)
{
	if (data->is_player_shooting) //is_player_moving
	{
		bool just_started;

		just_started = true;
		data->hud_frame_timer++;
		if (data->hud_frame_timer >= SEC_PER_FRAME)
		{
			data->hud_frame = (data->hud_frame + 1) % data->hud_frame_count;
			data->hud_frame_timer = 0;
			just_started = false;
		}
		//printf("frame is %d\n", data->hud_frame);
		if (data->hud_frame == 0 && !just_started)
			data->is_player_shooting = 0;
	}
	else
	{
		data->hud_frame = 0;
		data->hud_frame_timer = 0;
	}
}

// void draw_hud_hands(t_data *data)
// {
// 	mlx_texture_t	*hand_tex = data->hud_hands[data->hud_frame];

// 	float			target_height = data->height * 0.6f;
// 	float			scale_factor = target_height / hand_tex->height;

// 	int				new_width = hand_tex->width * scale_factor;
// 	int				new_height = hand_tex->height * scale_factor;

// 	// bottom center of the screen
// 	int pos_x = (data->width - new_width) / 2;
// 	int pos_y = data->height - new_height;

// 	int y = 0;
// 	while (y < new_height)
// 	{
// 		int x = 0;
// 		while (x < new_width)
// 		{
// 			int	src_x = x / scale_factor;
// 			int	src_y = y / scale_factor;

// 			uint32_t color = get_texture_color(hand_tex, src_x, src_y);
// 			// skip black pixels
// 			if ((color & 0x00FFFFFF) != 0)
// 			{
// 				color = simple_shading(color, 1.0);
// 				mlx_put_pixel(data->image, pos_x + x, pos_y + y, color);
// 			}
// 			x++;
// 		}
// 		y++;
// 	}
// }

void draw_hud_hands(t_data *data)
{
    mlx_texture_t *hand_tex = data->hud_hands[data->hud_frame];

    float target_height  = data->height * 0.6f;
    float scale_factor   = target_height / hand_tex->height;

    int new_width  = hand_tex->width  * scale_factor;
    int new_height = hand_tex->height * scale_factor;

    int pos_x = (data->width  - new_width)  / 2;
    int pos_y =  data->height - new_height;

    const uint8_t BLACK_THRESHOLD = 16;
    for (int y = 0; y < new_height; ++y) {
        for (int x = 0; x < new_width; ++x) {
            int src_x = x / scale_factor;
            int src_y = y / scale_factor;

            uint32_t color = get_texture_color(hand_tex, src_x, src_y);
            uint8_t r = (color >> 16) & 0xFF;
            uint8_t g = (color >>  8) & 0xFF;
            uint8_t b =  color        & 0xFF;

            // skip “almost black” pixels
            if (r < BLACK_THRESHOLD &&
                g < BLACK_THRESHOLD &&
                b < BLACK_THRESHOLD)
                continue;

            color = simple_shading(color, 1.0f);
            mlx_put_pixel(data->image, pos_x + x, pos_y + y, color);
        }
    }
}
