/*
 * SearchNode.h
 *
 *  Copyright (C) 2008-2012 Lars Otten
 *  This file is part of DAOOPT.
 *
 *  DAOOPT is free software: you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  DAOOPT is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with DAOOPT.  If not, see <http://www.gnu.org/licenses/>.
 *  
 *  Created on: Oct 9, 2008
 *      Author: Lars Otten <lotten@ics.uci.edu>
 */

#ifndef SEARCHNODE_H_
#define SEARCHNODE_H_

#include "_base.h"
#include "utils.h"
#include "SubprobStats.h"  // only for PARALLEL_STATIC

namespace daoopt {

class Problem;
class PseudotreeNode;

/* some constants for aggregating the boolean flags */
#define FLAG_LEAF 1 // node is a leaf node
#define FLAG_CACHABLE 2 // node is candidate for caching
#define FLAG_EXTERN 4 // subproblem was processed externally (in parallel setting)
#define FLAG_PRUNED 8 // subproblem below was pruned
#define FLAG_NOTOPT 16 // subproblem possibly not optimally solved (-> don't cache)
#define FLAG_ERR_EXT 32 // found an issue with externally solved subproblem

class SearchNode;

/*
struct SearchNodeComp {
  bool operator() (const SearchNode* a, const SearchNode* b) const;
};
*/

/* data types to store child pointers */
typedef SearchNode* NodeP;
typedef NodeP* CHILDLIST;

class SearchNode {
protected:
  unsigned char m_flags;             // for the boolean flags
  int m_depth;                       // depth in search space
  SearchNode* m_parent;              // pointer to the parent
  double m_nodeValue;                // node value (as in cost)
  double m_heurValue;                // heuristic estimate of the node's value

  CHILDLIST m_children;              // Child nodes
  size_t m_childCountFull;           // Number of total child nodes (initial count)
  size_t m_childCountAct;            // Number of remaining active child nodes

#if defined PARALLEL_DYNAMIC || defined PARALLEL_STATIC
  count_t m_subCount;                // number of nodes expanded below this node
#endif
#ifdef PARALLEL_DYNAMIC
  count_t m_subLeaves;               // number leaf nodes generated below this node
  count_t m_subLeafD;                // cumulative depth of leaf nodes below this node, division
                                     // by m_subLeaves yields average leaf depth
#endif
#ifndef NO_ASSIGNMENT
  vector<val_t> m_optAssignment;     // stores the optimal solution to the subproblem
#endif

public:
  virtual int getType() const = 0;
  virtual int getVar() const = 0;
  virtual val_t getVal() const = 0;

  virtual void setValue(double) = 0;
  virtual double getValue() const = 0;
//  virtual void setLabel(double) = 0;
  virtual double getLabel() const = 0;
  virtual void addSubSolved(double) = 0;
  virtual double getSubSolved() const = 0;
  void setHeur(double d) { m_heurValue = d; }
  // the first one is overridden in SearchNodeOR, the second one isn't
  virtual double getHeur() const { return m_heurValue; }
//  virtual double getHeurOrg() const { return m_heurValue; }

  virtual void setCacheContext(const context_t&) = 0;
  virtual const context_t& getCacheContext() const = 0;

  virtual void setCacheInst(size_t i) = 0;
  virtual size_t getCacheInst() const = 0;

#if defined PARALLEL_DYNAMIC || defined PARALLEL_STATIC
  count_t getSubCount() const { return m_subCount; }
  void setSubCount(count_t c) { m_subCount = c; }
  void addSubCount(count_t c) { m_subCount += c; }

  virtual void setInitialBound(double d) = 0;
  virtual double getInitialBound() const = 0;

  virtual void setComplexityEstimate(double d) = 0;
  virtual double getComplexityEstimate() const = 0;
#endif
#ifdef PARALLEL_DYNAMIC
  count_t getSubLeaves() const { return m_subLeaves; }
  void setSubLeaves(count_t c) { m_subLeaves = c; }
  void addSubLeaves(count_t c) { m_subLeaves += c; }

