/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pipex.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: miggarc2 <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/06 19:02:08 by miggarc2          #+#    #+#             */
/*   Updated: 2025/02/25 23:14:43 by miggarc2         ###   ########.fr       */
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
	if (var)
	{
		ft_clean(var);
		if (var->fd_in > 0)
			close(var->fd_in);
		if (var->fd_out > 0)
			close(var->fd_out);
	}
//	if (heredoc)
//		unlink("here_doc");
	if (err1 || err2)
	{
		ft_putstr_fd("pipex: ", STDERR_FILENO);
		ft_putstr_fd(err1, STDERR_FILENO);
		ft_putstr_fd(err2, STDERR_FILENO);
		exit(EXIT_FAILURE);
	}
	exit(EXIT_SUCCESS);
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

	ft_printf("end: %d\n", end);
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

void	ft_open_heredoc(t_var *var, char *limit)
{
	char	*line;
	int		here_fd;

	here_fd = open("here_doc", O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (!here_fd)
		ft_exit(var, "here_doc: ", strerror(errno));
	while(1)
	{
		ft_printf("pipex heredoc> ");
		line = get_next_line(STDIN_FILENO);
		if (!line)
			ft_exit(var, "get_next_line: ", strerror(errno));
		if (!strncmp(line, limit, ft_strlen(limit)))
			close(STDIN_FILENO);
		else
			ft_putstr_fd(line, here_fd);
		free(line);
	}
	close(here_fd);
}

void	ft_cmd_check(t_var *var, char *av, int i)
{
	int		j;
	char	*tmp;
	char	*cmd;

	var->cmds[i] = ft_split(av, ' ');
	if (!var->cmds[i])
		ft_exit(var, "ft_split var->cmds: ", strerror(errno));
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
		if (access(cmd, X_OK) == 0)
			break ;
		else
			free(cmd);
	}
	ft_printf("cmd: %s\n", cmd);
	tmp = var->cmds[i][0];
	var->cmds[i][0] = cmd;
	free(tmp);
	if (!var->paths[j])
		ft_exit(var, var->cmds[i][0], ": No such file or directory\n");
}

int	main(int ac, char **av, char **env)
{
	int		i;
	_Bool	heredoc;
	t_var	var;

	i = 0;
	heredoc = (_Bool)!strncmp(av[1], "heredoc", 9);
	if (ac < 5)
		ft_exit(NULL, "syntax error: ", \
				"use \'.pipex [here_doc] infile cmd_1... cmd_n outfile\'\n");
	while (env[i] && ft_strncmp(env[i], "PATH=", 5))
		i++;
	if (!env[i])
		ft_exit(NULL, av[2 + heredoc], ": No such file or directory\n");
	var.paths = ft_split(env[i] + 5, ':');
	if (!var.paths)
		ft_exit(&var, "ft_split paths: ", strerror(errno));
	var.cmds = (char ***)ft_calloc(ac - 2 - heredoc, sizeof(char **));
	if (!var.cmds)
		ft_exit(&var, "ft_calloc cmds:", strerror(errno));
	i = -1;
	while (++i < ac - 3 - heredoc)
		ft_cmd_check(&var, av[i + 2 + heredoc], i);
	if (heredoc)
		ft_open_heredoc(&var, av[2]);
	var.fd_in = open(av[1], O_RDONLY);
	if (var.fd_in < 0)
		ft_exit(&var, strerror(errno), av[1 + heredoc]);
	if (heredoc)
		var.fd_out = open(av[ac - 1], O_WRONLY | O_CREAT | O_APPEND, 0644);
	else
		var.fd_out = open(av[ac - 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (var.fd_out < 0)
		ft_exit(&var, NULL, av[ac - 1]);
	ft_pipex(&var, ac - 4 - heredoc);
	ft_exit(&var, NULL, NULL);
}
