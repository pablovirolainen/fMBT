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

#ifndef __coverage_proxy_hh__
#define __coverage_proxy_hh__

#include "coverage.hh"
#include "coverage.hh"
#include "proxy.hh"

extern Proxy callback_proxy;

class Coverage_proxy : public Coverage, public Proxy {
public:
  Coverage_proxy(Log& l,Coverage* _c,const std::string& _n);

  virtual ~Coverage_proxy() {
    delete c;
  }

  virtual void push() {
    c->push();
  }
  virtual void pop() {
    c->pop();
  }

  virtual bool set_instance(int instance) {
    return c->set_instance(instance);
  }

  virtual void history(int action, std::vector<int>& props,
 		       Verdict::Verdict verdict) {
    c->history(action,props,verdict);
  }

  virtual bool execute(int action) {
    return c->execute(action);
  }

  virtual float getCoverage() {
    return c->getCoverage();
  }

  virtual std::string stringify() {
    return c->stringify();
  }

  virtual int fitness(int* actions,int n, float* fitness) {
    return c->fitness(actions,n,fitness);
  }
  virtual void set_model(Model* _model) {
    Coverage::set_model(_model);
    c->set_model(model);
  }

protected:

  bool get(std::string params,std::string& ret_str) {
    printf("Kutsuttiin coverage_proxy:n get-metodia!\n");
    if (c) {
      ret_str=name;
    } else {
      ret_str="";
    }
    return true;
  }
  
  bool set(std::string params,std::string& ret_str) {
    Coverage* cc=new_coverage(log,params);
    printf("Kutsuttiin coverage_proxy:n set-metodia!\n");
    if (cc) {
      name=params;
      delete c;
      c=cc;
      ret_str="True";
    } else {
      ret_str="False";
    }
    return true;
  }

  Coverage* c;
  std::string name;

};

#endif
