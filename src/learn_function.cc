/*
 * fMBT, free Model Based Testing tool
 * Copyright (c) 2012, Intel Corporation.
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

#include "learn_function.hh"
#include "helper.hh"
#include "adapter.hh"
#include "function_export.hh"

Learn_time_function::Learn_time_function(Log&l,std::string&s): Learn_time(l,"") {
  Export_double _exp("duration",&_duration);
  f=new_function(s);

  if (!f) {
    status=false;
    errormsg="Can't create function \""+s+"\"";
  } else {
    status=f->status;
    errormsg=f->errormsg;
  }
}

float Learn_time_function::getF(int action) {
  return f->fval();
}

float Learn_time_function::getC(int sug,int exe) {
  return f->fval();
}

float Learn_time_function::getE(int action) {
  return f->fval();
}

<<<<<<< HEAD
=======
/*
Function* Learn_action_function::creator(std::string params,void* p) {
  Learn_action_function* f=(Learn_action_function*)p;
  return new ;
}
*/

>>>>>>> remotes/origin/factory
Learn_action_function::Learn_action_function(Log&l,std::string&s): Learn_action(l,""),index_action(0) {
  Export_int _exp("pos",&learn_as_function);

  //FunctionFactory::add_factory("pos",Learn_action_function::creator,this);
  f=new_function(s);
<<<<<<< HEAD
=======
  //FunctionFactory::remove_factory("pos");
>>>>>>> remotes/origin/factory

  if (!f) {
    Learn_action::status=false;
    Learn_action::errormsg="Can't create function \""+s+"\"";
  } else {
    Learn_action::status=f->status;
    Learn_action::errormsg=f->errormsg;
  }
}

void Learn_action_function::suggest(int action) {
  Learn_action::suggest(action);
  index_action=action;
  learn_as_function=pos[index_action];
}

void Learn_action_function::execute(int action) {
  if (suggested) {
    suggested=false;
    if (suggested_action==action) {
      if (pos[suggested_action]) {
	pos[suggested_action]--;
      }
    } else {
      pos[suggested_action]++;
    }
  } else {
    index_action=action;
  }
  learn_as_function=pos[index_action];
}

float Learn_action_function::getF(int action) {
  index_action=action;
  return pos[action];
<<<<<<< HEAD
=======
  /*
  return f->fval();
  */
>>>>>>> remotes/origin/factory
}

float Learn_action_function::getC(int sug,int exe) {
  index_action=sug;
  return pos[sug];
<<<<<<< HEAD
=======
  /*
  return f->fval();
  */
>>>>>>> remotes/origin/factory
}

float Learn_action_function::getE(int action) {
  index_action=action;
  return pos[action];
<<<<<<< HEAD
=======
  /*
  return f->fval();
  */
>>>>>>> remotes/origin/factory
}

FACTORY_DEFAULT_CREATOR(Learning, Learn_time_function, "time_function")
FACTORY_DEFAULT_CREATOR(Learning, Learn_action_function, "action_function")
