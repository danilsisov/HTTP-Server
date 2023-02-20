#include <iostream>
#include <stdio.h>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sqlite3.h>
#include <vector>
#include <thread>
#define PORT 8002

using namespace std;

class Server
{
    private:
        int socketDescriptor = 0, newSocketDescriptor = 0, addressLenght = 0;
        struct sockaddr_in address;
        char buffer[1024] = {0};
        string sql = "SELECT * FROM PERSON";
        string content, response;
    public:
        Server();
    protected:
        void socketCreation();
        void bindCreation();
        void listenMode();
        void acceptConnection();
        void handleRequest(int socketDescriptor);
};

Server::Server()
{
    socketCreation();

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    sockaddr_in server_address;

    bindCreation();
    listenMode();
    acceptConnection();
}

void Server::socketCreation()
{
    if ((socketDescriptor = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("Socket creation failed\n");
        exit(EXIT_FAILURE);
    }
    else
    {
        cout << "Socket created successfully" << endl;
    }
}

void Server::bindCreation()
{
    if (bind(socketDescriptor, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("Bind creation failed\n");
        exit(EXIT_FAILURE);
    }
    else
    {
       cout << "Bind created successfully" << endl;
    }
}

void Server::listenMode()
{
    // 5 as maximum pending connections
    if (listen(socketDescriptor, 5) < 0)
    {
        perror("Listen mode Error");
        exit(EXIT_FAILURE);
    }
    else
    {
        cout << "Socket in Listen State (Max Connection: 5)" << endl;
    }
}

void Server::acceptConnection()
{
    sockaddr_in client_address;
    socklen_t client_address_size = sizeof(client_address);
    if ((newSocketDescriptor = accept(socketDescriptor, (struct sockaddr *)&address, (socklen_t*)&addressLenght)) < 0) {
        perror("Error in accepting connection\n");
        exit(EXIT_FAILURE);
    }
    else
    {
        cout << "Accepting connection was done successfully" << endl;
        handleRequest(newSocketDescriptor);
    }
}

void Server::handleRequest(int socketDescriptor)
{
    read(socketDescriptor, buffer, 1024);

    // Start and end indices for finding tokens
    size_t start = 0, end = 0;
    // Start and end indices for parsing the request line
    size_t start2 = 0, end2 = 0;
    // Convert the incoming data from the client socket to a string
    string message(buffer);

    // Split the HTTP request into tokens
    vector<string> tokens;
    while ((end = message.find("\r\n", start)) != string::npos)
    {
        tokens.push_back(message.substr(start, end - start));
        start = end + 2;
    }

     // Extract the request line from the HTTP request
    vector<string> request_line;
    while ((end2 = tokens[0].find(" ", start2)) != string::npos)
    {
        request_line.push_back(tokens[0].substr(start2, end2 - start2));
        start2 = end2 + 1;
    }

    // The first token in the request line
    string method = request_line[0];
    string path = request_line[1];

    if (path == "/database.db") {
        sqlite3* db;
        int dataFile = sqlite3_open("database.db", &db);
        if (dataFile) {
            cout << "Cannot open database: " << sqlite3_errmsg(db) << endl;
            sqlite3_close(db);
            return;
        }

        // Declare a SQLite prepared statement for executing SQL queries
        sqlite3_stmt* stmt;
        dataFile = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, 0);
        if (dataFile != SQLITE_OK) {
            cout << "Error executing query: " << sqlite3_errmsg(db) << endl;
            sqlite3_close(db);
            return;
        }

        // Amout of rows depends on database rows
        int end = 8;
        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            for (int rows = 0; rows <= 8; rows++) {
                content += string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, rows))) + ", ";
                if (rows == end) {
                    content += string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8))) + "\n";
                }
            }
        }

        sqlite3_finalize(stmt);
        sqlite3_close(db);

        string response = "HTTP/1.1 200 OK\r\nContent-Length: " + to_string(content.length()) + "\r\n\r\n" + content;
        write(socketDescriptor, response.c_str(), response.length());
    }
    else
    {
        string response = "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n";
        write(socketDescriptor, response.c_str(), response.length());
    }
  close(socketDescriptor);
}

int main()
{
    Server server;
    return 0;
}
