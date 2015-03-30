/*
 * fMBT, free Model Based Testing tool
 * Copyright (c) 2011-2014, Intel Corporation.
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

/* Coverage_Market measures coverage requirement based on a coverage
 * language. Coverage requirements are given as a parameter. For
 * syntax refer to requirements.g.
 */

#ifndef __coverage_market_hh__
#define __coverage_market_hh__

#include <stack>

#include "coverage.hh"

#include <map>
#include <vector>

#include <cstdlib>

#define MAX(a,b) (((a)>(b))?(a):(b))
#define MIN(a,b) (((a)<(b))?(a):(b))

#include "coverage_tree.hh"
#include "coverage_prop.hh"
#include "log_null.hh"
#include "helper.hh"
#include "random.hh"

class Model;

class Coverage_Market;
extern Coverage_Market* cobj;

class Coverage_Market: public Coverage {
public:
  class unit;
  class unit_tag;
  Coverage_Market(Log& l,const std::string& _params);
  virtual ~Coverage_Market() {
    for(size_t i=0;i<Units.size();i++) {
      delete Units[i];
    }
  }

  virtual std::string stringify();

  virtual bool set_instance(int instance,bool restart=false) {
    for (unsigned int i = 0; i < Units.size(); i++) {
      Units[i]->set_instance(instance,current_instance);
      if (restart) {
	Units[i]->reset();
      }
      // Is this actually needed?
      Units[i]->update();
    }
    current_instance=instance;
    return true;
  }

  virtual void push() {
    next_prev_save.push(prev);
    next_prev_save.push(next);
    for (unsigned int i = 0; i < Units.size(); i++) Units[i]->push();
  };
  virtual void pop() {
    next=next_prev_save.top();
    next_prev_save.pop();

    prev=next_prev_save.top();
    next_prev_save.pop();
    for (unsigned int i = 0; i < Units.size(); i++) Units[i]->pop();
  };

  virtual void history(int action, std::vector<int>& props,
                       Verdict::Verdict verdict);
  virtual bool execute(int action);
  virtual float getCoverage();

  virtual int fitness(int* actions,int n, float* fitness);

  Model* get_model()
  {
    return model;
  }

  virtual void set_model(Model* _model)
  {
    model=_model;
    add_requirement(params);
    if (status) {
      int* p;
      int j=model->getprops(&p);
      std::vector<int> tmp;
      if (j) {
	tmp.assign(p,p+j);
      }
      prev=tmp;
      execute(0);
      prev=tmp;
    }
  }

  void add_requirement(std::string& req);

  unit* req_rx_action(const char m,const std::string &action,unit_tag* l=NULL,unit_tag* r=NULL,int persistent=0);

  unit_tag* req_rx_tag(const std::string &tag,char op='e');
  unit_tag* req_rx_tag(const char m,const std::string &tag,int count=1,bool exactly=false);

  void add_unit(unit* u) {
    Units.push_back(u);
  }

  std::vector<int> prev,next;

  std::stack<std::vector<int> > next_prev_save;

  /**
   * coveragerequirement
   *
   * (a action1 or e action2) and action3 then ((action4 or action5) not action1)
   *
   */


  typedef std::pair<int, int> val;

  class unit: public Alphabet_update{
  public:
    unit():alphabet(NULL) {value.first=0;value.second=0;}
    val& get_value() {
      return value;
    }
    virtual ~unit() {
      value.first=0;
      value.second=0;
    }

    virtual void alphabet_update(Alphabet* a) {
      alphabet=a;
    }

    // No need to define copy-constructor. Default is just fine

    // Derived class needs to define clone so that we'll have nice derived-deep-copy
    virtual unit* clone()=0;

    virtual void execute(const std::vector<int>& prev,int action,const std::vector<int>& next)=0;
    virtual void update()=0;
    virtual void push()=0;
    virtual void pop()=0;
    virtual void reset() {}
    virtual void set_instance(int instance,int current_instance, bool force=false) =0;
    val value;
    Alphabet* alphabet;
    virtual std::string stringify(Alphabet& a)=0;

    class eunit {
    public:
      std::vector<int> prev;
      int action;
      std::vector<int> next;

      inline bool operator< (const eunit& r) const {
	if (action<r.action || prev<r.prev || next<r.next) {
	  return true;
	}
	return false;
      }

    };

    virtual void execute(eunit& un) {
      execute(un.prev,un.action,un.next);
    }
  protected:
    unit(const unit &obj):value(obj.value),alphabet(obj.alphabet) {
      if (alphabet)
	alphabet->add_alphabet_update(this);
    }
  };

  class unit_coverage_tag: public unit {
  public:
    unit_coverage_tag(std::string* s,Coverage_Market* _m):p(*s),m(_m) {
      delete s;
      if (m) {
	child = new Coverage_Prop(m->log,p);

	if (child && !child->status && m->status) {
	  m->status=child->status;
	  m->errormsg=child->errormsg;
	} else {
	  if (child) {
	    reset();
	    if (!child->status && m->status) {
	      m->status=child->status;
	      m->errormsg=child->errormsg;
	    }
	  } else {
	    m->status=false;
	    m->errormsg="Can't create tag coverage";
	  }
	}
      }
    }
    virtual std::string stringify(Alphabet&a) {
      return "tag("+p+")";
    }

    virtual ~unit_coverage_tag() {
      if (child)
	delete child;
    }

