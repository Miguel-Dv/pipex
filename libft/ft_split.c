/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_split.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: miggarc2 <miggarc2@student.42barcelona.co  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/15 17:54:28 by miggarc2          #+#    #+#             */
/*   Updated: 2024/09/19 00:00:26 by miggarc2         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdlib.h>

static size_t	ft_word_len(char const *s, char c)
{
	size_t	i;

	i = 0;
	while (s[i] && s[i] != c)
		i++;
	return (i);
}

static int	ft_word_count(char const *s, char c)
{
	int	size;

	size = 0;
	while (*s)
	{
		if (*s != c)
		{
			size++;
			while (*s && *s != c)
				s++;
		}
		while (*s == c)
			s++;
	}
	return (size);
}

static int	ft_fill_words(char const *s, char **split, char c, int size)
{
	int		word;
	size_t	len;
	size_t	i;

	word = 0;
	while (*s && word < size)
	{
		while (*s && *s == c)
			s++;
		len = ft_word_len(s, c);
		split[word] = (char *)malloc(sizeof(char) * (len + 1));
		if (!split[word])
		{
			while (--word >= 0)
				free(split[word]);
			return (0);
		}
		i = 0;
		while (i < len)
			split[word][i++] = *s++;
		split[word++][i] = 0;
		s++;
	}
	split[word] = 0;
	return (1);
}

char	**ft_split(char const *s, char c)
{
	char	**split;
	int		size;

	size = 0;
	if (*s && c)
		size = ft_word_count(s, c);
	else if (*s)
		size++;
	split = (char **)malloc(sizeof(char *) * (size + 1));
	if (!split)
		return (0);
	if (!ft_fill_words(s, split, c, size))
	{
		free(split);
		return (0);
	}
	return (split);
}
