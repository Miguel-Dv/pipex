/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strrchr.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: miggarc2 <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/12 18:19:05 by miggarc2          #+#    #+#             */
/*   Updated: 2024/09/19 21:32:04 by miggarc2         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stddef.h>

char	*ft_strrchr(const char *s, int c)
{
	size_t	rslt_pos;
	size_t	i;

	rslt_pos = 0;
	while (s[rslt_pos])
		rslt_pos++;
	i = 0;
	while (s[i])
		if (s[i++] == (char)c)
			rslt_pos = i - 1;
	if (s[rslt_pos] == (char)c)
		return ((char *)&s[rslt_pos]);
	return (0);
}