    virtual void set_instance(int instance,int current_instance, bool force=false) {
      child->set_instance(instance);
    }

    virtual void execute(const std::vector<int>& prev,int action,const std::vector<int>& next)
    {
      child->execute(action);
    }

    virtual void update()
    {
      value.first=child->props_seen;
      value.second=child->props_total;
      if (value.second==0) {
	value.first=1;
	value.second=1;
      }
    }

    virtual void pop()
    {
      child->pop();
    }

    virtual void push()
    {
      child->push();
    }

    virtual void reset()
    {
      if (!child->status) {
	m->status=child->status;
	m->errormsg=child->errormsg;
	return;
      }

      child->set_model(m->get_model());

      if (!child->status) {
	m->status=child->status;
	m->errormsg=child->errormsg;
	return;
      }

      for(unsigned i=0;i<child->data.size();i++) {
	child->data[i]=false;
      }
      child->props_seen=0;

      value.first=child->props_seen;
      value.second=child->props_total;
    }

    virtual unit* clone() {
      return new unit_coverage_tag(*this);
    }

    std::string p;
    Coverage_Market* m;
    Coverage_Prop* child;
  };

  class unit_perm: public unit {
  public:
    unit_perm(int i,Coverage_Market* _m):m(_m),p(to_string(i)) {
      reset();
      l.ref();
    }

    virtual std::string stringify(Alphabet&a) {
      return "perm("+p+")";
    }

    virtual ~unit_perm() {
      if (child)
	delete child;
    }

    virtual void set_instance(int instance,int current_instance, bool force=false) {
      child->set_instance(instance);
    }

    virtual void push()
    {
      child->push();
      child_save.push(child);
    }

    virtual void pop()
    {
      if (child_save.top()!=child) {
	delete child;
	child=child_save.top();
      }
      child->pop();
      child_save.pop();
    }

    virtual void reset()
    {
      if (!child_save.empty() && child_save.top()!=child) {
	delete child;
      }
      child=new Coverage_Tree(l,p);
      child->set_model(m->get_model());
      value.second=child->max_count;
    }

    virtual void update()
    {
      value.first=child->node_count-1;
      value.second=child->max_count;
    }

    virtual void execute(const std::vector<int>& prev,int action,const std::vector<int>& next)
    {
      if (action)
	child->execute(action);
    }

    Log_null l;
    Coverage_Tree* child;
    std::stack<Coverage_Tree*> child_save;
    Coverage_Market* m;
    std::string p;

    virtual unit* clone() {
      return new unit_perm(*this);
    }
  protected:
    //I'll think that the default works.
    //unit_perm(const unit_perm &obj);
  };

  class unit_walk: public unit {
  public:
    virtual std::string stringify(Alphabet&a) {
      if (minimi) {
	return "uwalks("+child->stringify(a)+")";
      }
      return "eageruwalks("+child->stringify(a)+")";
    }

    unit_walk(unit* c,bool _min):child(c),count(0),minimi(_min) {
      push_depth=0;
    }

    virtual void set_instance(int instance,int current_instance, bool force=false) {
      child->set_instance(instance,current_instance,true);
      walk_instance_map[current_instance]=executed;
      executed=walk_instance_map[instance];
    }

    virtual ~unit_walk() {
      delete child;
    }

    void minimise() {
      bool child_update_needed=true;
      sexecuted.push(executed);
      while (executed.size()>1) {
	// vector pop_front
	executed.erase(executed.begin());
	if (child_update_needed) {
	  child->push();
	  child->reset();
	}
	child->execute(executed[0]);
	child->update();
	if (child->get_value().first>0) {
	  child_update_needed=true;
	  for(unsigned i=1;i<executed.size();i++) {
	    child->execute(executed[i]);
	  }
	  child->update();
	  val v=child->get_value();
	  if (v.first==v.second) {
	    // New minimal trace!
	    sexecuted.pop();
	    sexecuted.push(executed);
	  }
	  child->pop();
	} else {
	  child_update_needed=false;
	}
      }
      if (!child_update_needed) {
	child->pop();
      }
      executed=sexecuted.top();
      sexecuted.pop();
    }

    virtual void execute(const std::vector<int>& prev,int action,const std::vector<int>& next) {
      if (action) {
	bool added=false;

	child->update();
	val tmp=child->get_value();
	if (tmp.first>0) {
	  eunit u={prev,action,next};
	  executed.push_back(u);
	  added=true;
	}
	child->execute(prev,action,next);
	child->update();
	tmp=child->get_value();

	if (tmp.first>0 && !added) {
	  eunit u={prev,action,next};
	  executed.push_back(u);
	  added=true;
	}

	if (tmp.first==tmp.second) {
	  if (!added) {
	    eunit u={prev,action,next};
	    executed.push_back(u);
	  }

	  // minimise...

	  if (minimi) {
	    minimise();
	  }

	  if (push_depth!=0 &&
	      tcount_save[push_depth-1][executed]==0) {
	    tcount_save[push_depth-1][executed]=tcount[executed];
	  }

	  tcount[executed]++;
	  executed.clear();
	  child->reset();
	  value=tmp;
	  value.first=0;
	} else {
	  value=tmp;
	}
	value.first+=tcount.size()*value.second;
      }
    }

    virtual void update() {
      child->update();
      value=child->get_value();
      value.first+=(tcount.size()-count)*value.second;
    }

    virtual void reset() {
      child->reset();
      count=tcount.size();
    }

