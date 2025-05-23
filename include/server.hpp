/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gyong-si <gyong-si@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/18 15:31:35 by gyong-si          #+#    #+#             */
/*   Updated: 2025/05/19 10:59:49 by gyong-si         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>
#include <string>
#include <cstdlib>
#include <cctype>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <vector>
#include <csignal>
#include <cerrno>
#include <sys/epoll.h>
#include <fcntl.h>
#include <sstream>
#include <cstdio>
#include <ctime>
#include "../include/replies.hpp"
#include "../include/errors.hpp"
#include "../include/client.hpp"
#include "../include/channel.hpp"
#include "../include/utils.hpp"

# define RT		"\033[0m"
# define RED	"\033[31m"
# define YELLOW	"\033[33m"
# define BLUE	"\033[34m"
# define CYAN	"\033[0;36m"
# define GREEN	"\033[32m"
# define MAG	"\e[0;35m"

# define MAX_EVENTS 10

class Client;
class Channel;

class Server
{
	private:
		std::string				_name;
		long 					_port;
		std::string				_password;
		static bool				_signal;
		int						_socket_fd;
		struct sockaddr_in		_serverAdd;
		int 					_epoll_fd;
		std::string				_created_time;
		std::vector<Client*>	_clients;
		std::vector<Channel>	_channels;

		Server(const Server &src);
		Server &operator=(const Server &src);

	public:
		Server(const std::string &port, const std::string &password);
		~Server();
		void		runServer();
		void		serverInit();
		static void signalHandler(int signum);

		// getter
		const std::string &getName() const;
		const std::string &getCreatedTime() const;
		Client* getClientByNick(const std::string &clientNick);

		// add and remove clients
		void		handleIncomingNewClient();
		void		handleClientConnection(int fd);

		// user commands
		void		handlePass(int fd, std::list<std::string> cmd_list);
		void		handleUser(int fd, std::list<std::string> cmd_list);
		void		handleNick(int fd, std::list<std::string> cmd_list);
		void		handleJoin(int fd, std::list<std::string> cmd_list);
		void		handlePart(int fd, std::list<std::string> cmd_list);
		void		handlePrivmsg(int fd, std::list<std::string> cmd_list);
		void		handleMode(int fd, std::list<std::string> cmd_lst);
		void		handlePing(int fd, std::list<std::string> cmd_lst);

		void		sendWelcome(Client *client);

		void		execute_cmd(int fd, std::list<std::string> cmd);
		void		removeClient(int fd);
		const 		std::vector<Client*>& getClients() const;
		Client* 	getClientByFd(int fd);
		void		closeClients();

		// channel
		Channel*	getChannelByName(const std::string &channelName);
		void		removeChannel(const std::string &channelName);

		//mode -marcus-
		std::string	modeTo_execute(char opera, char mode);
		std::string	invite_only(Channel *targetChannel, char operation, int fd);
		std::string	topic_restriction(Channel *targetChannel, char operation, int fd);
};
