/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gyong-si <gyong-si@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/24 14:56:17 by gyong-si          #+#    #+#             */
/*   Updated: 2025/05/16 17:38:05 by gyong-si         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include <iostream>
#include <list>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <iomanip>
#include <sstream>
#include <string>
#include "server.hpp"

class Server;

bool    isValidPort(const char *portStr);
bool	isValidPassword(const std::string password);
void    setupSignalHandler();
std::list<std::string> splitString(std::string &cmd);
void    sendError(int fd, const std::string &message);
void    sendReply(int fd, const std::string &message);
std::string getFormattedTime();
int     setnonblocking(int client_fd);
std::string sha256(const std::string &str);