  count_t getSubLeafD() const { return m_subLeafD; }
  void setSubLeafD(count_t d) { m_subLeafD = d; }
  void addSubLeafD(count_t d) { m_subLeafD += d; }
#endif
#ifdef PARALLEL_STATIC
  virtual SubprobFeatures* getSubprobFeatures() { assert(false); return NULL; }  // OR only
  virtual const SubprobFeatures* getSubprobFeatures() const { assert(false); return NULL; }
#endif
#if defined PARALLEL_DYNAMIC || defined PARALLEL_STATIC
  virtual void setSubprobContext(const context_t&) = 0;
  virtual const context_t& getSubprobContext() const = 0;
#endif

  virtual int getDepth() const = 0;
  virtual void getPST(vector<double>&) const = 0;
//  void getPST(vector<double>&) const;

  SearchNode* getParent() const { return m_parent; }

  void setChild(SearchNode*);
  void addChildren(const vector<SearchNode*>&);
  NodeP* getChildren() const { return m_children; }
  size_t getChildCountFull() const { return m_childCountFull; }
  size_t getChildCountAct() const { return m_childCountAct; }

  bool hasChild(SearchNode* node) const;
  void eraseChild(SearchNode* node);
  void clearChildren();

#ifndef NO_ASSIGNMENT
  vector<val_t>& getOptAssig() { return m_optAssignment; }
  void setOptAssig(const vector<val_t>& assign) { m_optAssignment = assign; }
  void clearOptAssig() { vector<val_t> v; m_optAssignment.swap(v); }
#endif

  void setLeaf() { m_flags |= FLAG_LEAF; }
  bool isLeaf() const { return m_flags & FLAG_LEAF; }
  void setCachable() { m_flags |= FLAG_CACHABLE; }
  bool isCachable() const { return m_flags & FLAG_CACHABLE; }
  void setExtern() { m_flags |= FLAG_EXTERN; }
  bool isExtern() const { return m_flags & FLAG_EXTERN; }
  void setPruned() { m_flags |= FLAG_PRUNED; }
  bool isPruned() const { return m_flags & FLAG_PRUNED; }
  void setNotOpt() { m_flags |= FLAG_NOTOPT; }
  bool isNotOpt() const { return m_flags & FLAG_NOTOPT; }
  void setErrExt() { m_flags |= FLAG_ERR_EXT; }
  bool isErrExt() const { return m_flags & FLAG_ERR_EXT; }

  virtual void setHeurCache(double* d) = 0;
  virtual double* getHeurCache() const = 0;
  virtual void clearHeurCache() = 0;

protected:
  SearchNode(SearchNode* parent);

public:
  static bool heurLess(const SearchNode* a, const SearchNode* b);
  static string toString(const SearchNode* a);

public:
  virtual ~SearchNode();
};


class SearchNodeAND : public SearchNode {
protected:
  val_t m_val;          // Node value, assignment to OR parent variable
  double m_nodeLabel;   // Label of arc <X_i,a>, i.e. instantiated function costs
  double m_subSolved;   // Saves solutions of optimally solved subproblems, so that
                        // their nodes can be deleted
  static context_t emptyCtxt;
  static std::list<std::pair<double,double> > emptyPSTList;

public:
  int getType() const { return NODE_AND; }
  int getVar() const { assert(m_parent); return m_parent->getVar(); }
  val_t getVal() const { return m_val; }

  void setValue(double d) { m_nodeValue = d; }
  double getValue() const { return m_nodeValue; }
//  void setLabel(double d) { m_nodeLabel = d; }
  double getLabel() const { return m_nodeLabel; }
  void addSubSolved(double d) { m_subSolved OP_TIMESEQ d; }
  double getSubSolved() const { return m_subSolved; }

  int getDepth() const { return m_parent->getDepth(); }

