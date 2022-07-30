#include "../src/Server/Server/Server.hpp"
using namespace std;
class test : public TcpServerSpi
{
public:
    test() {}
    void onAccepted(std::shared_ptr<Connection> conn)
    {
        std::string str = packMessage("hi", 1);
        spdlog::info("reply {}, conn = {}", str, conn->sendfd_);
        conn->sendMessage(0, str.data(), str.length());
    }
    void onDisconnected(std::shared_ptr<Connection> conn, int reason, const char *reasonMsg)
    {
        spdlog::info("Server onDisconnected, reason {} {}", reason, reasonMsg);
    }
    void onMessage(std::shared_ptr<Connection> conn, int msgType, const void *payload, size_t size)
    {
        std::string str((char *)payload, size);
        if (str == "closeme")
        {
            conn->close(0);
            return;
        }
        std::string rep = packMessage(str, 1);
        spdlog::trace("onMessage, size = {}", size);
        conn->sendMessage(0, rep.data(), rep.length());
    }
};

int main()
{
    spdlog::set_level(spdlog::level::trace);
    TcpServerApi server;
    test *tmp = new test;
    TcpServerSpi *spi = tmp;
    server.bindAddress("127.0.0.1:1234");
    server.registerSpi(spi);
    server.run();
}