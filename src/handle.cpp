/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   handle.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gyong-si <gyong-si@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/19 10:19:08 by gyong-si          #+#    #+#             */
/*   Updated: 2025/05/19 10:54:46 by gyong-si         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/server.hpp"

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

// handle /NICK command, and checks if password has been provided first then assign nick
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

/**
 * Sets the user's nick, username and hostname
 */
void Server::handleUser(int fd, std::list<std::string> cmd_list)
{
	Client *client = getClientByFd(fd);
	if (!client)
		return ;
	if (cmd_list.size() < 5)
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
	client->set_username(username);
	++it;++it;
	std::string hostname = *it;

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

	if (cmd_list.size() < 2 || cmd_list.size() > 3)
	{
		send(fd, "ERROR :JOIN command requires 1 or 2 arguments\r\n", 47, 0);
		return ;
	}
	// extract the channel name
	std::list<std::string>::const_iterator it = cmd_list.begin();
	++it;
	const std::string channelName = *it;
	//check if there is a password provided by user
	std::string password = "";
	if (++it != cmd_list.end())
		password = *it;
	Channel *channel = getChannelByName(channelName);

	// get all the variable to avoid calling again and again.
	std::string serverName = this->getName();
	std::string userNick = client->getNick();
	std::string userName = client->getUserName();
	std::string userHost = client->getHostName();
	// if the channel does not exit, create the channel
	if (!channel)
	{
		// create a new channel with the name given
		_channels.push_back(Channel(channelName, password));
		// get a reference to the channel created
		channel = &_channels.back();
		// add the client as operator
		channel->addOperator(client);
		// server displays message to show new channel is created
		std::cout << "[INFO] New channel " << channelName
				  << " created by " << client->getNick() << "\r\n";
		if (!password.empty())
			std::cout << channelName << " is a password protected channel" << "\r\n";
		std::string clientList = channel->getClientList();
		// send all the message together to irssi
		sendReply(fd,
		RPL_JOINMSG(userNick, userName, userHost, channelName) +
		RPL_TOPIC(serverName, userNick, channelName, channel->getTopic()) +
		RPL_CREATIONTIME(serverName, userNick, channelName, channel->getCreationTime()) +
		RPL_NAMEREPLY(serverName, userNick, channelName, clientList) +
		RPL_ENDOFNAMES(serverName, userNick, channelName));
		// broadcast the join mesasge to all others except user
		channel->broadcast(RPL_JOINMSG(userNick, userName, userHost, channelName), client);
	}
	else
	{
		// this channel already exist, add the client as member
		// need to check if client is already a member
		if (!channel->isMember(client))
		{
			// check if the person channel is invite only and if the person is in _invitelist
			if (channel->getchannelIsInviteOnly() == true)
			{
				std::cout << RED << "[DEBUG] I am channel is invite only!" << RT << std::endl;
				if (channel->getisClientInvited(fd) == false)
				{
					std::cout << RED << "Error: " << userNick 
						<< " tried to join but was not invited" << RT << std::endl;
					sendError(fd, ERR_INVITEONLYCHAN(serverName, userNick, channelName));
					return ;
				}
			}
			// check if there is password and if the password provided is the same
			if (channel->hasPassword() && channel->getPassword() != password)
			{
				std::cout << "Error: " << userNick << " tried to join without a correct password" << std::endl;
				sendError(fd, ERR_BADCHANNELKEY(serverName, userNick, channelName));
				return ;
			}
			channel->addMember(client);

			std::cout << "[JOIN] " << userNick << " joined " << channelName << "\n";
			std::cout << "[USERS] " << channel->getClientList() << "\n";

			std::string clientList = channel->getClientList();
			// send all the message together to irssi
			sendReply(fd,
				RPL_JOINMSG(userNick, userName, userHost, channelName) +
				RPL_TOPIC(serverName, userNick, channelName, channel->getTopic()) +
				RPL_CREATIONTIME(serverName, userNick, channelName, channel->getCreationTime()) +
				RPL_NAMEREPLY(serverName, userNick, channelName, clientList) +
				RPL_ENDOFNAMES(serverName, userNick, channelName));
			std::cout << RPL_JOINMSG(userNick, userName, userHost, channelName) << std::endl;
			// broadcast the join mesasge to all others except user
			channel->broadcast(RPL_JOINMSG(userNick, userName, userHost, channelName), client);
		}
	}
}

