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

#define _FUNCTION_INTERNAL_
#include "params.hh"
#include "function.hh"

FACTORY_IMPLEMENTATION(Function)

/*
std::map<std::string, std::pair<FunctionFactory::creator,void*> >* FunctionFactory::creators = 0;

FACTORY_ADD_FACTORY(Function)

Function* FunctionFactory::create(
    FACTORY_CREATE_PARAMS FACTORY_CREATE_DEFAULT_PARAMS)
{
  if (!creators) return NULL;
  std::map<std::string, std::pair<creator,void*> >::iterator i = (*creators).find(name);
  if (i!=creators->end()) return (i->second.first)(log,params,i->second.second);
  return NULL;
}
*/

Function* new_function(const std::string& s) {
  std::string name,option;
  param_cut(s,name,option);
<<<<<<< HEAD

  Function::PREFER p=Function::CARE;
  Function* ret;

  if (name=="INT") {
    p=Function::INT;
  }
  if (name=="FLOAT") {
    p=Function::FLOAT;
  }

  if (p==Function::CARE) {
    ret=FunctionFactory::create(name, option);
  } else {
    ret=new_function(option);
    if (ret) {
      ret->prefer=p;
    }
  }
=======
>>>>>>> remotes/origin/factory

  Function::PREFER p=Function::CARE;
  Function* ret;

  if (name=="INT") {
    p=Function::INT;
  }
  if (name=="FLOAT") {
    p=Function::FLOAT;
  }

  if (p==Function::CARE) {
    ret=FunctionFactory::create(name, option);
  } else {
    ret=new_function(option);
    if (ret) {
      ret->prefer=p;
    }
  }
  
  if (ret) {
    return ret;
  }

  // Let's try a const one.
  char* endp;
  (void)strtod(s.c_str(),&endp);

  if (*endp==0) {
    ret=FunctionFactory::create("const", name);
  }

  return ret;
}
