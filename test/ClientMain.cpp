#include "../src/Client/Client.hpp"
#include <bits/stdc++.h>
using namespace std;
double mx = 0, mn = 1'000'000'000, sum = 0;
int n = 1;
chrono::steady_clock sc;
static std::chrono::_V2::steady_clock::time_point start;
class test : public TcpClientSpi
{
public:
    int p = 0;
    TcpClientApi *api;
    test() {}
    void onConnected()
    {
        spdlog::info("onConnected");
    }
    void onDisconnected(int reasonCode, const char *reasonMsg)
    {
        spdlog::info("Client onDisconnected, reason {} {}", reasonCode, reasonMsg);
    }
    void onMessage(int msgType, const void *payload, size_t size)
    {
        /*cntt += 1;
        if (cntt % 1000 == 0)
            cerr << cntt << endl;
        if (cntt >= n * maxn / 100 * 95 and type)
        {
            auto end = sc.now();
            auto time_span = static_cast<chrono::duration<double>>(end - start);
            cout << "Operation took: " << time_span.count() << " seconds" << endl;
            exit(0);
        }*/
        auto end = sc.now();
        auto time_span = static_cast<chrono::duration<double>>(end - start);
        mx = max(mx, time_span.count() * 1000);
        mn = min(mn, time_span.count() * 1000);
        sum += time_span.count() * 1000;
        //cout << "Operation took: " << time_span.count() * 1000 << " ms" << endl;

        std::string str((char *)payload, size);
        spdlog::info("Client received {}, id = {}", str, p);
    }
};

int main()
{
    srand(time(0));
    spdlog::set_level(spdlog::level::trace);
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
    string input;
    while (cin >> input)
    {
        auto packed = packMessage(input, 1);
        cli[0].sendMessage(0, packed.c_str(), packed.length());
    }
    cli[0].shutdown();
}