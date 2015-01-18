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
#ifndef __test_engine_h__
#define __test_engine_h__

#include "heuristic.hh"
#include "adapter.hh"
#include "log.hh"
#include "policy.hh"
#include "end_condition.hh"

#include <list>
#include <vector>

class Test_engine {
public:
  Test_engine(Heuristic& h,Adapter& a,Log& l,Policy& p, std::vector<End_condition*>& ecs,std::map<int,bool>& _disable_tags);
  virtual ~Test_engine();
  Verdict::Verdict run(time_t _end_time=-1,bool disable_tagverify=false);
  void interactive();

  /*
    matching_end_condition returns the index of the first end
    condition that evaluates to true in the current setting, or -1 if
    all end conditions are false.
  */
  virtual int matching_end_condition(int step_count,int state=0,int action=-1);
  static time_t    end_time;

  const std::string& verdict_msg();
  const std::string& reason_msg();
  Verdict::Verdict verdict();
protected:
  void verify_tags(const std::vector<std::string>& tnames);
  void print_time(struct timeval& start_time,
		  struct timeval& total_time);
  Verdict::Verdict stop_test(End_condition* ec);
  Verdict::Verdict stop_test(Verdict::Verdict v, const char* _reason);

  int       step_count;
  Heuristic &heuristic;
  Adapter   &adapter;
  Log       &log;
  Policy    &policy;
  std::vector<End_condition*> &end_conditions;
  int       last_step_cov_growth;
  Verdict::Verdict m_verdict;
  std::string m_verdict_msg;
  std::string m_reason_msg;

  struct timeval start_time;
  struct timeval total_time;
  bool break_check;

  std::vector<int> mismatch_tags;
  bool        tagverify_disabled;
  std::map<int,bool>& disabled_tags;
};

#endif
