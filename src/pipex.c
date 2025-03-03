/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pipex.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: miggarc2 <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/06 19:02:08 by miggarc2          #+#    #+#             */
/*   Updated: 2025/03/03 19:19:42 by miggarc2         ###   ########.fr       */
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

void	ft_err_chk(t_var *var, char *err1, char *err2, int to_exit)
{
	if (err1 || err2)
	{
		ft_putstr_fd("pipex: ", STDERR_FILENO);
		ft_putstr_fd(err1, STDERR_FILENO);
		ft_putstr_fd(err2, STDERR_FILENO);
	}
	if (to_exit)
	{
		if (access("here_doc", F_OK) == 0)
			unlink("here_doc");
		if (access(".empty_file", F_OK) == 0)
			unlink(".empty_file");
		if (var->pipes)
			free(var->pipes);
		if (var->fd_in > 0)
			close(var->fd_in);
		if (var->fd_out > 0)
			close(var->fd_out);
		if (var)
			ft_clean(var);
		exit(to_exit);
	}
}

void	ft_exec_child(t_var *var, int i, int end, char **env)
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
	execve(var->cmds[i][0], var->cmds[i], env);
	ft_err_chk(var, var->cmds[i][0], strerror(errno), errno);
}

int	ft_pipex(t_var *var, int end, char **env)
{
	int		i;
	int		to_exit;
	int		status;
	pid_t	child;

	var->pipes = (int *)ft_calloc(end * 2, sizeof(int));
	if (!var->pipes)
		ft_err_chk(var, strerror(errno), "\n", 1);
	i = -1;
	while (var->cmds[++i])
	{
		if (i < end && pipe(&var->pipes[2 * i]) < 0)
			ft_err_chk(var, strerror(errno), "\n", 1);
		child = fork();
		if (child < 0)
			ft_err_chk(var, strerror(errno), "\n", 1);
		else if (child == 0)
			ft_exec_child(var, i, end, env);
		if (i < end)
			close(var->pipes[2 * i + 1]);
	}
	while (i--)
		if (waitpid(-1, &status, 0) == child && WIFEXITED(status))
			to_exit = WEXITSTATUS(status);
	return (to_exit);
}