    virtual void push() {
      push_depth++;
      tcount_save.resize(push_depth);
      child->push();
      st.push(count);
      sexecuted.push(executed);
      //tcount_save.push(tcount);
    }

    virtual void pop() {
      child->pop();
      count=st.top();
      st.pop();
      executed=sexecuted.top();
      sexecuted.pop();

      push_depth--;
      std::map<std::vector<eunit >, int>::iterator i;
      std::map<std::vector<eunit >, int>::iterator e;
      i=tcount_save[push_depth].begin();
      e=tcount_save[push_depth].end();
      for(;i!=e;++i) {
	if (i->second) {
	  tcount[i->first] = i->second;
	} else {
	  tcount.erase(i->first);
	}
      }
      tcount_save.resize(push_depth);
    }

    virtual unit* clone() {
      return new unit_walk(*this);
    }
  protected:
    unit_walk(const unit_walk& o) {
      // Let's hope this triggers call to the copy constructor
      child = & (* o.child);
      minimi = o.minimi;
      push_depth=0;
      count=0;
    }

    unsigned push_depth;
    unit* child;
    unsigned count;
    std::stack<unsigned> st;

    std::vector<eunit> executed;
    std::stack<std::vector<eunit> > sexecuted;

    std::map<std::vector<eunit>, int> tcount;
    std::vector<std::map<std::vector<eunit>, int> > tcount_save;
    bool minimi;

    std::map<int,std::vector<eunit> > walk_instance_map;

  };

  class unit_many: public unit {
  public:
    unit_many() {}
    virtual ~unit_many() {
      for(size_t i=0;i<units.size();i++) {
	delete units[i];
      }
    }

    virtual void reset() {
      for(size_t i=0;i<units.size();i++) {
	units[i]->reset();
      }
    }

    virtual void push() {
      for(size_t i=0;i<units.size();i++) {
	units[i]->push();
      }
    }

    virtual void pop() {
      for(size_t i=0;i<units.size();i++) {
	units[i]->pop();
      }
    }

    virtual void update() {
      for(size_t i=0;i<units.size();i++) {
	units[i]->update();
      }
    }

    virtual void execute(const std::vector<int>& prev,int action,const std::vector<int>& next) {
      for(size_t i=0;i<units.size();i++) {
	units[i]->execute(prev,action,next);
      }
    }
    std::vector<unit*> units;

  protected:
    unit_many(const unit_many &obj);
  };

  class unit_manyleaf: public unit {
  public:
    unit_manyleaf(std::string _action):actions(0),action(_action) {}
    virtual ~unit_manyleaf() {}

    virtual void alphabet_update(Alphabet*);

    virtual void set_instance(int instance,int current_instance,
			      bool forced=false) {
      if (forced) {
	unsigned vecsize=value.size();
	manyleaf_instance_map[current_instance]=value;
	value=manyleaf_instance_map[instance];
	value.resize(vecsize);
      }
    }

    virtual void reset() {
      unit::value.first=0;
      for(unsigned i=0;i<value.size();i++) {
	value[i]=0;
      }
    }

    virtual void push() {
      st.push(value);
      st2.push(unit::value);
    }

    virtual void pop() {
      value=st.top();
      st.pop();
      unit::value=st2.top();
      st2.pop();
    }

    virtual void update() {
    }

    int actions;
    std::string action;
    std::vector<int> my_action;
    std::vector<int> value;

    std::stack<std::vector<int> > st;
    std::stack<val> st2;
    std::map<int,std::vector<int> > manyleaf_instance_map;
  protected:
    unit_manyleaf(const unit_manyleaf &obj):
      unit(obj),actions(obj.actions),action(obj.action),my_action(obj.my_action),
      value(obj.value) {}
  };

  class unit_manyleafand: public unit_manyleaf {
  public:
    virtual std::string stringify(Alphabet&a) {
      //TODO
      std::string ret="\""+a.getActionName(my_action[0])+"\"";
      for(size_t i=1;i<my_action.size();i++) {
	ret+=" and \"" + a.getActionName(my_action[i])+"\"";
      }
      return ret;

    }
    unit_manyleafand(std::string _action):unit_manyleaf(_action) {}
    virtual ~unit_manyleafand() {}

    virtual void execute(const std::vector<int>& prev,int action,const std::vector<int>& next) {
      if (unit::value.first==unit::value.second) {
	return;
      }
      for(unsigned i=0;i<my_action.size();i++) {
	if (action==my_action[i]) {
	  if (value[i]<1) {
	    value[i]++;
	    unit::value.first++;
	    return;
	  }
	}
      }
    }

    virtual unit* clone() {
      return new unit_manyleafand(*this);
    }
  };

  class unit_manyleafor: public unit_manyleaf {
  public:
    virtual std::string stringify(Alphabet&a) {
      //TODO
      std::string ret="\""+a.getActionName(my_action[0])+"\"";
      for(size_t i=1;i<my_action.size();i++) {
	ret+=" or \"" + a.getActionName(my_action[i])+"\"";
      }
      return ret;
    }

    unit_manyleafor(std::string _action):unit_manyleaf(_action) {}
    virtual ~unit_manyleafor() {}

    virtual void execute(const std::vector<int>& prev,int action,const std::vector<int>& next) {
      if (unit::value.first==unit::value.second) {
	return;
      }
      for(unsigned i=0;i<my_action.size();i++) {
	if (action==my_action[i]) {
	  if (value[i]<unit::value.second) {
	    value[i]++;
	    unit::value.first=unit::value.second;
	    return;
	  }
	}
      }
    }

