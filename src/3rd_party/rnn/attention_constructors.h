#pragma once

#include "marian-lite/marian.h"

#include "marian-lite/layers/factory.h"
#include "marian-lite/3rd_party/rnn/attention.h"
#include "marian-lite/3rd_party/rnn/constructors.h"
#include "marian-lite/3rd_party/rnn/types.h"

namespace marian {
namespace rnn {

class AttentionFactory : public InputFactory {
protected:
  Ptr<EncoderState> state_;

public:
//  AttentionFactory(Ptr<ExpressionGraph> graph) : InputFactory(graph) {}

  Ptr<CellInput> construct(Ptr<ExpressionGraph> graph) override {
    ABORT_IF(!state_, "EncoderState not set");
    return New<Attention>(graph, options_, state_);
  }

  Accumulator<AttentionFactory> set_state(Ptr<EncoderState> state) {
    state_ = state;
    return Accumulator<AttentionFactory>(*this);
  }

  int dimAttended() {
    ABORT_IF(!state_, "EncoderState not set");
    return state_->getAttended()->shape()[1];
  }
};

typedef Accumulator<AttentionFactory> attention;
}  // namespace rnn
}  // namespace marian
