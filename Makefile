# Name of the executable
NAME = cub3D

# Compiler and flags
CC = cc
CFLAGS = -Wall -Wextra -g -O2 \
	-I/opt/homebrew/include \
	-I/opt/homebrew/opt/curl/include \
	-I./includes \
	-I./MLX42/include/MLX42

# Linker flags
LDFLAGS = \
	-L/opt/homebrew/lib \
	-lglfw -pthread -lm -lcurl -lcjson \
	-L"./MLX42/include/MLX42" \
	-L"./includes" \
	-L"/opt/homebrew/Cellar/glfw/3.4/lib/"

# Libraries
LIBMLX = ./MLX42
MLX_42 = $(LIBMLX)/build/libmlx42.a

# Include paths
INCLUDES = -Iincludes -I$(LIBMLX)/include

# Source files
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

# --- STB Image headers download ---
STB_DIR := includes
STB_HEADERS := \
	$(STB_DIR)/stb_image.h \
	$(STB_DIR)/stb_image_write.h

$(STB_HEADERS):
	@mkdir -p $(STB_DIR)
	@if [ ! -f $(STB_DIR)/stb_image.h ]; then \
		curl -L https://raw.githubusercontent.com/nothings/stb/master/stb_image.h \
		-o $(STB_DIR)/stb_image.h; \
	fi
	@if [ ! -f $(STB_DIR)/stb_image_write.h ]; then \
		curl -L https://raw.githubusercontent.com/nothings/stb/master/stb_image_write.h \
		-o $(STB_DIR)/stb_image_write.h; \
	fi

# Ensure movement.o waits on STB headers
src/movement.o: $(STB_HEADERS)

# Rule for building the final executable
all: $(NAME)

# Compile libft library
libft/libft.a:
	make -C ./libft
	make bonus -C ./libft

# Clone MLX42 if missing
mlx_clone:
	@if [ ! -d "$(LIBMLX)" ]; then \
		echo "Cloning MLX42..."; \
		git clone https://github.com/codam-coding-college/MLX42.git $(LIBMLX); \
	fi

# Build MLX42 library
$(MLX_42): mlx_clone
	@if [ ! -f "$(MLX_42)" ]; then \
		echo "Building MLX42..."; \
		cmake $(LIBMLX) -B $(LIBMLX)/build && make -C $(LIBMLX)/build -j4; \
	fi

# Compile object files
%.o: %.c | mlx_clone
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ -c $<

# Link executable
$(NAME): $(OBJ) libft/libft.a $(MLX_42)
	$(CC) $(CFLAGS) $(OBJ) libft/libft.a $(MLX_42) -o $(NAME) $(LDFLAGS)

# Clean object files
clean:
	rm -rf $(OBJ)
	rm -rf $(LIBMLX)/build
	make -C libft clean

# Full clean
fclean: clean
	rm -f $(NAME)
	make fclean -C libft

# Rebuild
re: fclean all

# Bonus
bonus: $(NAME)

.PHONY: all clean fclean re bonus mlx_clone