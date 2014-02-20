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

#define _FUNCTION_INTERNAL_
#include "function_sum.hh"
#include "helper.hh"

Function_sum::Function_sum(const std::string& param) {
  std::vector<std::string> subs;
  commalist(param,subs);
  for(size_t i=0;i<subs.size();i++) {
    Function* f=new_function(subs[i]);
    if (!f) {
      status=false;
      errormsg="Can't create function "+subs[i];
      return;
    }
    if (!f->status) {
      status=false;
      errormsg=f->errormsg;
      delete f;
      return;
    }
    funcs.push_back(f);
  }
}

Function_sum::~Function_sum()
{
  for(std::list<Function*>::iterator i=funcs.begin();i!=funcs.end();i++) {
    delete *i;
  }
}

signed long Function_sum::val() {
  signed long ret=0;
  for(std::list<Function*>::iterator i=funcs.begin();i!=funcs.end();i++) {
    ret+=(*i)->val();
  }
  return ret;
}

double Function_sum::fval() {
  double ret=0;
  for(std::list<Function*>::iterator i=funcs.begin();i!=funcs.end();i++) {
    ret+=(*i)->fval();
  }
  return ret;
}

FACTORY_DEFAULT_CREATOR(Function, Function_sum, "sum")