//handles the "MODE" while in the channel
void	Server::handleMode(int fd, std::list<std::string> cmd_lst)
{
	Client	*client = getClientByFd(fd);
	if (!client)
		return ;
	if (cmd_lst.size() != 3)
	{
		std::cout << RED << "[DEBUG] cmd_lst.size() = " << cmd_lst.size() << RT << std::endl;
		return;
	}

	std::list<std::string>::iterator	it = cmd_lst.begin();
	++it;

	//checks that you wrote '#' as well as channel name to it:
	std::string	hash_and_channelName;
	hash_and_channelName.clear();
	if (!(*it).empty())
		hash_and_channelName = *it;
	if (hash_and_channelName.empty() || hash_and_channelName[0] != '#')
		return;

	//find the channel
	Channel	*targetChannel = NULL;
	for (size_t i = 0; i < _channels.size(); ++i)
	{
		std::cout << "[DEBUG] Comparing _channel: " << GREEN << _channels[i].getName() << RT << std::endl;
		if (_channels[i].getName() == hash_and_channelName)
		{
			targetChannel = &_channels[i];
			std::cout << "[DEBUG] FOUND CHANNEL = " << GREEN << targetChannel->getName() << RT << std::endl;
			break;
		}
	}

	//checking if channel exists
	if (!targetChannel)
	{
		std::cout << GREEN << "[DEBUG] No such channel friend." << RT << std::endl;
		sendReply(fd, ERR_NOSUCHCHANNEL(getName(), client->getNick(), hash_and_channelName));
		return;
	}

	//Checking the membership and operator status
	if (!targetChannel->isMember(client))
	{
		std::cout << GREEN << "[DEBUG] Member is not in channel." << RT << std::endl;
		sendReply(fd, ERR_NOTONCHANNEL(getName(), client->getNick(), targetChannel->getName()));
		return;
	}
	if (!targetChannel->isOperator(client))
	{
		std::cout << GREEN << "[DEBUG] U ain't the operator. GET OUTTA HERE!" << RT << std::endl;
		sendReply(fd, ERR_CHANOPRIVSNEEDED(getName(), client->getNick(), targetChannel->getName()));
		return;
	}

	++it;
	//checking the third argument requirements
	std::string	modeCommand;
	if (!(*it).empty())
		modeCommand = *it;
	if (modeCommand.empty() && modeCommand.size() != 2)
	{
		//command for mode must be "+i" or "-i"
		std::cout << RED << "[DEBUG] Your Command for mode is not enough or empty." << RT << std::endl;
		return;
	}

	char	operation = '\0';
	std::stringstream	mode_chain;
	mode_chain.clear();

	std::cout << YELLOW << "[DEBUG] IF THE VALUE OF operation is = " << operation << RT << std::endl;
	for (size_t i = 0; i < modeCommand.size(); i++)
	{
		if (!modeCommand.empty() && (modeCommand[i] == '+' || modeCommand[i] == '-'))
		{
			//this is in case the first array, modeCommand[1] on onwards still has "+/-"
			operation = modeCommand[i];
			continue ;
		}
		if (i != 0 && modeCommand[i] && operation)
		{
			std::cout << YELLOW << "[DEBUG] (operation = \"" RT << operation << YELLOW 
				<< "\" | Value of this = \"" << RT << modeCommand[i] 
				<< YELLOW << "\")" << RT << std::endl;
			// [i] invite
			if (modeCommand[i] == 'i')
				mode_chain << invite_only(targetChannel , operation, fd);
			else if (modeCommand[i] == 't') //topic restriction mode
				mode_chain << topic_restriction(targetChannel, operation, fd);
			// [t] set/remove restrictions to channel operators
				//Find out how are operator privileges given to anyone 
				//in the server, how is it done here?
			// [k] set/remove channel key(password)
			// [o] Give/take Channel operator privilege
			// [l] Set/remove the user limit to channel
		}
	}
	std::string chain = mode_chain.str();
	if (chain.empty())
		return;
 	//targetChannel->sendTo_all(RPL_CHANGEMODE(cli->getHostname(), channel->GetName(), mode_chain.str(), arguments));
	std::cout << GREEN << "[DEBUG] FINISH!!! WELL DONE" << RT << std::endl;
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

// handle /PART which allows the user to exit the channel
void	Server::handlePart(int fd, std::list<std::string> cmd_list)
{
	Client *client = getClientByFd(fd);
	if (!client)
		return ;
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

// handle /PRIVMSG command, send message to channel or send to specific user
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
			ERR_NOSUCHNICK(getName(), client->getNick(), target);
			return ;
		}
		std::string out = ":" + client->getPrefix() + " PRIVMSG " + target + " :" + message + CRLF;
		std::cout << out << std::endl;
		// send the message to the target user
		sendReply(targetUser->getFd(), out);
	}
}
