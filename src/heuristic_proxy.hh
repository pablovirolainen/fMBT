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

#ifndef __heuristic_proxy_hh__
#define __heuristic_proxy_hh__

#include "heuristic.hh"
#include "coverage.hh"
#include "proxy.hh"

extern Proxy callback_proxy;

class Heuristic_proxy : public Heuristic, public Proxy {
public:
  Heuristic_proxy(Log& l,Heuristic* _h,const std::string& _n);

  virtual ~Heuristic_proxy() {
    delete h;
  }
  virtual int getAction() {
    return h->getAction();
  }
  virtual int getIAction() {
    return h->getIAction();
  }
  virtual void set_model(Model* _model) {
    Heuristic::set_model(_model);
    h->set_model(model);
  }

  virtual void set_coverage(Coverage* c) {
    Heuristic::set_coverage(c);
    h->set_coverage(c);
  }
protected:

  bool get(std::string params,std::string& ret_str) {
    printf("Kutsuttiin heuristic_proxy:n get-metodia!\n");
    if (h) {
      ret_str=name;
    } else {
      ret_str="";
    }
    return true;
  }
  
  bool set(std::string params,std::string& ret_str) {
    Heuristic* hh=new_heuristic(log,params);
    printf("Kutsuttiin heuristic_proxy:n set-metodia!\n");
    if (hh) {
      name=params;
      delete h;
      h=hh;
      ret_str="True";
    } else {
      ret_str="False";
    }
    return true;
  }

  Heuristic* h;
  std::string name;

};

#endif
