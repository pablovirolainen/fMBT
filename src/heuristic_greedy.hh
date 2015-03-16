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

#ifndef __heuristic_greedy_hh__
#define __heuristic_greedy_hh__

#include <vector>
#include <string>

#include "heuristic.hh"
#include "coverage.hh"
#include "lts.hh"

class Random;
class Function;
class AlgPathToBestCoverage;
#include "function_const.hh"

class Heuristic_greedy : public Heuristic {
public:
  Heuristic_greedy(Log& l,bool _adaptive,const std::string& params);
  virtual ~Heuristic_greedy();

  virtual bool execute(int action);

  virtual int getAction();
  virtual int getIAction();
private:
  inline double search(AlgPathToBestCoverage& alg,double current_score);
  Function* m_search_depth;
  bool m_burst;
  std::vector<std::pair<int,double> > m_path;
  Random* r;
  Function* randomise_function;
protected:
  bool adaptive;
  Function_const* const_index;
  int  array_size;
public:
  bool end_condition;
};

class Heuristic_lookahead: public Heuristic_greedy {
public:
  Heuristic_lookahead(Log& l,const std::string& params):
    Heuristic_greedy(l,false,params) { }
  virtual ~Heuristic_lookahead() {}
};

class Heuristic_adaptive_lookahead: public Heuristic_greedy {
public:
  Heuristic_adaptive_lookahead(Log& l,const std::string& params):
    Heuristic_greedy(l,true,params) { }
  virtual ~Heuristic_adaptive_lookahead() {}
  virtual void set_learn(Learning* _learn);
};


#endif
