NAME = ft_ping

CC = cc
FLAGS = -Wall -Werror -Wextra

SRC_DIR = srcs/
SRCS = \
	  main.c	\
	  checksum.c	\
	  init.c		\
	  utils.c		\
	  validate.c	\
	  ping_output.c	\
	  ping.c		

OBJ_DIR = objs/
OBJS = $(addprefix $(OBJ_DIR), $(SRCS:.c=.o))

all : $(NAME)

$(NAME) : $(OBJS)
	$(CC) $(FLAGS) $^ -lm -o $@

$(OBJ_DIR)%.o : $(SRC_DIR)%.c | $(OBJ_DIR)
	$(CC) $(FLAGS) -c $^ -o $@

$(OBJ_DIR) :
	@mkdir -p $@

clean :
	@rm -rf $(OBJ_DIR)

fclean : clean
	@rm -rf $(NAME)

re : fclean all
