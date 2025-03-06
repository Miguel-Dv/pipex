# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: miggarc2 <miggarc2@student.42barcelona.co  +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2024/09/18 19:51:13 by miggarc2          #+#    #+#              #
#    Updated: 2025/03/06 21:45:21 by miggarc2         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

CC= cc
CFLAGS= -Wall -Wextra -Werror -I includes -I libft 
RM= rm -f
NAME= pipex
LIB= libft/libft.a
OBJ= ${SRC:.c=.o}
BONUS=

ifdef BONUS
	INC= pipex_bonus.h
	SRC= src/pipex_bonus.c src/pipex_input_bonus.c
else
	INC= pipex.h
	SRC= src/pipex.c src/pipex_input.c
endif

%.o: %.c $(INC)
	$(CC) $(CFLAGS) -c $< -o $@

all: makelibft $(NAME)

bonus:
	@make BONUS=true

makelibft:
	make -C libft

$(NAME): $(OBJ) $(LIB) Makefile
	$(CC) $(CFLAGS) $(OBJ) $(LIB) -o $@

clean:
	make -C libft clean
	$(RM) $(wildcard src/*.o)

fclean: clean
	make -C libft fclean
	$(RM) $(NAME)

re: fclean all

.PHONY: all clean fclean re
