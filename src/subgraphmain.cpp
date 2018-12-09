#include <iostream>
#include <vector>
#include <csv.h>
#include "subgraph.hpp"
#include <chrono>
#include <sys/stat.h> 
#include <fcntl.h>
#include <unistd.h>
#include <iterator>
#include <chrono>
#include <thread>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <exception>
#include <errno.h>
#include <string.h>

const std::string PREFIX{"SubgraphEngineCPP: "};

template<typename E>
using LinkStruct = typename stream::subgraph<E>::link;

class ErrnoException: public std::exception
{
public:
    ErrnoException(int unixErrno)
    {
        dscr = strerror(unixErrno);
    }
    const char* what() const noexcept override
    {
        return dscr.c_str();
    }
private:
    std::string dscr;
};

class SocketReadWriteError: public ErrnoException
{
public:
    using ErrnoException::ErrnoException;
};

class SocketCreationError: public ErrnoException
{
public:
    using ErrnoException::ErrnoException;
};

class Connector
{
public:
    Connector(const std::string pathname): pathname(pathname)
    {
        int ret = 0;
        serv = socket(AF_UNIX, SOCK_STREAM, 0);
        if(serv < 0)
        {
            throw SocketCreationError(errno);
        }

        sockaddr_un addr;
        addr.sun_family = AF_UNIX;
        strcpy(addr.sun_path, pathname.c_str());
        ret = bind(serv, (sockaddr*) &addr, sizeof(addr));
        if(ret < 0)
        {
            throw SocketCreationError(errno);
        }

        ret = listen(serv, 1);
        if(ret < 0)
        {
            throw SocketCreationError(errno);
        }

        socklen_t sizeOfAddr = sizeof(clientAddr);
        client = accept(serv, (sockaddr*) &clientAddr, &sizeOfAddr);
        if(client < 0)
        {
            throw SocketCreationError(errno);
        }
    }


    enum class SocketState
    {
        ON_RUN,
        CLOSED
    };
    SocketState rawRead(void * buffer, unsigned int byteNum) const
    {
        const int ret = read(client, buffer, byteNum);
        if(ret < 0)
        {
            std::cout << PREFIX << " SocketReadWriteError :)" << std::endl;
            throw SocketReadWriteError(errno);
        }
        else if(ret == 0)
        {
            return SocketState::CLOSED;
        }
        return SocketState::ON_RUN;
    }

    void rawWrite(const void * buffer, unsigned int byteNum) const
    {
    
        const int ret = write(client, buffer, byteNum);
        if(ret < 0)
        {
            throw SocketReadWriteError(errno);
        }
    }

    ~Connector()
    {
        close(client);
        close(serv);
        unlink(pathname.c_str());
    }
private:
    const std::string pathname;
    int serv;
    int client;
    sockaddr_un clientAddr;
};

const unsigned SEND_ELEMS = 3;
const unsigned GET_MODEL = 4;
const unsigned REMOVE_OLD_CHUNK = 5;
const unsigned CLOSE = 6;
const unsigned PRINT = 7;

struct Elem
{
    int32_t x{0},y{0};
};

class SubgraphConnector
{
public:
    SubgraphConnector(const Connector& _con): con(_con)
    {
    }
    unsigned getOrder()
    {
        char buf[1];
        auto socketState = con.rawRead(buf, 1);
        if(socketState == Connector::SocketState::CLOSED)
        {
            std::cout << PREFIX << "Socket is closed: it will be translated to order CLOSE " << std::endl;
            return CLOSE;
        }
        const int order = static_cast<unsigned>(buf[0]);
        std::cout << PREFIX << "Order aquired: " << order << std::endl;
        return order;
    }

    std::vector<Elem> getElems()
    {
        int32_t buff[100000];
        int32_t numOfElems;
    
        con.rawRead(&numOfElems, sizeof(numOfElems));

        std::cout << PREFIX << numOfElems << std::endl;

        con.rawRead(buff, 4*numOfElems);

        std::vector<Elem> elems; 
        elems.reserve(numOfElems);

        for(int i=0; i<numOfElems;)
        {
            elems.push_back({buff[i], buff[i+1]});
            i+=2;
        }
        return elems;
    }

    void sendTime(uint32_t time)
    {
        std::cout << PREFIX << "send time: "<< time << std::endl;
        con.rawWrite(&time, 4);
    }

    template<typename E>
    void sendLinks(const std::vector< typename stream::subgraph<E>::link >& links)
    {
        uint32_t num = links.size();
        con.rawWrite(&num, sizeof(num) );
        LinkStruct<E> l{0,0,0};
        con.rawWrite(links.data(), sizeof(l) * links.size());
    }

private:
    const Connector& con;
};

std::chrono::milliseconds get_now()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
}

int main(int argc, char * argv[])
{
    Connector con(argv[1]);
    SubgraphConnector sgcon(con);

    auto distance = [](const Elem &e1, const Elem &e2){
        return (e1.x - e2.x)*(e1.x - e2.x) + (e1.y - e2.y) * (e1.y - e2.y);
    };
    stream::subgraph<Elem> sgraph(distance);

    bool run = true;
    while(run)
    {
        auto order = sgcon.getOrder();
        switch(order)
        {
            case SEND_ELEMS:
            {
                std::cout << PREFIX << "SEND_ELEMS" << std::endl;
                std::vector<Elem> els = sgcon.getElems();

                for(const auto& e: els)
                {
                    std::cout << PREFIX << "elem: (" << e.x << ", " << e.y << ")" << std::endl;
                }
                
                std::cout << PREFIX << "elems processing start" << std::endl;
                auto start = get_now();
                sgraph.add_elements(els);
                auto end   = get_now();
                std::cout << PREFIX << "elems processing stop" << std::endl;
                uint32_t duration = (end.count() - start.count());

                std::cout << PREFIX << "durarion: " << duration << std::endl;

                sgcon.sendTime(duration);

            }
            break;
            case GET_MODEL:
            {
                std::cout << PREFIX << "GET_MODEL" << std::endl;
                sgraph.update_links_in_model();

                auto start = get_now();
                auto model = sgraph.get_model();
                auto end   = get_now();
                
                for(const auto l: model)
                {
                    std::cout << PREFIX << "link: (" << l.a << ", " << l.b << ", " << l.lenght <<  ")" << std::endl;
                }

                uint32_t duration = (end.count() - start.count());
                
                sgcon.sendTime(duration);
                std::cout << PREFIX << "GET_MODEL. times has been sent" << std::endl;
                sgcon.sendLinks<Elem>(model);
                std::cout << PREFIX << "GET_MODEL. links has been sent" << std::endl;
            }
            break;
            case REMOVE_OLD_CHUNK:
            {
                std::cout << PREFIX << "REMOVE_OLD_CHUNK" << std::endl;
                auto start = get_now();
                sgraph.remove_oldest_chunk();
                auto end   = get_now();

                uint32_t duration = (end.count() - start.count());
                sgcon.sendTime(duration);
            }
            break;
            case PRINT:
            {
                std::cout << PREFIX << "PRINT" << std::endl;
            }
            break;
            case CLOSE:
            {
                run = false;                
            }
            break;
            default:
            std::cout << "ERROR: UNKNOWN ORDER. Program will be closed soon" << std::endl;
            run = false;
        }
    }
    std::cout << PREFIX <<"CLOSE" << std::endl;
}