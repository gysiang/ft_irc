#include "../include/channel.hpp"

// -marcus-
//|--------------------------------------|
//|             -INVITE-                 |
//|--------------------------------------|
void Channel::SetInviteOnly(bool enable_invite, int fd)
{
	(void)fd;
	if (enable_invite == true && this->_inviteOnly != true)
	{
		//clear the invite list before making the channel into a invite only channel
		this->clearInviteList();
		std::cout << YELLOW << "[DEBUG] Checking if list is cleared" << RT << std::endl;
		if (this->_inviteList.empty())
		{
			std::cout << GREEN << "[SUCCESS] List is cleared!" << RT << std::endl;
		}
		else
			std::cout << RED << "[DEBUG] Failed to clear list? WHY?" << RT << std::endl;
	}
	this->_inviteOnly = enable_invite;
	//sendReply(fd, "mode/" + targetChannel->getName() + " [+i] by client " + client->getNick());
}

bool	Channel::getchannelIsInviteOnly() const
{
	return (this->_inviteOnly);
}

void	Channel::inviteClient(int clientFd)
{
	if (std::find(_inviteList.begin(), _inviteList.end(), clientFd) == _inviteList.end())
		this->_inviteList.push_back(clientFd);
}

bool	Channel::getisClientInvited(int clientFd) const
{
	return (std::find(_inviteList.begin(), _inviteList.end(), clientFd) != _inviteList.end());
}

// once you joined after invite you can't join back unless invited again
void	Channel::removeInvite(int clientFd)
{
	std::vector<int>::iterator it = std::find(_inviteList.begin(), _inviteList.end(), clientFd);
	if (it != _inviteList.end())
		this->_inviteList.erase(it);
}

void	Channel::clearInviteList()
{
	this->_inviteList.clear();
}




//|--------------------------------------|
//|              -TOPIC-                 |
//|--------------------------------------|
void	Channel::setTopicRestriction(bool setTopic, int fd)
{
	(void)fd;
	this->_topicRestricted = setTopic;
	//sendReply(fd, "mode/" + targetChannel->getName() + " [+i] by client " + client->getNick());
}

bool	Channel::getisTopicRestricted() const
{
	return (this->_topicRestricted);
}
