/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parsing.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aapadill <aapadill@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/20 09:32:54 by aapadill          #+#    #+#             */
/*   Updated: 2025/04/03 09:29:49 by aapadill         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "cub3D.h"

static bool	valid_character(char c)
{
	return (c == '0' || c == '1' || c == 'N' || c == 'S'
		|| c == 'E' || c == 'W' || c == 'D' || c == ' ' || c == 'X');
}

bool	is_map_line(char *line)
{
	while (*line)
	{
		if (!valid_character(*line))
			return (false);
		line++;
	}
	return (true);
}

bool	line_is_only_spaces(char *line)
{
	size_t	count;
	char	*original;
	
	count = 0;
	original = line;
	while (*line)
	{
		if (*line == ' ')
			count++;
		line++;
	}
	if (ft_strlen(original) == count)
		return (true);
	return (false);
}

int	count_char(char *str, char c)
{
	int count;

	count = 0;
	while (*str)
	{
		if (*str == c)
			count++;
		str++;
	}
	return (count);
}

bool	is_number(char *str)
{
	while (*str)
	{
		if (!ft_isdigit(*str))
			return false;
		str++;
	}
	return true;
}

bool	load_texture(char *path, mlx_texture_t **texture)
{
	*texture = mlx_load_png(path);
	if (!*texture)
		return (false);
	return (true);
}

bool	textures_ready(t_data *data)
{
	t_textures		*t;

	t = data->textures;
	return (t->north && t->south && t->west && t->east);
}

//*key and *rest point at new malloc strings
static bool split_line(char *line, char **key, char **rest)
{
	char	*trimmed;
	char	*sep;

	trimmed = gc_strtrim(line, " \t\n\r");
	if (!trimmed)
		return (false);
	sep = trimmed;
	//find the first space, that will be our separator
	while (*sep && !ft_isspace(*sep))
		sep++;
	if (!*sep)
		return (false);
	*sep = '\0';
	sep++;
	//skip spaces before the rest
	while (ft_isspace(*sep))
		sep++;
	if (!*sep)
		return (false);
	*key = gc_strdup(trimmed);
	*rest = gc_strtrim(sep, " \t\n\r");
	return (*key && *rest);
}

bool	parse_color_values(char *str, uint32_t *color)
{
	int count;
	char **vals;
	int i;
	int nums[3];

	if (count_char(str, ',') != 2)
		return (false);
	vals = gc_split(str, ',', &count);
	if (!vals || count != 3)
		return (false);
	i = 0;
	while (i < 3)
	{
		vals[i] = gc_strtrim(vals[i], " \t\n\r");
		if (!is_number(vals[i]))
			return (false);
		nums[i] = ft_atoi(vals[i]);
		if (nums[i] < 0 || nums[i] > 255)
			return (false);
		i++;
	}
	*color = get_rgba(nums[0], nums[1], nums[2], 255);
	return (true);
}

//(F | C) <r,g,b>
bool	parse_color_line(char *line, t_data *data)
{
	char	*key;
	char	*vals;
	bool	status;

	status = false;
	if (!split_line(line, &key, &vals))
		return (false);
	if (!ft_strncmp(key, "F", 2) && !data->f_color_found)
	{
		status = parse_color_values(vals, &data->floor);
		if (status)
			data->f_color_found = true;
	}
	else if (!ft_strncmp(key, "C", 2) && !data->c_color_found)
	{
		status = parse_color_values(vals, &data->ceiling);
		if (status)
			data->c_color_found = true;
	}
	//gc_free(key);
	//gc_free(vals);
	return (status);
}

//(NO | SO | WE | EA) <path>
bool	parse_texture_line(char *line, t_data *data)
{
	char		*key;
	char		*path;
	bool		ok;
	t_textures	*t;

	ok = false;
	t = data->textures;
	if (!split_line(line, &key, &path))
		return (false);
	if (!ft_strncmp(key, "NO", 3) && !t->north)
		ok = load_texture(path, &t->north);
	else if (!ft_strncmp(key, "SO", 3) && !t->south)
		ok = load_texture(path, &t->south);
	else if (!ft_strncmp(key, "WE", 3) && !t->west)
		ok = load_texture(path, &t->west);
	else if (!ft_strncmp(key, "EA", 3) && !t->east)
		ok = load_texture(path, &t->east);
	else if (!ft_strncmp(key, "DO", 3) && !t->door)
		ok = load_texture(path, &t->door);
	gc_free(key);
	gc_free(path);
	return (ok);
}

bool	parse_player_pos(t_data *data)
{
	int i, j, already_assigned = 0;

	i = 0;
	while (data->map.grid[i])
	{
		j = 0;
		while (data->map.grid[i][j])
		{
			if ((data->map.grid[i][j] == 'N' || data->map.grid[i][j] == 'S' || data->map.grid[i][j] == 'E' || data->map.grid[i][j] == 'W') && !already_assigned)
			{
				if (data->map.grid[i][j] == 'N')
					data->player.angle = 3 * M_PI / 2; // 270 degrees
				else if (data->map.grid[i][j] == 'S')
					data->player.angle = M_PI / 2; // 90 degrees
				else if (data->map.grid[i][j] == 'E')
					data->player.angle = 0;
				else if (data->map.grid[i][j] == 'W')
					data->player.angle = M_PI;
				data->player.x = j + 0.5;
				data->player.y = i + 0.5;
				data->map.grid[i][j] = '0';
				already_assigned = 1;
			}
			else if((data->map.grid[i][j] == 'N' || data->map.grid[i][j] == 'S' || data->map.grid[i][j] == 'E' || data->map.grid[i][j] == 'W') && already_assigned)
			{
				//ft_putstr_fd("Error, multiple player positions\n", 2);
				data->error_msg = "Multiple player positions detected";
				return (true);
			}
			else if (data->map.grid[i][j] == 'D')
			{
				if (data->textures->door == NULL)
				{
					data->error_msg = "Door texture not defined";
					return (true);
				}
				data->doors[data->door_count].x = j;
				data->doors[data->door_count].y = i;
				data->doors[data->door_count].state = CLOSED;
				data->doors[data->door_count].progress = 0;
				data->door_count++;
			}
			else if (data->map.grid[i][j] == 'X')
			{
				data->sprites[0].x = j + 0.5;
				data->sprites[0].y = i + 0.5;
				data->map.grid[i][j] = '0';
			}
			else if (data->map.grid[i][j] == ' ')
			{
				data->map.grid[i][j] = '0';
				//printf("ok trying to remove spaces\n");
			}
			//neccessary?
			else if (data->map.grid[i][j] != '0' && data->map.grid[i][j] != '1')
			{
				//ft_putstr_fd("Error, invalid character in map\n", 2);
				data->error_msg = "Invalid character in map";
				return (true);
			}
			j++;
		}
		i++;
	}
	if (!already_assigned)
	{
		data->error_msg = "No player position found";
		return (true);
	}
	return (false);
}

bool is_map_closed_strict(t_data *data)
{
	int i, j, line_len;
	int height = data->map.height;
	i = 0;
	line_len = 0;

	//printf("player position: %f %f\n", data->player.x, data->player.y);
	while (i < height)
	{
		j = 0;
		line_len = ft_strlen(data->map.grid[i]);
		//printf("%i has %i chars---->", i, line_len);
		while (j < line_len)
		{
			if (i == 0 || i == height - 1 || j == 0 || j == line_len - 1)
			{
				if (data->map.grid[i][j] != '1')
				{
					//ft_putstr_fd("Error, map is not closed\n", 2);
					//printf("X\n");
					return (false);
				}
			}
			else if (data->map.grid[i][j] == '0' || data->map.grid[i][j] == 'D')
			{
				if (data->map.grid[i - 1][j] == '\0' || data->map.grid[i + 1][j] == '\0' || data->map.grid[i][j - 1] == '\0' || data->map.grid[i][j + 1] == '\0')
				{
					//ft_putstr_fd("Error, map is not closed (intermediate)\n", 2);
					//printf("X\n");
					return (false);
				}
			}
			else
			{
				if (data->map.grid[i][j] != '1')
				{
					//ft_putstr_fd("Error, map is not closed\n", 2);
					//printf("X\n");
					return (false);
				}
			}
			//if (data->map.grid[i][j] != '\0')
				//printf("%c", data->map.grid[i][j]);
			//else
			//	printf("X");
			j++;
		}
		//printf("<----\n");
		i++;
	}
	return true;
}

//flood-fill from (i,j), returns false if any path leaks out
//uses visited array so we don't edit data->map.grid
static bool	flood_fill(t_data *data, bool **visited, int i, int j)
{
	char	**grid;
	int		height;
	int		len;

	grid   = data->map.grid;
	height = data->map.height;
	//off top/bottom means open
	if (i < 0 || i >= height)
		return (false);
	//off left/right means open
	len = ft_strlen(grid[i]);
	if (j < 0 || j >= len)
		return (false);
	//wall, this path is sealed
	if (grid[i][j] == '1')
		return (true);
	//already visited, sealed
	if (visited[i][j])
		return (true);
	//anything other than floor or door, treat as sealed
	if (grid[i][j] != '0' && grid[i][j] != 'D')
		return (true);
	//mark visited
	visited[i][j] = true;
	//recursively, if leaks out from anywhere propagate false
	if (!flood_fill(data, visited, i - 1, j))
		return (false);
	if (!flood_fill(data, visited, i + 1, j))
		return (false);
	if (!flood_fill(data, visited, i, j - 1))
		return (false);
	if (!flood_fill(data, visited, i, j + 1))
		return (false);
	return (true);
}

bool	is_map_closed(t_data *data)
{
	int		height;
	char	**grid;
	bool	**visited;
	int		i;
	int		len;
	int		start_i;
	int		start_j;
	bool	ok;

	height = data->map.height;
	grid   = data->map.grid;
	//allocate visited (bool) array
	visited = malloc(sizeof(bool *) * height);
	if (!visited)
		return (false);
	i = 0;
	while (i < height)
	{
		len = ft_strlen(grid[i]);
		visited[i] = malloc(sizeof(bool) * len);
		if (!visited[i])
			return (false);
		ft_memset(visited[i], 0, sizeof(bool) * len);
		i++;
	}
	//starting cell from player position
	start_i = (int)(data->player.y - 0.5);
	start_j = (int)(data->player.x - 0.5);
	ok = flood_fill(data, visited, start_i, start_j);
	//free visited
	i = 0;
	while (i < height)
	{
		free(visited[i]);
		i++;
	}
	free(visited);
	return (ok);
}

int	parse_cubfile(char *filepath, t_data *data)
{
	char	*line;
	char	*nl;
	int		fd;
	bool	map_started = false;
	bool	status = 0;
	size_t	i;
	int	row_len;
	int		doors_count = 0;

	line = NULL;
	//data->player.angle = 0;
	fd = open(filepath, O_RDONLY); //check more errors
	if (fd < 0)
	{
		perror("open failed");
		return (1);
	}
	t_vec	map_vec;
	if (vec_new(&map_vec, VEC_INIT_SIZE, sizeof(char *)) < 0)
	{
		//ft_putstr_fd("Vec alloc failed\n", 2);
		data->error_msg = "Vec alloc failed";
		close(fd);
		return (1);
	}
	line = gc_next_line(fd, READ_LINE);
	while (line)
	{
		//skip empty lines
		if (line[0] == '\n' && !map_started)
		{
			//free(line);
			line = gc_next_line(fd, READ_LINE);
			continue ;
		}
		nl = ft_strchr(line, '\n');
		if (nl)
			*nl = '\0';
		//printf("line->%s<-\n", line);
		/*
		if (is_map_line(line))
		{
			if (!map_started)
				map_started = true;
			printf("is map.grid line\n");
		}
		*/
		if (is_map_line(line) && !line_is_only_spaces(line))
		{
			if (!data->f_color_found || !data->c_color_found || !textures_ready(data))
			{
				//ft_putstr_fd("You're trying to initialize map before colors or textures, exiting\n", 2);
				data->error_msg = "Please define colors/textures before the map";
				status = 1;
				break ;
			}
			if (!map_started)
				map_started = true;
			if (vec_push(&map_vec, &line) < 0)
			{
				//ft_putstr_fd("Vec push failed\n", 2);
				data->error_msg = "Vec push failed";
				//free(line);
				status = 1;
				break ;
			}
		}
		/*
		else if (!map_started)
		{
			if (is_texture_line(line))
				printf("is texture line\n");
			if (is_color_line(line))
				printf("is color line\n");
		}
		*/
		else if (!map_started)
		{
			//remove_spaces(line);
			if (!parse_texture_line(line, data) && !parse_color_line(line, data))
			{
				data->error_msg = "Check your color/textures";
				//ft_putstr_fd("Either some invalid line or trying to define the same thing twice\n", 2);
				//free(line);
				status = 1;
				break ;
			}
		}
		/*
		else
			printf("error: either map.grid is not last or line not valid\n");
		*/
		else
		{
			//ft_putstr_fd("Map must be the last thing in file\n", 2);
			data->error_msg = "Map must be the last thing in file";
			//free(line);
			status = 1;
			break ;
		}
		line = gc_next_line(fd, READ_LINE);
	}
	gc_next_line(fd, CLEAN_LINE);
	close(fd);
	//clean static buffer from gnl
	if (!status)
	{
		data->map.height = map_vec.len;
		data->map.grid = gc_alloc((map_vec.len + 1) * sizeof(char *));
		if (!data->map.grid)
		{
			//ft_putstr_fd("Map alloc failed\n", 2);
			data->error_msg = "Map alloc failed";
			status = 1;
		}
		else
		{
			i = 0;
			while (i < map_vec.len)
			{
				data->map.grid[i] = *(char **)vec_get(&map_vec, i);
				//printf("->%s<-\n", data->map.grid[i]);
				/*should this be here?*/
				int j = 0;
				while (data->map.grid[i][j])
				{
					if (data->map.grid[i][j] == 'D')
					{
						doors_count++;
					}
					/*
					else if (data->map.grid[i][j] == 'X')
					{
						//data->enemy_count++;
					}
					*/
					j++;
				}
				/*should this be here?*/
				row_len = ft_strlen(data->map.grid[i]);
				if (row_len > data->map.width)
					data->map.width = row_len;
				i++;
			}
			if (doors_count > 0)
			{
				data->doors = gc_alloc(doors_count * sizeof(t_door));
				ft_memset(data->doors, 0, doors_count * sizeof(t_door));
			}
			else
			{
				data->doors = NULL;
			}
			data->map.grid[map_vec.len] = NULL;
		}
	}
	if (!status)
	{
		status = parse_player_pos(data);
	}
	if (!status)
	{
		/*
		i = 0;
		while (i < map_vec.len)
		{
			printf("->%s<-\n", data->map.grid[i]);
			i++;
		}
		*/
		//ft_putstr_fd("Map is not closed\n", 2);
		if (data->strict)
		{
			if (!is_map_closed_strict(data))
			{
				//ft_putstr_fd("Map is not closed (using strict checking)\n", 2);
				data->error_msg = "Map is open (strict checking)";
				status = 1;
			}
		}
		else
		{
			if (!is_map_closed(data))
			{
				//ft_putstr_fd("Map is not closed\n", 2);
				data->error_msg = "Map is open";
				status = 1;
			}
		}
	}
	vec_free(&map_vec);
	//printf("reached end of parsing\n");
	return (status);
}

