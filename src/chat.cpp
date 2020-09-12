#include <iostream>
#include <thread>
#include <string>
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>

class Server
{
    private:
        boost::asio::io_service service;
        boost::asio::ip::tcp::endpoint endPoint;
        boost::asio::ip::tcp::acceptor acceptor;
        std::map<std::string, boost::shared_ptr<boost::asio::ip::tcp::socket>> clients;
        void multicastToOtherClients(std::string message, std::string senderClientAddress);
        void processClientSession(boost::shared_ptr<boost::asio::ip::tcp::socket> sock);
    public:
        Server();
        Server(uint16_t port);
        void listen();
};

class Client
{
    private:
        boost::asio::io_service service;
        boost::asio::ip::tcp::endpoint endPoint;
        boost::asio::ip::tcp::socket sock;
        void listen();
        void talk();
    public:
        Client() : service(), endPoint(boost::asio::ip::address::from_string("0.0.0.0"), 2020), sock(service) { };
        void connect();
};

Server::Server() : service(), endPoint(boost::asio::ip::tcp::v4(), 2020), acceptor(service, endPoint)
{
    clients.clear();
}

Server::Server(uint16_t port) : service(), endPoint(boost::asio::ip::tcp::v4(), port), acceptor(service, endPoint)
{
    clients.clear();
}

void Server::processClientSession(boost::shared_ptr<boost::asio::ip::tcp::socket> sock)
{
    boost::system::error_code ec;
    std::string fullAddress = sock->remote_endpoint(ec).address().to_string() + ":" + std::to_string(sock->remote_endpoint(ec).port());
    std::string message;

    while (true)
    {
        char data[512] = {};
        
        try
        {
            size_t len = sock->read_some(boost::asio::buffer(data));

            if (len > 0)
            {
                message = fullAddress + " > " + std::string(data);
                std::cout << message << std::endl;
                std::thread t(&Server::multicastToOtherClients, this, message, fullAddress);
                t.detach();
            }
        }
        catch (std::exception& e)
        {
            message = fullAddress + " has disconnected";
            std::cout << message << std::endl;
            clients.erase(fullAddress);
            std::thread t(&Server::multicastToOtherClients, this, message, std::string(""));
            t.detach();
            sock->close();
            break;
        }
    }
}

void Server::multicastToOtherClients(std::string message, std::string senderClientAddress)
{
    for (std::pair<std::string, boost::shared_ptr<boost::asio::ip::tcp::socket>> sock : clients)
    {
        if (senderClientAddress == "" || sock.first != senderClientAddress) 
        {
            sock.second->write_some(boost::asio::buffer(message));
        }
    }
}

void Server::listen()
{
    std::cout << "Started server at port " << endPoint.port() << std::endl;

    while (true)
    {
        boost::shared_ptr<boost::asio::ip::tcp::socket> sock(new boost::asio::ip::tcp::socket(service));
        std::string fullAddress = "";
        std::string message = "";

        acceptor.accept(*sock);
        fullAddress += sock->remote_endpoint().address().to_string() + ":" + std::to_string(sock->remote_endpoint().port());
        message = fullAddress + " has connected";
        std::cout << message << std::endl;
        std::thread mThread(&Server::multicastToOtherClients, this, message, fullAddress);
        mThread.detach();
        clients.insert({fullAddress, sock});
        std::thread pThread(&Server::processClientSession, this, sock);
        pThread.detach();
    }
}

void Client::connect()
{
    std::string address;
    boost::system::error_code ec;

    do
    {
        std::cout << "Gimme the address (v4): ";
        std::cin >> address;

        endPoint.address(boost::asio::ip::address::from_string(address));

        sock.connect(endPoint, ec);

        if (!ec)
        {
            std::cout << "Connected to " << address << ":" << 2020 << std::endl;

            std::thread lThread(&Client::listen, this);
            std::thread tThread(&Client::talk, this);

            lThread.detach();
            tThread.detach();
        }
        else
        {
            std::cout << "Couldn't connect; is smth wrong with the address? Try again, please" << std::endl;
        }
    }
    while (ec);
    
    while (true) {};
}

void Client::listen()
{
    while (true)
    {
        char data[4096] = {};

        sock.read_some(boost::asio::buffer(data));
        std::cout << std::string(data) << std::endl;
    }
}

void Client::talk()
{
    std::string msg;

    while (true)
    {
        std::getline(std::cin, msg);

        sock.write_some(boost::asio::buffer(msg));
    }
}

int main()
{
    std::string selection;

    std::cout << "Hi World, I'm Chat\nWhatcha wanna do?\n1. Serve\n2. Connect" << std::endl;

    std::cin >> selection;

    if (selection == "1")
    {
        Server* newServer = new Server();

        newServer->listen();
    }
    else if (selection == "2")
    {
        Client newClient;

        newClient.connect();
    }
    else
    {
        std::cout << "No can do, goodbye!" << std::endl;
    }
    
    return 0;
}
