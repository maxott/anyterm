// daemon/AnytermClientDaemon.hh
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


#ifndef AnytermClientDaemon_hh
#define AnytermClientDaemon_hh

#include "Daemon.hh"
#include "Session.hh"
#include "Anyterm.hh"



class AnytermClientDaemon: public Daemon, SessionActivityListener {
private:
  Anyterm anyterm;
  std::string host;
  int sockfd;

public:


  AnytermClientDaemon(std::string host_, short port_,
    std::string user="",
    std::string command="",
    std::string name="",
    std::string device="",
                std::string charset="ascii",
                bool diff=true):
    host(host_),
    Daemon(port_, "httpd", LOG_LOCAL0, (name=="") ? "anyterm" : name, "",
        1, false),
    anyterm(command, device, charset, diff, 1)
  {}

  void session(pbe::FileDescriptor& in_fd, pbe::FileDescriptor& out_fd) {};
  void run_writer_thread(void);

  void on_session_activity(Session* session);

protected:
  virtual void open_socket();
  virtual void run();
  void _write(std::string session_id, std::string msg);

};


#endif
