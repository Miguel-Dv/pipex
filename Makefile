# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: miggarc2 <miggarc2@student.42barcelona.co  +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2024/09/18 19:51:13 by miggarc2          #+#    #+#              #
#    Updated: 2025/02/06 19:01:50 by miggarc2         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

CC= cc
CFLAGS= -Wall -Wextra -Werror -I .
RM= rm -f
NAME= pipex
INC= includes/pipex.h
LIB= libft/libft.a
SRC= src/pipex.c
OBJ= ${SRC:.c=.o}

%.o: %.c $(INC) Makefile
	$(CC) $(CFLAGS) -c $< -o $@

all: makelibft $(NAME)

makelibft:
	make -C libft

$(NAME): $(OBJ)
	$(CC) $(CFLAGS) $^ $(LIB) -o $@

clean:
	make -C libft clean
	$(RM) $(wildcard src/*.o)

fclean: clean
	make -C libft fclean
	$(RM) $(NAME)

re: fclean all

.PHONY: all clean fclean re
