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
#ifndef __alphabet_hh__
#define __alphabet_hh__

#include <string>
#include <list>
#include <vector>

class Alphabet;

class Alphabet_update {
public:
  virtual void alphabet_update(Alphabet*) {
  }
};

class Alphabet_updater {
public:
  // Register callback
  virtual void add_alphabet_update(Alphabet_update* u) {
    alphabet_update_callbacks.push_back(u);
  }

  // Unregister callback
  virtual void remove_alphabet_update(Alphabet_update* u) {
    alphabet_update_callbacks.remove(u);
  }

  virtual void update(Alphabet* alpha=NULL);
protected:
  std::list<Alphabet_update*> alphabet_update_callbacks;
};

class Alphabet:public Alphabet_updater {
public:
enum values {
  OUTPUT_ONLY =-1,
  DEADLOCK = -2,
  SILENCE = -3,
  TIMEOUT = -4,
  ERROR   = -5,
  UPDATE  = -6,
  ALPHABET_MIN = UPDATE
};

  virtual ~Alphabet() {}
  //! Returns names of all actions available.
  virtual std::vector<std::string>& getActionNames() = 0;

  //! Returns names of all available state propositions
  virtual std::vector<std::string>& getSPNames()     = 0;

  //! Returns the name of the given action
  virtual std::string& getActionName(int action)     = 0;

  virtual int action_number(const std::string& s);

protected:
  std::vector<std::string> prop_names; /* proposition names.. */
  std::vector<std::string> action_names; /* action names.. */
};

#endif
