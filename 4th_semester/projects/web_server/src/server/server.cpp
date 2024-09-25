#include "server.h"
#include <sys/fcntl.h>
#include <sys/wait.h>

// class SocketAddress:

SocketAddress::SocketAddress() {
    saddr.sin_family = AF_INET;                      // denotes the addressing family
    saddr.sin_port = htons(PORT);                    // sets the port number in network byte order
    saddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);  // sets the IP address
}

SocketAddress::SocketAddress(const char* ip, short port) {
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(port);
    saddr.sin_addr.s_addr = inet_addr(ip);
}

SocketAddress::SocketAddress(unsigned int ip, short port) {
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(port);
    saddr.sin_addr.s_addr = htonl(ip);
}

// class Socket:

Socket::Socket() {
    // AF_INET - IPv4 protocol (domain - address family);
    // SOCK_STREAM - TCP(reliable, connection oriented) - type of communication
    sd_ = socket(AF_INET, SOCK_STREAM, 0);  // the 3rd parameter is a specific protocol
    if (sd_ < 0) {
        std::cout << "Error: Can't create a new socket" << std::endl;
    }
}

// class ServerSocket

void ServerSocket::bind_to(const SocketAddress& ipaddr) {           // binding sockets to a specific address:
    if (bind(sd_, ipaddr.get_addr(), ipaddr.get_addr_len()) < 0) {  // returns 0 if successful
        std::cout << "Error: Can't bind in ServerSocket" << std::endl;
        exit(1);
    }
}

int ServerSocket::accept_to(SocketAddress& client_addr) {
    size_t len = client_addr.get_addr_len();
    int res = accept(sd_, client_addr.get_addr(), (socklen_t*)&len);
    if (res < 0) {
        std::cout << "Error: Can't accept in ServerSocket" << std::endl;
        exit(1);
    }
    return res;
}

void ServerSocket::listen_to(int backlog) {  // listening state:
    if (listen(sd_, backlog) < 0) {
        std::cout << "Error: Can't listen in ServerSocket" << std::endl;
        exit(1);
    }
}

// class ConnectedSocket

void ConnectedSocket::to_write(const std::string& str) {
    if (send(sd_, str.c_str(), str.length(), 0) < 0) {  // or write(sd_, buf, buflen)
        std::cout << "Error: Can't write in ConnectedSocket" << std::endl;
        exit(1);
    }
}

void ConnectedSocket::to_write(const std::vector<uint8_t>& bytes) {
    if (send(sd_, bytes.data(), bytes.size(), 0) < 0) {  // or write(sd_, buf, buflen)
        std::cout << "Error: Can't write in ConnectedSocket" << std::endl;
        exit(1);
    }
}

void ConnectedSocket::to_read(std::string& str) {  // TODO - rebuild
    int buflen = 4096;
    char buf[buflen];
    if (recv(sd_, buf, buflen, 0) < 0) {  // or read(sd_, buf, buflen)
        std::cout << "Error: Can't read in ConnectedSocket" << std::endl;
        exit(1);
    }
    str = buf;
}

std::vector<std::string> split_lines(std::string str) {
    // splits the request into sentences, delimiter - indicates the end of the sentence
    std::string delimiter = "\r\n";
    int pos_start = 0, pos_end, delim_len = delimiter.length();
    std::string token;
    std::vector<std::string> res;

    while ((pos_end = str.find(delimiter, pos_start)) > 0) {
        token = str.substr(pos_start, pos_end - pos_start);
        pos_start = pos_end + delim_len;
        res.push_back(token);
    }
    res.push_back(str.substr(pos_start));
    return res;
}

std::string parse_path(std::string str) {
    std::string res_path = "src/";
    // Analysis, for example, between Get and HTTP:
    for (auto i = 0; i < str.length() - 1; ++i) {
        if (str[i] == ' ') {
            while (str[i + 1] != ' ' && str[i + 1] != '\n') {
                res_path += str[i + 1];
                i++;
            }
            break;
        }
    }
    if (res_path == "src//") {  // if it's empty - select the start page
        res_path = "src/index.html";
    }
    return res_path;
}

std::vector<uint8_t> to_vector(int fd) {
    std::vector<uint8_t> v;
    char c;
    while (read(fd, &c, 1)) v.push_back(c);
    return v;
}

std::string get_cgi_file_name(std::string path) {
    std::string temp;
    int i = 0;
    while (path[i] != '?') {
        temp += path[i];
        i++;
    }
    return temp;
}

std::string get_cgi_query(std::string path) {
    std::string temp;
    int i = get_cgi_file_name(path).length() + 1;
    while (i != path.length()) {
        temp += path[i];
        i++;
    }
    return temp;
}

char** create_array(std::vector<std::string>& v) {
    char** env = new char*[v.size() + 1];
    for (auto i = 0; i < v.size(); ++i) {
        env[i] = (char*)v[i].c_str();
    }
    env[v.size()] = NULL;
    return env;
}

