/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gyong-si <gyong-si@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/16 16:21:05 by gyong-si          #+#    #+#             */
/*   Updated: 2025/05/19 10:51:20 by gyong-si         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/channel.hpp"

Channel::Channel(const std::string &name, const std::string &password)
{
	_name = name;
	_password = password;
	_topic = "Welcome to " + getName();
	_created_time = getFormattedTime();
	_inviteOnly = false;
	_topicRestricted = false;
	//for k
	//for 0
	//for l
};

const std::string &Channel::getName() const
{
	return (_name);
}

const std::string &Channel::getTopic()
{
	return (_topic);
}

const std::vector<ChannelUser> &Channel::getUsers() const
{
	return (_users);
}

std::string Channel::getClientList()
{
	std::string clientList = "";

	for (size_t i = 0; i < _users.size(); ++i)
	{
		if (!clientList.empty())
			clientList += " ";
		if (_users[i].isOperator)
			clientList += "@" + _users[i].client->getNick();
		else
			clientList += _users[i].client->getNick();
	}
	//std::cout << "Printing client list" << std::endl;
	//std::cout << clientList << std::endl;
	return (clientList);
}

const std::string &Channel::getPassword() const
{
	return (_password);
}

const std::string &Channel::getCreationTime() const
{
	return (_created_time);
}

void Channel::setName(const std::string &name)
{
	_name = name;
}

void Channel::setPassword(const std::string &password)
{
	_password = password;
}

bool Channel::isMember(Client *client)
{
	for (std::vector<ChannelUser>::iterator it = _users.begin(); it != _users.end(); ++it)
	{
		if (it->client == client)
			return (true);
	}
	return (false);
}

void Channel::addMember(Client *client)
{
	_users.push_back(ChannelUser(client, false));
}


bool Channel::isOperator(Client *client) const
{
	for (size_t i = 0; i < _users.size(); ++i)
	{
		if (_users[i].client == client && _users[i].isOperator)
			return (true);
	}
	return (false);
}

void Channel::addOperator(Client *client)
{
	_users.push_back(ChannelUser(client, true));
}

bool Channel::hasPassword() const
{
	return (!_password.empty());
}


void Channel::removeUser(Client *client)
{
	for (std::vector<ChannelUser>::iterator it = _users.begin(); it != _users.end(); ++it)
	{
		if (it->client == client)
		{
			_users.erase(it);
			break ;
		}
	}
}

/**
 * Sends a message to all members of a channel except the user specified
 */
void Channel::broadcast(const std::string &message, const Client *exclude)
{
	// loop through the _users vector to send the message
	for (std::vector<ChannelUser>::iterator it = _users.begin(); it != _users.end(); ++it)
	{
		if (it->client != exclude)
		{
			std::cout << "Sending to client with fd: " << it->client->getFd() << std::endl;
			sendReply(it->client->getFd(), message);
		}
	}
}

/**
 * Sends the message to everybody.
 */
void Channel::broadcast(const std::string &message)
{
	for (std::vector<ChannelUser>::iterator it = _users.begin(); it != _users.end(); ++it)
	{
		sendReply(it->client->getFd(), message);
	}
}
