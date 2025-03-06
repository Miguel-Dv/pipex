/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pipex_input.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: miggarc2 <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/06 19:02:08 by miggarc2          #+#    #+#             */
/*   Updated: 2025/03/06 21:39:29 by miggarc2         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "pipex.h"

char	*ft_cmd_resolve(t_var *var, int i)
{
	int		j;
	char	*tmp;
	char	*cmd;

	j = -1;
	while (var->paths[++j])
	{
		if (var->cmds[i][0] && ft_strncmp(var->cmds[i][0], "/", 1))
		{
			tmp = ft_strjoin(var->paths[j], "/");
			cmd = ft_strjoin(tmp, var->cmds[i][0]);
			free(tmp);
		}
		else
			cmd = ft_strdup(var->cmds[i][0]);
		if (access(cmd, F_OK | X_OK) == 0)
			break ;
		free(cmd);
	}
	if (var->paths[j])
		free(var->cmds[i][0]);
	else
		return (var->cmds[i][0]);
	return (cmd);
}

void	ft_start_args(t_var *var, char **av, int ac)
{
	int	i;

	var->fd_in = open(av[1], O_RDONLY);
	if (var->fd_in < 0)
		ft_perror(strerror(errno), ": ", av[1], 1);
	var->fd_out = open(av[ac - 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (var->fd_out < 0)
		ft_perror(strerror(errno), ": ", av[ac - 1], 1);
	var->cmds = (char ***)ft_calloc(ac - 2, sizeof(char **));
	if (!var->cmds)
		ft_exit(var, ft_perror("", strerror(errno), "", errno));
	i = -1;
	while (++i <= ac - 4)
	{
		var->cmds[i] = ft_split(av[i + 2], ' ');
		if (!var->cmds[i])
			ft_exit(var, ft_perror("", strerror(errno), "", errno));
		var->cmds[i][0] = ft_cmd_resolve(var, i);
	}
}

int	main(int ac, char **av, char **env)
{
	int		i;
	t_var	var;

	ft_bzero(&var, sizeof(t_var));
	if (ac != 5)
		ft_exit(&var, ft_perror(" syntax: ", \
				"./pipex infile cmd_1 cmd_2 outfile", "", 1));
	i = 0;
	while (env[i] && ft_strncmp(env[i], "PATH=", 5))
		i++;
	if (!env[i])
		ft_exit(&var, ft_perror("", strerror(errno), "", errno));
	var.paths = ft_split(env[i] + 5, ':');
	if (!var.paths)
		ft_exit(&var, ft_perror("", strerror(errno), "", errno));
	var.pipes = (int *)ft_calloc((ac - 4) * 2, sizeof(int));
	if (!var.pipes)
		ft_exit(&var, ft_perror("", strerror(errno), "", errno));
	ft_start_args(&var, av, ac);
	ft_exit(&var, ft_pipex(&var, ac - 4, env, 0));
}
