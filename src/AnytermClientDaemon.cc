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

#include "version.hh"
#include "AnytermClientDaemon.hh"
#include "Anyterm.hh"
#include "segv_backtrace.hh"
#include "UrlEncodedCgiParams.hh"
#include "Error.hh"

#include <sstream>
#include <algorithm>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <string> 
#include <boost/bind.hpp>



using namespace std;
using namespace pbe;


void AnytermClientDaemon::open_socket() {
  struct sockaddr_in serv_addr;
  struct hostent *server;

  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) {
    throw_ErrnoException("socket()");
  }
  server = gethostbyname(host.c_str());
  if (server == NULL) {
    cerr << "ERROR, no such host" << endl;
    exit(-1);
  }
  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  bcopy((char *)server->h_addr,
	(char *)&serv_addr.sin_addr.s_addr,
	server->h_length);
  serv_addr.sin_port = htons(port);
  if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
    throw_ErrnoException("connect()");
  }
}

void AnytermClientDaemon::run() {
  if (!sockfd) return; // should have never gotten here

  char buffer[256];
  _write("H", server_ctxt + ":V" + ANYTERM_VERSION);

  Thread pinger(boost::bind(&AnytermClientDaemon::pinger,this));

  while (sockfd) {
    int l = read(sockfd, buffer, 255);
    if (l < 0) {
      cerr << "Socket seems to have closed" << endl;
      sockfd = 0;
      exit(-1);
    } else if (l > 0) {
      buffer[l] = '\0';
      while (buffer[l - 1] == '\n') {
	buffer[--l] = '\0';
      }
      //cout << "Read " << l << " '" << buffer << "'" << endl;
      if (l > 0) {
	try {
	  CgiParams params = UrlEncodedCgiParams(buffer);
	  Anyterm::response_t r = anyterm.process_request(params, "unknown");
	  // This is a bit of a hack
	  if (params.get("a") == "open") {
	    // r is supposed to be the session id of a new session. Let's hook up to it
	    Session* s = anyterm.session(r.body);
	    s->set_session_activity_listener(this);
	  }
	  _write("S", r.body);
	  //cout << "SEND: " << r.body << endl;
	} catch (Exception& E) {
	  E.report(cerr);
	} catch (Error& E) {
	  _write("E", ":" + E.get_msg());
	} catch (...) {
	  cerr << "Caught some unknown exception";
	}
      }
    }
  }

  close(sockfd);
}

void AnytermClientDaemon::on_session_activity(Session* session, SessionActivity activity) {
  try {
    switch(activity) {
    case CHANGED: {
      std::string r = session->rcv(0.0F);
      _write("A", session->id.str() + ":" + r);
      break;
    }
    case CLOSED:
      _write("C", session->id.str());
      break;
    case ERROR:
      _write("E", session->id.str() + ":" + session->error_msg);
    }
  } catch (Exception& E) {
    E.report(cerr);
  } catch (Error& E) {
    cerr << E.get_msg();
  } catch (...) {
    cerr << "Caught some unknown exception";
  }
}

void AnytermClientDaemon::_write(string cmd, string msg) {
  {
    Lock<write_lock_t> lock(write_lock);

    std::ostringstream ss;
    ss << "<" << cmd << ":" << msg.length() << ":" << msg << ">";
    const char* buf = ss.str().c_str();
    int offset = 0;
    int l = strlen(buf);
    int rm = 0;
    while ((rm = l - offset) > 0) {
      int s = write(sockfd, buf, rm);
      //cout << "Sent " << s << " bytes." << endl;
      if (s < 0) {
	cerr << "Socket seems to have closed" << endl;
	sockfd = 0;
	exit(-1);
      }
      offset += s;
    }
  }
  //cout << "ACTIVITY: <<" << buf << ">>" << endl;
};

void AnytermClientDaemon::pinger(void)
{
  while (sockfd) {
    sleep(60);
    _write("P", server_ctxt);
  }
}




//void AnytermClientDaemon::handle(const HttpRequest& req0, HttpResponse& resp)
//{
//  Anyterm::response_t r = anyterm.process_request(req);
//}




