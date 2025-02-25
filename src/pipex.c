/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pipex.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: miggarc2 <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/06 19:02:08 by miggarc2          #+#    #+#             */
/*   Updated: 2025/02/25 01:57:20 by miggarc2         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "pipex.h"

void	ft_clean_mem(t_var *var, char **folders)
{
	int	i;
	int	j;

	if (var->cmds)
	{
		i = 0;
		while (var->cmds[i])
		{
			j = 0;
			while (var->cmds[i][j])
				free(var->cmds[i][j++]);
			free(var->cmds[i++]);
		}
		free(var->cmds);
	}
	if (folders)
	{
		i = 0;
		while (folders[i])
			free(folders[i++]);
		free(folders);
	}
}

//void	ft_error_handle(

void	ft_cmd_check(char ***args, char **folders, char *av)
{
	char	*tmp;
	char	*cmd;

	*args = ft_split(av, ' ');
	while (*(++folders))
	{
		if (strncmp(*args[0], "/", 1) != 0)
		{
			tmp = ft_strjoin(*folders, "/");
			cmd = ft_strjoin(tmp, *args[0]);
			free(tmp);
		}
		else
			cmd = ft_strdup(*args[0]);
		if (access(cmd, X_OK) == 0)
			break ;
		else
			free(cmd);
	}
	tmp = *args[0];
	*args[0] = cmd;
	free(tmp);
	if (!*folders)
		exit(EXIT_FAILURE);
}

void	ft_exec_child(t_var *var, int i, int end)
{
	if (i == 0)
	{
		close(var->pipes[2 * i]);
		dup2(var->fd_in, STDIN_FILENO);
		dup2(var->pipes[2 * i + 1], STDOUT_FILENO);
	}
	else if (i < end)
	{
		close(var->pipes[2 * i]);
		dup2(var->pipes[2 * i - 2], STDIN_FILENO);
		dup2(var->pipes[2 * i + 1], STDOUT_FILENO);
	}
	else
	{
		dup2(var->pipes[2 * i - 2], STDIN_FILENO);
		dup2(var->fd_out, STDOUT_FILENO);
	}
	execve(var->cmds[i][0], var->cmds[i], NULL);
}

void	ft_pipex(t_var *var, int end)
{
	int		i;
	int		status;
	pid_t	child;

	var->pipes = (int *)ft_calloc(end * 2, sizeof(int));
	if (!var->pipes)
		exit(EXIT_FAILURE);
	i = -1;
	while (var->cmds[++i])
	{
		if (i < end && pipe(&var->pipes[2 * i]) < 0)
			exit(EXIT_FAILURE);
		child = fork();
		if (child < 0)
			exit(EXIT_FAILURE);
		else if (child == 0)
			ft_exec_child(var, i, end);
		waitpid(child, &status, 0);
		if (!WIFEXITED(status))
			perror("child");
		if (i < end)
			close(var->pipes[2 * i + 1]);
	}
	free(var->pipes);
}

int	main(int ac, char **av, char **env)
{
	int		i;
	t_var	var;
	char	**folders;

	if (ac < 5)
		return (0);
	var.fd_in = open(av[1], O_RDONLY);
	var.fd_out = open(av[ac - 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (var.fd_in < 0 || var.fd_out < 0)
		return (0);
	var.cmds = (char ***)ft_calloc(ac - 2, sizeof(char **));
	i = 0;
	while (env[i] && ft_strncmp(env[i], "PATH=", 5))
		i++;
	if (!env[i])
		return (0);
	folders = ft_split(env[i] + 5, ':');
	i = -1;
	while (++i < ac - 3)
		ft_cmd_check(&var.cmds[i], folders - 1, av[i + 2]);
	if (var.cmds[0])
		ft_pipex(&var, ac - 4);
	ft_clean_mem(&var, folders);
	close(var.fd_in);
	close(var.fd_out);
}
