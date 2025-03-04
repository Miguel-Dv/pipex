/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pipex.h                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: miggarc2 <miggarc2@student.42barcelona.co  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/29 17:10:44 by miggarc2          #+#    #+#             */
/*   Updated: 2025/03/04 20:11:08 by miggarc2         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PIPEX_H
# define PIPEX_H

# include "libft.h"
# include <unistd.h>
# include <stdio.h>
# include <fcntl.h>
# include <unistd.h>
# include <sys/wait.h>
# include <string.h>
# include <errno.h>

typedef struct s_var
{
	_Bool	hdoc;
	int		*pipes;
	int		fd_in;
	int		fd_out;
	char	**paths;
	char	***cmds;
}			t_var;

void	ft_exit(t_var *var, int exit_code);
int		ft_perror(char *err1, char *err2, char *err3, int err_no);
int		ft_pipex(t_var *var, int end, char **env);

#endif
