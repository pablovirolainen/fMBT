/*
 * fMBT, free Model Based Testing tool
 * Copyright (c) 2011, Intel Corporation.
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

#include <stack>

#include "conf.hh"
#include "log.hh"
#include "log_null.hh"
#include "helper.hh"
#include <iostream>
#include <unistd.h>
#include <cstdlib>
#include <signal.h>
#include "coverage.hh"
#include "adapter.hh"
#include "model.hh"

#include "verdict.hh"
#include "config.h"

#include <glib-object.h>

#include "error.hh"

#include <cstdio>
#include <getopt.h>

void print_usage()
{
  std::printf(
    "Usage: fmbt [options] configfile\n"
    "Options:\n"
    "    -D     enable debug output (written to the log)\n"
    "    -E     print precompiled configuration in human readable form\n"
    "    -e     print precompiled configuration in machine readable form\n"
    "    -h     help\n"
    "    -i     start in interactive mode\n"
    "    -L<f>  append log to file f (default: standard output)\n"
    "    -l<f>  overwrite log to file f (default: standard output)\n"
    "    -q     quiet, do not print test verdict\n"
    "    -V     print version ("VERSION FMBTBUILDINFO")\n"
    "    -o     configfile line\n"
    "    -C     print coverage modules.\n"
    );
}

void nop_signal_handler(int signum)
{
}

int main(int argc,char * const argv[])
{
  FILE* logfile=stdout;
  bool interactive=false;
  bool debug_enabled=false;
  bool E=false;
  bool quiet=false;
  int c;
  std::string config_options;

  static struct option long_opts[] = {
    {"help", no_argument, 0, 'h'},
    {"version", no_argument, 0, 'V'},
    {0, 0, 0, 0}
  };

  while ((c = getopt_long (argc, argv, "DEL:heil:qCo:VAMH", long_opts, NULL)) != -1)
    switch (c)
    {
    case 'V':
      printf("Version: "VERSION FMBTBUILDINFO"\n");
      return 0;
      break;
    case 'o': {
      config_options=config_options+optarg+"\n";
      break;
    }
    case 'C': {
      /* For debugging. print coverage modules */
      std::map<std::string, std::pair<CoverageFactory::creator,void*> >::iterator i;
      std::map<std::string, std::pair<CoverageFactory::creator,void*> >::iterator e;
      if (CoverageFactory::creators) {
	i=CoverageFactory::creators->begin();
	e=CoverageFactory::creators->end();

	for(;i!=e;++i) {
	  printf("%s\n",i->first.c_str());
	}
      }
    }
      return 0;
      break;

    case 'H': {
      /* For debugging. print heuristic modules */
      std::map<std::string, std::pair<HeuristicFactory::creator,void*> >::iterator i;
      std::map<std::string, std::pair<HeuristicFactory::creator,void*> >::iterator e;
      if (HeuristicFactory::creators) {
	i=HeuristicFactory::creators->begin();
	e=HeuristicFactory::creators->end();

	for(;i!=e;++i) {
	  printf("%s\n",i->first.c_str());
	}
      }
    }
      return 0;
      break;


    case 'A': {
      /* For debugging. print adapter modules */
      std::map<std::string, std::pair<AdapterFactory::creator,void*> >::iterator i;
      std::map<std::string, std::pair<AdapterFactory::creator,void*> >::iterator e;
      if (AdapterFactory::creators) {
	i=AdapterFactory::creators->begin();
	e=AdapterFactory::creators->end();

	for(;i!=e;++i) {
	  printf("%s\n",i->first.c_str());
	}
      }
    }
      return 0;
      break;
    case 'M': {
      /* For debugging. print model modules */
      std::map<std::string, std::pair<ModelFactory::creator,void*> >::iterator i;
      std::map<std::string, std::pair<ModelFactory::creator,void*> >::iterator e;
      if (ModelFactory::creators) {
	i=ModelFactory::creators->begin();
	e=ModelFactory::creators->end();

	for(;i!=e;++i) {
	  printf("%s\n",i->first.c_str());
	}
      }
    }
      return 0;
      break;
    case 'D':
      debug_enabled=true;
      break;
    case 'E':
      human_readable=true;
      E=true;
      break;
    case 'L':
    case 'l':
      if (logfile!=stdout) {
        error(33, 0, "too many logfiles given.\n");
      }
      logfile=fopen(optarg,c=='L'?"a":"w");
      if (!logfile) {
        error(34, 0, "cannot open logfile \"%s\"\n",optarg);
      }
      break;
    case 'e':
      human_readable=false;
      E=true;
      break;
    case 'i':
      interactive=true;
      break;
    case 'q':
      quiet=true;
      break;
    case 'h':
      print_usage();
      return 0;
    default:
      return 2;
    }

  if (optind == argc) {
    print_usage();
    error(0, 0, "test configuration file missing.\n");
    return 32;
  }
#ifndef __MINGW32__
  signal(SIGPIPE, nop_signal_handler);
#endif
#if !GLIB_CHECK_VERSION(2, 35, 0)
  g_type_init ();
#endif
  {
    Log* log;

    if (E) {
      if (logfile!=NULL && logfile!=stderr) {
	fclose(logfile);
	logfile=NULL;
      }
      log=new Log_null;
    } else {
      log=new Log(logfile);
    }

    Conf c(*log,debug_enabled);
    std::string conffilename(argv[optind]);
    c.load(conffilename,config_options);

    if (!c.status) {
      c.exit_status=4;
      error(0, 0, "%s\n", c.stringify().c_str());
      // Let's handle verdict
      c.handle_hooks(Verdict::W_ERROR);
      return c.exit_status;
    }
    if (E) {
      std::fprintf(stderr, "%s\n",c.stringify().c_str());
    } else {
      Verdict::Verdict v = c.execute(interactive);
      if (!c.status) {
        std::fprintf(stderr, "%s\n",c.stringify().c_str());
      } else if (!quiet && v != Verdict::UNDEFINED) {
        std::fprintf(stderr, "%s\n",c.errormsg.c_str());
      }
      return c.exit_status;
    }
  }

  return 0;
}
