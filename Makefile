NAME = cub3D

CC = cc
# CFLAGS =  -Wall -Werror -Wextra -g -O2 #-fsanitize=address #-fPIE 
# LDFLAGS = -lglfw -pthread -lm -L./MLX42/include/MLX42 -L./includes -L"/opt/homebrew/Cellar/glfw/3.4/lib/" #-ldl #-pie -lft 



CFLAGS = -Wall  -Wextra -g -O2 -fsanitize=address \
	-I/opt/homebrew/include \
	-I/opt/homebrew/opt/curl/include \
	-I./includes \
	-I./MLX42/include/MLX42


LDFLAGS = \
	-L/opt/homebrew/lib \
	-lglfw -pthread -lm -lcurl -lcjson \
	-L"./MLX42/include/MLX42" \
	-L"./includes" \
	-L"/opt/homebrew/Cellar/glfw/3.4/lib/"

LIBMLX = ./MLX42


LIBMLX = ./MLX42
MLX_42 = $(LIBMLX)/build/libmlx42.a

INCLUDES = -Iincludes -I$(LIBMLX)/include

SRC = \
src/parsing.c \
src/color_utils.c \
src/main.c \
src/shading.c \
src/texture.c \
src/ray.c \
src/rays.c \
src/utils.c \
src/minimap.c \
src/movement.c \
src/sprites.c \
src/hands.c \
src/free.c \
src/api.c

OBJ = $(SRC:.c=.o)

# Rule for building the final executable
all: $(NAME)

# Compile libft library with -fPIE
libft/libft.a:
	#@make CFLAGS="-Wall -Wextra -Werror -fPIE" -C libft
	make -C ./libft
	make bonus -C ./libft

# Clone MLX42 if it does not exist
mlx_clone: 
	@if [ ! -d "$(LIBMLX)" ]; then \
		echo "Cloning MLX42..."; \
		git clone https://github.com/codam-coding-college/MLX42.git $(LIBMLX); \
	fi

# Build the MLX42 library only if it hasn't been built yet
$(MLX_42): mlx_clone
	@if [ ! -f "$(MLX_42)" ]; then \
		echo "Building MLX42..."; \
		cmake $(LIBMLX) -B $(LIBMLX)/build && make -C $(LIBMLX)/build -j4; \
	fi

# Compile object files from source
%.o: %.c | mlx_clone
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ -c $<

# Compile the project and link it with libft and MLX42
$(NAME): $(OBJ) libft/libft.a $(MLX_42)
	$(CC) $(CFLAGS) $(OBJ) libft/libft.a $(MLX_42) -o $(NAME) $(LDFLAGS)	

# Clean object files
clean:
	rm -rf $(OBJ)
	rm -rf $(LIBMLX)/build
	@make -C libft clean
	
# Full clean (removes executable and object files)
fclean: clean
	rm -f $(NAME)
	@make fclean -C libft

# Rebuild everything
re: fclean all

# Bonus target to compile the bonus version without relinking
bonus: $(NAME)

.PHONY: all clean fclean re bonus mlx_clone
