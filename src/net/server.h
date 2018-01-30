//
//  server.h
//  Residue
//
//  Copyright © 2017 Muflihun Labs
//

#ifndef Server_h
#define Server_h

#include <boost/asio.hpp>
#include "src/non-copyable.h"

using boost::asio::ip::tcp;

namespace residue {

class RequestHandler;

///
/// \brief Server containing abstract request handler that determines request
/// handler at constructor time and calls the handler with new session
///
class Server final : NonCopyable
{
public:
    explicit Server(boost::asio::io_service& io_service, int port, RequestHandler* requestHandler);
    ~Server();
private:
    void accept();

    tcp::acceptor m_acceptor;
    tcp::socket m_socket;

    RequestHandler* m_requestHandler;
};
}
#endif /* Server_h */