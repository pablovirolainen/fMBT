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

#include "end_condition.hh"
#include "adapter.hh"
#include "model.hh"
#include "coverage.hh"
#include <stdlib.h>

#include <stdio.h> // DEBUG

#include "helper.hh"

End_condition::End_condition(Conf* _conf,Counter _counter,
			     Verdict::Verdict v, const std::string& p)
  : verdict(v),counter(_counter), param(p),
    param_float(-1.0), param_long(-1), param_time(-1),notify_step(-1)
{
}

End_condition::~End_condition()
{

}

End_condition* new_end_condition(Verdict::Verdict v,const std::string& s,Conf* conf)
{
  End_condition* ret=NULL;
  std::string name,option;
  param_cut(s,name,option);

  ret = End_conditionFactory::create(v,name,option,conf);

  if (ret) {
    return ret;
  }

  //Let's try old thing.
  split(s, name, option);
  ret = End_conditionFactory::create(v,name,option,conf);

  if (ret) {
      fprintf(stderr,
	      "DEPRECATED END CONDITION SYNTAX. %s\nNew syntax is %s(%s)\n",
	      s.c_str(),name.c_str(),option.c_str());
  }


  return ret;
}

End_condition_duration::End_condition_duration
(Conf* _conf,Verdict::Verdict v, const std::string& p):
    End_condition(_conf,DURATION,v,p) {
    er="time limit reached";
    status=true;
    param_time = -1;
#ifndef DROI
    char* out = NULL;
    int stat;
    std::string ss = "date --date='" + param + "' +%s.%N";

    if (g_spawn_command_line_sync(ss.c_str(), &out, NULL, &stat, NULL)) {
      if (!stat) {
	// Store seconds to param_time and microseconds to param_long
	param_time = atoi(out);
	param_long = (strtod(out, NULL) - param_time) * 1000000;
	status = true;
	if (_conf) {
	  struct timeval tv;
	  gettime(&tv);
	  _conf->log.debug("Until %i,current %i",param_time,tv.tv_sec);
	}
      } else {
	errormsg = "Parsing 'duration' parameter '" + param + "' failed.";
	errormsg += " Date returned an error when executing '" + ss + "'";
	status = false;
      }
    } else {
      errormsg = "Parsing 'duration' parameter '" + param + "' failed, could not execute '";
      errormsg += ss + "'";
      status = false;
    }
    if (out) {
      g_free(out);
    }
#else
    char* endp;
    long r = strtol(param.c_str(), &endp, 10);
    if (*endp == 0) {
      param_time = r;
      status = true;
    } else {
      // Error on str?
      errormsg = "Parsing duration '" + param + "' failed.";
      status = false;
    }
#endif
  }


End_condition_coverage::End_condition_coverage(Conf* _conf,Verdict::Verdict v, const std::string& p):
  End_condition(_conf,COVERAGE,v,p) {
  if (param.empty()) {
    er="coverage reached";
  } else {
    er="coverage "+param+" reached";
  }
  status = true;
  cconst=false;
  l.ref();
  c = new_coverage(l,param);
  if (c==NULL) {
    status=false;
    errormsg=param+" not valid coverage";
  } else {
    if (c->status) {
      if ((dynamic_cast<Coverage_Const*>(c))!=NULL) {
	cconst=true;
	param_float = strtod(param.c_str(),NULL);
      } else {
	cconst=false;
	_conf->set_model(c);
      }
    } else {
      status=false;
      errormsg=c->errormsg;
    }
  }
}

bool End_condition_tag::match(int step_count,int state, int action,int last_step_cov_growth,Heuristic& heuristic,std::vector<int>& mismatch_tags) {
  int *t;
  int s = heuristic.get_model()->getprops(&t);
  for(int i=0; i<s; i++) {
    if (t[i] == param_long) return true;
  }
  return false;
}

bool End_condition_duration::match(int step_count,int state, int action,int last_step_cov_growth,Heuristic& heuristic,std::vector<int>& mismatch_tags) {
  if (Adapter::current_time.tv_sec > param_time + 1 ||
      (Adapter::current_time.tv_sec == param_time
       && Adapter::current_time.tv_usec >= param_long)
      ) return true;

  return false;
}

