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
#include "Mutex.hh"
#include "Condition.hh"

class AnytermClientDaemon: public Daemon, SessionActivityListener {
private:
  Anyterm anyterm;
  std::string host;
  std::string server_ctxt;
  int sockfd;
  typedef pbe::Mutex<> write_lock_t;
  write_lock_t write_lock;

public:


  AnytermClientDaemon(
    std::string host_, short port_,
    std::string user="",
    std::string command="",
    std::string name="",
    std::string device="",
    std::string charset="ascii",
    bool diff=true,
    std::string server_ctxt_ = "undefined",
    int session_timeout = 60
  ): Daemon(port_, "httpd", LOG_LOCAL0, (name=="") ? "anyterm" : name, "", 1, false),
     anyterm(command, device, charset, diff, 20, session_timeout, true),
     //anyterm(command, device, charset, diff, 20, 10, true),
     host(host_), server_ctxt(server_ctxt_)
  {}

  void session(pbe::FileDescriptor& in_fd, pbe::FileDescriptor& out_fd) {};
  void run_writer_thread(void);

  void on_session_activity(Session* session, SessionActivity activity);

protected:
  virtual void open_socket();
  virtual void run();
  void _write(std::string cmd, std::string msg);
  void pinger(void);

};


#endif
