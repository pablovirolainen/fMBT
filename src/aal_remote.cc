/*
 * fMBT, free Model Based Testing tool
 * Copyright (c) 2012 Intel Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU Lesser General Public License,
 * version 2.1, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
 * more details.
 *
 * You should have received a copy of the GNU Lesser General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#include "config.h"
#ifndef DROID
#include "aal_remote.hh"
#include <glib-object.h>
#include <glib.h>
#include "helper.hh"
#include <stdio.h>
#include <errno.h>

extern int _g_simulation_depth_hint;

aal_remote::aal_remote(Log&l,std::string& s)
  : aal(l,s),
    d_stdin(NULL), d_stdout(NULL), d_stderr(NULL), accel(0),lts(NULL)
{

  int _stdin=-1,_stdout=-1,_stderr=-1;
  gchar **argv = NULL;
  gint argc=0;
  GError *gerr=NULL;
  char* read_buf=NULL;
  size_t read_buf_pos=0;


  g_shell_parse_argv(s.c_str(),&argc,&argv,&gerr);

  if (gerr) {
    errormsg = "aal_remote: g_shell_parse_argv error: " + std::string(gerr->message)
      + " when parsing " + s;
    _log.debug(errormsg.c_str());
    status = false;
    return;
  }

  _g_spawn_async_with_pipes(NULL,argv,NULL,(GSpawnFlags)(G_SPAWN_SEARCH_PATH|G_SPAWN_DO_NOT_REAP_CHILD),NULL,NULL,&pid,&_stdin,&_stdout,&_stderr,&gerr);

  for(int i=0;i<argc;i++) {
    if (argv[i]) {
      free(argv[i]);
    }
  }
  free(argv);

  if (gerr) {
    errormsg = "aal_remote: g_spawn_async_with_pipes error: " + std::string(gerr->message);
    _log.debug(errormsg.c_str());
    status = false;
    return;
  }

  monitor(&status);

  prefix="aal remote("+s+")";

  //#ifndef __MINGW32__
  d_stdin=g_io_channel_unix_new(_stdin);
  d_stdout=g_io_channel_unix_new(_stdout);
  d_stderr=g_io_channel_unix_new(_stderr);
  /*#else
  d_stdin=g_io_channel_win32_new_fd(_stdin);
  d_stdout=g_io_channel_win32_new_fd(_stdout);
  d_stderr=g_io_channel_win32_new_fd(_stderr);
#endif
  */
  g_io_channel_set_encoding(d_stdout,NULL,NULL);

  g_io_channel_set_flags(d_stderr,(GIOFlags)(G_IO_FLAG_NONBLOCK|
				  (int)g_io_channel_get_flags(d_stderr))
			 ,NULL);

  _log.debug("Waiting for actions");

  ssize_t red=bgetline(&read_buf,&read_buf_pos,d_stdout,l,d_stdin);

  _log.debug("aname %i",red);

  action_names.push_back("TAU");

  while (red>1) {
    action_names.push_back(read_buf);
    red=bgetline(&read_buf,&read_buf_pos,d_stdout,l,d_stdin);
    _log.debug("aname %i",red);
  }

  _log.debug("Waiting for tags");
  red=bgetline(&read_buf,&read_buf_pos,d_stdout,l,d_stdin);
  _log.debug("tname %i",red);

  tag_names.push_back("TAU");

  while (red>1) {
    tag_names.push_back(read_buf);
    red=bgetline(&read_buf,&read_buf_pos,d_stdout,l,d_stdin);
    _log.debug("tname %i",red);
  }

  g_free(read_buf);
  read_buf=NULL;
  read_buf_pos=0;

  g_io_channel_flush(d_stdin,NULL);
}

void aal_remote::handle_stderr() {
  char* line=NULL;
  size_t n=0;
  char* read_buf=NULL;
  size_t read_buf_pos=0;

  if (nonblock_getline(&line,&n,d_stderr,read_buf,read_buf_pos) && line) {
    const char * escaped = escape_string(line);
    if (escaped) {
        const char* m[] = {"<remote stderr=\"%s\">\n","%s"};
        _log.error(m, escaped);
        escape_free(escaped);
    }
    free(line);
  }
  free(read_buf);
}

// TODO: Filter out duplicate alphabets?
void aal_remote::read_alphabet_update() {
  char* read_buf=NULL;
  size_t read_buf_pos=0;
  ssize_t red=bgetline(&read_buf,&read_buf_pos,d_stdout,_log,d_stdin);

  _log.debug("Waiting for action names");
  while (red>1) {
    _log.debug("aname %i (%s)",red,read_buf);
    action_names.push_back(read_buf);
    red=bgetline(&read_buf,&read_buf_pos,d_stdout,_log,d_stdin);
  }

  _log.debug("Waiting for tags");
  red=bgetline(&read_buf,&read_buf_pos,d_stdout,_log,d_stdin);

  while (red>1) {
    _log.debug("tname %i (%s)",red,read_buf);
    tag_names.push_back(read_buf);
    red=bgetline(&read_buf,&read_buf_pos,d_stdout,_log,d_stdin);
  }

}

