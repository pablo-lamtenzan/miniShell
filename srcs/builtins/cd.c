/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cd.c                                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: chamada <chamada@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/09/22 21:57:11 by plamtenz          #+#    #+#             */
/*   Updated: 2020/11/14 01:33:48 by chamada          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <execution.h>

// TODO: Check allocation errors on map_set

int		ft_cd(t_exec *args, t_term *t)
{
	const char	*home_dir;
	char		path[PATH_MAX];
	char		cwd[PATH_MAX];

	if (args->ac == 1)
	{
		if (!(home_dir = env_get(t->env, "HOME", 4)))
		{
			ft_dprintf(2, "%s: %s: HOME not set\n", t->name, args->av[0]);
			return (1);
		}
		if (chdir(home_dir))
		{
			ft_dprintf(2, "%s: %s: %s\n", t->name, args->av[0], strerror(errno));
			return (1);
		}
		env_set(&t->env, "PWD", home_dir, true);
		return (0);
	}
	else
	{
		ft_bzero(path, PATH_MAX);
		ft_memcpy(path, args->av[1], ft_strlen(args->av[1]));
		if (path[0] == '/' && chdir(path) == 0)
		{
			env_set(&t->env, "PWD", path, true);
			return (0);
		}
		else
		{
			if (!(getcwd(cwd, sizeof(cwd))))
				return (1);
			ft_strlcat(cwd, "/", ft_strlen(cwd) + 2);
			ft_strlcat(cwd, path, ft_strlen(cwd) + 2 + ft_strlen(path));
			if (chdir(cwd) == 0)
			{
				getcwd(cwd, sizeof(cwd));
				env_set(&t->env, "PWD", cwd, true);
				return (0);
			}
			else
				ft_dprintf(2, "cd: no such file or directory: %s\n", args->av[1]);
		}
	}
	return (1);
}
