#include "search/edge_generator.hh"

#include "lm/left.hh"
#include "search/context.hh"
#include "search/vertex.hh"
#include "search/vertex_generator.hh"

namespace search {

bool EdgeGenerator::Init(Edge &edge) {
  from_ = &edge;
  PartialEdge root;
  root.score = GetRule().Bound();
  for (unsigned int i = 0; i < GetRule().Arity(); ++i) {
    root.nt[i] = edge.GetVertex(i).RootPartial();
    if (root.nt[i].Empty()) return false;
    root.score += root.nt[i].Bound();
  }
  for (unsigned int i = GetRule().Arity(); i < 2; ++i) {
    root.nt[i] = kBlankPartialVertex;
  }
  for (unsigned int i = 0; i < GetRule().Arity() + 1; ++i) {
    root.between[i] = GetRule().Lexical(i);
  }
  // wtf no clear method?
  generate_ = Generate();
  generate_.push(root);
  top_ = root.score;
  return true;
}

template <class Model> bool EdgeGenerator::Pop(Context<Model> &context, VertexGenerator &parent) {
  assert(!generate_.empty());
  const PartialEdge &top = generate_.top();
  unsigned int victim = 0;
  unsigned char lowest_length = 255;
  for (unsigned int i = 0; i != GetRule().Arity(); ++i) {
    if (!top.nt[i].Complete() && top.nt[i].Length() < lowest_length) {
      lowest_length = top.nt[i].Length();
      victim = i;
    }
  }
  if (lowest_length == 255) {
    // All states report complete.  
    lm::ngram::ChartState state;
    RecomputeFinal(context, top, state);
    parent.NewHypothesis(state, *from_, top);
    generate_.pop();
    top_ = generate_.empty() ? -kScoreInf : generate_.top().score;
    return !generate_.empty();
  }

  unsigned int stay = !victim;
  PartialEdge continuation, alternate;
  continuation.nt[stay] = top.nt[stay];
  alternate.nt[stay] = top.nt[stay];
  // The alternate's score will change because alternate.nt[victim] changes.  
  alternate.score = top.score - top.nt[victim].Bound();
  bool split = top.nt[victim].Split(continuation.nt[victim], alternate.nt[victim]);
  generate_.pop();
  // top is now a dangling reference.  

  if (split) {
    // We have an alternate.  
    alternate.score += alternate.nt[victim].Bound();
    // TODO: dedupe?  
    generate_.push(alternate);
  }
  continuation.score = GetRule().Bound() + Adjustment(context, continuation) * context.GetWeights().LM();
  for (unsigned int i = 0; i < GetRule().Arity(); ++i) {
    continuation.score += continuation.nt[i].Bound();
  }
  // TODO: dedupe?  
  generate_.push(continuation);
  top_ = generate_.top().score;
  return true;
}

template <class Model> void EdgeGenerator::RecomputeFinal(Context<Model> &context, const PartialEdge &to, lm::ngram::ChartState &state) {
  if (GetRule().Arity() == 0) {
    state = GetRule().Lexical(0);
    return;
  }
  lm::ngram::RuleScore<Model> scorer(context.LanguageModel(), state);
  if (GetRule().BeginSentence()) {
    scorer.BeginSentence();
    scorer.NonTerminal(GetRule().Lexical(0));
  } else {
    scorer.BeginNonTerminal(GetRule().Lexical(0));
  }
  scorer.NonTerminal(to.nt[0].State());
  scorer.NonTerminal(GetRule().Lexical(1));
  if (GetRule().Arity() == 2) {
    scorer.NonTerminal(to.nt[1].State());
    scorer.NonTerminal(GetRule().Lexical(2));
  }
  scorer.Finish();
  return;
}

// TODO: this can be done WAY more efficiently.
template <class Model> Score EdgeGenerator::Adjustment(Context<Model> &context, const PartialEdge &to) const {
  if (GetRule().Arity() == 0) return 0.0;
  lm::ngram::ChartState state;
  lm::ngram::RuleScore<Model> scorer(context.LanguageModel(), state);
  scorer.BeginNonTerminal(GetRule().Lexical(0));
  scorer.NonTerminal(to.nt[0].State());
  float total = 0.0;
  if (!to.nt[0].Complete()) {
    total += scorer.Finish();
    scorer.Reset();
    scorer.BeginNonTerminal(to.nt[0].State());
  }
  scorer.NonTerminal(GetRule().Lexical(1));
  if (GetRule().Arity() == 1) return total + scorer.Finish();
  assert(GetRule().Arity() == 2);
  scorer.NonTerminal(to.nt[1].State());
  if (!to.nt[1].Complete()) {
    total += scorer.Finish();
    scorer.Reset();
    scorer.BeginNonTerminal(to.nt[1].State());
  }
  scorer.NonTerminal(GetRule().Lexical(2));
  return total + scorer.Finish();
}

template bool EdgeGenerator::Pop(Context<lm::ngram::RestProbingModel> &context, VertexGenerator &parent);
template bool EdgeGenerator::Pop(Context<lm::ngram::ProbingModel> &context, VertexGenerator &parent);
template void EdgeGenerator::RecomputeFinal(Context<lm::ngram::RestProbingModel> &context, const PartialEdge &to, lm::ngram::ChartState &state);
template void EdgeGenerator::RecomputeFinal(Context<lm::ngram::ProbingModel> &context, const PartialEdge &to, lm::ngram::ChartState &state);
template Score EdgeGenerator::Adjustment(Context<lm::ngram::RestProbingModel> &context, const PartialEdge &to) const;
template Score EdgeGenerator::Adjustment(Context<lm::ngram::ProbingModel> &context, const PartialEdge &to) const;

} // namespace search
