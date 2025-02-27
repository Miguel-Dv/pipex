/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pipex.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: miggarc2 <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/06 19:02:08 by miggarc2          #+#    #+#             */
/*   Updated: 2025/02/27 01:55:42 by miggarc2         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "pipex.h"

void	ft_clean(t_var *var)
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
	if (var->paths)
	{
		i = 0;
		while (var->paths[i])
			free(var->paths[i++]);
		free(var->paths);
	}
}

void	ft_exit(t_var *var, char *err1, char *err2)
{
	char	*err;

	err = NULL;
	if (err1 || err2)
	{
		err = "pipex: ";
		ft_putstr_fd(err, STDERR_FILENO);
		ft_putstr_fd(err1, STDERR_FILENO);
		ft_putstr_fd(err2, STDERR_FILENO);
	}
	if (var)
	{
		ft_clean(var);
		if (var->pipes)
			free(var->pipes);
		if (var->fd_in > 0)
			close(var->fd_in);
		if (var->fd_out > 0)
			close(var->fd_out);
	}
	if (var->hdoc)
		unlink("here_doc");
	if (err)
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
		ft_exit(var, "pipes fd: ", strerror(errno));
	i = -1;
	while (var->cmds[++i])
	{
		if (i < end && pipe(&var->pipes[2 * i]) < 0)
			ft_exit(var, "pipes fd: ", strerror(errno));
		child = fork();
		if (child < 0)
			ft_exit(var, "child process: ", strerror(errno));
		else if (child == 0)
			ft_exec_child(var, i, end);
		else if (i < end)
			close(var->pipes[2 * i + 1]);
		waitpid(child, &status, 0);
		if (!WIFEXITED(status))
			ft_exit(var, "child process: ", strerror(errno));
	}
}

void	ft_open_heredoc(t_var *var, char *limit, int limit_len)
{
	char	*line;
	int		here_fd;

	here_fd = open("here_doc", O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (!here_fd)
		ft_exit(var, "here_doc: ", strerror(errno));
	while (1)
	{
		ft_putstr_fd("pipex heredoc> ", STDOUT_FILENO);
		line = get_next_line(STDIN_FILENO);
		if (!line)
			break ;
		if (!strncmp(line, limit, limit_len))
		{
			close(STDIN_FILENO);
			free(line);
			line = get_next_line(STDIN_FILENO);
			break ;
		}
		else
			ft_putstr_fd(line, here_fd);
		free(line);
	}
	close(here_fd);
}

char	*ft_access_check(t_var *var, int i)
{
	int		j;
	char	*tmp;
	char	*cmd;

	j = -1;
	while (var->paths[++j])
	{
		if (strncmp(var->cmds[i][0], "/", 1) != 0)
		{
			tmp = ft_strjoin(var->paths[j], "/");
			cmd = ft_strjoin(tmp, var->cmds[i][0]);
			free(tmp);
		}
		else
			cmd = ft_strdup(var->cmds[i][0]);
		if (access(cmd, F_OK) == 0)
			break ;
		else
			free(cmd);
	}
	if (!var->paths[j])
		ft_exit(var, var->cmds[i][0], ": No such file or directory\n");
	free(var->cmds[i][0]);
	return (cmd);
}

void	ft_start_args(t_var *var, char **av, int ac)
{
	int	i;

	if (var->hdoc)
		ft_open_heredoc(var, av[2], ft_strlen(av[2]));
	var->fd_in = open(av[1], O_RDONLY);
	if (var->fd_in < 0)
		ft_exit(var, strerror(errno), av[1 + var->hdoc]);
	if (var->hdoc)
		var->fd_out = open(av[ac - 1], O_WRONLY | O_CREAT | O_APPEND, 0644);
	else
		var->fd_out = open(av[ac - 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (var->fd_out < 0)
		ft_exit(var, NULL, av[ac - 1]);
	var->cmds = (char ***)ft_calloc(ac - 2 - var->hdoc, sizeof(char **));
	if (!var->cmds)
		ft_exit(var, "ft_calloc cmds:", strerror(errno));
	i = -1;
	while (++i <= ac - 4 - var->hdoc)
	{
		var->cmds[i] = ft_split(av[i + 2 + var->hdoc], ' ');
		if (!var->cmds[i])
			ft_exit(var, "ft_split var->cmds: ", strerror(errno));
		var->cmds[i][0] = ft_access_check(var, i);
	}
}

int	main(int ac, char **av, char **env)
{
	int		i;
	t_var	var;

	if (ac < 5)
		ft_exit(NULL, "syntax error: ", \
				"use \'.pipex [here_doc] infile cmd_1... cmd_n outfile\'\n");
	ft_bzero(&var, sizeof(t_var));
	var.hdoc = (_Bool) !strncmp(av[1], "here_doc", 9);
	i = 0;
	while (env[i] && ft_strncmp(env[i], "PATH=", 5))
		i++;
	if (!env[i])
		ft_exit(NULL, av[2 + var.hdoc], ": No such file or directory\n");
	var.paths = ft_split(env[i] + 5, ':');
	if (!var.paths)
		ft_exit(&var, "ft_split paths: ", strerror(errno));
	ft_start_args(&var, av, ac);
	ft_pipex(&var, ac - 4 - var.hdoc);
	ft_exit(&var, NULL, NULL);
}
