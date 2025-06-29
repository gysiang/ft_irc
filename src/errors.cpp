/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   errors.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gyong-si <gyong-si@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/12 12:36:35 by gyong-si          #+#    #+#             */
/*   Updated: 2025/05/18 10:44:29 by gyong-si         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/errors.hpp"

std::string RPL_NOTOPIC(const std::string &serverName, const std::string &clientNick, const std::string &channelName)
{
	return (":" + serverName + " 331 " + clientNick + " " + channelName + " :No topic is set" + CRLF);
}

std::string RPL_INVITING(const std::string &serverName, const std::string &clientNick, const std::string &targetNick, const std::string &channelName)
{
	return (":" + serverName + " 341 " + clientNick + " " + targetNick + " " + channelName + CRLF);
}

std::string ERR_UNKNOWNCOMMAND(const std::string &serverName, const std::string &clientNick, const std::string &cmd)
{
	return (":" + serverName + " 421 " + clientNick + " " + cmd + " :Unknown command" + CRLF);
}

std::string ERR_NOSUCHNICK(const std::string &serverName, const std::string &senderNick, const std::string &targetNick)
{
	return (":" + serverName + " 401 " + senderNick + " " + targetNick + " :No such nick/channel");
}

std::string ERR_NOSUCHCHANNEL(const std::string &serverName, const std::string &clientNick, const std::string &channelName)
{
	return (":" + serverName + " 403 " + clientNick + " " + channelName + " :No such channel" + CRLF);
}

std::string ERR_CANNOTSENDTOCHAN(const std::string &serverName, const std::string &clientNick, const std::string &channelName)
{
	return (":" + serverName  + " 404 " + clientNick + " " + channelName + " :Cannot send to channel" + CRLF);
}

std::string ERR_USERNOTINCHANNEL(const std::string &serverName, const std::string &clientNick, const std::string &targetNick, const std::string &channelName)
{
	return (":" + serverName + " 441 " + clientNick + " " + targetNick + " " + channelName + " :They aren't on that channel" + CRLF);
}

std::string ERR_NOTONCHANNEL(const std::string &serverName, const std::string &clientNick, const std::string &channelName)
{
	return (":" + serverName + " 442 " + clientNick + " " + channelName + " :You're not on that channel" + CRLF);
}

std::string ERR_USERONCHANNEL(const std::string &serverName, const std::string &clientNick, const std::string &targetNick, const std::string &channelName)
{
	return (":" + serverName + " 443 " + clientNick + " " + targetNick + " " + channelName + " :is already on channel" + CRLF);
}

std::string ERR_NOTREGISTERED(const std::string &serverName, const std::string &clientNick)
{
	return (":" + serverName + " 451 " + clientNick + " :You have not registered" + CRLF);
}

std::string ERR_NEEDMOREPARAMS(const std::string &serverName, const std::string &clientNick, const std::string &cmd)
{
	return (":" + serverName + " 461 " + clientNick + " " + cmd + " :Not enough parameters" + CRLF);
}

std::string ERR_CHANNELISFULL(const std::string &serverName, const std::string &clientNick, const std::string &channelName)
{
	return (":" + serverName + " 471 " + clientNick + " " + channelName + " :Cannot join channel (+l) (Channel is full)" + CRLF);
}

std::string ERR_INVITEONLYCHAN(const std::string &serverName, const std::string &clientNick, const std::string &channelName)
{
	return (":" + serverName + " 473 " + clientNick + " " + channelName + " :Cannot join channel (+i)" + CRLF);
}

std::string ERR_BADCHANNELKEY(const std::string &serverName, const std::string &clientNick, const std::string &channelName)
{
	return (":" + serverName + " 475 " + clientNick + " " + channelName + " :Cannot join channel (+k) - bad key" + CRLF);
}

std::string ERR_CHANOPRIVSNEEDED(const std::string &serverName, const std::string &clientNick, const std::string &channelName)
{
	return (":" + serverName  + " 482 " + clientNick + " " + channelName + " :You're not channel operator" + CRLF);
}

