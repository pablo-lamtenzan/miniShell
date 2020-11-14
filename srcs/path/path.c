#include <path.h>

static char	*path_cat(const char *a, const char *b)
{
	const size_t	len = ft_strlen(a) + ft_strlen(b) + 1;
	char			*cat;
	char			*c;

	if (len > PATH_MAX || !(cat = ft_calloc(len + 1, sizeof(*cat))))
		return (NULL);
	c = cat;
	while (*a)
		*c++ = *a++;
	*c++ = '/';
	while (*b)
		*c++ = *b++;
	return (cat);
}

char	*path_get(const char *name, const char *path)
{
	char		**paths;
	char		*absolute;
	struct stat	s;
	int			i;

	absolute = NULL;
	if (name && *name)
	{
		if (*name == '/' || *name == '.')
		{
			if ((stat(name, &s) == 0 && s.st_mode & S_IXUSR))
				absolute = ft_strdup(name);
		}
		else if ((paths = ft_split(path, ':')))
		{
			i = 0;
			while (paths[i]
			&& (absolute = path_cat(paths[i++], name))
			&& !(stat(absolute, &s) == 0 && s.st_mode & S_IXUSR))
				free(absolute);
			if (!paths[i])
				absolute = NULL;
			strs_unload(paths);
		}
	}
	return (absolute);
}