  /* empty implementations, functions meaningless for AND nodes */
  void setCacheContext(const context_t& c) { assert(false); }
  const context_t& getCacheContext() const { assert(false); return emptyCtxt; }
  void setCacheInst(size_t i) { assert(false); }
  size_t getCacheInst() const { assert(false); return 0; }
#if defined PARALLEL_DYNAMIC || defined PARALLEL_STATIC
  void setInitialBound(double d) { assert(false); }
  double getInitialBound() const { assert(false); return 0.0; }
  void setComplexityEstimate(double d) { assert(false); }
  double getComplexityEstimate() const { assert(false); return 0.0; }
#endif
#if defined PARALLEL_DYNAMIC || defined PARALLEL_STATIC
  void setSubprobContext(const context_t& t) { assert(false); }
  const context_t& getSubprobContext() const { assert(false); return emptyCtxt; }
#endif
  void getPST(vector<double>&) const { assert(false); };
  /* empty implementations, functions meaningless for AND nodes */
  void setHeurCache(double* d) {}
  double* getHeurCache() const { return NULL; }
  void clearHeurCache() {}

public:
  SearchNodeAND(SearchNode* p, val_t val, double label = ELEM_ONE);
  virtual ~SearchNodeAND() { /* empty */ }
};


class SearchNodeOR : public SearchNode {
protected:
  int m_var;             // Node variable
  int m_depth;           // Depth of corresponding variable in pseudo tree
#ifdef PARALLEL_DYNAMIC
  size_t m_cacheInst;    // Cache instance counter
#endif
#if defined PARALLEL_DYNAMIC || defined PARALLEL_STATIC
  double m_initialBound; // the lower bound when the node was first generated
  double m_complexityEstimate; // subproblem complexity estimate
#endif
  double* m_heurCache;   // Stores the precomputed heuristic values of the AND children
  context_t m_cacheContext; // Stores the context (for caching)

#if defined PARALLEL_DYNAMIC || defined PARALLEL_STATIC
  context_t m_subprobContext; // Stores the context values to this subproblem
#endif
#ifdef PARALLEL_STATIC
  SubprobFeatures m_subprobFeatures; // subproblem feature set
#endif

public:
  int getType() const { return NODE_OR; }
  int getVar() const { return m_var; }
  val_t getVal() const { assert(false); return NONE; } // no val for OR nodes!

  int getDepth() const { return m_depth; }

  void setValue(double d) { m_nodeValue = d; }
  double getValue() const { return m_nodeValue; }
//  void setLabel(double d) { assert(false); } // no label for OR nodes!
  double getLabel() const { assert(false); return 0; } // no label for OR nodes!
  void addSubSolved(double d) { assert(false); } // not applicable for OR nodes
  double getSubSolved() const { assert(false); return 0; } // not applicable for OR nodes

  /* overrides SearchNode::getHeur() */
//  double getHeur() const;

  void setCacheContext(const context_t& t) { m_cacheContext = t; }
  const context_t& getCacheContext() const { return m_cacheContext; }

#ifdef PARALLEL_DYNAMIC
  void setCacheInst(size_t i) { m_cacheInst = i; }
  size_t getCacheInst() const { return m_cacheInst; }
#else
  void setCacheInst(size_t i) { }
  size_t getCacheInst() const { return 0; }
#endif

#ifdef PARALLEL_STATIC
  SubprobFeatures* getSubprobFeatures() { return &m_subprobFeatures; }
  const SubprobFeatures* getSubprobFeatures() const { return &m_subprobFeatures; }
#endif

#if defined PARALLEL_DYNAMIC || defined PARALLEL_STATIC
  void setInitialBound(double d) { m_initialBound = d; }
  double getInitialBound() const { return m_initialBound; }

  void setComplexityEstimate(double d) { m_complexityEstimate = d; }
  double getComplexityEstimate() const { return m_complexityEstimate; }

#endif

#if defined PARALLEL_DYNAMIC || defined PARALLEL_STATIC
  void setSubprobContext(const context_t& c) { m_subprobContext = c; }
  const context_t& getSubprobContext() const { return m_subprobContext; }
#endif
  void getPST(vector<double>&) const;

