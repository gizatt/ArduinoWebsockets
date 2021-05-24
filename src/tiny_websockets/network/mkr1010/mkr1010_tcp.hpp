#pragma once

#ifdef ARDUINO_SAMD_MKRWIFI1010

#include <tiny_websockets/internals/ws_common.hpp>
#include <tiny_websockets/network/tcp_client.hpp>
#include <tiny_websockets/network/tcp_server.hpp>
#include <tiny_websockets/network/generic_esp/generic_esp_clients.hpp>

#include <WiFiNINA.h>

namespace websockets { namespace network {
  template <class Client> 
  class GenericMkr1010TcpClient : public TcpClient {
  public:
    GenericMkr1010TcpClient(Client c) : client(c) {
      //client.setNoDelay(true);
    }
    
    GenericMkr1010TcpClient() {}

    bool connect(const WSString& host, const int port) {
      yield();
      auto didConnect = client.connect(host.c_str(), port);
      //client.setNoDelay(true);
      return didConnect;
    }

    bool poll() {
      yield();
      return client.available();
    }

    bool available() override {
      return client.connected();
    }

    void send(const WSString& data) override {
      yield();
      client.write(reinterpret_cast<uint8_t*>(const_cast<char*>(data.c_str())), data.size());
      yield();
    }

    void send(const WSString&& data) override {
      yield();
      client.write(reinterpret_cast<uint8_t*>(const_cast<char*>(data.c_str())), data.size());
      yield();
    }

    void send(const uint8_t* data, const uint32_t len) override {
      yield();
      client.write(data, len);
      yield();
    }
    
    WSString readLine() override {
      WSString line = "";

      int ch = -1;
      while( ch != '\n' && available()) {
        ch = client.read();
        if (ch < 0) continue;
        line += (char) ch;
      }

      return line;
    }

    uint32_t read(uint8_t* buffer, const uint32_t len) override {
      yield();
      return client.read(buffer, len);
    }

    void close() override {
      yield();
      client.stop();
    }

    virtual ~GenericMkr1010TcpClient() {
      client.stop();
    }

  protected:
    Client client;

    int getSocket() const override {
      return -1;
    }    
  };

  typedef GenericMkr1010TcpClient<WiFiClient> Mkr1010TcpClient;
  
  class SecuredMkr1010TcpClient : public GenericMkr1010TcpClient<WiFiSSLClient> {
  public:
  };


  class Mkr1010TcpServer : public TcpServer {
  public:
    Mkr1010TcpServer() : server(80) {}
    bool poll() override {
      yield();
      return server.status() != CLOSED;
    }

    bool listen(const uint16_t port) override {
      yield();
      // port being discarded: WiFiServer from Nina doesn't support closing / 
      // reopening to requested port.
      server.begin();
      return available();
    }
    
    TcpClient* accept() override {
      while(available()) {
        auto client = server.available();
        if(client) {
          return new Mkr1010TcpClient{client};
        }
      }
      return new Mkr1010TcpClient;
    }

    bool available() override {
      yield();
      return server.available();
    }
    
    void close() override {
      yield();
      // no close operation available... hope this never gets called...
    }

    virtual ~Mkr1010TcpServer() {
      if(available()) close();
    }

  protected:
    int getSocket() const override {
      return -1; // Not Implemented
    }
  
  private:
    WiFiServer server;
  };
}} // websockets::network

#endif // #ifdef Mkr1010 
