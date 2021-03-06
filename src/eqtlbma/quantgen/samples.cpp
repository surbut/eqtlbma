/** \file samples.cpp
 *
 *  `Samples' is a class 
 *  Copyright (C) 2013 Timothee Flutre
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <algorithm>

#include "utils/utils_math.hpp"
#include "utils/utils_io.hpp"

#include "quantgen/samples.hpp"
#include "quantgen/gene.hpp"

using namespace std;

using namespace utils;

namespace quantgen {

  Samples::Samples(void)
  {
  }

  string Samples::GetSample(const size_t & idx) const
  {
    string res;
    if(idx > GetTotalNbSamples())
      cerr << "ERROR: idx " << idx << " is bigger than the number of samples"
	   << endl;
    else
      res = all_[idx];
    return res;
  }

  bool Samples::IsPresent(const string & sample) const
  {
    return find(all_.begin(), all_.end(), sample) != all_.end();
  }


  bool Samples::IsAbsent(const string & sample) const
  {
    return ! IsPresent(sample);
  }

  void Samples::AddSamplesIfNew(const vector<string> & samples)
  {
    for (vector<string>::const_iterator it = samples.begin();
	 it != samples.end(); ++it) {
      if (find(all_.begin(), all_.end(), *it) == all_.end())
	all_.push_back(*it);
    }
    sort(all_.begin(), all_.end());
  }

/** \brief subgroup2genotypes_["s2"][0] = 5 means that the 1st sample in all_
 *  corresponds to the 6th genotype sample in subgroup "s2"
 *  \note 'npos' means that the sample is absent from the given subgroup
 */
  const vector<size_t> Samples::MapAllSamplesToTheGivenSubgroup(
    const vector<string> * pt_samples) const
  {
    vector<size_t> indices_of_all_in_subgroup(all_.size(), string::npos);
    vector<string>::const_iterator it_s; // points to a sample in a subgroup
  
    // for each sample in 'all_', record its index in 'pt_samples',
    // which corresponds to the proper column from the input file
    for (vector<string>::const_iterator it_a = all_.begin();
	 it_a != all_.end(); ++it_a) {
      it_s = find(pt_samples->begin(), pt_samples->end(), *it_a);
      if (it_s != pt_samples->end())
	indices_of_all_in_subgroup[it_a - all_.begin()] =
	  it_s - pt_samples->begin();
    }
  
    return indices_of_all_in_subgroup;
  }

  /** \brief Fill attribute subgroup2present_
   */
  void Samples::AddSubgroup(const string & subgroup,
			    const vector<size_t> & indices_of_all_in_subgroup)
  {
    if(subgroup2present_.find(subgroup) == subgroup2present_.end())
      subgroup2present_.insert(
	make_pair(subgroup, vector<bool>(GetTotalNbSamples(), false)));
  
    for(vector<size_t>::const_iterator it = 
	  indices_of_all_in_subgroup.begin();
	it != indices_of_all_in_subgroup.end(); ++it)
      if(*it != string::npos)
	subgroup2present_.find(subgroup)->second
	  [it - indices_of_all_in_subgroup.begin()] = true;
  }

  void Samples::AddSamplesFromData(
    const map<string,vector<string> > & subgroup2samples,
    const string & type_data)
  {
    const vector<string> * pt_samples;
    vector<size_t> indices_of_all_in_subgroup;
    for(map<string,vector<string> >::const_iterator it = 
	  subgroup2samples.begin(); it != subgroup2samples.end(); ++it) {
      pt_samples = &(it->second);
      indices_of_all_in_subgroup = MapAllSamplesToTheGivenSubgroup(pt_samples);
      if(type_data.compare("genotype") == 0)
	subgroup2genotypes_.insert(make_pair(it->first,
					     indices_of_all_in_subgroup));
      else if(type_data.compare("explevel") == 0)
	subgroup2explevels_.insert(make_pair(it->first,
					     indices_of_all_in_subgroup));
      else if(type_data.compare("covariate") == 0)
	subgroup2covariates_.insert(make_pair(it->first,
					      indices_of_all_in_subgroup));
      AddSubgroup(it->first, indices_of_all_in_subgroup);
    }
  }

  size_t Samples::GetIndexExplevel(const size_t & idx, const string & subgroup) const
  {
    return subgroup2explevels_.find(subgroup)->second[idx];
  }

  size_t Samples::GetIndexGenotype(const size_t & idx, const string & subgroup) const
  {
    return subgroup2genotypes_.find(subgroup)->second[idx];
  }

  size_t Samples::GetIndexCovariate(const size_t & idx, const string & subgroup) const
  {
    return subgroup2covariates_.find(subgroup)->second[idx];
  }

  void Samples::GetCommonAndUniqueIndividualsBetweenPairOfSubgroups(
    const string & subgroup1,
    const string & subgroup2,
    const Gene & gene,
    vector<size_t> & inds_s1s2,
    vector<size_t> & inds_s1,
    vector<size_t> & inds_s2) const
  {
    inds_s1s2.clear();
    inds_s1.clear();
    inds_s2.clear();
    size_t idx_explevel;
    bool present_in_s1, present_in_s2;
    for (size_t idx_all = 0; idx_all < all_.size(); ++idx_all) {
      
      idx_explevel = GetIndexExplevel(idx_all, subgroup1);
      if(gene.GetName() != "")
	present_in_s1 = (
	  GetIndexGenotype(idx_all, subgroup1) != string::npos
	  && idx_explevel != string::npos
	  && ! isNan(gene.GetExplevel(subgroup1, idx_explevel)));
      else // used in ShowPairs() below
	present_in_s1 = (
	  GetIndexGenotype(idx_all, subgroup1) != string::npos
	  && idx_explevel != string::npos);
      
      idx_explevel = GetIndexExplevel(idx_all, subgroup2);
      if(gene.GetName() != "")
	present_in_s2 = (
	  GetIndexGenotype(idx_all, subgroup2) != string::npos
	  && idx_explevel != string::npos
	  && ! isNan(gene.GetExplevel(subgroup2, idx_explevel)));
      else // used in ShowPairs() below
	present_in_s2 = (
	  GetIndexGenotype(idx_all, subgroup2) != string::npos
	  && idx_explevel != string::npos);
      
      if (present_in_s1 && present_in_s2)
	inds_s1s2.push_back(idx_all);
      else if (present_in_s1 && ! present_in_s2)
	inds_s1.push_back(idx_all);
      else if (! present_in_s1 && present_in_s2)
	inds_s2.push_back(idx_all);
    }
  }

  void Samples::ShowPairs(ostream & os) const
  {
    vector<string> subgroup_names;
    keys2vec(subgroup2present_, subgroup_names);
    string subgroup1, subgroup2;
    vector<size_t> inds_s1s2, inds_s1, inds_s2;
    for(size_t s1 = 0; s1 < subgroup_names.size() - 1; ++s1){
      subgroup1 = subgroup_names[s1];
      for(size_t s2 = s1 + 1; s2 < subgroup_names.size(); ++s2){
	subgroup2 = subgroup_names[s2];
	GetCommonAndUniqueIndividualsBetweenPairOfSubgroups(
	  subgroup1, subgroup2, Gene(), inds_s1s2, inds_s1, inds_s2);
	os << subgroup1 << "-" << subgroup2 << ": "
	   << inds_s1s2.size() << " in both, "
	   << inds_s1.size() << " in " << subgroup1 << ", "
	   << inds_s2.size() << " in " << subgroup2
	   << endl;
      }
    }
  }

  void Samples::ShowAllMappings(ostream & os) const
  {
    size_t idx;
    for(vector<string>::const_iterator it_all = all_.begin();
	it_all != all_.end(); ++it_all){
      os << (it_all - all_.begin()) + 1 << "/" << all_.size()
	 << " sample " << *it_all << ":" << endl;
      for(map<string,vector<bool> >::const_iterator it_sbgrp =
	    subgroup2present_.begin();
	  it_sbgrp != subgroup2present_.end(); ++it_sbgrp){
	os << "subgroup " << it_sbgrp->first << ":";
	if(! it_sbgrp->second[it_all - all_.begin()])
	  os << "absent";
	else{
	  os << " genotype=";
	  idx = subgroup2genotypes_.find(it_sbgrp->first)->second
	    [it_all - all_.begin()];
	  os << (idx == string::npos ? -1 : idx);
	  os << " explevel=";
	  idx = subgroup2explevels_.find(it_sbgrp->first)->second
	    [it_all - all_.begin()];
	  if(idx == string::npos)
	    os << "missing";
	  else
	    os << idx;
	  os << " covariate=";
	  if(subgroup2covariates_.find(it_sbgrp->first) !=
	     subgroup2covariates_.end()){
	    idx = subgroup2covariates_.find(it_sbgrp->first)->second
	      [it_all - all_.begin()];
	    if(idx == string::npos)
	      os << "missing";
	    else
	      os << idx;
	  }
	  else
	    os << "none";
	}
	os << endl;
      }
    }
  }

} // namespace quantgen
