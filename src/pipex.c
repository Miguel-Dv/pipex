/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pipex.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: miggarc2 <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/06 19:02:08 by miggarc2          #+#    #+#             */
/*   Updated: 2025/02/28 01:24:50 by miggarc2         ###   ########.fr       */
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

void	ft_err_chk(t_var *var, char *err1, char *err2, int to_exit_err)
{
	if (err1 || err2)
	{
		ft_putstr_fd("pipex: ", STDERR_FILENO);
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
	if (to_exit_err)
		exit(EXIT_FAILURE);
}

void	ft_exec_child(t_var *var, int i, int end)
{
	if (i == 0)
	{
		close(var->pipes[2 * i]);
		if (var->fd_in && dup2(var->fd_in, STDIN_FILENO) < 0)
			ft_err_chk(var, strerror(errno), "\n", 1);
		if (dup2(var->pipes[2 * i + 1], STDOUT_FILENO) < 0)
			ft_err_chk(var, strerror(errno), "\n", 1);
	}
	else if (i < end)
	{
		close(var->pipes[2 * i]);
		if (dup2(var->pipes[2 * i - 2], STDIN_FILENO) < 0)
			ft_err_chk(var, strerror(errno), "\n", 1);
		if (dup2(var->pipes[2 * i + 1], STDOUT_FILENO) < 0)
			ft_err_chk(var, strerror(errno), "\n", 1);
	}
	else
	{
		if (dup2(var->pipes[2 * i - 2], STDIN_FILENO) < 0)
			ft_err_chk(var, strerror(errno), "\n", 1);
		if (dup2(var->fd_out, STDOUT_FILENO) < 0)
			ft_err_chk(var, strerror(errno), "\n", 1);
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
		ft_err_chk(var, "pipes fd: ", strerror(errno), 1);
	i = -1;
	while (var->cmds[++i])
	{
		if (i < end && pipe(&var->pipes[2 * i]) < 0)
			ft_err_chk(var, "pipes fd: ", strerror(errno), 1);
		child = fork();
		if (child < 0)
			ft_err_chk(var, "child: ", strerror(errno), 1);
		else if (child == 0)
			ft_exec_child(var, i, end);
		else if (i < end)
			close(var->pipes[2 * i + 1]);
		waitpid(child, &status, 0);
		if (!WIFEXITED(status))
			ft_err_chk(var, strerror(errno), "\n", 1);
	}
}

void	ft_open_heredoc(t_var *var, char *limit, int limit_len)
{
	char	*line;
	int		here_fd;

	here_fd = open("here_doc", O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (!here_fd)
		ft_err_chk(var, "here_doc: ", strerror(errno), 1);
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
		if (!var->cmds[i][0])
			ft_err_chk(var, "permission denied", ":\n", 1);
		else if (strncmp(var->cmds[i][0], "/", 1))
		{
			tmp = ft_strjoin(var->paths[j], "/");
			cmd = ft_strjoin(tmp, var->cmds[i][0]);
			free(tmp);
		}
		else
			cmd = ft_strdup(var->cmds[i][0]);
		if (!access(cmd, F_OK))
			break ;
		free(cmd);
	}
	if (!var->paths[j])
		ft_err_chk(var, var->cmds[i][0], ": No such file or directory\n", 1);
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
		ft_err_chk(var, av[1 + var->hdoc], ": no such file or directory\n", 0);
	if (var->hdoc)
		var->fd_out = open(av[ac - 1], O_WRONLY | O_CREAT | O_APPEND, 0644);
	else
		var->fd_out = open(av[ac - 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (var->fd_out < 0)
		ft_err_chk(var, strerror(errno), av[ac - 1], 1);
	var->cmds = (char ***)ft_calloc(ac - 2 - var->hdoc, sizeof(char **));
	if (!var->cmds)
		ft_err_chk(var, "ft_calloc cmds:", strerror(errno), 1);
	i = -1;
	while (++i <= ac - 4 - var->hdoc)
	{
		var->cmds[i] = ft_split(av[i + 2 + var->hdoc], ' ');
		if (!var->cmds[i])
			ft_err_chk(var, "ft_split var->cmds: ", strerror(errno), 1);
		var->cmds[i][0] = ft_access_check(var, i);
	}
}

int	main(int ac, char **av, char **env)
{
	int		i;
	t_var	var;

	if (ac < 5)
		ft_err_chk(NULL, "syntax error: ", \
				"use \'.pipex [here_doc] infile cmd_1... cmd_n outfile\'\n", 1);
	ft_bzero(&var, sizeof(t_var));
	var.hdoc = (_Bool) !strncmp(av[1], "here_doc", 9);
	i = 0;
	while (env[i] && ft_strncmp(env[i], "PATH=", 5))
		i++;
	if (!env[i])
		ft_err_chk(NULL, av[2 + var.hdoc], ": No such file or directory\n", 1);
	var.paths = ft_split(env[i] + 5, ':');
	if (!var.paths)
		ft_err_chk(&var, "ft_split paths: ", strerror(errno), 1);
	ft_start_args(&var, av, ac);
	ft_pipex(&var, ac - 4 - var.hdoc);
	ft_err_chk(&var, NULL, NULL, 1);
}
