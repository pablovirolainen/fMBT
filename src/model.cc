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
#include "model.hh"

#include "helper.hh"
#include <iostream>

int _g_simulation_depth_hint;

FACTORY_IMPLEMENTATION(Model)

Model::Model(Log&l,const std::string& params_):
log(l), parent(NULL), params(params_)
{
  log.ref();
}

Model::~Model()
{
  log.unref();
}

std::vector<std::string>& Model::getActionNames()
{
  return action_names;
}

std::vector<std::string>& Model::getSPNames()
{
  return prop_names;
}

std::string& Model::getActionName(int action)
{
  return action_names[action];
}

bool Model::is_output(int action)
{
  if (outputs.size()<inputs.size()) {
    for(size_t i=0;i<outputs.size();i++) {
      if (outputs[i]==action) {
        return true;
      }
    }
    return false;
  }

  for(size_t i=0;i<inputs.size();i++) {
    if (inputs[i]==action) {
      return false;
    }
  }

  return true;
}

void Model::precalc_input_output()
{
  for(size_t i=0;i<action_names.size();i++) {
    if (isOutputName(action_names[i])) {
      outputs.push_back(i);
    }

    if (isInputName(action_names[i])) {
      inputs.push_back(i);
    }
  }
}

Model* Model::up()
{
  return parent;
}

Model* Model::down(unsigned int a)
{
  return NULL;
}

std::vector<std::string>& Model::getModelNames()
{
  return model_names;
}

void Model::setparent(Model* m)
{
  parent = m;
}

Model* new_model(Log& l,const std::string& s) {
  std::string name,option;
  param_cut(s,name,option);
  Model* ret=ModelFactory::create(l, name, option);

  if (ret) {
    return ret;
  }

  //Let's try old thing.
  split(s, name, option);

  ret=ModelFactory::create(l, name, option);

  if (ret) {
    fprintf(stderr,"DEPRECATED MODEL SYNTAX. %s\nNew syntax is %s(%s)\n",
	    s.c_str(),name.c_str(),option.c_str());
  } else {
    ret=ModelFactory::create(l, filetype(s), s);
  }

  return ret;
}
