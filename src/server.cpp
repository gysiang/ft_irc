/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gyong-si <gyong-si@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/18 15:41:53 by gyong-si          #+#    #+#             */
/*   Updated: 2025/05/19 10:54:53 by gyong-si         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/server.hpp"

bool Server::_signal = false;

Server::Server(const std::string &port, const std::string &password)
{
	if (port.empty() || password.empty())
	{
		std::cerr << RED << "Error: Arguments are empty!" << RT << std::endl;
		exit(1);
	}
	if (!isValidPort(port.c_str()))
	{
		std::cerr << RED << "Error: Invalid port number. Must be between 1024 and 65535."
			<< RT << std::endl;
		exit(1);
	}
	if (!isValidPassword(password))
	{
		std::cerr << RED << "Error: Password is invalid. Does it have spaces?"
			<< RT << std::endl;
		exit(1);
	}
	_name = "ircserv";
	_port = std::strtol(port.c_str(), NULL, 10);
	_password = sha256(password);
	_created_time = getFormattedTime();

	// setup the TCP socket
	this->serverInit();
}

Server::~Server()
{
	close(_socket_fd);
	std::cout << "Server with port " << _port << " is shutting down." << std::endl;
}


Server::Server(const Server &src)
{
	*this = src;
}

Server &Server::operator=(const Server &src)
{
	if (this != &src)
	{
		_port = src._port;
		_password = src._password;
	}
	return (*this);
}

const std::string &Server::getName() const
{
	return (_name);
}


Client* Server::getClientByNick(const std::string &clientNick)
{
	for (std::vector<Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it)
	{
		if ((*it)->getNick() == clientNick)
			return (*it);
	}
	return (NULL);
}

void Server::serverInit()
{
	_signal = false;

	// create the TCP socket
	_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (_socket_fd == -1)
	{
		std::cerr << "Error: Could not create socket" << std::endl;
		exit(1);
	}
	// create the epoll fd
	_epoll_fd = epoll_create1(0);
	if (_epoll_fd == -1)
	{
		std::cerr << "Error: Could not create epoll instance\n";
		exit(1);
	}

	// Add the server socket to epoll
	struct epoll_event ev;
	ev.events = EPOLLIN;  // Listening for incoming connections
	ev.data.fd = _socket_fd;

	if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, _socket_fd, &ev) == -1)
	{
		std::cerr << "Error: Could not add server socket to epoll\n";
		exit(1);
	}

	// configure the sockaddr
	memset(&_serverAdd, 0, sizeof(_serverAdd));
	_serverAdd.sin_family = AF_INET;
	_serverAdd.sin_port = htons(_port);
	_serverAdd.sin_addr.s_addr = INADDR_ANY;

	// sets a timeout of the socket
	struct timeval timeout;
	timeout.tv_sec = 1;  // Set timeout to 1 second
	timeout.tv_usec = 0;
	if (setsockopt(_socket_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0)
	{
		std::cerr << "Error: Could not set timeout on accept()" << std::endl;
	}

	// bind the socket to IP and port
	if (bind(_socket_fd, (struct sockaddr *)&_serverAdd, sizeof(_serverAdd)) == -1)
	{
		std::cerr << "Error: Could not bind to port " << _port << std::endl;
		close(_socket_fd);
		exit(1);
	}

	// put the server in listening mode
	// 5 is the backlog number, increase this if needed
	if (listen(_socket_fd, 5) == -1)
	{
		std::cerr << "Error: Could not listen on port " << _port << std::endl;
		close(_socket_fd);
		exit(1);
	}
	std::cout << "Server created on port: " << _port << " with password" << std::endl;
	std::cout << "Server is listening on port " << _port << std::endl;
}

void Server::runServer()
{
	while (1)
	{
		if (_signal)
			break;

		struct epoll_event events[MAX_EVENTS];

		int n = epoll_wait(_epoll_fd, events, MAX_EVENTS, -1);
		if (n == -1)
		{
			if (_signal)
				break;
			std::cerr << "epoll_wait failed\n";
			continue;
		}

		for (int i = 0; i < n; ++i)
		{
			int fd = events[i].data.fd;
			if (fd == _socket_fd)
				handleIncomingNewClient();
			else
				handleClientConnection(fd);
		}
	}
	closeClients();
	close(_socket_fd);
	close(_epoll_fd);
	std::cout << "Shutting down server..." << std::endl;
}

void Server::signalHandler(int signum)
{
	(void)signum;
	std::cout << "\n";
	std::cout << "Signal Received! Stopping server..." << std::endl;
	_signal = true;
}

void	Server::handleIncomingNewClient()
{
	// creates the client fd
	struct sockaddr_in clientAddr;
	socklen_t clientLen = sizeof(clientAddr);
	int client_fd = accept(_socket_fd, (struct sockaddr *)&clientAddr, &clientLen);

	if (client_fd == -1)
	{
		std::cerr << "Error: Failed to accept client" << std::endl;
		return ;
	}

	// get the client ip
	std::string client_ip = inet_ntoa(clientAddr.sin_addr);
	std::cout << "Client connected from: " << client_ip << ":"
		<< ntohs(clientAddr.sin_port) << std::endl;

	// set the client fd to non blocking
	if (setnonblocking(client_fd) == -1)
	{
		close(client_fd);
		return ;
	}

	// Create and store the new client
	Client* newClient = new Client(client_fd, client_ip);

	// this adds the client into the clients list
	_clients.push_back(newClient);

	// this adds the client fd into epoll
	struct epoll_event clientEvent;
	clientEvent.events = EPOLLIN;
	clientEvent.data.fd = client_fd;
	if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, client_fd, &clientEvent) == -1)
	{
		std::cerr << "Failed to add client to epoll: " << strerror(errno) << std::endl;
		close(client_fd);
		return ;
	}
}


void Server::sendWelcome(Client *client)
{
	std::string serverName = this->getName();
	std::string nick = client->getNick();
	int fd = client->getFd();

	sendReply(fd, RPL_WELCOME(serverName, nick));
	sendReply(fd, RPL_YOURHOST(serverName, nick));
	sendReply(fd, RPL_CREATED(serverName, nick, _created_time));
	sendReply(fd, RPL_MYINFO(serverName, nick));

	sendReply(fd, RPL_MOTDSTART(serverName, nick));
	sendReply(fd, RPL_MOTD(serverName, nick));
	sendReply(fd, RPL_ENDOFMOTD(serverName, nick));
}

void	Server::execute_cmd(int fd, std::list<std::string> cmd_lst)
{
	Client* client = getClientByFd(fd);
	const std::string &cmd = cmd_lst.front();

	if (cmd == "PASS")
		// authenticate the user
		handlePass(fd, cmd_lst);
	else if (cmd == "NICK")
		// set the nickname
		handleNick(fd, cmd_lst);
	else if (cmd == "USER")
		handleUser(fd, cmd_lst);
	else if (cmd == "JOIN")
		handleJoin(fd, cmd_lst);
	else if (cmd == "PING")
		handlePing(fd, cmd_lst);
	else if (cmd == "CAP")
		;
	else if (cmd == "MODE")
		handleMode(fd, cmd_lst);
	else if (cmd == "PART")
		handlePart(fd, cmd_lst);
	else if (cmd == "PRIVMSG")
		handlePrivmsg(fd, cmd_lst);
	else
		sendError(fd, ERR_UNKNOWNCOMMAND(getName(), client->getNick(), cmd));
}


void Server::handleClientConnection(int fd)
{
	char buffer[513];
	ssize_t bytesRead = recv(fd, buffer, sizeof(buffer) - 1, 0);

	// if the client quits, recv will receive 0 when closed or -1 when there is error
	if (bytesRead <= 0)
	{
		std::cout << "Client " << fd << " disconnected.\n";
		epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, fd, NULL);
		removeClient(fd);
	}
	else
	{
		buffer[bytesRead] = '\0';
		std::string command(buffer);

		std::cout << "Received from client " << fd << ": " << command << std::endl;

		// break the command from the user into individual str
		// the client can send multiple lines to the user
		// run a loop and break each line to execute it.
		std::istringstream	iss(command);
		std::string			line;

		while (std::getline(iss, line))
		{
			if (!line.empty() && line[line.size() - 1] == '\r')
				line.erase(line.size() - 1, 1);
			//std::cout << line << std::endl;
			std::list<std::string> cmd_lst = splitString(line);
			// function to execute cmd
			if (!cmd_lst.empty())
				execute_cmd(fd, cmd_lst);
		}
	}
}

Client*	Server::getClientByFd(int fd)
{
	for (std::vector<Client*>::iterator it = _clients.begin(); it != _clients.end(); it++)
	{
		if ((*it)->getFd() == fd)
		{
			return (*it);
		}
	}
	return (NULL);
};

Channel* Server::getChannelByName(const std::string &channelName)
{
	for (std::vector<Channel>::iterator it = _channels.begin(); it != _channels.end(); it++)
	{
		if (it->getName() == (channelName))
		{
			return &(*it);
		}
	}
	return (NULL);
}

void	Server::removeChannel(const std::string &channelName)
{
	for (std::vector<Channel>::iterator it = _channels.begin(); it != _channels.end(); ++it)
	{
		if (it->getName() == channelName)
		{
			_channels.erase(it);
			std::cout << "[INFO] Channel \"" << channelName << "\" has been removed." << std::endl;
			return ;
		}
	}
}


// remove the client from the client list
void	Server::removeClient(int fd)
{
	for (std::vector<Client*>::iterator it = _clients.begin(); it != _clients.end(); it++)
	{
		if ((*it)->getFd() == fd)
		{
			close(fd);
			delete *it;
			_clients.erase(it);
			std::cout << "Client " << fd << " removed." << std::endl;
			return ;
		}
	}
}

void	Server::closeClients()
{
	for (std::vector<Client*>::iterator it = _clients.begin(); it != _clients.end(); it++)
	{
		close((*it)->getFd());
	}
	_clients.clear();
	std::cout << "ALL the remaining client Fds are closed." << std::endl;
}

const std::vector<Client*>& Server::getClients() const
{
	return (_clients);
}

//Marcus functions
std::string	Server::modeTo_execute(char opera, char mode)
{
	std::stringstream ss;
	ss.clear();

	if (opera && mode)
		ss << opera << mode;
	return (ss.str());
}
//sets channel to invite only
std::string	Server::invite_only(Channel *targetChannel, char operation, int fd)
{
	std::string	param;
	param.clear();
	(void)fd;

	//Client* client = getClientByFd(fd);
	//having issues trying to get client's name and sending reply:
	//sendreply not appearing in my channel
	if (operation == '+')
		targetChannel->SetInviteOnly(true, fd);//set the channel as invite only
	else if (operation == '-')
		targetChannel->SetInviteOnly(false, fd);
	param = modeTo_execute(operation, 'i');
	if (targetChannel->getchannelIsInviteOnly() == true)
		std::cout << GREEN << "[DEBUG] Channel is invite only now." << RT << std::endl;
	std::cout << YELLOW << "[DEBUG] Current value of channel-> \"" << RED 
		<< targetChannel->getchannelIsInviteOnly() << RT << "\"" << std::endl;
	return (param);
}

std::string	Server::topic_restriction(Channel *targetChannel, char operation, int fd)
{
	std::string	param;
	param.clear();
	(void)fd;

	if (operation == '+')
		targetChannel->setTopicRestriction(true, fd);
	else if (operation == '-')
		targetChannel->setTopicRestriction(false, fd);
	param = modeTo_execute(operation, 't');
	if (targetChannel->getisTopicRestricted() == true)
		std::cout << GREEN << "[DEBUG] Channel has a Topic now." << RT << std::endl;
	std::cout << YELLOW << "[DEBUG] Topic value-> \"" << RED 
		<< targetChannel->getTopic() << RT << "\"" << std::endl;
	return (param);
}
//Marcus functions