    /*
    virtual void update(){
      for(size_t i=0;i<value.size();i++) {
	if (value[i]) {
	  unit::value.first = unit::value.second;
	  return;
	}
      }
      unit::value.first=0;
    }
    */
    virtual unit* clone() {
      return new unit_manyleafor(*this);
    }
  };

  class unit_manyleafrandom: public unit_manyleaf {
  public:
    virtual std::string stringify(Alphabet&a) {
      //TODO
      std::string ret="\""+a.getActionName(my_action[0])+"\"";
      for(size_t i=1;i<my_action.size();i++) {
	ret+=" or \"" + a.getActionName(my_action[i])+"\"";
      }
      return ret;
    }

    unit_manyleafrandom(std::string _action):unit_manyleaf(_action) {
      r = Random::default_random();
      random_pos=-1;
    }

    virtual ~unit_manyleafrandom() {
      r->unref();
    }

    Random* r;
    int random_pos;

    virtual void reset() {
      unit_manyleaf::reset();
      random_pos=-1;
    }

    virtual void execute(const std::vector<int>& prev,int action,const std::vector<int>& next) {
      if (unit::value.first==unit::value.second) {
	return;
      }

      if (random_pos==-1) {
	random_pos=r->drand48()*my_action.size();
      }
      
      if (action==my_action[random_pos]) {
	if (value[random_pos]<unit::value.second) {
	  value[random_pos]++;
	  unit::value.first=unit::value.second;
	  return;
	}
      }
    }
    /*
    virtual void update(){
      for(size_t i=0;i<value.size();i++) {
	if (value[i]) {
	  unit::value.first = unit::value.second;
	  return;
	}
      }
      unit::value.first=0;
    }
    */
    virtual unit* clone() {
      return new unit_manyleafrandom(*this);
    }
  };

  class unit_manyand: public unit_many {
  public:
    virtual std::string stringify(Alphabet&a) {
      std::string ret="(" + units[0]->stringify(a)+")";
      for(size_t i=1;i<units.size();i++) {
	ret+=" and (" + units[i]->stringify(a)+")";
      }
      return ret;
    }

    unit_manyand() {}
    virtual ~unit_manyand() {
    }

    virtual void set_instance(int instance,int current_instance, bool force=false) {
      // not implemented..
    }

    virtual void update() {
      units[0]->update();
      value=units[0]->get_value();
      for(size_t i=1;i<units.size();i++) {
	units[i]->update();
	val vr=units[i]->get_value();
	value.first +=vr.first;
	value.second+=vr.second;
      }
    }
    virtual unit* clone() {
      return new unit_manyand(*this);
    }
    unit_manyand(const unit_manyand&obj):unit_many(obj) {}
  };

  class unit_manyor: public unit_many {
  public:
    virtual std::string stringify(Alphabet&a) {
      std::string ret="(" + units[0]->stringify(a)+")";
      for(size_t i=1;i<units.size();i++) {
	ret+=" or (" + units[i]->stringify(a)+")";
      }
      return ret;
    }
    unit_manyor() {}
    virtual ~unit_manyor() {
    }

    virtual void set_instance(int instance,int current_instance, bool force=false) {
      // not implemented..
    }

    virtual void update() {
      units[0]->update();
      value=units[0]->get_value();
      for(size_t i=1;i<units.size();i++) {
	units[i]->update();
	val vr=units[i]->get_value();
	value.first  =
	  MAX(value.first/value.second,
	      vr.first/vr.second)*(value.second+vr.second);
	value.second+=vr.second;
      }
    }
    virtual unit* clone() {
      return new unit_manyor(*this);
    }
  };

  class unit_dual: public unit {
  public:
    unit_dual(unit* l,unit* r):left(l),right(r) {
    }

    virtual void set_instance(int instance,int current_instance, bool force=false) {
      left ->set_instance(instance,current_instance,force);
      right->set_instance(instance,current_instance,force);
    }

    virtual void reset() {
      left->reset();
      right->reset();
    }

    virtual void push() {
      left->push();
      right->push();
    }

    virtual void pop() {
      left->pop();
      right->pop();
    }

    virtual ~unit_dual() {
      delete left;
      delete right;
    }

    virtual void execute(const std::vector<int>& prev,int action,const std::vector<int>& next) {
      left->execute(prev,action,next);
      right->execute(prev,action,next);
    }
    /*
    virtual unit* clone() {
      return new unit_dual(*this);
    }
    */
  protected:
    unit_dual(const unit_dual &obj);
    unit* left,*right;

  };

  class unit_and: public unit_dual {
  public:
    virtual std::string stringify(Alphabet&a) {
      return "("+left->stringify(a)+") and ("+right->stringify(a)+")";
    }
    unit_and(unit* l,unit* r) : unit_dual(l,r) {}

    virtual void update() {
      left->update();
      right->update();
      val vl=left->get_value();
      val vr=right->get_value();
      value.first = vl.first+vr.first;
      value.second=vl.second+vr.second;
    }
    virtual unit* clone() {
      return new unit_and(*this);
    }
  };

  class unit_or: public unit_dual {
  public:
    virtual std::string stringify(Alphabet&a) {
      return "("+left->stringify(a)+") or ("+right->stringify(a)+")";
    }
    unit_or(unit* l,unit* r) : unit_dual(l,r) {}

