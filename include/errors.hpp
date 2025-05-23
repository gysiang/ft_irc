/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   errors.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gyong-si <gyong-si@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/12 12:36:25 by gyong-si          #+#    #+#             */
/*   Updated: 2025/05/18 10:33:31 by gyong-si         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#define once

#include "iostream"

#define CRLF "\r\n"

// 421
std::string ERR_UNKNOWNCOMMAND(const std::string &serverName, const std::string &clientNick, const std::string &cmd);
// 461
std::string ERR_NEEDMOREPARAMS(const std::string &serverName, const std::string &clientNick, const std::string &cmd);
// 401
std::string ERR_NOSUCHNICK(const std::string &serverName, const std::string &senderNick, const std::string &targetNick);
// 403
std::string ERR_NOSUCHCHANNEL(const std::string &serverName, const std::string &clientNick, const std::string &channelName);
// 404
std::string ERR_CANNOTSENDTOCHAN(const std::string &serverName, const std::string &clientNick, const std::string &channelName);
// 442
std::string ERR_NOTONCHANNEL(const std::string &serverName, const std::string &clientNick, const std::string &channelName);
// 473
std::string ERR_INVITEONLYCHAN(const std::string &serverName, const std::string &clientNick, const std::string &channelName);
// 475
std::string ERR_BADCHANNELKEY(const std::string &serverName, const std::string &clientNick, const std::string &channelName);
// 482
std::string ERR_CHANOPRIVSNEEDED(const std::string &serverName, const std::string &clientNick, const std::string &channelName);