  void setHeurCache(double* d) { m_heurCache = d; }
  double* getHeurCache() const { return m_heurCache; }
  void clearHeurCache();

public:
  SearchNodeOR(SearchNode* parent, int var, int depth);
  virtual ~SearchNodeOR();
};


/* cout function */
ostream& operator << (ostream&, const SearchNode&);

/* Inline definitions */
inline SearchNode::SearchNode(SearchNode* parent) :
    m_flags(0), m_parent(parent), m_nodeValue(ELEM_NAN), m_heurValue(INFINITY),
    m_children(NULL), m_childCountFull(0), m_childCountAct(0)
#if defined PARALLEL_DYNAMIC || defined PARALLEL_STATIC
  , m_subCount(0)
#endif
#ifdef PARALLEL_DYNAMIC
  , m_subLeaves(0), m_subLeafD(0)
#endif
  { /* intentionally empty */ }

inline SearchNode::~SearchNode() {
  this->clearChildren();
}

inline void SearchNode::setChild(SearchNode* node) {
  m_children = new NodeP[1];
  m_children[0] = node;
  m_childCountFull = m_childCountAct = 1;
}

inline void SearchNode::addChildren(const vector<SearchNode*>& nodes) {
  m_children = new NodeP[nodes.size()];
  for (size_t i = 0; i < nodes.size(); ++i) {
    m_children[i] = nodes[i];
  }
  m_childCountFull = m_childCountAct = nodes.size();
}

inline bool SearchNode::hasChild(SearchNode* node) const {
  for (size_t i = 0; i < m_childCountFull; ++i) {
    if (m_children[i] == node)
      return true;
  }
  return false;
}

inline void SearchNode::eraseChild(SearchNode* node) {
  for (size_t i = 0; i < m_childCountFull; ++i) {
    if (m_children[i] == node) {
      delete m_children[i];
      m_children[i] = NULL;
      --m_childCountAct;
      return;
    }
  }
}

inline void SearchNode::clearChildren() {
  if (!m_children) return;
  for (size_t i = 0; i < m_childCountFull; ++i) {
    if (m_children[i]) {
      delete m_children[i];
      m_children[i] = NULL;
    }
  }
  m_childCountAct = 0;
  delete[] m_children;
  m_children = NULL;
}


inline void SearchNodeOR::clearHeurCache() {
  if (m_heurCache) {
    delete[] m_heurCache;
    m_heurCache = NULL;
  }
}


inline SearchNodeAND::SearchNodeAND(SearchNode* parent, val_t val, double label) :
    SearchNode(parent), m_val(val), m_nodeLabel(label), m_subSolved(ELEM_ONE)
{
  m_nodeValue = ELEM_NAN;
}


inline SearchNodeOR::SearchNodeOR(SearchNode* parent, int var, int depth) :
  SearchNode(parent), m_var(var), m_depth(depth), m_heurCache(NULL) //, m_cacheContext(NULL)
#if defined PARALLE_STATIC || defined PARALLEL_DYNAMIC
  , m_initialBound(ELEM_NAN), m_complexityEstimate(ELEM_NAN)
#endif
  { /* empty */ }


inline SearchNodeOR::~SearchNodeOR() {
  this->clearHeurCache();
}

/*
inline bool SearchNodeComp::operator ()(const SearchNode* a, const SearchNode* b) const {
//  return a->getHeur() < b->getHeur();
  if (a->getHeur() == b->getHeur())
    return a<b; // TODO better criterion needed
  else
    return a->getHeur() < b->getHeur();
}
*/

inline bool SearchNode::heurLess(const SearchNode* a, const SearchNode* b) {
  assert(a && b);
  return a->getHeur() < b->getHeur();
}

}  // namespace daoopt

#endif /* SEARCHNODE_H_ */