    virtual void update() {
      left->update();
      right->update();
      val vl=left->get_value();
      val vr=right->get_value();
      /* ???? */
      value.first =
        MAX(vl.first/vl.second,
            vr.first/vr.second)*(vl.second+vr.second);
      value.second=vl.second+vr.second;
    }
    virtual unit* clone() {
      return new unit_or(*this);
    }
  };

  class unit_not: public unit {
  public:
    virtual std::string stringify(Alphabet&a) {
      return "not("+child->stringify(a)+")";
    }
    unit_not(unit *c):child(c) {
    }

    virtual void set_instance(int instance,int current_instance, bool force=false) {
      child->set_instance(instance,current_instance,force);
    }

    virtual void reset() {
      child->reset();
    }

    virtual void push() {
      child->push();
    }

    virtual void pop() {
      child->pop();
    }
    virtual ~unit_not() {
      delete child;
    }
    virtual void execute(const std::vector<int>& prev,int action,const std::vector<int>& next) {
      child->execute(prev,action,next);
    }

    virtual void update() {
      child->update();
      val v=child->get_value();
      value.first=v.second-v.first;
      value.second=v.second;
    }
    virtual unit* clone() {
      return new unit_not(*this);
    }
  protected:
    unit_not(const unit_not &obj);
    unit* child;
  };

  class unit_then: public unit_many {
  public:
    int cpos;
    std::stack<int> csave;

    virtual std::string stringify(Alphabet&a) {
      //TODO
      return "... then ... ";
    }

    virtual void set_instance(int instance,int current_instance, bool force=false) {
      // not implemented..
    }

    unit_then(unit* l,unit* r) {
      unit_then* ut=dynamic_cast<unit_then*>(r);
      cpos=0;
      if (ut) {
	// our right child is a then. We'll add left to the first.
	units.push_back(l);
	units.insert(units.end(),ut->units.begin(),ut->units.end());
	ut->units.clear();
	delete ut;
      } else {
	units.push_back(l);
	units.push_back(r);
      }
    }

    virtual void push() {
      csave.push(cpos);
      unit_many::push();
    }

    virtual void pop() {
      cpos=csave.top();
      csave.pop();
      unit_many::pop();
    }

    virtual void reset()
    {
      cpos=0;
      unit_many::reset();
    }

    virtual void execute(const std::vector<int>& prev,int action,const std::vector<int>& next) {
      for(size_t i=cpos;i<units.size()-1;i++) {
	units[i]->update();
	val v=units[i]->get_value();
	if (v.first==v.second) {
	  continue;
	} else {
	  cpos=i;
	  units[i]->execute(prev,action,next);
	  return;
	}
      }
      cpos=units.size();
      units.back()->execute(prev,action,next);
    }

    virtual void update() {
      value.first=0;
      value.second=0;
      for(size_t i=cpos;i<units.size();i++) {
	units[i]->update();
	val v=units[i]->get_value();
	value.first+=v.first;
	value.second+=v.second;
      }
    }
    virtual unit* clone() {
      return new unit_then(*this);
    }
  };

  class unit_then_: public unit_dual {
  public:
    virtual std::string stringify(Alphabet&a) {
      return "("+left->stringify(a)+") then ("+right->stringify(a)+")";
    }
    unit_then_(unit* l,unit* r) : unit_dual(l,r) {}
    virtual ~unit_then_()  {}

    virtual void set_instance(int instance,int current_instance, bool force=false) {
      left ->set_instance(instance,current_instance,true);
      right->set_instance(instance,current_instance,true);
    }


    virtual void execute(const std::vector<int>& prev,int action,const std::vector<int>& next) {
      // if left, then right
      left->update();
      val v=left->get_value();
      if (v.first==v.second) {
        right->execute(prev,action,next);
      } else {
        left->execute(prev,action,next);
	left->update();
	v=left->get_value();
	if (v.first==v.second) {
	  right->execute(prev,0,next);
	}
      }
    }

    virtual void update() {
      left->update();
      right->update();
      val vl=left->get_value();
      val vr=right->get_value();
      /* ???? */
      value.first=vl.first+vr.first;
      value.second=vl.second+vr.second;
    }
    virtual unit* clone() {
      return new unit_then_(*this);
    }
  };

  class unit_tag : public unit {
  public:
    unit_tag():unit(),left_side(false) {
    }

    virtual std::string stringify(Alphabet&a) {
      return "";
    }

    virtual ~unit_tag() { }
    virtual void set_left(bool l) {
      left_side=l;
    }

    virtual void execute(const std::vector<int>&, int, const std::vector<int>&) {}
    virtual void update() {}
    virtual void push() {}
    virtual void pop() {}
    virtual void set_instance(int, int, bool) {}

    virtual unit* clone() {
      return new unit_tag(*this);
    }
  protected:
    unit_tag(const unit_tag&obj):unit(obj),left_side(obj.left_side) {
    }
    bool left_side;

  };

  class unit_tagelist: public unit_tag {
  public:
    virtual std::string stringify(Alphabet&a) {
      // TODO
      return "";
    }
    unit_tagelist(char _op,unit_tag* l, unit_tag* r): op(_op),left(l),right(r) {
    }

    virtual ~unit_tagelist() {

    }
    virtual void set_left(bool l) {
      left_side=l;
      left->set_left(l);
      right->set_left(l);
    }
    char op;
    unit_tag *left,*right;
    virtual unit* clone() {
      return new unit_tagelist(*this);
    }
  protected:
    unit_tagelist(const unit_tagelist &obj);
  };

