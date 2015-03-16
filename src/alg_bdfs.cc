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

/* Implementations of Bounded Depth-First-Search for models and
 * coverages with push() and pop() */

#include "alg_bdfs.hh"
#include "model.hh"
#include "coverage.hh"
#include <algorithm>
#include <cstdlib>
#include "helper.hh"
#include "learn_proxy.hh"
#include "function.hh"

extern int _g_simulation_depth_hint;

AlgBDFS::AlgBDFS(Model& _model,int searchDepth, Learning* learn,Function* function):
  m_search_depth(searchDepth), m_learn(learn),m_function(function),m_model(_model) {
  if (learn != NULL && ((Learn_proxy*)learn)->lt!=NULL) // TODO: learn knows how to learn exec times
    m_learn_exec_times = true;
  else
    m_learn_exec_times = false;
}

double AlgPathToBestCoverage::search(Coverage& coverage, std::vector<std::pair<int,double> >& path)
{
    return search(coverage,path,m_search_depth);
}

double AlgPathToBestCoverage::search(Coverage& coverage, std::vector<std::pair<int,double> >& path,int search_depth)
{
    m_coverage = &coverage;
    m_search_depth = search_depth;
    return path_to_best_evaluation(path, m_search_depth);
}

double AlgPathToBestCoverage::evaluate()
{
    return m_coverage->getCoverage();
}

void AlgPathToBestCoverage::doExecute(int action,bool push)
{
    if (!status) {
        return;
    }

    if (push) {
      m_model.push();
      m_coverage->push();
    }

    if (!m_model.execute(action)) { errormsg="Model execute error"; status=false; return;}
    m_coverage->execute(action);
}

void AlgPathToBestCoverage::undoExecute()
{
    if (!status) {
        return;
    }

    m_model.pop();
    m_coverage->pop();
}

double AlgPathToAction::search(int find_this_action, std::vector<std::pair<int,double> > &path)
{
    return search(find_this_action,path,m_search_depth);
}

double AlgPathToAction::search(int find_this_action, std::vector<std::pair<int,double> > &path,int search_depth)
{
    m_find_this_action = find_this_action;
    m_search_depth = search_depth;
    return path_to_best_evaluation(path, m_search_depth);
}

double AlgPathToAction::evaluate()
{
    int *actions;
    int action_count;
    /* Is action m_find_this_action available in the current state? */
    action_count = m_model.getActions(&actions);

    if (!status) {
        return 0.0;
    }

    for (int i = 0; i < action_count; i++)
    {
        if (actions[i] == m_find_this_action) return 1.0;
    }
    return 0.0;
}

void AlgPathToAction::doExecute(int action,bool push)
{
    if (push) {
        m_model.push();
    }
    m_model.execute(action);
}

void AlgPathToAction::undoExecute()
{
    m_model.pop();
}

