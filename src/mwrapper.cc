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

#include "mwrapper.hh"
#include <cstring>

# define UPDATE_ALPHABET_RETRY(expression)                                    \
  (__extension__                                                              \
   ({ int __result = (expression);                                            \
      while (__result == UPDATE) {                                            \
        handle_alphabet_update();                                             \
	__result = (expression);					      \
      }                                                                       \
      __result; }))

Mwrapper::~Mwrapper()
{
  if (model) {
    model->unref();
  }
}

Mwrapper::Mwrapper(Log&l,const std::string& params, aal* _model):
  Model(l,""), model(_model)
{
  if (model) {
    model->ref();
    status = model->status;
    errormsg = model->errormsg;
    action_names=model->getActionNames();
    prop_names=model->getSPNames();
    status = model->status;
    precalc_input_output();
    model->add_alphabet_update(this);
  } else {
    status=false;
  }
}

void Mwrapper::alphabet_update(Alphabet*) {
  handle_alphabet_update();
  update(this);
}

int Mwrapper::getActions(int** actions) {
  if (!status) {
    return 0;
  }
  int ret=UPDATE_ALPHABET_RETRY(model->getActions(actions));
  status = model->status;
  return ret;
}

#include <algorithm>

void Mwrapper::handle_alphabet_update() {
  std::vector<std::string>& updated_action_names=model->getActionNames();
  std::vector<std::string>& updated_prop_names=model->getSPNames();

  if (updated_action_names.size()>action_names.size()) 
    action_names.insert(action_names.end(),
			updated_action_names.begin()+action_names.size(),
			updated_action_names.end());

  if (updated_prop_names.size()>prop_names.size()) 
    prop_names.insert(prop_names.end(),
		      updated_prop_names.begin()+prop_names.size(),
		      updated_prop_names.end());

  precalc_input_output();
  update(this);
}

int Mwrapper::getIActions(int** actions) {
  if (!status) {
    return 0;
  }

  int a=UPDATE_ALPHABET_RETRY(model->getActions(actions));
  int ret=a;
  for(int i=0;i<a;i++) {
    if (is_output((*actions)[i])) {
      ret--;
      memmove(&(*actions)[i],&(*actions)[i+1],sizeof(int)*
              (a-i-1));
      i--;
      a--;
    }
  }
  status = model->status;
  return ret;
}

bool Mwrapper::reset() {
  status = model->reset();
  errormsg = model->errormsg;
  return status;
}

/* No props */
int Mwrapper::getprops(int** props)
{
  int ret= UPDATE_ALPHABET_RETRY(model->getprops(props));
  status = model->status;
  return ret;
}

int Mwrapper::execute(int action)
{
  int rv = 0;
  if (-42 != (rv = UPDATE_ALPHABET_RETRY(model->model_execute(action)))) {
    status = model->status;
    return rv;
  }
  status = model->status;
  return 0;
}

void Mwrapper::push() {
  model->push();
  status = model->status;
}

void Mwrapper::pop() {
  model->pop();
  status = model->status;
}

bool Mwrapper::init()
{
  return true;
}

std::string Mwrapper::stringify()
{
  if (!status) return errormsg;
  return std::string("");
}