  class unit_tagnot: public unit_tag {
  public:
    virtual std::string stringify(Alphabet&a) {
      return "not("+child->stringify(a)+")";
    }
    unit_tagnot(unit_tag* t): child(t),ex(false) {
      if (child)
	value=child->value;
    }

    virtual ~unit_tagnot() {
      if (child)
	delete child;
    }

    virtual void set_instance(int instance,int current_instance, bool force=false) {
      child->set_instance(instance,current_instance,force);
    }

    virtual void set_left(bool l) {
      unit_tag::set_left(l);
      child->set_left(l);
    }

    virtual void reset() {
      ex = false;
      child->reset();
      update();
    }

    virtual void push() {
      child->push();
      st.push(ex);
    }

    virtual void pop() {
      child->pop();
      ex=st.top();
      st.pop();
    }

    virtual void execute(const std::vector<int>& prev,int action,const std::vector<int>& next){
      ex=true;
      child->execute(prev,action,next);
    }

    virtual void update() {
      child->update();
      value=child->get_value();
      if (ex)
	value.first=value.second-value.first;
      else
	value.first=0;
    }

    unit_tag* child;
    virtual unit* clone() {
      return new unit_tagnot(*this);
    }
  protected:
    bool ex;
    std::stack<bool> st;
    unit_tagnot(const unit_tagnot &obj);
  };

  class unit_manytag: public unit_tag {
  public:
    // No stringify. See unit_tagmanyand and unit_tagmanyor

    unit_manytag(std::string _tag_expr):unit_tag(),tags(0),tag_expr(_tag_expr) { }

    virtual void alphabet_update(Alphabet* alphabet);

    virtual void set_instance(int instance,int current_instance,
			      bool forced=false) {
      if (forced) {
	unsigned vecsize=value.size();
	manytag_instance_map[current_instance]=value;
	value=manytag_instance_map[instance];
	// Handles alphabet upates when using different instance
	value.resize(vecsize);
      }
    }

    virtual void reset() {
      unit::value.first=0;
      for(unsigned i=0;i<value.size();i++) {
	value[i]=0;
      }
    }

    virtual void push() {
      st.push(value);
      st2.push(unit::value);
    }

    virtual void pop() {
      value=st.top();
      st.pop();
      unit::value=st2.top();
      st2.pop();
    }

    virtual void update() { }

    virtual ~unit_manytag() { }

    virtual unit* clone() {
      return new unit_manytag(*this);
    }
  protected:
    int tags;
    std::string tag_expr;
    std::vector<int> my_tag;
    std::vector<int> value;

    std::stack<std::vector<int> > st;
    std::stack<val> st2;
    std::map<int,std::vector<int> > manytag_instance_map;
    unit_manytag(const unit_manytag &obj):
      unit_tag(obj),tags(obj.tags),tag_expr(obj.tag_expr),my_tag(obj.my_tag),
      value(obj.value),manytag_instance_map(obj.manytag_instance_map) {}

  };

  class unit_manytagor: public unit_manytag {
  public:
    unit_manytagor(std::string _tag_expr):unit_manytag(_tag_expr) { }
    virtual ~unit_manytagor() {}

    virtual std::string stringify(Alphabet&a) {
      //TODO
      /*
      std::string ret="\""+a.getSPName(my_tag[0])+"\"";
      for(size_t i=1;i<my_tags.size();i++) {
	ret+=" or \"" + a.getSPName(my_tag[i])+"\"";
      }
      return ret;
      */
      return "";
    }

    virtual void execute(const std::vector<int>& prev,int action,const std::vector<int>& next) {
      if (unit::value.first==unit::value.second) {
	return;
      }
      for(unsigned i=0;i<my_tag.size();i++) {
	if (left_side) {
	  if (std::find(prev.begin(), prev.end(), my_tag[i])!=prev.end()) {
	    unit::value.first=unit::value.second;
	    return;
	  }
	} else {
	  if (std::find(next.begin(), next.end(), my_tag[i])!=next.end()) {
	    unit::value.first=unit::value.second;
	    return;
	  }
	}
      }
    }

    virtual unit* clone() {
      return new unit_manytagor(*this);
    }
  protected:
    //unit_manytagor(const unit_manytagor &obj);
  };

  class unit_manytagand: public unit_manytag {
  public:
    unit_manytagand(std::string _tag_expr):unit_manytag(_tag_expr) { }
    virtual ~unit_manytagand() {}

    virtual std::string stringify(Alphabet&a) {
      //TODO
      /*
      std::string ret="\""+a.getSPName(my_tag[0])+"\"";
      for(size_t i=1;i<my_tags.size();i++) {
	ret+=" or \"" + a.getSPName(my_tag[i])+"\"";
      }
      return ret;
      */
      return "";
    }

    virtual void execute(const std::vector<int>& prev,int action,const std::vector<int>& next) {
      if (unit::value.first==unit::value.second) {
	return;
      }
      for(unsigned i=0;i<my_tag.size();i++) {
	if (value[i]<1) {
	  if (left_side) {
	    // Should be use binary_search?
	    // At least my_tag is sorted...
	    if (std::find(prev.begin(), prev.end(), my_tag[i])!=prev.end()) {
	      value[i]++;
	      unit::value.first++;
	    }
	  } else {
	    if (std::find(next.begin(), next.end(), my_tag[i])!=next.end()) {
	      value[i]++;
	      unit::value.first++;
	    }
	  }
	  if (unit::value.first==unit::value.second) {
	    return;
	  }
	}
      }
    }
    virtual unit* clone() {
      return new unit_manytagand(*this);
    }
  protected:
    //unit_manytagand(const unit_manytagand &obj);
  };

