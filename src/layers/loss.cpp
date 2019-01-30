#include "layers/loss.h"

namespace marian {

Ptr<LossBase> LossFactory(Ptr<Options> options, bool inference) {
  float smoothing = inference ? 0.f : options->get<float>("label-smoothing");
  std::string costType = options->get<std::string>("cost-type", "ce-mean");
  if(costType == "ce-mean" || costType == "cross-entropy") {
    return New<CrossEntropyMeanLoss>(smoothing);
  } else if(costType == "ce-mean-words") {
    return New<CrossEntropyMeanWordsLoss>(smoothing);
  } else if(costType == "ce-sum") {
    return New<CrossEntropySumLoss>(smoothing);
  } else if(costType == "perplexity") {
    return New<PerplexityLoss>(smoothing);
  } else if(costType == "ce-rescore") {
    return New<CrossEntropyRescoreLoss>(smoothing);
  } else if(costType == "ce-rescore-mean") {
    return New<CrossEntropyRescoreMeanLoss>(smoothing);
  } else {  // same as ce-mean
    return New<CrossEntropyMeanLoss>(smoothing);
  }
}

Expr LossBase::getCrossEntropy(const Logits& logits,
                               Expr indices,
                               Expr mask,
                               Expr weights) {
  // logits may be factored; in that case, the getLoss() function computes one loss for each, and sums them up
  auto ce = logits.getLoss(indices, [&](Expr logits, Expr indices) {
    Expr ce = cross_entropy(logits, indices);
    if (smoothing_ > 0) {
      // ce = -sum_i y^_i log y_i(h)
      // with smoothing:
      // ce' = -sum_i ((1-smoothing_) y^_i + smoothing_/N) log y_i(h)
      //     = -(1-smoothing_) sum_i y^_i log y_i(h) - smoothing_ mean_i log y_i(h)
      //     = (1-smoothing_) ce - smoothing_ mean_i log y_i(h)
      auto ceqNeg = mean(logits, /*axis=*/ -1) - logsumexp(logits, /*axis=*/ -1);
      ce = (1 - smoothing_) * ce - smoothing_ * ceqNeg;
      //ce = ce - smoothing_ * (ce + ceqNeg); // writing it this way saves one op :)
    }
    return ce;
  });

  if(mask)
    ce = ce * mask;

  if(weights)
    ce = ce * weights;

  return ce;
}

Expr CrossEntropyMeanLoss::getCost(const Logits& logits,
                                   Expr indices,
                                   Expr mask,
                                   Expr weights) {
  auto ce = getCrossEntropy(logits, indices, mask, weights);
  // Time axis (words): -3
  // Batch axis (sentences): -2
  // if(weights) {
  //   return sum(sum(ce, /*axis =*/ -3) /*axis =*/ -2);
  //          / sum(mean(mask * weights, /*axis =*/ -3) /*axis =*/ -2);
  // }
  // else {
    return mean(sum(ce, /*axis =*/ -3), /*axis =*/ -2);
  // }
}

Expr CrossEntropyMeanWordsLoss::getCost(const Logits& logits,
                                        Expr indices,
                                        Expr mask,
                                        Expr weights) {
  auto ce = getCrossEntropy(logits, indices, mask, weights);
  // if(weights) {
  //   return (sum(sum(ce, /*axis =*/ -3), /*axis =*/ -2)
  //          / sum(sum(mask * weights, /*axis =*/ -3), /*axis =*/ -2));
  // }
  // else {
    return sum(sum(ce, /*axis =*/ -3), /*axis =*/ -2) // sum CE over all words in the batch
           / sum(sum(mask, /*axis =*/ -3), /*axis =*/ -2); // divide by number of words (sum over mask)
  // }
}

Expr CrossEntropySumLoss::getCost(const Logits& logits,
                                  Expr indices,
                                  Expr mask,
                                  Expr weights) {
  auto ce = getCrossEntropy(logits, indices, mask, weights);
  // if(weights) {
  //   return sum(sum(ce, /*axis =*/ -3), /*axis =*/ -2)
  //          / mean(mean(mask * weights, /*axis =*/ -3), /*axis =*/ -2);
  // }
  // else {
    return sum(sum(ce, /*axis =*/ -3), /*axis =*/ -2);
  // }
}

Expr PerplexityLoss::getCost(const Logits& logits,
                             Expr indices,
                             Expr mask,
                             Expr weights) {
  auto ce = getCrossEntropy(logits, indices, mask, weights);
  // if(weights) {
  //   return exp(sum(sum(ce, /*axis =*/ -3), /*axis =*/ -2)
  //              / sum(sum(mask * weights, /*axis =*/ -3), /*axis =*/ -2));
  // }
  // else {
    return exp(sum(sum(ce, /*axis =*/ -3), /*axis =*/ -2) // sum CE over all words in the batch
               / sum(sum(mask, /*axis =*/ -3), /*axis =*/ -2)); // divide by number of words (sum over mask)
  // }
}

Expr CrossEntropyRescoreLoss::getCost(const Logits& logits,
                                      Expr indices,
                                      Expr mask,
                                      Expr weights) {
  auto ce = getCrossEntropy(logits, indices, mask, weights);
  return -sum(ce, /*axis =*/ -3);
}

Expr CrossEntropyRescoreMeanLoss::getCost(const Logits& logits,
                                          Expr indices,
                                          Expr mask,
                                          Expr weights) {
  auto ce = getCrossEntropy(logits, indices, mask, weights);
  // divide by number of words in sentence
  return -sum(ce, /*axis =*/ -3) / sum(mask, /*axis =*/ -3);
}

}  // namespace marian
