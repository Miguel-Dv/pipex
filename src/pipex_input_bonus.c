/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pipex_input_bonus.c                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: miggarc2 <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/06 19:02:08 by miggarc2          #+#    #+#             */
/*   Updated: 2025/02/28 10:56:40 by miggarc2         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "pipex.h"

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
		if (strncmp(var->cmds[i][0], "/", 1))
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

	var->fd_in = open(av[1], O_RDONLY);
	if (var->fd_in < 0)
	{
		ft_err_chk(var, av[1 + var->hdoc], ": no such file or directory\n", 0);
		var->fd_in = open(".empty_file", O_CREAT, O_TRUNC, 0644);
	}
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

	ft_bzero(&var, sizeof(t_var));
	if (ac < 5)
		ft_err_chk(&var, "syntax error: ", \
				"use \'.pipex [here_doc] infile cmd_1... cmd_n outfile\'\n", 1);
	var.hdoc = (_Bool) !strncmp(av[1], "here_doc", 9);
	i = 0;
	while (env[i] && ft_strncmp(env[i], "PATH=", 5))
		i++;
	if (!env[i])
		ft_err_chk(NULL, av[2 + var.hdoc], ": No such file or directory\n", 1);
	var.paths = ft_split(env[i] + 5, ':');
	if (!var.paths)
		ft_err_chk(&var, "ft_split paths: ", strerror(errno), 1);
	if (var.hdoc)
		ft_open_heredoc(&var, av[2], ft_strlen(av[2]));
	ft_start_args(&var, av, ac);
	ft_pipex(&var, ac - 4 - var.hdoc);
	ft_err_chk(&var, NULL, NULL, 1);
}