  class unit_tagleaf: public unit_tag {
  public:
    virtual std::string stringify(Alphabet&a) {
      //TODO
      return "\""+a.getSPNames()[my_tag]+"\"";
    }
    unit_tagleaf(int tag):my_tag(tag) {
      value.first=0;
      value.second=1;
    }

    virtual ~unit_tagleaf() {
    }

    virtual void set_instance(int instance,int current_instance, bool force=false) {
      if (force) {
	leaf_instance_map[current_instance]=value.first;
	value.first=leaf_instance_map[instance];
      }
    }

    virtual void reset() {
      value.first=0;
    }

    virtual void push() {
      st.push(value);
    }

    virtual void pop() {
      value=st.top();
      st.pop();
    }

    virtual void execute(const std::vector<int>& prev,int action,const std::vector<int>& next){
      if (action) {
	if (left_side) {
	  if (std::find(prev.begin(), prev.end(), my_tag)!=prev.end()) {
	    value.first=value.second;
	    return;
	  }
	} else {
	  if (std::find(next.begin(), next.end(), my_tag)!=next.end()) {
	    value.first=value.second;
	    return;
	  }
	}
	value.first=0;
      }
    }
    virtual unit* clone() {
      return new unit_tagleaf(*this);
    }
  protected:
    unit_tagleaf(const unit_tagleaf &obj):unit_tag(obj),my_tag(obj.my_tag) { }
    int my_tag;
    std::stack<val> st;
    std::map<int,int> leaf_instance_map;
  };

  class unit_leaf: public unit {
  public:
    virtual std::string stringify(Alphabet&a) {
      return a.getActionName(my_action);
    }

    virtual void alphabet_update(Alphabet* alpha)
    {
      my_action = alpha->action_number(action.c_str());
      if (my_action) {
	value.second=1;
	alpha->remove_alphabet_update(this);
	alphabet=NULL;
      }
    }

    unit_leaf(int action, int count=1) : my_action(action)
    {
      value.second=count;
    }

    virtual void set_instance(int instance,int current_instance, bool force=false) {
      if (force) {
	leaf_instance_map[current_instance]=value.first;
	value.first=leaf_instance_map[instance];
      }
    }

    virtual void reset() {
      value.first=0;
    }

    virtual void push() {
      st.push(value);
    }

    virtual void pop() {
      value=st.top();
      st.pop();
    }

    virtual void execute(const std::vector<int>& prev,int action,const std::vector<int>& next) {
      if (action==my_action && value.first<value.second)
          value.first++;
    }

    virtual void update() {
    }
    virtual unit* clone() {
      return new unit_leaf(*this);
    }
    std::string action;
  protected:
    int my_action;
    std::stack<val> st;
    std::map<int,int> leaf_instance_map;
  };

  class unit_tagdual : public unit_tag {
  public:
    unit_tagdual(unit_tag*l,unit_tag*r): left(l),right(r) {
      if (left)
	left->set_left(true);
    }

    virtual void set_instance(int instance,int current_instance, bool force=false) {
      left->set_instance(instance,current_instance,force);
      right->set_instance(instance,current_instance,force);
    }

    virtual void set_left(bool l) {
      unit_tag::set_left(l);
      left->set_left(l);
      right->set_left(l);
    }

    virtual void reset() {
      left->reset();
      right->reset();
    }

    virtual void push() {
      left->push();
      right->push();
    }

    virtual void pop() {
      left->pop();
      right->pop();
    }

    /*
    virtual void execute(const std::vector<int>& prev,int action,const std::vector<int>& next) {
      val v(right->get_value());

      if (v.first==v.second) {
	left->execute(prev,action,next);
      } else {
	right->execute(prev,action,next);
	right->update();
	v=right->get_value();
	if (v.first==v.second) {
	  left->execute(prev,action,next);
	}
      }
    }
    */
    virtual void update() {
      right->update();
      left->update();
    }

    virtual ~unit_tagdual() {
      delete left;
      delete right;
    }

    unit_tag* left;
    unit_tag* right;
    virtual unit* clone() {
      return new unit_tagdual(*this);
    }
  protected:
    unit_tagdual(const unit_tagdual &obj);
  };

  class unit_tagunit: public unit_tagdual {
  public:
    virtual std::string stringify(Alphabet&a) {
      std::string ls=left->stringify(a);
      std::string rs=right->stringify(a);
      std::string ret;

      if (!ls.empty()) {
	if (persistent&1) {
	  ret="@";
	}
	ret+="["+ls+"] ";
      }
      ret+="("+child->stringify(a)+")";
      if (!rs.empty()) {
	if (persistent&2) {
	  ret+="@";
	}
	ret+=" ["+rs+"]";
      }
      return ret;
    }
    unit_tagunit(unit_tag* l, unit* _child,unit_tag* r,int _persistent=0):
      unit_tagdual(l,r),child(_child),persistent(_persistent) {
      value.first=0;
      value.second=l->value.second+right->value.second+child->value.second;
    }

    virtual ~unit_tagunit() {
      delete child;
    }

