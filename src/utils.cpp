/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gyong-si <gyong-si@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/24 14:56:47 by gyong-si          #+#    #+#             */
/*   Updated: 2025/04/24 21:30:08 by gyong-si         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/utils.hpp"

void setupSignalHandler()
{
	signal(SIGINT, Server::signalHandler);
	signal(SIGQUIT, Server::signalHandler);
}

bool isValidPort(const char *portStr)
{
	for (size_t i = 0; portStr[i]; i++)
		if (!isdigit(portStr[i]))
			return false;

	long portNum = std::strtol(portStr, NULL, 10);

	if (portNum < 1024 || portNum > 65535)
		return false;
	return true;
}

std::list<std::string> splitString(std::string &cmd)
{
	std::list<std::string> lst;
	std::istringstream stm(cmd);
	std::string token;

	while (stm >> token)
	{
		lst.push_back(token);
		token.clear();
	}
	return (lst);
}

void sendError(int fd, const std::string &message)
{
	if (send(fd, message.c_str(), message.size(),0) == -1)
		std::cerr << "Response sent" << std::endl;
}