double AlgBDFS::path_to_best_evaluation(std::vector<std::pair<int,double> >& path, int depth)
{
    volatile double current_score = evaluate();
    volatile double best_score = 0;
    std::vector<std::pair<int,double> > hinted_path;

    if (!m_model.status || !status) {
        if (!m_model.status)
	  errormsg = "Model error: "+m_model.errormsg;
        status=false;
        return 0.0;
    }

    _g_simulation_depth_hint = depth;
    m_model.push();
    _g_simulation_depth_hint = 0;

    if (path.size() > 0) {
        // Path includes a hint to look at this path at first before
        // considering others.
        //
        // Evaluating the hinted options first has too effects:
        //
        // - If one path has been chosen earlier, it's preferred to
        //   continue execution that way instead of switching it to
        //   another path that seems to be as good as the
        //   original. This prevents oscillation between equally good
        //   paths.
        //
        // - If maximal growth rate is known, search starting with a
        //   good "so far best score" is able to drop unnecessary
        //   lookups. (Not implemented)

        int invalid_step = -1;
	bool push_execute=true;
        for (unsigned int pos = 0; pos < path.size() && invalid_step == -1; pos++) {
	    doExecute(path[pos].first,push_execute);
	    push_execute=false;
            if (status == false) {
                invalid_step = pos;
                status = true;
            }
            if (!m_model.status) {
                status = false;
		errormsg = "Model: " + m_model.errormsg;
                return 0.0;
            }
        }

        if (invalid_step > -1) {
            // Hinted path is no more valid, throw it away.
            path.resize(0);
	    if (!push_execute) {
	        undoExecute();
	    }
        } else {
	    std::vector<std::pair<int,double> > additional_path;
	    double state_evaluation;

            best_score = _path_to_best_evaluation(additional_path, depth - path.size(), current_score,state_evaluation);

            if (!m_model.status || !status) {
	        if (!m_model.status)
		    errormsg = "Model error: "+m_model.errormsg;
		status=false;
		return 0.0;
            }

	    if (!push_execute) {
	        undoExecute();
	    }

            if (!m_model.status || !status) {
	      if (!m_model.status)
		errormsg = "Model error: "+m_model.errormsg;
	      status=false;
	      return 0.0;
            }

            if (best_score > current_score) {
	        hinted_path = path;
                for (int i = additional_path.size() - 1; i >= 0; i--)
                    hinted_path.push_back(additional_path[i]);
                current_score = best_score;
            }
        }
    }
    double state_evaluation;
    best_score = _path_to_best_evaluation( path, depth, current_score,state_evaluation);
    m_model.pop();

    if (!m_model.status || !status) {
      if (!m_model.status)
	  errormsg = "Model error: "+m_model.errormsg;
      status=false;
      return 0.0;
    }

    if (best_score > current_score || (
            best_score == current_score &&
            path.size() < hinted_path.size())) {
        // The real algorithm constructs the best path in the opposite
        // direction to the path vector, this wrapper reverses it.
        std::reverse(path.begin(), path.end());
    } else if (hinted_path.size() > 0) {
        path = hinted_path;
    } else {
        path.resize(0);
    }
    return best_score;
}

bool AlgBDFS::grows_first(std::vector<std::pair<int,double> >& first_path, int first_path_start,
                          std::vector<std::pair<int,double> >& second_path, int second_path_start)
{
    if (first_path.size() != second_path.size()) {
        errormsg="first_path.size() != second_path.size()";
	status=false;
	return false;
    }

    for(int i=first_path.size()-1; i>=0;i--) {
        if (first_path[i].second>second_path[i].second) {
	    return true;
	}
	if (first_path[i].second>second_path[i].second) {
	    return false;
	}
    }

    return false;

    /*

    volatile double current_score = evaluate();

    first_path.push_back(std::pair<int,double>(first_path_start,0.0));
    second_path.push_back(std::pair<int,double>(second_path_start,0.0));

    int first_difference = first_path.size();
    volatile double first_score;
    bool push_me=true;
    for (int i = first_path.size() - 1; i >= 0; i--) {
        doExecute(first_path[i].first,push_me);
       	push_me=false;
        first_score = evaluate();

        if (!status)
            return false;

        if (first_score > current_score) {
            first_difference = i;
            break;
        }
    }
    if (first_difference == (int)first_path.size()) {
        errormsg = "first_difference == (int)first_path.size() "+to_string(first_difference);
	status=false;
	return false;
    }

    if (!push_me) {
        undoExecute();
	push_me=true;
    }

    int second_difference = second_path.size();
    volatile double second_score;
    for (int i = second_path.size() - 1; i >= 0; i--) {
        doExecute(second_path[i].first,push_me);
	push_me=false;
        second_score = evaluate();
        if (second_score > current_score) {
            second_difference = i;
            break;
        }
    }
    if (second_difference == (int)second_path.size()) {
        errormsg = "second_difference == (int)second_path.size()";
	status=false;
	return false;
    }

    if (!push_me) {
      undoExecute();
    }
    first_path.pop_back();
    second_path.pop_back();

    if (first_difference > second_difference) return true;
    else return false;
    */
}

