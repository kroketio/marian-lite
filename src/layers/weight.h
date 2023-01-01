#pragma once

#include "marian-lite/common/options.h"
#include "marian-lite/data/corpus.h"
#include "marian-lite/graph/expression_graph.h"
#include "marian-lite/graph/expression_operators.h"
#include "marian-lite/graph/node_initializers.h"

namespace marian {

class WeightingBase {
public:
  WeightingBase(){};
  virtual Expr getWeights(Ptr<ExpressionGraph> graph,
                          Ptr<data::CorpusBatch> batch)
      = 0;
  virtual void debugWeighting(std::vector<float> /*weightedMask*/,
                              std::vector<float> /*freqMask*/,
                              Ptr<data::CorpusBatch> /*batch*/){};
  virtual ~WeightingBase() {}
};

class DataWeighting : public WeightingBase {
protected:
  std::string weightingType_;

public:
  DataWeighting(std::string weightingType)
      : WeightingBase(), weightingType_(weightingType){};
  Expr getWeights(Ptr<ExpressionGraph> graph, Ptr<data::CorpusBatch> batch) override;
};

Ptr<WeightingBase> WeightingFactory(Ptr<Options> options);
}  // namespace marian
