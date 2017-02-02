#pragma once

#include <map>
#include <memory>

#include "kernels/tensor_operators.h"
#include "optimizers/clippers.h"

namespace marian {

// @TODO: modify computation graph to group all paramters in single matrix object.
// This will allow to perform a single large SGD update per batch. Currently there
// are as many updates as different parameters.

class OptimizerBase {
  public:
    void update(Ptr<ExpressionGraph> graph) {
      graph->backprop();
      updateRule(graph);
    }

    void updateRule(Ptr<ExpressionGraph> graph) {
      Tensor p = graph->params().vals();
      Tensor g = graph->params().grads();
      updateRule(p, g);  
    }
    
    virtual void updateRule(Tensor params, Tensor grads) = 0;
};

class Sgd : public OptimizerBase {
  public:
    Sgd(float eta=0.01) : eta_(eta) {}

    void updateRule(Tensor params, Tensor grads) {
      Element(_1 -= eta_ * _2, params, grads);
    }

  private:
    float eta_;
};

// @TODO: Add serialization for historic gradients and parameters
class Adagrad : public OptimizerBase {
  public:
    Adagrad(float eta=0.01, float eps=1e-8)
    : eta_(eta), eps_(eps)
    {}

    void updateRule(Tensor params, Tensor grads) {
      if(!alloc_)
        alloc_ = New<TensorAllocator>(params->getDevice());

      if(!gt_) {
        int totalSize = params->size();
        alloc_->reserveExact(totalSize);
        alloc_->allocate(gt_, {1, totalSize});
        gt_->set(0);
      }
      
      Element(_1 += (_2 * _2),
              gt_, params);

      Element(_1 -= (eta_ / (Sqrt(_2) + eps_)) * _3,
              params, gt_, grads);
    }

  private:
    float eta_;
    float eps_;
    Ptr<TensorAllocator> alloc_;
    Tensor gt_;
};


// @TODO: Add serialization for historic gradients and parameters
// https://arxiv.org/pdf/1412.6980v8.pdf
class Adam : public OptimizerBase {
  public:
    template <typename ...Args>
    Adam(float eta = 0.001, Args ...args)
    : eta_(eta),
      beta1_(Get(keywords::beta1, 0.9, args...)),
      beta2_(Get(keywords::beta2, 0.999, args...)),
      eps_(Get(keywords::eps, 1e-8, args...)),
      clipper_(Get(keywords::clip, nullptr, args...)),
      t_(0)
    {}

    void updateRule(Tensor params, Tensor grads) {
      if(!mtAlloc_)
        mtAlloc_ = New<TensorAllocator>(params->getDevice());
      if(!vtAlloc_)
        vtAlloc_ = New<TensorAllocator>(params->getDevice());

      if(clipper_) {
        clipper_->clip(grads);
      }

      if(!mt_) {
        int totalSize = params->size();
        mtAlloc_->reserveExact(totalSize);
        mtAlloc_->allocate(mt_, {1, totalSize});
        mt_->set(0);

        vtAlloc_->reserveExact(totalSize);
        vtAlloc_->allocate(vt_, {1, totalSize});
        vt_->set(0);
      }

      t_++;
      float denom1 = 1 - pow(beta1_, t_);
      float denom2 = 1 - pow(beta2_, t_);
      
      Element(_1 = (beta1_ * _1) + ((1 - beta1_) * _2),
              mt_, grads);
      Element(_1 = (beta2_ * _1) + ((1 - beta2_) * (_2 * _2)),
              vt_, grads);

      Element(_1 -= eta_ * (_2 / denom1) / (Sqrt(_3 / denom2) + eps_),
              params, mt_, vt_);
    }

  private:
    float eta_;
    float beta1_;
    float beta2_;
    float eps_;
    size_t t_;

    Ptr<TensorAllocator> mtAlloc_;
    Tensor mt_;
    Ptr<TensorAllocator> vtAlloc_;
    Tensor vt_;

    Ptr<ClipperBase> clipper_;
};

template <class Algorithm, typename ...Args>
Ptr<OptimizerBase> Optimizer(Args&& ...args) {
  return Ptr<OptimizerBase>(new Algorithm(args...));
}

}