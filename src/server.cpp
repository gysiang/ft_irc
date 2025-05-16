/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gyong-si <gyong-si@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/18 15:41:53 by gyong-si          #+#    #+#             */
/*   Updated: 2025/05/16 17:28:48 by gyong-si         ###   ########.fr       */
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

	// set the client fd to non blocking
	//int flags = fcntl(client_fd, F_GETFL, 0);
	//fcntl(client_fd, F_SETFL, flags | O_NONBLOCK);

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

/**
 * handles the client's password and checks if it matches with the server.
 */
void Server::handlePass(int fd, std::list<std::string> cmd_list)
{
	Client *client = getClientByFd(fd);
	if (!client)
		return ;
	if (cmd_list.size() != 2)
	{
		// need to change this?
		send(fd, "ERROR :PASS command requires exactly one argument\r\n", 50, 0);
		return ;
	}
	// iterate to the second item in the list
	std::list<std::string>::const_iterator it = cmd_list.begin();
	++it;
	const std::string &provided = *it;
	if (_password == sha256(provided))
	{
		client->authenticate();
		send(fd, "NOTICE AUTH :Password accepted\r\n", 33, 0);
		std::cout << "Client " << fd << " : has been authenticated.\n";
	}
	else
		send(fd, "ERROR :Invalid password\r\n", 26, 0);
}