bool AlgBDFS::grows_faster(std::vector<std::pair<int,double> >& first_path, int first_path_start,
                           std::vector<std::pair<int,double> >& second_path, int second_path_start)
{
    float first_path_time = m_learn->getE(first_path_start);
    float second_path_time = m_learn->getE(second_path_start);

    for (int i = first_path.size() - 1; i >= 0; i--) {
        first_path_time += m_learn->getE(first_path[i].first);
    }
    for (int i = second_path.size() - 1; i >= 0; i--) {
        second_path_time += m_learn->getE(second_path[i].first);
    }

    return first_path_time < second_path_time;
}

static inline void _copy_input_actions(Function* m_function,
				       std::vector<int>& action_candidates,
				       int* input_actions,
				       int input_action_count) {
  action_candidates.reserve(input_action_count);
  if (m_function) {
      int base;
      if (m_function->prefer==Function::FLOAT) {
	  base=m_function->fval()*input_action_count;
      } else {
	  base=m_function->val()%input_action_count;
      }
      for (int i = 0; i < input_action_count; i++)
	  action_candidates.push_back(input_actions[(base+i)%input_action_count]);
  } else {
      for (int i = 0; i < input_action_count; i++)
	  action_candidates.push_back(input_actions[i]);
  }
}

double AlgPathToAdaptiveCoverage::_path_to_best_evaluation
(std::vector<std::pair<int,double> >& path, int depth,double best_evaluation,
 double& current_state_evaluation)
{
    int *input_actions = NULL;
    int input_action_count = 0;

    if (!status) {
        return 0.0;
    }

    //volatile double 
    current_state_evaluation = evaluate();

    if (current_state_evaluation > best_evaluation)
        best_evaluation = current_state_evaluation;

    /* Can we still continue the search? */
    if (depth <= 0)
        return current_state_evaluation;

    /* Recursive search for the best path */
    input_action_count = m_model.getIActions(&input_actions);

    if (!m_model.status) {
        errormsg = "Model error: "+m_model.errormsg;
        status=false;
        return 0.0;
    }

    std::vector<int> action_candidates;

    _copy_input_actions(m_function,action_candidates,input_actions,input_action_count);

    std::vector<std::pair<int,double> > best_path;
    unsigned int best_path_length = 0;
    int best_action = -1;
    double best_current_state_evaluation;
    std::vector<double> pre_evaluation;
    std::vector<double> an_evaluation;
    std::vector<std::vector<std::pair<int,double> > > a_path;
    std::vector<double> state_evaluation;
    an_evaluation.resize(input_action_count+1);
    a_path.resize(input_action_count+1);
    pre_evaluation.reserve(input_action_count+1);
    state_evaluation.resize(input_action_count+1);

    for (int i = 0; i < input_action_count; i++)
    {
	double state_evaluation;
        doExecute(action_candidates[i]);

	a_path[i].resize(0);
	pre_evaluation.push_back(_path_to_best_evaluation(a_path[i], depth - 1, best_evaluation,state_evaluation));
	undoExecute();
	
	if (!m_model.status || !status) {
	    if (!m_model.status)
	        errormsg = "Model error: "+m_model.errormsg;
	    status=false;
	    return 0.0;
	}
    }

    if (m_learn) {
        for (int i = 0; i < input_action_count; i++)
	  {
	    float weight_total=0.0;

	    for (int j = 0; j < input_action_count; j++)
	      {
		float weight=m_learn?m_learn->getC(action_candidates[i],action_candidates[j]):0.0;

		if (i==j) {
		    weight+=1.0;
		}
		weight_total+=weight;

		an_evaluation[i]+=pre_evaluation[j]*weight;
	      }
	    if (weight_total>0.0) {
	        an_evaluation[i]=an_evaluation[i]/weight_total;
	    } else {
	        an_evaluation[i]=0.0;
	    }
	  }
    } else {
        an_evaluation.swap(pre_evaluation);
    }

    for (int i = 0; i < input_action_count; i++) {
        if (an_evaluation[i] > current_state_evaluation &&
            (an_evaluation[i] > best_evaluation ||
             (an_evaluation[i] == best_evaluation &&
              (best_action == -1 ||
               (best_action > -1 &&
                ((m_learn_exec_times && grows_faster(a_path[i], action_candidates[i], best_path, best_action)) ||
                 (a_path[i].size() < best_path_length ||
                  (best_path_length>0 && a_path[i].size() == best_path_length &&
                   grows_first(a_path[i], action_candidates[i], best_path, best_action)))))))))
        {
            best_path_length = a_path[i].size();
            best_path.swap(a_path[i]);
            best_action = action_candidates[i];
            best_evaluation = an_evaluation[i];
	    best_current_state_evaluation = state_evaluation[i];
        }
    }

    if ((int)best_action > -1) {
        path.swap(best_path);
	path.reserve(m_search_depth);
        path.push_back(std::pair<int,double>(best_action,(double)best_current_state_evaluation));
        return best_evaluation;
    }
    return current_state_evaluation;
}

