#include "search/edge_generator.hh"

#include "lm/left.hh"
#include "search/context.hh"
#include "search/vertex.hh"
#include "search/vertex_generator.hh"

namespace search {

void EdgeGenerator::Init(Edge &edge) {
  from_ = &edge;
  PartialEdge root;
  root.score = GetRule().Bound();
  for (unsigned int i = 0; i < GetRule().Arity(); ++i) {
    root.nt[i] = edge.GetVertex(i).RootPartial();
    if (root.nt[i].Empty()) return;
    root.score += root.nt[i].Bound();
  }
  for (unsigned int i = GetRule().Arity(); i < 2; ++i) {
    root.nt[i] = kBlankPartialVertex;
  }
  // wtf no clear method?
  generate_ = Generate();
  generate_.push(root);
  top_ = root.score;
}

unsigned int EdgeGenerator::PickVictim(const PartialEdge &in) const {
  // TODO: better decision rule.
  return in.nt[0].Length() >= in.nt[1].Length();
}

bool EdgeGenerator::Pop(Context &context, VertexGenerator &parent) {
  const PartialEdge &top = generate_.top();
  unsigned int victim;
  if (top.nt[0].Complete()) {
    if (top.nt[1].Complete()) {
      lm::ngram::ChartState state;
      RecomputeFinal(context, top, state);
      parent.NewHypothesis(state, *from_, top);
      generate_.pop();
      top_ = generate_.empty() ? -kScoreInf : generate_.top().score;
      return !generate_.empty();
    }
    victim = 1;
  } else if (top.nt[1].Complete()) {
    victim = 0;
  } else {
    victim = PickVictim(top);
  }
  unsigned int stay = !victim;
  PartialEdge continuation, alternate;
  continuation.nt[stay] = top.nt[stay];
  alternate.nt[stay] = top.nt[stay];
  // The alternate's score will change because alternate.nt[victim] changes.  
  alternate.score = top.score - alternate.nt[victim].Bound();
  if (top.nt[victim].Split(continuation.nt[victim], alternate.nt[victim])) {
    // We have an alternate.  
    alternate.score += alternate.nt[victim].Bound();
    // TODO: dedupe?  
    generate_.push(alternate);
  }
  continuation.score = GetRule().Bound() + continuation.nt[0].Bound() + continuation.nt[1].Bound() + Adjustment(context, continuation);
  // TODO: dedupe?  
  generate_.push(continuation);
  generate_.pop();
  top_ = generate_.top().score;
  return true;
}

void EdgeGenerator::RecomputeFinal(Context &context, const PartialEdge &to, lm::ngram::ChartState &state) {
  if (GetRule().Arity() == 0) {
    state = GetRule().Lexical(0);
    return;
  }
  lm::ngram::RuleScore<lm::ngram::RestProbingModel> scorer(context.LanguageModel(), state);
  if (GetRule().BeginSentence()) {
    scorer.BeginSentence();
    scorer.NonTerminal(GetRule().Lexical(0));
  } else {
    scorer.BeginNonTerminal(GetRule().Lexical(0));
  }
  scorer.NonTerminal(to.nt[0].State());
  scorer.NonTerminal(GetRule().Lexical(1));
  if (GetRule().Arity() == 1) return;
  scorer.NonTerminal(to.nt[1].State());
  scorer.NonTerminal(GetRule().Lexical(2));
  return;
}

// TODO: this can be done WAY more efficiently.
Score EdgeGenerator::Adjustment(Context &context, const PartialEdge &to) const {
  if (GetRule().Arity() == 0) return 0.0;
  lm::ngram::ChartState state;
  lm::ngram::RuleScore<lm::ngram::RestProbingModel> scorer(context.LanguageModel(), state);
  if (GetRule().BeginSentence()) {
    scorer.BeginSentence();
    scorer.NonTerminal(GetRule().Lexical(0));
  } else {
    scorer.BeginNonTerminal(GetRule().Lexical(0));
  }
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

} // namespace search