void Server::handleNick(int fd, std::list<std::string> cmd_list)
{
	Client *client = getClientByFd(fd);
	if (!client)
		return ;
	if (cmd_list.size() != 2)
	{
		// I need to change this
		send(fd, "ERROR :PASS command requires exactly one argument\r\n", 50, 0);
		return ;
	}
	// check if user has authenticated.
	if (!client->is_authenticated())
	{
		const std::string &errorMsg = "ERROR :You must authenticate with PASS first\r\n";
		sendError(fd, errorMsg);
		std::cout << "[WARN] Client " << fd << " tried to send NICK/USER before PASS\r\n";
		return ;
	}
	// iterate to the second item in the list
	// /NICK nickname
	std::list<std::string>::const_iterator it = cmd_list.begin();
	++it;
	std::string newNick = *it;
	// check if the nickname has a max of 9 characters

	if (newNick.length() > 9)
	{
		sendError(fd, "ERROR :Nickname too long\r\n");
		return;
	}
	std::string oldNick = client->getNick();
	client->set_nick(newNick);
	std::cout << "[NICK] " << oldNick << " changed to " << newNick << std::endl;
	std::string nickReply = ":" + oldNick + "!" + client->getUserName() + "@" +
	client->getHostName() + " NICK :" + newNick + CRLF;
	sendReply(fd, nickReply);
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

/**
 * Sets the user's nick, username and hostname
 */
void Server::handleUser(int fd, std::list<std::string> cmd_list)
{
	Client *client = getClientByFd(fd);
	if (!client)
		return ;
	if (cmd_list.size() != 5)
	{
		const std::string &errorMsg = "ERROR :USER command requires three argument\r\n";
		sendError(fd, errorMsg);
		return ;
	}
	// check if user has authenticated.
	if (!client->is_authenticated())
	{
		const std::string &errorMsg = "ERROR :You must authenticate with PASS first\r\n";
		sendError(fd, errorMsg);
		std::cout << "[WARN] Client " << fd << " tried to send NICK/USER before PASS\n";
		return ;
	}
	// iterate to the second item in the list
	// /USER username something hostname
	std::list<std::string>::const_iterator it = cmd_list.begin();
	++it;
	std::string username = *it;
	//std::cout << second << std::endl;
	client->set_username(username);
	++it;++it;
	std::string hostname = *it;
	//std::cout << fourth << std::endl;

	client->set_hostname(hostname);
	std::cout << "[USER] : " << username << ", " << "[USER] : "<< hostname  << std::endl;
	// check if all is set, return a message
	if (!client->getNick().empty() && !client->getUserName().empty()
		&& !client->getHostName().empty() && !client->is_registered())
	{
		client->register_client();
		sendWelcome(client);
	}
}

/**
 * Helps user join a channel. If channel is not created on server, this creates the new channel.
 * For now we create channels that does not require passwords
 */
void Server::handleJoin(int fd, std::list<std::string> cmd_list)
{
	Client *client = getClientByFd(fd);
	if (!client)
		return ;
	if (cmd_list.size() != 2)
	{
		// need to change this?
		send(fd, "ERROR :JOIN command requires exactly one argument\r\n", 50, 0);
		return ;
	}
	// extract the channel name
	std::list<std::string>::const_iterator it = cmd_list.begin();
	++it;
	const std::string channelName = *it;

	// iterate over _channels to search if the channel already exist
	Channel *channel = getChannelByName(channelName);

	// if the channel does not exit, create the channel
	if (!channel)
	{
		// create a new channel with the name given
		_channels.push_back(Channel(channelName));
		// get a reference to the channel created
		channel = &_channels.back();
		// add the client as operator
		channel->addOperator(client);
		// server displays message to show new channel is created
		std::cout << "[INFO] New channel " << channelName
				  << " created by " << client->getNick() << "\r\n";

		std::string clientList = channel->getClientList();
		// send all the message together to irssi
		sendReply(fd,
		RPL_JOINMSG(client->getNick(), client->getUserName(), client->getHostName(), channelName) +
		RPL_TOPIC(getName(), client->getNick(), channelName, channel->getTopic()) +
		RPL_NAMEREPLY(getName(), client->getNick(), channelName, clientList) +
		RPL_ENDOFNAMES(getName(), client->getNick(), channelName));
		// broadcast the join mesasge to all others except user
		channel->broadcast(RPL_JOINMSG(client->getNick(), client->getUserName(), client->getHostName(), channelName), client);
	}
	else
	{
		// this channel already exist, add the client as member
		// need to check if client is already a member
		if (!channel->isMember(client))
		{
			channel->addMember(client);

			std::cout << "[JOIN] " << client->getNick() << " joined " << channelName << "\n";
			std::cout << "[USERS] " << channel->getClientList() << "\n";

			std::string clientList = channel->getClientList();

			// send all the message together to irssi
			sendReply(fd,
			RPL_JOINMSG(client->getNick(), client->getUserName(), client->getHostName(), channelName) +
			RPL_TOPIC(getName(), client->getNick(), channelName, channel->getTopic()) +
			RPL_NAMEREPLY(getName(), client->getNick(), channelName, clientList) +
			RPL_ENDOFNAMES(getName(), client->getNick(), channelName));
			std::cout << RPL_JOINMSG(client->getNick(), client->getUserName(), client->getHostName(), channelName) << std::endl;
			// broadcast the join mesasge to all others except user
			channel->broadcast(RPL_JOINMSG(client->getNick(), client->getUserName(), client->getHostName(), channelName), client);
		}
	}
}

void	Server::handlePing(int fd, std::list<std::string> cmd_lst)
{
	if (cmd_lst.size() < 2)
	{
		sendError(fd, "ERROR :No ping argument\r\n");
		return;
	}
	std::list<std::string>::iterator it = cmd_lst.begin();
	++it;
	std::string token = *it;
	sendReply(fd, RPL_PONG(token));
	std::cout << "[PING] Replied with: " << RPL_PONG(token);
}



void	Server::handlePart(int fd, std::list<std::string> cmd_list)
{
	Client *client = getClientByFd(fd);
	if (!client)
	{
		std::cout << "[PART] No client found for fd: " << fd << std::endl;
		return ;
	}
	// if there are only one param, throw error
	if (cmd_list.size() == 1)
	{
		sendError(fd, ERR_NEEDMOREPARAMS(getName(), client->getNick(), cmd_list.front()));
		std::cout << "[PART] Not enough parameters" << std::endl;
		return ;
	}
	// check if channel exit
	// extract the channel name
	std::list<std::string>::const_iterator it = cmd_list.begin();
	++it;
	const std::string channelName = *it;
	std::string reason = "";
	if (cmd_list.size() > 2)
	{
		++it;
		while (it != cmd_list.end())
		{
			if (!reason.empty())
			{
				reason += " ";
			}
			reason += *it;
			++it;
		}
	}
	std::cout << "Reason" << std::endl;
	std::cout << reason << std::endl;
	// iterate over _channels to search if the channel already exist
	Channel *channel = getChannelByName(channelName);

	if (!channel)
	{
		sendError(fd, ERR_NOSUCHCHANNEL(getName(), client->getNick(), channelName));
		std::cout << "[PART] Channel " << channelName << " not found" << std::endl;
		return;
	}
	if (!channel->isMember(client) && !channel->isOperator(client))
	{
		sendError(fd, ERR_NOTONCHANNEL(getName(), client->getNick(), channelName));
		std::cout << "[PART] Client not in channel " << channelName << std::endl;
		return;
	}

	// may need include the parting message

	std::string partMsg = ":" + client->getPrefix() + " PART " + channelName;
	if (!reason.empty())
		partMsg += " " + reason;
	else
		partMsg + " :";
	partMsg += "\r\n";

	std::cout << "DEBUG PART MSG: [" << partMsg << "]" << std::endl;
	sendReply(client->getFd(), partMsg);
	channel->broadcast(partMsg, client);
	// remove the member/operator from the channel;
	channel->removeUser(client);

	// after the user has been removed, broad the message to other users
	// if there is no more users in the channel, remove it from the channel vector in the server
	if (channel->getUsers().empty())
	{
		removeChannel(channel->getName());
		sendError(fd, ERR_NOSUCHCHANNEL(getName(), client->getNick(), channelName));
	}
	else
		sendError(fd, ERR_NOTONCHANNEL(getName(), client->getNick(), channelName));
}

void	Server::handlePrivmsg(int fd, std::list<std::string> cmd_list)
{
	Client *client = getClientByFd(fd);
	if (!client)
	{
		std::cout << "[PRIVMSG] No client found for fd: " << fd << std::endl;
		return ;
	}
	// if there are only one param, throw error
	if (cmd_list.size() < 3)
	{
		sendError(fd, ERR_NEEDMOREPARAMS(getName(), client->getNick(), cmd_list.front()));
		std::cout << "[PRIVMSG] Not enough parameters" << std::endl;
		return ;
	}
	std::list<std::string>::iterator it = cmd_list.begin();
	++it;
	std::string target = *it++;
	std::string message;
	// add all the strings into message
	while (it != cmd_list.end())
	{
		if (!message.empty())
			message += " ";
		message += *it++;
	}
	if (message[0] == ':')
		message = message.substr(1);

	if (target[0] == '#') {
		std::cout << "[DEBUG] Message is for channel: " << target << std::endl;
	} else {
		std::cout << "[DEBUG] Message is for user: " << target << std::endl;
	}

	if (target[0] == '#')
	{
		Channel *channel = getChannelByName(target);
		if (!channel)
		{
			sendError(fd, ERR_NOSUCHCHANNEL(getName(), client->getNick(), channel->getName()));
			return ;
		}
		if (!channel->isMember(client))
		{
			sendError(fd, ERR_CANNOTSENDTOCHAN(getName(), client->getNick(), channel->getName()));
			return ;
		}
		std::string out = ":" + client->getPrefix() + " PRIVMSG " + target + " :" + message + CRLF;
		std::cout << out << std::endl;
		channel->broadcast(out, client);
	}
	else
	{
		// find the target in the server _clients
		Client* targetUser = getClientByNick(target);
		if (!targetUser)
		{
			std::cerr << "[DEBUG] No such user: " << target << std::endl;
			ERR_NOSUCHNICK(getName(), client->getNick(), target);
			return ;
		}
		std::cout << "[DEBUG] Target nick: " << targetUser->getNick() << std::endl;
		std::string out = ":" + client->getPrefix() + " PRIVMSG " + target + " :" + message + CRLF;
		std::cout << out << std::endl;
		// send the message to the target user
		sendReply(targetUser->getFd(), out);
	}
}



void	Server::execute_cmd(int fd, std::list<std::string> cmd_lst)
{
	Client* client = getClientByFd(fd);
	const std::string &cmd = cmd_lst.front();
	//const std::string &errorMsg = "ERROR :Unknown command\r\n";
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
		;
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
	std::cout << "ALl the remaining client Fds are closed." << std::endl;
}

const std::vector<Client*>& Server::getClients() const
{
	return (_clients);
}
