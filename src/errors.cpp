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

// 421
std::string ERR_UNKNOWNCOMMAND(const std::string &serverName, const std::string &clientNick, const std::string &cmd)
{
	return (":" + serverName + " 421 " + clientNick + " " + cmd + " :Unknown command" + CRLF);
}

// 461
std::string ERR_NEEDMOREPARAMS(const std::string &serverName, const std::string &clientNick, const std::string &cmd)
{
	return (":" + serverName + " 461 " + clientNick + " " + cmd + " :Not enough parameters" + CRLF);
}

// 401
std::string ERR_NOSUCHNICK(const std::string &serverName, const std::string &senderNick, const std::string &targetNick)
{
	return (":" + serverName + " 401 " + senderNick + " " + targetNick + " :No such nick/channel");
}


// 403
std::string ERR_NOSUCHCHANNEL(const std::string &serverName, const std::string &clientNick, const std::string &channelName)
{
	return (":" + serverName + " 403 " + clientNick + " " + channelName + " :No such channel" + CRLF);
}

std::string ERR_CANNOTSENDTOCHAN(const std::string &serverName, const std::string &clientNick, const std::string &channelName)
{
	return (":" + serverName  + " 404 " + clientNick + " " + channelName + " :Cannot send to channel" + CRLF);
}


// 442
std::string ERR_NOTONCHANNEL(const std::string &serverName, const std::string &clientNick, const std::string &channelName)
{
	return (":" + serverName + " 442 " + clientNick + " " + channelName + " :You're not on that channel" + CRLF);
}


// 473
std::string ERR_INVITEONLYCHAN(const std::string &serverName, const std::string &clientNick, const std::string &channelName)
{
	return (":" + serverName + " 473 " + clientNick + " " + channelName + " :Cannot join channel (+i)" + CRLF);
}


// 475
std::string ERR_BADCHANNELKEY(const std::string &serverName, const std::string &clientNick, const std::string &channelName)
{
	return (":" + serverName + " 475 " + clientNick + " " + channelName + " :Cannot join channel (+k) - bad key" + CRLF);
}


// 482
std::string ERR_CHANOPRIVSNEEDED(const std::string &serverName, const std::string &clientNick, const std::string &channelName)
{
	return (":" + serverName  + " 482 " + clientNick + " " + channelName + " :You're not channel operator" + CRLF);
}

