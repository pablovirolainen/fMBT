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

#include "heuristic_weight.hh"
#include "helper.hh"
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include "learn_proxy.hh"
#include "random.hh"

#include "dparse.h"
extern "C" {
  extern D_ParserTables parser_tables_weight;
}

extern Heuristic_weight* Hw;

Heuristic_weight::Heuristic_weight(Log& l,const std::string& params) :
  Heuristic(l), r(NULL),use_learn(false)
{
  std::vector<std::string> subs;
  commalist(params,subs);
  switch (subs.size()) {
  case 2:
    prm=subs[0];
    r=new_random(subs[1]);

    if (!r) {
      status=false;
      errormsg="Can't create random "+subs[1];
    } else {
      r->ref();
      if (!r->status) {
	status=false;
	errormsg=r->errormsg;
      } else {
	l.push("Heuristic=weight");
	l.print("<random init=\"%s\"/>\n",r->stringify().c_str());
	l.pop();
      }
    }

    break;
  case 1:
    prm=subs[0];
    r=Random::default_random();
    r->ref();
    break;
  default:
    status=false;
    errormsg="Illegal amount of parameters ("+to_string((unsigned int)subs.size())+"). Expecting 1 or 2";
  }
}

Heuristic_weight::~Heuristic_weight()
{
  if (r)
    r->unref();
}

float Heuristic_weight::getCoverage() {
  if (my_coverage==NULL) {
    return 0.0;
  }
  return my_coverage->getCoverage();
}

void Heuristic_weight::add(std::vector<std::string*> p,
			   std::vector<std::string*> a,
			   int w,int o)
{
  static int weight_id = 0;
  std::vector<int> props;
  std::vector<int> actions;
  std::vector<int>::iterator it;

  for(unsigned i=0;i<p.size();i++) {
    regexpmatch(*(p[i]),model->getSPNames(),props,false);
    delete p[i];
  }

  for(unsigned i=0;i<a.size();i++) {
    regexpmatch(*(a[i]),model->getActionNames(),actions,false,1,1);
    delete a[i];
  }

  // Remove duplicate matches
  std::sort(actions.begin(), actions.end());
  it = std::unique(actions.begin(), actions.end());
  actions.resize(it - actions.begin());

  std::sort(props.begin(), props.end());
  it = std::unique(props.begin(), props.end());
  props.resize(it - props.begin());

  // List of required propositions is empty =>
  // don't care about propositions, match all states
  if (p.empty())
    props.push_back(-1);

  if (actions.empty()) {
    return;
  }

  for(unsigned i=0;i<props.size();i++) {
    for(unsigned j=0;j<actions.size();j++) {
      switch (o) {
      case 1:
	weights[std::pair<int,int>(props[i],actions[j])]=w;
	break;
      default:
	weights[std::pair<int,int>(props[i],actions[j])]+=w;
      }
      weight_ids[std::pair<int,int>(props[i],actions[j])]=weight_id;
    }
  }
  weight_id++;
}

void Heuristic_weight::set_learn(Learning* _learn) {
  Heuristic::set_learn(_learn);

  if (learn && ((Learn_proxy*)learn)->la) {
    use_learn=true;
  }
}


int Heuristic_weight::weight_select(int i,int* actions)
{
  int* props;
  int p=model->getprops(&props);
  float total=0;
  std::vector<float> f;
  std::map< std::pair<int,int>, bool > seen_weight_ids;

  f.resize(i);

  // Go through props
  for(int j=0;j<p;j++) {
    // Go through actions
    for(int k=0;k<i;k++) {
      // Make sure every weight spec has an effect to each enabled
      // action at most once - even if a weight spec matches to this
      // state because of multiple state tag hits.
      std::pair<int,int> pa(props[j],actions[k]);
      float ff = weights[pa];
      int id = weight_ids[pa];
      if (ff != 0.0 && !seen_weight_ids[std::pair<int,int>(actions[k],id)]) {
        f[k]+=ff;
        seen_weight_ids[std::pair<int,int>(actions[k],id)] = true;
      }
    }
  }

  // And then without prop
  // Go through actions
  for(int k=0;k<i;k++) {
    float ff=weights[std::pair<int,int>(-1,actions[k])];
    f[k]+=ff;
  }

  // Make sure there are no negative weights
  for (int k=0; k<i; k++) {
    if (f[k] < 0.0) {
      f[k] = 0.0;
    } else {
      if (use_learn) {
	f[k]*=learn->getF(actions[k]);
      }
    }
    total += f[k];
  }

  // Total weight 0?
  if (total==0.0) {
    return r->drand48()*i;
  }

  float cut=r->drand48()*total;
  int pos;
  for(pos=0;cut>f[pos];cut-=f[pos],pos++);

  return pos;
}

int Heuristic_weight::getAction()
{
  int* actions;
  int i;

  i=model->getActions(&actions);

  if (i==0) {
    // DEADLOCK
    return Alphabet::DEADLOCK;
  }

  int pos=weight_select(i,actions);

  return actions[pos];
}

int Heuristic_weight::getIAction()
{
  int* actions;
  int i;

  i=model->getIActions(&actions);

  if (i==0) {
    // Ok.. no output actions
    i=model->getActions(&actions);
    if (i==0) {
      return Alphabet::DEADLOCK;
    }
    return Alphabet::OUTPUT_ONLY;
  }

  int pos=weight_select(i,actions);

  return actions[pos];
}

extern int weight_node_size;

void Heuristic_weight::alphabet_update(Alphabet*)
{
  char* s=readfile(prm.c_str());
  if (s) {
    weights.clear();
    weight_ids.clear();
    Heuristic_weight* h=Hw;
    Hw=this;
    D_Parser *p = new_D_Parser(&parser_tables_weight, weight_node_size);
    p->loc.pathname = prm.c_str();
    bool ret=dparse(p,s,std::strlen(s));
    ret=p->syntax_errors==0 && ret;
    free_D_Parser(p);
    free(s);
    status=ret;
    if (!ret) {
      errormsg="Syntax error while parsing \""+prm+"\"";
    }
    Hw=h;
  } else {
    status=false;
    errormsg="Can't read inputfile \"" + prm + "\"";
  }
}

void Heuristic_weight::set_model(Model* _model)
{
  Heuristic::set_model(_model);
  alphabet_update(model);
}

void Heuristic_weight::set_coverage(Coverage* c)
{
  Heuristic::set_coverage(c);
}

FACTORY_DEFAULT_CREATOR(Heuristic, Heuristic_weight, "weight")
