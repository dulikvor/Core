#pragma once

#include <string>
#include <atomic>
#include <utility>
#include <netdb.h>

namespace core
{

	enum SocketRole
	{
		Server,
		Client
	};
	//Provides all the basic capabilities of a tcp socket, including:
	//1) Writing/Reading.
	//2) Socket Close.
	//3) Accepting new connection while in server variation.
	class TCPSocket
	{
	public:
		TCPSocket(SocketRole role);
		TCPSocket(SocketRole role, int m_fd);
		~TCPSocket();

		TCPSocket(TCPSocket&& obj);
		void operator = (TCPSocket&& obj);
		
		TCPSocket(const TCPSocket&) = delete;
		void operator = (const TCPSocket&) = delete;
		//If on client role and not already connected, will attempt to create a tcp socket
		//and initiate a connection to a given host address.
		void Connect(const std::string& host, uint16_t port);
		//Will attempt to close the underline socket.
		void Close();
		//Receives a reference to a buffer, will use an already connected socket
		//to send the received buffer to the host. if success a count of how many bytes
		//were successfuly sent will be returned. the command will not permit SIGPIPE signal
		//to be commenced, and errno will be returned instead.
		int Send(char* buf, size_t bufSize);
		//Receive will try to retrieve requested buffer size from and underlined socket. if
		//succesfull the read buffer will be returned. if 0 is returned there are two options -
		//the user request 0 sized buffer to be returned and this operation is valid, 
		//the connection to the host is not presented any more and nothing was read.
		std::pair<char*, ssize_t> Receive(size_t sizeToRead);
		//If on server tole and not already connected will attempt to "name" a newly created
		//socket with a received address.
		void Bind(const std::string& host, int port);
		//Will designate an underlined "named" server socket with the future role of accepting
		//connections.
		void Listen();
		//Attempts to receive new received connection, the operation is bound wait,
		//a newly received connection will be wrapped with a TCPSocket instance.
		TCPSocket Accept();
		static sockaddr_in GetSocketAddress(const std::string& host, uint16_t port);

	private:
		const int Invalid_Socket = -1;
		const int Max_Incoming_connections = 500;
		int m_fd;
		SocketRole m_role;
		std::atomic_bool m_connected;
		std::string m_hostAddress;
		int m_port;
	};
}
