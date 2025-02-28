/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pipex_bonus.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: miggarc2 <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/06 19:02:08 by miggarc2          #+#    #+#             */
/*   Updated: 2025/02/28 10:26:17 by miggarc2         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "pipex_bonus.h"

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
	if (to_exit_err && var)
	{
		ft_clean(var);
		if (var->pipes)
			free(var->pipes);
		if (var->fd_in > 0)
			close(var->fd_in);
		if (var->fd_out > 0)
			close(var->fd_out);
	}
	if (to_exit_err && var->hdoc)
		unlink("here_doc");
	if (to_exit_err && access(".empty_file", F_OK))
		unlink(".empty_file");
	if (to_exit_err)
		exit(EXIT_FAILURE);
}

void	ft_exec_child(t_var *var, int i, int end)
{
	if (i == 0)
	{
		close(var->pipes[2 * i]);
		if (var->fd_in > 0 && dup2(var->fd_in, STDIN_FILENO) < 0)
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
