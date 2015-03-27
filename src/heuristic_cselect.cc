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

#include "heuristic_cselect.hh"
#include "helper.hh"
#include <cstdlib>
#include <cstring>

#include "random.hh"

Heuristic_cselect::~Heuristic_cselect()
{
  for(unsigned i=0;i<cc.size();i++) {
    delete cc[i];
  }
}

Heuristic_cselect::Heuristic_cselect(Log& l,const std::string& params) :
  Heuristic_multichild(l)
{
  std::vector<std::string> s;

  commalist(params,s);

  for(unsigned i=0;i+1<s.size();i+=2) {
    Coverage* cov=new_coverage(log,s[i]);

    if (cov==NULL) {
      status=false;
      errormsg=std::string("Can't create coverage \"")+s[i]+
        std::string("\"");
      return;
    }

    if (cov->status==false) {
      status=false;
      errormsg=cov->errormsg;
      return;
    }

    Heuristic* heu=new_heuristic(log,s[i+1]);

    if (heu==NULL) {
      delete cov;
      status=false;
      errormsg=std::string("Can't create heuristic \"")+s[i+1]+
        std::string("\"");
      return;
    }
    if (heu->status==false) {
      delete cov;
      status=false;
      errormsg=heu->errormsg;
      return;
    }

    h.push_back(std::pair<float,Heuristic*>(0,heu));
    cc.push_back(cov);
  }

  if (h.empty()) {
    status=false;
    errormsg=std::string("no subheuristics?");
  }

}

bool Heuristic_cselect::execute(int action) {
  for(unsigned i=0;i<cc.size();i++) {
    cc[i]->execute(action);
  }
  return Heuristic_cselect::execute(action);
}

void Heuristic_cselect::set_model(Model* _model) {
  for(unsigned i=0;i<cc.size();i++) {
    cc[i]->set_model(_model);
    if (! cc[i]->status) {
      status=false;
      errormsg=cc[i]->errormsg;
      return;
    }
  }
  Heuristic_multichild::set_model(_model);
}

int Heuristic_cselect::getAction()
{
  for(unsigned i=0;i<h.size();i++) {
    if (cc[i]->getCoverage()>=1.0) {
      return h[i].second->getAction();
    }
  }
  return h.rbegin()->second->getAction();
}

int Heuristic_cselect::getIAction()
{
  for(unsigned i=0;i<h.size();i++) {
    if (cc[i]->getCoverage()>=1.0) {
      return h[i].second->getIAction();
    }
  }
  return h.rbegin()->second->getAction();
}

FACTORY_DEFAULT_CREATOR(Heuristic, Heuristic_cselect, "cselect")
