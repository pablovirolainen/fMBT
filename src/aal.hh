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
#ifndef __aal_hh__
#define __aal_hh__

#include "factory.hh"
#include "log.hh"
#include <vector>
#include <string>
#include <map>

template<class InputIterator, class T>
  InputIterator find_second ( InputIterator first, InputIterator last, const T& value )
  {
    for ( ;first!=last; first++) {
      if ( first->second==value ) {
	break;
      }
    }
    return first;
  }

class aal;

#include "writable.hh"
#include "verdict.hh" 
#include "alphabet.hh"

class aal: public Writable, public Alphabet_update, public Alphabet_updater {
public:
  aal(Log&l, std::string& _params): refcount(0),_log(l), params(_params) {_log.ref();};
  virtual ~aal() {_log.unref();};
  virtual int adapter_execute(int action,const char* params)=0;
  virtual int model_execute(int action)  =0;
  virtual int getActions(int** act)      =0;
  virtual bool reset() { // Model
    return true;
  }
  virtual bool init() { // Adapter
    return true;
  }
  virtual void adapter_exit(Verdict::Verdict verdict,
			    const std::string& reason) { // Adapter
  }
  virtual std::vector<std::string>& getActionNames() {
    return action_names;
  }
  virtual std::vector<std::string>& getSPNames() {
    return tag_names;
  }
  virtual void push() {}
  virtual void pop() {}
  virtual int getprops(int** props) {
    return 0;
  }

  virtual int check_tags(std::vector<int>& tag,std::vector<int>& t) {
    // Default implementation...
    t.resize(0);
    return 0;
  }

  virtual int  observe(std::vector<int> &action,bool block=false) {
    return 0;
  }

  virtual void log(const char* format, ...);

  void ref() {
    refcount++;
  }

  void unref() {
    refcount--;
    if (refcount<=0 && storage) {
      std::map<std::string,aal*>::iterator it=
	find_second(storage->begin(),storage->end(),this);
      if (it!=storage->end()) {
	storage->erase(it);
      }
      if (storage->empty()) {
	delete storage;
	storage=NULL;
      }
      delete this;
    }
  }

  int action(const std::string&);
  int input(const std::string&);
  int output(const std::string&);

  static std::map<std::string,aal*>* storage;

  Alphabet_update* alphabet_update_forward;

protected:
  int refcount;
  std::vector<int> actions;
  std::vector<int> tags;
  std::vector<std::string> action_names; /* action names.. */
  std::vector<std::string> tag_names; /* tag/state proposition names.. */
  Log& _log;
  std::string params;
};

#include "model.hh"
#include "adapter.hh"
#include "awrapper.hh"
#include "mwrapper.hh"

#ifndef  ASSERT_EQ
#define  ASSERT_EQ(x,v) \
    if (!((x)==(v))) {                                 \
        _log.print("<aal type=\"ASSERT_EQ\" msg=\"failed: %d == %d\"/>\n", x, v); \
        return 0;                                      \
    }
#endif

#ifndef  ASSERT_NEQ
#define  ASSERT_NEQ(x,v) \
    if ((x)==(v)) {                                     \
        _log.print("<aal type=\"ASSERT_NEQ\" msg=\"failed: %d != %d\"/>\n", x, v); \
        return 0;                                       \
    }
#endif

#endif