bool End_status_error::match(int step_count,int state, int action,
			     int last_step_cov_growth,Heuristic& heuristic,std::vector<int>& mismatch_tags)
{
  if (!heuristic.status) {
    er="Heuristic error: "+heuristic.errormsg;
    return true;
  }
  if (!heuristic.get_model()->status) {
    er="Model error: "+heuristic.get_model()->errormsg;
    return true;
  }
  if (!heuristic.get_coverage()->status) {
    er="Coverage error: "+heuristic.get_coverage()->errormsg;
    return true;
  }
  return false;
}

bool End_condition_tagverify::match(int step_count,int state, int action,
				    int last_step_cov_growth,Heuristic&
				    heuristic,std::vector<int>& mismatch_tags)
{
  bool rv=false;
  er="verifying tags ";
  for(unsigned i=0;i<mismatch_tags.size();i++) {
    if (std::find(filter.begin(),filter.end(),mismatch_tags[i])==filter.end()) {
      er=er+"\""+heuristic.get_model()->getSPNames()[mismatch_tags[i]]+"\" ";
      rv=true;
    }
  }
  er=er+"failed.";
  return rv;
}

bool End_condition_tagverify::evaluate_filter(std::vector<std::string>& tag)
{
  return evaluate_filter(tag,param);
}

std::string End_condition_dummy::stringify() {
  if (!status) {
    if (!c) {
      return "Can't create coverage " + param;
    }
    if (!c->status) {
      return "Coverage error " + c->errormsg;
    }
    return "????";
  }
  return errormsg;
}

bool End_condition_tagverify::evaluate_filter(std::vector<std::string>& tags,std::string& s)
{
  if (s=="") {
    return true;
  }
  std::string name,option;
  std::vector<std::string> f;
  param_cut(s,name,option);
  commalist(option,f);

  if (name=="include") {
    std::vector<int> tmp;
    strlist(f);
    find(tags,f,tmp);
    for(unsigned i=0;i<tags.size();i++) {
      if (std::find(tmp.begin(),tmp.end(),i)==tmp.end()) {
	filter.push_back(i);
      }
    }
    return true;
  }

  if (name=="exclude") {
    strlist(f);
    find(tags,f,filter);
    return true;
  }
  return false;
}

#undef FACTORY_CREATE_PARAMS
#undef FACTORY_CREATOR_PARAMS
#undef FACTORY_CREATOR_PARAMS2
#undef FACTORY_CREATE_DEFAULT_PARAMS

#define FACTORY_CREATE_DEFAULT_PARAMS /* */

#define FACTORY_CREATE_PARAMS Verdict::Verdict v,	               \
                       std::string name,                               \
                       std::string params,                             \
                       Conf* co

#define FACTORY_CREATOR_PARAMS Verdict::Verdict v, std::string params,Conf* co
#define FACTORY_CREATOR_PARAMS2 v, params,co

FACTORY_IMPLEMENTATION(End_condition)
#undef FACTORY_CREATOR_PARAMS2
#define FACTORY_CREATOR_PARAMS2 co, v, params

FACTORY_DEFAULT_CREATOR(End_condition, End_condition_steps,     "steps")
FACTORY_DEFAULT_CREATOR(End_condition, End_condition_coverage,  "coverage")
FACTORY_DEFAULT_CREATOR(End_condition, End_condition_tag,       "tag")
FACTORY_DEFAULT_CREATOR(End_condition, End_condition_tag,       "statetag")
FACTORY_DEFAULT_CREATOR(End_condition, End_condition_duration,  "duration")
FACTORY_DEFAULT_CREATOR(End_condition, End_condition_noprogress,"noprogress")
FACTORY_DEFAULT_CREATOR(End_condition, End_condition_noprogress,"no_progress")
FACTORY_DEFAULT_CREATOR(End_condition, End_condition_deadlock,  "deadlock")
FACTORY_DEFAULT_CREATOR(End_condition, End_condition_tagverify, "failing_tag")
