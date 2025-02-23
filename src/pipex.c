/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pipex.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: miggarc2 <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/06 19:02:08 by miggarc2          #+#    #+#             */
/*   Updated: 2025/02/23 03:42:29 by miggarc2         ###   ########.fr       */
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
		if (strncmp(args[0], "/", 1))
		{
			tmp = ft_strjoin(*folders++, "/");
			cmd = ft_strjoin(tmp, args[0]);
			free(tmp);
		}
		if (access(cmd, X_OK) < 0)
		{
			free(cmd);
			cmd = NULL;
		}
	}
	if (cmd)
		free(cmd);
	else
		perror(strerror(errno));
	return (args);
}

int	main(int ac, char **av, char **env)
{
	int		i;
	int		status;
	pid_t	child;
	t_var	var;

	if (ac < 5)
		return (0);
	var.fd_in = open(av[1], O_RDONLY);
	var.fd_out = open(av[ac - 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (var.fd_in < 0 || var.fd_out < 0)
		return (0);
	var.cmds = (char ***)ft_calloc(ac - 2, sizeof(char **));
	i = 0;
	if (!env || !env[i])
		return (0);
	while (env[i] && ft_strncmp(env[i], "PATH=", 5))
		i++;
	if (!env[i])
		return (0);
	var.folders = ft_split(env[i] + 5, ':');
	i = -1;
	while (++i < ac - 3)
		var.cmds[i] = ft_cmd_check(var.folders, av[i + 2]);
	i = -1;
	if (var.cmds[++i])
	{
		if (pipe(var.pipe_in) < 0 || pipe(var.pipe_out) < 0)
			return (0);
		child = fork();
		ft_printf("cmd: %s\n", var.cmds[0][0]);
		if (child < 0)
			return (0);
		else if (child == 0)
		{
			close(var.pipe_in[0]);
			if (dup2(var.fd_in, STDIN_FILENO) < 0 || \
					dup2(var.pipe_in[1], STDOUT_FILENO) < 0)
				return (0);
			close(var.pipe_in[1]);
			execve(var.cmds[i][0], var.cmds[i], env);
		}
		else
		{
			waitpid(child, &status, 0);
			close(var.pipe_in[1]);
//			ft_printf("status: %d", status);
			if (dup2(var.pipe_in[0], STDIN_FILENO) < 0 || \
					dup2(var.fd_out, STDOUT_FILENO) < 0)
				return (0);
			i++;
			execve(var.cmds[i][0], var.cmds[i], env);
		}
	}
	ft_clean_mem(&var);
	close(var.fd_in);
	close(var.fd_out);
}
