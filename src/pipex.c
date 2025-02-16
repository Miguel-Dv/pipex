/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pipex.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: miggarc2 <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/06 19:02:08 by miggarc2          #+#    #+#             */
/*   Updated: 2025/02/16 23:18:46 by miggarc2         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "pipex.h"

void	ft_clean_mem(t_var *var)
{
	int	i;
	int	j;

	if (var->cmds)
	{
		i = 0;
		while (var->cmds[i])
		{
			j = 0;
			while(var->cmds[i][j])
				free(var->cmds[i][j++]);
			free(var->cmds[i++]);
		}
		free(var->cmds);
	}
	if (var->folders)
	{
		i = 0;
		while (var->folders[i])
			free(var->folders[i++]);
		free(var->folders);
	}
	if (var->buff)
		free(var->buff);
}

//void	ft_error_handle(

char	**ft_cmd_check(char **folders, char *av)
{
	char	*tmp;
	char	*cmd;
	char	**args;

	cmd = NULL;
	args = ft_split(av, ' ');
	while (!cmd && *folders)
	{
		tmp = ft_strjoin(*folders++, "/");
		cmd = ft_strjoin(tmp, args[0]);
		free(tmp);
		if (access(cmd, X_OK) < 0)
		{
			free(cmd);
			cmd = NULL;
		}
	}
	if (cmd)
	{
		tmp = args[0];
		args[0] = cmd;
		free(tmp);
	}
	else
		perror(strerror(errno));
	return (args);
}

int	main(int ac, char **av, char **env)
{
	int		i;
	int		child;
	t_var	var;

	if (ac < 5)
		return (0);
	var.fd_in = open(av[1], O_RDONLY);
	var.fd_out = open(av[ac - 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (var.fd_in < 0 || var.fd_out < 0)
		return (0);
	var.cmds = (char ***)ft_calloc(ac - 2, sizeof(char **));
	i = 0;
	while (ft_strncmp(env[i], "PATH=", 5))
		i++;
	var.folders = ft_split(env[i] + 5, ':');
	i = -1;
	while (++i < ac - 3)
		var.cmds[i] = ft_cmd_check(var.folders, av[i + 2]);
	var.buff = (char *)malloc(1024);
	i = -1;
	while (var.cmds[++i])
	{
		if (pipe(var.tube) < 0)
			return (0);
		child = fork();
//		wait(&child);
		if (child < 0)
			return (0);
		else if (child == 0)
		{
			close(var.tube[0]);
			if (dup2(var.tube[1], STDOUT_FILENO) < 0)
				return (-1);
			execve(var.cmds[i][0], var.cmds[i], env);
		}
		else
		{
			close(var.tube[1]);
			read(var.tube[0], var.buff, 1024);
			close(var.tube[1]);
//			ft_putstr_fd(var.buff, var.fd_out);
			ft_printf("out: %s", var.buff);
		}
	}
	ft_clean_mem(&var);
	close(var.fd_in);
	close(var.fd_out);
}
