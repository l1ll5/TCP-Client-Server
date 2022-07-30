#include "../src/Client/Client.hpp"
#include <bits/stdc++.h>
using namespace std;
double mx = 0, mn = 1'000'000'000, sum = 0;
int n = 5;
int fin = 0;
int query = 100000;
chrono::steady_clock sc;
static std::chrono::_V2::steady_clock::time_point start;
class test : public TcpClientSpi
{
public:
    int p = 0;
    int cnt = 0;
    TcpClientApi *api;
    test() {}
    void onConnected()
    {
        spdlog::info("onConnected");
    }
    void onDisconnected(int reasonCode, const char *reasonMsg)
    {
    }
    void onMessage(int msgType, const void *payload, size_t size)
    {
        // cerr << p << ' ' << cnt << endl;
        cnt += 1;
        if (cnt == query + 5)
        {
            //cerr << "p finished, fin = " << fin << endl;
            fin += 1;
            api->shutdown();
        }
        /*
        std::string str((char *)payload, size);
        spdlog::info("Client received {}, id = {}", str, p);*/
    }
};

int main()
{
    srand(time(0));
    spdlog::set_level(spdlog::level::err);
    vector<TcpClientApi> cli(n);
    for (int i = 0; i < n; i += 1)
    {
        test *tmp = new test;
        tmp->api = &cli[i];
        tmp->p = i;
        TcpClientSpi *spi = tmp;
        cli[i].setServerAddress("127.0.0.1:1234");
        cli[i].registerSpi(tmp);
    }
    for (int i = 0; i < n; i += 1)
    {
        //sleep(1);
        cli[i].start();
    } //connection
    sleep(3);
    std::string input;
    for (int j = 0; j < 100000; j += 1)
        input.push_back('a');
    std::string packed = packMessage(input, 1);

    for (int i = 0; i < 5; i += 1)
    {
        for (int j = 0; j < n; j += 1)
        {
            cli[j].sendMessage(0, packed.c_str(), packed.length());
        }
    }
    sleep(5);
    //warmup

    start = sc.now();
    input = "hi";
    packed = packMessage(input, 1);
    mx = 0, mn = 1'000'000'000, sum = 0;
    for (int test = 0; test < query; test += 1)
    {
        for (int p = 0; p < n; p += 1)
            cli[p].sendMessage(0, packed.c_str(), packed.length());
        //cerr << packed << endl;
    }
    while (1)
    {
        if (fin == n)
        {
            auto end = sc.now();
            auto time_span = static_cast<chrono::duration<double>>(end - start);
            cout << "Operation took: " << time_span.count() * 1000 << " ms" << endl;
            break;
        }
    }
    sleep(5);
}