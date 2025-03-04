/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pipex.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: miggarc2 <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/06 19:02:08 by miggarc2          #+#    #+#             */
/*   Updated: 2025/03/04 23:47:41 by miggarc2         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "pipex.h"

int	ft_perror(char *err1, char *err2, char *err3, int err_no)
{
	ft_putstr_fd("pipex: ", STDERR_FILENO);
	ft_putstr_fd(err1, STDERR_FILENO);
	ft_putstr_fd(err2, STDERR_FILENO);
	ft_putstr_fd(err3, STDERR_FILENO);
	return (err_no);
}

void	ft_clean(char **var_ptr)
{
	int	i;

	if (var_ptr)
	{
		i = 0;
		while (var_ptr[i])
			free(var_ptr[i++]);
		free(var_ptr);
	}
}

void	ft_exit(t_var *var, int exit_code)
{
	int	i;

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
	i = 0;
	if (var->cmds)
	{
		while (var->cmds[i])
			ft_clean(var->cmds[i++]);
		free(var->cmds);
	}
	if (var->paths)
		ft_clean(var->paths);
	exit(exit_code);
}

void	ft_exec_child(t_var *var, int i, int end, char **env)
{
	if (i == 0)
	{
		close(var->pipes[2 * i]);
		if (var->fd_in > 0 && dup2(var->fd_in, STDIN_FILENO) < 0)
			ft_exit(var, ft_perror("", strerror(errno), "\n", errno));
		if (dup2(var->pipes[2 * i + 1], STDOUT_FILENO) < 0)
			ft_exit(var, ft_perror("", strerror(errno), "\n", errno));
	}
	else if (i < end)
	{
		close(var->pipes[2 * i]);
		if (dup2(var->pipes[2 * i - 2], STDIN_FILENO) < 0)
			ft_exit(var, ft_perror("", strerror(errno), "\n", errno));
		if (dup2(var->pipes[2 * i + 1], STDOUT_FILENO) < 0)
			ft_exit(var, ft_perror("", strerror(errno), "\n", errno));
	}
	else
	{
		if (dup2(var->pipes[2 * i - 2], STDIN_FILENO) < 0)
			ft_exit(var, ft_perror("", strerror(errno), "\n", errno));
		if (dup2(var->fd_out, STDOUT_FILENO) < 0)
			ft_exit(var, ft_perror("", strerror(errno), "\n", errno));
	}
	execve(var->cmds[i][0], var->cmds[i], env);
	ft_exit(var, ft_perror("command not found: ", var->cmds[i][0], "\n", 127));
}

int	ft_pipex(t_var *var, int end, char **env)
{
	int		i;
	int		exit_code;
	int		status;
	pid_t	child;

	i = -1;
	while (var->cmds[++i])
	{
		if (i < end && pipe(&var->pipes[2 * i]) < 0)
			ft_exit(var, ft_perror("", strerror(errno), "\n", errno));
		child = fork();
		if (child < 0)
			ft_exit(var, ft_perror("", strerror(errno), "\n", errno));
		else if (child == 0)
			ft_exec_child(var, i, end, env);
		if (i < end)
			close(var->pipes[2 * i + 1]);
	}
	while (i)
	{
		if (waitpid(-1, &status, 0) == child && WIFEXITED(status))
			exit_code = WEXITSTATUS(status);
		if (--i)
		close(var->pipes[2 * (i - 1)]);
	}
	return (exit_code);
}
