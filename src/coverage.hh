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
#ifndef __coverage_hh__
#define __coverage_hh__

#include "factory.hh"
#include "writable.hh"
#include "log.hh"
#include "verdict.hh"
#include "alphabet.hh"

class Model;

class Coverage: public Writable,public Alphabet_update {

public:
  Coverage(Log& l);
  virtual ~Coverage();
  virtual void push()=0;
  virtual void pop()=0;

  virtual bool set_instance(int instance) {
    return false;
  }

  virtual void history(int action, std::vector<int>& props,
 		       Verdict::Verdict verdict) {
    if (action) 
      execute(action);
  }
  virtual bool execute(int action)=0;
  virtual float getCoverage()=0;

  virtual std::string stringify();

  virtual int fitness(int* actions,int n, float* fitness)=0;
  virtual void set_model(Model* _model); // for input alphabet

protected:
  Model* model;
  int current_instance;
public:
  Log& log;
  bool on_report;  
};

FACTORY_DECLARATION(Coverage)

Coverage* new_coverage(Log&,const std::string&);

#include "model.hh"

#endif