int aal_remote::alphabet_getint(GIOChannel* out,GIOChannel* in,Log& log,
				Writable* w,
				GIOChannel* magic) {

  int r=getint(out,in,log,Alphabet::ALPHABET_MIN,
	       action_names.size(),w,magic);
  /*
  if (r==Alphabet::UPDATE) {
    read_alphabet_update();
  }
  */
  while (r==Alphabet::UPDATE) {
    read_alphabet_update();
    update();
    r=getint(out,in,log,Alphabet::ALPHABET_MIN,action_names.size(),w,magic);
  }

  return r;
}

int aal_remote::adapter_execute(int action,const char* params) {
  while(g_main_context_iteration(NULL,FALSE));

  if (!status) {
    return 0;
  }

  _log.debug("adapter_execute enter");

  if (params)
    fprintf(d_stdin, "ap%s\n",params);

  fprintf(d_stdin, "a%i\n", action);
  int r= alphabet_getint(d_stdin,d_stdout,_log,this,d_stdin);
  _log.debug("adapter_execute exit");
  return r;
}

int aal_remote::model_execute(int action) {
  if (!status) {
    return 0;
  }

  if (accel>0) {
    return lts->execute(action);
  }

  while(g_main_context_iteration(NULL,FALSE));
  handle_stderr();

  fprintf(d_stdin, "m%i\n", action);
  return alphabet_getint(d_stdin,d_stdout,_log,this,d_stdin);
}

void aal_remote::adapter_exit(Verdict::Verdict verdict,
			      const std::string& reason)
{
  if (!status) {
    return;
  }

  while(g_main_context_iteration(NULL,FALSE));
  handle_stderr();

  std::string me_reason(reason);
  escape_string(me_reason);

  fprintf(d_stdin,"ae %s %s\n",to_string(verdict).c_str(),me_reason.c_str());

  while(g_main_context_iteration(NULL,FALSE));
  handle_stderr();
  getint(d_stdin, d_stdout,_log,0,1,this,d_stdin);
}

bool aal_remote::get_accel() {
  std::string s="lts"+to_string(_g_simulation_depth_hint+1)+"\n";
  if (fprintf(d_stdin,s.c_str())!=(int)s.length()) {
    status=false;
  }
  g_io_channel_flush(d_stdin,NULL);
  if (g_io_channel_flush(d_stdin,NULL)!=G_IO_STATUS_NORMAL) {
    status=false;
  }

  int r=getint(d_stdin,d_stdout,_log,0,INT_MAX,this,d_stdin);
  if (status && r>0) {
    char* lts_content = new char[r+2];
    lts_content[r]=0;
    gsize bytes_read;
    gsize total_read=0;
    GIOStatus status;
    do {
      bytes_read=0;
      status=g_io_channel_read_chars(d_stdout,lts_content+total_read,r-total_read,&bytes_read,NULL);
      total_read+=bytes_read;
    } while ((long)total_read < r && status != G_IO_STATUS_ERROR &&
	     status != G_IO_STATUS_EOF );
    _log.debug("Got %i bytes strlen %i",total_read,strlen(lts_content));
    lts=new Lts(_log,std::string("remote.lts#")+lts_content);
    delete[] lts_content;
    if (lts && lts->status && lts->init() && lts->reset() && lts->status) {
      accel=1;
      return true;
    } else {
      if (lts) {
	delete lts;
	lts=NULL;
      }
    }
  }
  handle_stderr();
  while(g_main_context_iteration(NULL,FALSE));
  accel=-1; // do not try local lts acceleration another time
  return false;
}

void aal_remote::push() {
  if (status) {
    if (accel>0) {
      accel_push();
      return;
    }

    while(g_main_context_iteration(NULL,FALSE));
    handle_stderr();

    if (accel==0 && _g_simulation_depth_hint > 0) {
      if (get_accel()) {
	return;
      }
    }
    send_command("mu\n");
  }
}

void aal_remote::send_command(const char* cmd) {
  
  if (fprintf(d_stdin,cmd)!=3) {
    status=false;
  }

  if (g_io_channel_flush(d_stdin,NULL)!=G_IO_STATUS_NORMAL) {
    status=false;
  }

}


void aal_remote::accel_push() {
  accel++;
  lts->push();
}

void aal_remote::accel_pop() {
  accel--;
  if (accel==0) {
    delete lts;
    lts=NULL;
  } else {
    lts->pop();
  }
}