/*
bool	remove_spaces(char *line)
{
	char	*src;
	char	*dst;

	src = line;
	dst = line;
	while (*src)
	{
		if (*src != ' ' && *src != '\t') //tabs?
			*dst++ = *src;
		src++;
	}
	*dst = '\0';
	return (true);
}

bool	parse_texture_line(char *line, t_data *data)
{
	t_textures	*t;

	t = data->textures;
	if (!ft_strncmp(line, "NO", 2) && !t->north)
		return (load_texture(line + 2, &t->north));
	else if (!ft_strncmp(line, "SO", 2)  && !t->south)
		return (load_texture(line + 2, &t->south));
	else if (!ft_strncmp(line, "WE", 2) && !t->west)
		return (load_texture(line + 2, &t->west));
	else if (!ft_strncmp(line, "EA", 2) && !t->east)
		return (load_texture(line + 2, &t->east));
	return (false);
}


bool	parse_color_line(char *line, t_data *data)
{
	bool	status;
	if (!data->f_color_found && line[0] == 'F')
	{
		//hardcoded jump of 1
		printf("Floor color:\n");
		status = parse_color_values(line + 1, &data->floor);
		printf("data->floor_color = 0x%08x\n", data->floor);
		if (status)
			data->f_color_found = 1;
		return (status);
	}
	if (!data->c_color_found && line[0] == 'C')
	{
		//hardcoded jump of 1
		printf("Ceiling color:\n");
		status = parse_color_values(line + 1, &data->ceiling);
		printf("data->ceiling_color = 0x%08x\n", data->ceiling);
		if (status)
			data->c_color_found = 1;
		return (status);
	}
	return false;
}

//check commas and numbers
bool	parse_color_values(char *str, uint32_t *color)
{
	int words;
	char **split;
	char **copy;
	int number;

	if (count_char(str, ',') != 2)
		return (false);
	split = gc_split(str, ',', &words);
	if (words != 3)
		return (false);
	copy = split;
	while (*copy)
	{
		number = ft_atoi(*copy);
		if (!is_number(*copy) || number < 0 || number > 255)
			return (false);
		copy++;
	}
	*color = get_rgba(ft_atoi(split[0]), ft_atoi(split[1]), ft_atoi(split[2]), 255);
	return (true);
}
*/