    virtual void set_instance(int instance,int current_instance, bool force=false) {
      unit_tagdual::set_instance(instance,current_instance,force);
      child->set_instance(instance,current_instance,force);
    }

    virtual void execute(const std::vector<int>& prev,int action,const std::vector<int>& next) {
      update();
      if (value.first == value.second) return;

      if (persistent&1) {
	left->reset();
      }
      left->update();
      val v=left->get_value();
      if (v.first<v.second) {
	left->execute(prev,action,next);
	left->update();
	v=left->get_value();
	if (v.first<v.second) {
	  return;
	}
      }

      child->update();
      v=child->get_value();
      if (v.first<v.second) {
	child->push();
	child->execute(prev,action,next);
	child->update();
	v=child->get_value();
	child->pop();
	if (v.first<v.second) {
	  if (v.first==0 || persistent&2) {
	    // Handle nothing has happened case, when we need left side to be filled
	    // when starting executing the child.
	    // btw... we won't execute anyting on the right side before the action part
	    // is covered.
	    left->reset();
	  } else {
	    // something happened. We aren't at the goal yet, but on the way!
	    // So this is something that we want to include
	    child->execute(prev,action,next);
	  }
	  return;
	} else {
	  // We are on the way to the goal!
	  // Let's check if this is allowed goal

	  right->execute(prev,action,next);
	  right->update();
	  v=right->get_value();
	  if (v.first==v.second) {
	    // GOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOAL
	    child->execute(prev,action,next);
	    return;
	  } else {
	    // No goal.
	    right->reset();
	  }
	}
      }
    }

    virtual void reset() {
      unit_tagdual::reset();
      child->reset();
    }

    virtual void update() {
      unit_tagdual::update();
      child->update();
      value.second=left->value.second+
	right->value.second+
	child->value.second;

      value.first=left->value.first+
	right->value.first+
	child->value.first;
    }

    virtual void push() {
      unit_tagdual::push();
      child->push();
    }

    virtual void pop() {
      unit_tagdual::pop();
      child->pop();
    }

    unit* child;
    int persistent;
    virtual unit* clone() {
      return new unit_tagunit(*this);
    }
  protected:
    unit_tagunit(const unit_tagunit &obj);
  };

  class unit_tagand : public unit_tagdual {
  public:
    virtual std::string stringify(Alphabet&a) {
      return "("+left->stringify(a)+") and ("+right->stringify(a)+")";
    }

    unit_tagand(unit_tag* l,unit_tag*r): unit_tagdual(l,r) {
      value.second=l->value.second+right->value.second;
    }
    virtual ~unit_tagand() {}

    virtual void execute(const std::vector<int>& prev,int action,const std::vector<int>& next) {
      left->execute(prev,action,next);
      right->execute(prev,action,next);
    }

    virtual void update() {
      unit_tagdual::update();
      value.first=left->value.first+right->value.first;
    }
    virtual unit* clone() {
      return new unit_tagand(*this);
    }
  };

  class unit_tagor : public unit_tagdual {
  public:
    virtual std::string stringify(Alphabet&a) {
      return "("+left->stringify(a)+") or ("+right->stringify(a)+")";
    }

    unit_tagor(unit_tag* l,unit_tag*r): unit_tagdual(l,r) {
      value.second=left->value.second+right->value.second;
    }
    virtual ~unit_tagor() {}

    virtual void execute(const std::vector<int>& prev,int action,const std::vector<int>& next) {
      left->execute(prev,action,next);
      right->execute(prev,action,next);
    }

    virtual void update() {
      unit_tagdual::update();
      if (left->value.first==left->value.second ||
	  right->value.first==right->value.second) {
	value.first=left->value.second+right->value.second;
      } else {
	value.first=left->value.first+right->value.first;
      }
    }
    virtual unit* clone() {
      return new unit_tagor(*this);
    }
  };

  class unit_mult: public unit {
  public:

    virtual std::string stringify(Alphabet&a) {
      return to_string(max)+"*("+child->stringify(a)+")";
    }

    unit_mult(unit* l,int i): max(i),child(l),count(0) {
    }

    virtual void set_instance(int instance,int current_instance, bool force=false) {
      instance_map[current_instance]=count;
      count=instance_map[instance];
      child->set_instance(instance,current_instance,force);
    }

    virtual void reset() {
      count=0;
      child->reset();
    }

    virtual void push() {
      child->push();
      st.push(count);
    }

    virtual void pop() {
      child->pop();
      count=st.top();
      st.pop();
    }

    virtual ~unit_mult() {
      delete child;
    }

    virtual void execute(const std::vector<int>& prev,int action,const std::vector<int>& next) {
      update();
      if (count<max)
	child->execute(prev,action,next);
      update();
    }

    virtual void update() {
      child->update();
      val v=child->get_value();
      if (v.first==v.second) {
	child->reset();
	child->update();
	count++;
      }
      value.second=max*v.second;
      value.first=count*v.second+v.first;

    }

    int max;
    virtual unit* clone() {
      return new unit_mult(*this);
    }
  protected:
    unit_mult(const unit_mult &obj);
    unit* child;
    int count;
    std::stack<int> st;
    std::map<int,int > instance_map;
  };

protected:
  std::vector<unit*> Units;
  std::string params;
};

Coverage_Market::unit* new_unit_tagunit(Coverage_Market::unit_tag* l,
					Coverage_Market::unit* u,
					Coverage_Market::unit_tag* r,
					int persistent);
#endif
