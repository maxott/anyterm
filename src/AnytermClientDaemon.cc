// daemon/AnytermClientDaemon.cc
// This file is part of Anyterm; see http://anyterm.org/
// (C) 2005-2007 Philip Endecott

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.


#include "AnytermClientDaemon.hh"

#include "Anyterm.hh"
#include "static_content.hh"

#include <sstream>
#include <algorithm>

#include "segv_backtrace.hh"

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

#include "UrlEncodedCgiParams.hh"

using namespace std;
using namespace pbe;

void AnytermClientDaemon::open_socket() {
  cout << ">>> CONNECT" << endl;
}

void AnytermClientDaemon::run() {
  cout << ">>> RUN" << endl;

  int sockfd, n;
  char buffer[256];

  struct sockaddr_in serv_addr;
  struct hostent *server;

  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) {
    throw_ErrnoException("socket()");
  }
  server = gethostbyname(host.c_str());
  if (server == NULL) {
    cerr << "ERROR, no such host\n";
    exit(0);
  }
  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  bcopy((char *)server->h_addr,
	(char *)&serv_addr.sin_addr.s_addr,
	server->h_length);
  serv_addr.sin_port = htons(port);
  if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) {
    throw_ErrnoException("connect()");
  }

  while (1) {
    read(sockfd,buffer,255);
    cout << "Read '" << buffer << "'." << endl;

    CgiParams params = UrlEncodedCgiParams(buffer);
    Anyterm::response_t r = anyterm.process_request(params, "unknown");
    cout << "SEND: " << r.body << endl;
  }

  close(sockfd);
}


//void AnytermClientDaemon::handle(const HttpRequest& req0, HttpResponse& resp)
//{
//  Anyterm::response_t r = anyterm.process_request(req);
//}




