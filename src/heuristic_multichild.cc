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

#include "heuristic_multichild.hh"
#include "helper.hh"
#include <cstdlib>
#include <cstring>

#include "random.hh"

Heuristic_multichild::~Heuristic_multichild()
{
  for(unsigned i=0;i<h.size();i++) {
    delete h[i].second;
  }
}

Heuristic_multichild::Heuristic_multichild(Log& l):Heuristic(l){}

float Heuristic_multichild::getCoverage() {
  if (my_coverage==NULL) {
    return 0.0;
  }
  return my_coverage->getCoverage();
}

void Heuristic_multichild::set_model(Model* _model)
{
  Heuristic::set_model(_model);
  for(unsigned i=0;i<h.size();i++) {
    h[i].second->set_model(_model);
    if (h[i].second->status==false) {
      status=false;
      errormsg=h[i].second->errormsg;
      return;
    }
  }
}

bool Heuristic_multichild::execute(int action)
{
  bool ret=true;
  for(unsigned i=0;i<h.size();i++) {
    model->push();
    my_coverage->push();
    ret&=h[i].second->execute(action);
    my_coverage->pop();
    model->pop();
  }
  ret&=Heuristic::execute(action);
  return ret;
}

void Heuristic_multichild::set_coverage(Coverage* c)
{
  Heuristic::set_coverage(c);
  for(unsigned i=0;i<h.size();i++) {
    h[i].second->set_coverage(c);
  }
}