double AlgBDFS::_path_to_best_evaluation(std::vector<std::pair<int,double> >& path,
					 int depth,
                                         double best_evaluation,
					 double& current_state_evaluation)
{
    int *input_actions = NULL;
    int input_action_count = 0;

    if (!status) {
        return 0.0;
    }

    //volatile double 
    current_state_evaluation = evaluate();

    if (current_state_evaluation > best_evaluation)
        best_evaluation = current_state_evaluation;

    /* Can we still continue the search? */
    if (depth <= 0)
        return current_state_evaluation;

    /* Recursive search for the best path */
    input_action_count = m_model.getIActions(&input_actions);

    if (!m_model.status) {
        errormsg = "Model error:"+m_model.errormsg;
	status=false;
	return 0.0;
    }

    std::vector<int> action_candidates;

    _copy_input_actions(m_function,action_candidates,input_actions,input_action_count);

    std::vector<std::pair<int,double> > best_path;
    unsigned int best_path_length = 0;
    int best_action = -1;
    double best_current_state_evaluation;
    for (int i = 0; i < input_action_count; i++)
    {
        std::vector<std::pair<int,double> > a_path;
        volatile double an_evaluation;
	double state_evaluation;

        doExecute(action_candidates[i]);

        a_path.resize(0);
        an_evaluation = _path_to_best_evaluation(a_path, depth - 1, best_evaluation,
						 state_evaluation);

        undoExecute();

        if (!m_model.status || !status) {
            if (!m_model.status)
	        errormsg = "Model error:"+m_model.errormsg;
	    status=false;
	    return 0.0;
        }

        if (an_evaluation > current_state_evaluation &&
            (an_evaluation > best_evaluation ||
             (an_evaluation == best_evaluation &&
              (best_action == -1 ||
               (best_action > -1 &&
                ((m_learn_exec_times && grows_faster(a_path, action_candidates[i], best_path, best_action)) ||
		 (a_path.size() < best_path_length ||
		  (best_path_length>0 && a_path.size() == best_path_length &&
		   grows_first(a_path, action_candidates[i], best_path, best_action)))))))))
        {
            best_path_length = a_path.size();
            best_path.swap(a_path);
            best_action = action_candidates[i];
            best_evaluation = an_evaluation;
	    best_current_state_evaluation = state_evaluation;
        }
    }

    if ((int)best_action > -1) {
        path.swap(best_path);
	path.reserve(m_search_depth);
        path.push_back(std::pair<int,double>(best_action,(double)best_current_state_evaluation));
        return best_evaluation;
    }
    return current_state_evaluation;
}
