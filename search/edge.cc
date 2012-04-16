#include "search/edge.hh"

#include "lm/left.hh"
#include "search/vertex.hh"

namespace search {

void EdgeGenerator::Init(Edge &edge) {
  from_ = &edge;
  PartialEdge root;
  root.score = GetRule().Bound();
  for (Index i = 0; i < GetRule().Arity(); ++i) {
    root.nt[i] = edge.GetVertex(i).RootPartial();
    root.score += root.nt[i].Bound();
  }
  for (Index i = GetRule().Arity(); i < 2; ++i) {
    root.nt[i] = kBlankPartialVertex;
  }
  generate_.clear();
  generate_.push(root);
  top_ = root.score;
}

void EdgeGenerator::Pop(Context &context, VertexGenerator &parent) {
  const PartialEdge &top = generate_.top();
  unsigned int victim;
  if (top.nt[0].Complete()) {
    if (top.nt[1].Complete()) {
      lm::ngram::ChartState state;
      RecomputeFinal(context, top, state);
      parent.NewHypothesis(state, *from_, top);
      generate_.pop();
      top_ = generate_.empty() ? -kScoreInf : generate_.top().score;
      return;
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
  if (top.nt[victim].Split(context, continuation.nt[victim], alternate.nt[victim])) {
    // We have an alternate.  
    alternate.score += alternate.nt[victim].Bound();
    // TODO: dedupe?  
    generate_.push(alternate);
  }
  continuation.score = rule_.Bound() + continuation.nt[0].Bound() + continuation.nt[1].Bound() + Adjustment(context, continuation);
  // TODO: dedupe?  
  generate_.push(continuation);
  generate_.pop();
  top_ = generate_.top().score;
}

void EdgeGenerator::RecomputeFinal(Context &context, const PartialEdge &to, lm::ngram::ChartState &state) {
  if (rule_.Arity() == 0) {
    state = rule_.Lexical(0);
    return;
  }
  lm::ngram::RuleScore<lm::ngram::RestProbingModel> scorer(context.LanguageModel(), state);
  if (rule_.BeginSentence()) {
    scorer.BeginSentence();
    scorer.NonTerminal(rule_.Lexical(0));
  } else {
    scorer.BeginNonTerminal(rule_.Lexical(0));
  }
  scorer.NonTerminal(to.nt[0].State());
  scorer.NonTerminal(rule_.Lexical(1));
  if (rule_.Arity() == 1) return;
  scorer.NonTerminal(to.nt[1].State());
  scorer.NonTerminal(rule_.Lexical(2));
  return;
}

// TODO: this can be done WAY more efficiently.
Score EdgeGenerator::Adjustment(Context &context, const PartialEdge &to) const {
  if (rule_.Arity() == 0) return 0.0;
  lm::ngram::ChartState state;
  lm::ngram::RuleScore<lm::ngram::RestProbingModel> scorer(context.LanguageModel(), state);
  if (rule_.BeginSentence()) {
    scorer.BeginSentence();
    scorer.NonTerminal(rule_.Lexical(0));
  } else {
    scorer.BeginNonTerminal(rule_.Lexical(0));
  }
  scorer.NonTerminal(to.nt[0].State());
  float total = 0.0;
  if (!to.nt[0].Complete()) {
    total += scorer.Finish();
    scorer.Reset();
    scorer.BeginNonTerminal(to.nt[0].State());
  }
  scorer.NonTerminal(rule_.Lexical(1));
  if (rule_.Arity() == 1) return total + scorer.Finish();
  assert(rule_.Arity() == 2);
  scorer.NonTerminal(to.nt[1].State());
  if (!to.nt[1].Complete()) {
    total += scorer.Finish();
    scorer.Reset();
    scorer.BeginNonTerminal(to.nt[1].State());
  }
  scorer.NonTerminal(rule_.Lexical(2));
  return total + scorer.Finish();
}

} // namespace search