void check_error(ConnectedSocket cs) {
    std::string error_str;
    switch (errno) {
        case EACCES:  // permission denied
            error_str = "HTTP/1.1 403 Forbidden\n";
            break;
        case ENETRESET:  // connection aborted by network
            error_str = "HTTP/1.1 503 Service Unavailable\n";
            break;
        default:
            error_str = "HTTP/1.1 404 Not Found\n";
            break;
    }
    std::cout << error_str << std::endl;
    cs.to_write(error_str);
}

bool is_cgi_connection(std::string str) {
    return !(str.find('?') == -1);  // true - if the '?' character is found
}

void cgi_connection(std::string path, int cd, const SocketAddress& client_addr, ConnectedSocket cs, std::string request) {
    int fd;
    pid_t pid = fork();
    switch (pid) {
        case -1: {
            perror("System error with pid");
            exit(1);
        }
        case 0: {
            /*
            Descendant-processing of a CGI program:
            - forming an array of environment variables env
            - redirecting standard output to a temporary file 
            - redirecting standard input (for the POST method)
            - actually starting the program
            */
            fd = open("log", O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd < 0) {
                std::cout << "Error: Can't open a new file" << std::endl;
            }
            dup2(fd, 1);
            close(fd);
            std::string file_name = get_cgi_file_name(path);
            std::string query = get_cgi_query(path);
            char* argv[] = {(char*)file_name.c_str(), NULL};

            std::vector<std::string> v;
            v.push_back(request);
            v.push_back(SERVER_ADDR);
            v.push_back(SERVER_PORT);
            v.push_back(SERVER_PROTOCOL);
            v.push_back(CONTENT_TYPE);
            v.push_back(QUERY_STRING + query);
            v.push_back(SCRIPT_NAME + file_name);

            char** env = create_array(v);

            execve(file_name.c_str(), argv, env);

            check_error(cs);

            perror("exec");
            exit(2);
        }
        default: {  // case > 0
            int status;
            wait(&status);

            if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
                fd = open("log", O_RDONLY);
                std::vector<uint8_t> vect = to_vector(fd);
                cs.to_write("HTTP/1.1 200 OK\0");
                std::cout << "HTTP/1.1 200 OK" << std::endl;
                std::string str = "\r\nVersion: HTTP/1.1\r\nContent-length: " + std::to_string(vect.size()) + "\r\n\r\n";
                std::cout << "Version: "
                          << "HTTP/1.1" << std::endl;
                std::cout << "Content-length: " << std::to_string(vect.size()) << std::endl;

                cs.to_write(str);
                cs.to_write(vect);
                close(fd);
                cs.shutting_down();
            }
            break;
        }
    }
}

void default_connection(std::string path, ConnectedSocket cs) {
    int fd = 0;
    if ((fd = open(path.c_str(), O_RDONLY)) < 0) {
        std::cout << "HTTP/1.1 404 Not Found" << std::endl;
        cs.to_write("HTTP/1.1 404 Not Found\r");
        if ((fd = open(ERROR_PAGE, O_RDONLY)) < 0) {
            std::cout << "Error: Page 404 is missing" << std::endl;
        }
    } else {
        cs.to_write("HTTP/1.1 200 OK\0");
    }
    std::vector<uint8_t> vect = to_vector(fd);
    std::string str = "\r\nVersion: HTTP/1.1\r\nContent-length: " + std::to_string(vect.size()) + "\r\n\r\n";

    std::cout << "Version: "
              << "HTTP/1.1" << std::endl;
    std::cout << "Content-length: " << std::to_string(vect.size()) << std::endl;

    cs.to_write(str);
    cs.to_write(vect);
    close(fd);
    cs.shutting_down();
}

void process_connection(int cd, const SocketAddress& client_addr) {
    ConnectedSocket cs(cd);
    std::string request;
    cs.to_read(request);
    std::vector<std::string> lines = split_lines(request);
    // lines[0] - RequestHeader
    // lines[i] i = 1, ... - HttpHeader
    // lines[lines[lines.size() - 1]] <=> empty line or body
    if (lines.size() > 0) {
        std::cout << lines[0] << std::endl;
    } else {
        std::cout << "Error: lines.size() <= 0 in process_connection()" << std::endl;
    }

    std::string path = parse_path(lines[0]);
    std::cout << "Path: " << path << std::endl;

    if (is_cgi_connection(path)) {
        cgi_connection(path, cd, client_addr, cs, request);
    } else {
        default_connection(path, cs);
    }
}

void server_loop() {
    SocketAddress server_address(BASE_ADDR, PORT);
    ServerSocket server_socket;
    server_socket.bind_to(server_address);  // bind to an address - what port am I on?
    std::cout << "The client was successfully binded" << std::endl;
    server_socket.listen_to(BACKLOG);  // listen on a port, and wait for a connection to be established
    for (;;) {
        SocketAddress client_addr;
        int cd = server_socket.accept_to(client_addr);
        process_connection(cd, client_addr);  // process cilent-server connection
        std::cout << std::endl;
    }
}