void aal_remote::pop() {
  if (status) {
    if (accel>0) {
      accel_pop();
    } else {
      while(g_main_context_iteration(NULL,FALSE));
      handle_stderr();
      send_command("mo\n");
    }
  }
}

bool aal_remote::reset() {
  handle_stderr();
  fprintf(d_stdin, "mr\n");
  bool rv = (getint(d_stdin,d_stdout,_log,0,1,this,d_stdin) == 1);
  if (!rv) {
    errormsg = "aal_remote model failed to reset \"" + params + "\".\n"
      "      (try executing: echo mr | " + params + ")";
    status = false;
  }
  return rv;
}

bool aal_remote::init() {
  fprintf(d_stdin, "ai\n");
  bool rv = (getint(d_stdin,d_stdout,_log,0,1,this,d_stdin) == 1);
  if (!rv) {
    errormsg = "aal_remote adapter failed to init \"" + params + "\".\n"
      "      (try executing: echo ai | " + params + ")";
    status = false;
  }
  return rv;
}

int aal_remote::getActions(int** act) {
  int rv;

  if (!status) {
    return 0;
  }

  if (accel>0) {
    return lts->getActions(act);
  }

  while(g_main_context_iteration(NULL,FALSE));
  handle_stderr();

  fprintf(d_stdin, "ma\n");
  if ((rv = getact(act,actions,d_stdin,d_stdout,_log,
		   1,action_names.size(),this,d_stdin)) >= 0) {
    return rv;
  }
  status = false;
  errormsg = "corrupted list of enabled actions";
  return 0;
}

int aal_remote::getprops(int** pro) {
  int rv;

  if (!status) {
    return 0;
  }

  if (accel>0) {
    return lts->getprops(pro);
  }

  while(g_main_context_iteration(NULL,FALSE));
  handle_stderr();

  fprintf(d_stdin, "mp\n");

  if ((rv = getact(pro,tags,d_stdin,d_stdout,_log,
                   0,tag_names.size(),this,d_stdin)) >= 0) {
    return rv;
  } else {
    status = false;
    errormsg = "corrupted list of tags";
    return 0;
  }
}

int aal_remote::check_tags(std::vector<int>& tag,std::vector<int>& t)
{
  int rv;
  while(g_main_context_iteration(NULL,FALSE));

  if (!status) {
    return 0;
  }

  handle_stderr();

  std::string s;

  for(size_t i=0;i<tag.size();i++) {
    s=s+" "+to_string(tag[i]);
  }

  fprintf(d_stdin, "act%s\n",s.c_str());

  if ((rv = getact(NULL,t,d_stdin,d_stdout,_log,
                   0,tag_names.size(),this,d_stdin)) >= 0) {
    return rv;
  }
  status = false;
  errormsg = "corrupted list of failed tags";
  return 0;
}

int aal_remote::observe(std::vector<int> &action, bool block)
{
  while(g_main_context_iteration(NULL,FALSE));

  if (!status) {
    action.clear();
    action.push_back(Alphabet::SILENCE);
    return true;
  }

  handle_stderr();

  if (block) {
    fprintf(d_stdin, "aob\n"); // block
  } else {
    fprintf(d_stdin, "aop\n"); // poll
  }
  int action_alternatives = getact(NULL, action, d_stdin, d_stdout,_log,
                                   Alphabet::ALPHABET_MIN,
                                   action_names.size(),this,d_stdin);
  if (action_alternatives < 0) {
      status = false;
      errormsg = "corrupted list of output actions";
      return 0;
  }

  if (action_alternatives > 0) {
    if (action[0] == Alphabet::SILENCE) {
      action.clear();
      return Alphabet::SILENCE;
    }
  }
  return action_alternatives != 0;
}

#include <cstring>
#include "helper.hh"

namespace {
  aal* al_helper(Log& l, std::string params) {
    std::string remotename(params);
    unescape_string(remotename);
    std::string fullname("aal_remote("+remotename+")");

    if (aal::storage==NULL) {
      aal::storage=new std::map<std::string,aal*>;
    }

    aal* al=(*aal::storage)[fullname];
    if (!al) {
      al=new aal_remote(l,remotename);
      (*aal::storage)[fullname]=al;
    }
    return al;
  }

  Adapter* adapter_creator(Log& l, std::string params,void*) {
    aal* al=al_helper(l,params);

    if (al) {
      return new Awrapper(l,params,al);
    }
    return NULL;
  }

  Model* model_creator(Log& l, std::string params,void*) {
    aal* al=al_helper(l,params);

    if (al) {
      return new Mwrapper(l,params,al);
    }
    return NULL;
  }

  static ModelFactory  ::Register Mo("aal_remote", model_creator);
  static AdapterFactory::Register Ad("aal_remote", adapter_creator);
}

#endif
