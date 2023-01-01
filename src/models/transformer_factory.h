// @TODO: rename to transformer.h eventually. This is not a Factory as in factory.h.
#pragma once

#include "marian-lite/marian.h"

#include "marian-lite/models/decoder.h"
#include "marian-lite/models/encoder.h"

namespace marian {
Ptr<EncoderBase> NewEncoderTransformer(Ptr<ExpressionGraph> graph, Ptr<Options> options);
Ptr<DecoderBase> NewDecoderTransformer(Ptr<ExpressionGraph> graph, Ptr<Options> options);
}  // namespace marian
