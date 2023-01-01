#pragma once

#include <string>

#include "marian-lite/data/batch_generator.h"
#include "marian-lite/data/corpus.h"
#include "marian-lite/data/shortlist.h"
#include "marian-lite/data/text_input.h"

#if USE_PTHREADS
#include "marian-lite/3rd_party/threadpool.h"
#endif

#include "marian-lite/translator/history.h"
#include "marian-lite/translator/output_collector.h"
#include "marian-lite/translator/output_printer.h"

#include "marian-lite/models/model_task.h"
#include "marian-lite/translator/scorers.h"

// currently for diagnostics only, will try to mmap files ending in *.bin suffix when enabled.
#include "marian-lite/3rd_party/mio/mio.hpp"

namespace marian {

template <class Search>
class Translate : public ModelTask {
private:
  Ptr<Options> options_;
  std::vector<Ptr<ExpressionGraph>> graphs_;
  std::vector<std::vector<Ptr<Scorer>>> scorers_;

  Ptr<data::Corpus> corpus_;
  Ptr<Vocab> trgVocab_;
  Ptr<const data::ShortlistGenerator> shortlistGenerator_;

  size_t numDevices_;

  std::vector<mio::mmap_source> model_mmaps_; // map
  std::vector<std::vector<io::Item>> model_items_; // non-mmap

public:
  Translate(Ptr<Options> options)
    : options_(New<Options>(options->clone())) { // @TODO: clone should return Ptr<Options> same as "with"?
    // This is currently safe as the translator is either created stand-alone or
    // or config is created anew from Options in the validator

    options_->set("inference", true,
                  "shuffle", "none");

    corpus_ = New<data::Corpus>(options_, true);

    auto vocabs = options_->get<std::vector<std::string>>("vocabs");
    trgVocab_ = New<Vocab>(options_, vocabs.size() - 1);
    trgVocab_->load(vocabs.back());
    auto srcVocab = corpus_->getVocabs()[0];

    if(options_->hasAndNotEmpty("shortlist")) {
      auto slOptions = options_->get<std::vector<std::string>>("shortlist");
      ABORT_IF(slOptions.empty(), "No path to shortlist file given");
      std::string filename = slOptions[0];
      if(data::isBinaryShortlist(filename))
        shortlistGenerator_ = New<data::BinaryShortlistGenerator>(
            options_, srcVocab, trgVocab_, 0, 1, vocabs.front() == vocabs.back());
      else
          shortlistGenerator_ = New<data::LexicalShortlistGenerator>(
              options_, srcVocab, trgVocab_, 0, 1, vocabs.front() == vocabs.back());
    }

    auto devices = Config::getDevices(options_);
    numDevices_ = devices.size();

#if USE_PTHREADS
    ThreadPool threadPool(numDevices_, numDevices_);
#endif
    scorers_.resize(numDevices_);
    graphs_.resize(numDevices_);

    auto models = options->get<std::vector<std::string>>("models");
    if(options_->get<bool>("model-mmap", false)) {
      for(auto model : models) {
        ABORT_IF(!io::isBin(model), "Non-binarized models cannot be mmapped");
        model_mmaps_.push_back(mio::mmap_source(model));
      }
    }
    else {
      for(auto model : models) {
        auto items = io::loadItems(model);
        model_items_.push_back(std::move(items));
      }
    }

    size_t id = 0;
    for(auto device : devices) {
      auto task = [&](DeviceId device, size_t id) {
        auto graph = New<ExpressionGraph>(true);
        auto prec = options_->get<std::vector<std::string>>("precision", {"float32"});
        graph->setDefaultElementType(typeFromString(prec[0]));
        graph->setDevice(device);
        graph->getBackend()->configureDevice(options_);
        graph->reserveWorkspaceMB(options_->get<size_t>("workspace"));
        graphs_[id] = graph;

        std::vector<Ptr<Scorer>> scorers;
        if(options_->get<bool>("model-mmap", false)) {
          scorers = createScorers(options_, model_mmaps_);
        }
        else {
          scorers = createScorers(options_, model_items_);
        }

        for(auto scorer : scorers) {
          scorer->init(graph);
          if(shortlistGenerator_)
            scorer->setShortlistGenerator(shortlistGenerator_);
        }

        scorers_[id] = scorers;
        graph->forward();
      };

#if USE_PTHREADS
      threadPool.enqueue(task, device, id++);
#else
      task(device, id++);
#endif
    }

    if(options_->get<bool>("output-sampling", false)) {
        if (options_->get<size_t>("beam-size") > 1) {

        }
    }
  }

  void run() override {
  #if USE_PTHREADS
    data::BatchGenerator<data::Corpus> bg(corpus_, options_);
  #else
    // Set to false to run non-async mode
    data::BatchGenerator<data::Corpus> bg(corpus_, options_, nullptr, false);
  #endif

#if USE_PTHREADS
    ThreadPool threadPool(numDevices_, numDevices_);
#endif

    size_t batchId = 0;
    auto collector = New<OutputCollector>(options_->get<std::string>("output"));
    auto printer = New<OutputPrinter>(options_, trgVocab_);
    if(options_->get<bool>("quiet-translation"))
      collector->setPrintingStrategy(New<QuietPrinting>());

    bg.prepare();

    bool doNbest = options_->get<bool>("n-best");
    for(auto batch : bg) {
      auto task = [=](size_t id) {
        thread_local Ptr<ExpressionGraph> graph;
        thread_local std::vector<Ptr<Scorer>> scorers;

        if(!graph) {
          graph = graphs_[id % numDevices_];
          scorers = scorers_[id % numDevices_];
        }

        auto search = New<Search>(options_, scorers, trgVocab_);
        auto histories = search->search(graph, batch);

        for(auto history : histories) {
          std::stringstream best1;
          std::stringstream bestn;
          printer->print(history, best1, bestn);
          collector->Write((long)history->getLineNum(),
                           best1.str(),
                           bestn.str(),
                           doNbest);
        }


        // progress heartbeat for MS-internal Philly compute cluster
        // otherwise this job may be killed prematurely if no log for 4 hrs
        if (getenv("PHILLY_JOB_ID")   // this environment variable exists when running on the cluster
            && id % 1000 == 0)  // hard beat once every 1000 batches
        {
          auto progress = 0.f; //fake progress for now
          fprintf(stderr, "PROGRESS: %.2f%%\n", progress);
          fflush(stderr);
        }
      };

#if USE_PTHREADS
      threadPool.enqueue(task, batchId++);
#else
      task(batchId++);
#endif

    }
  }
};

template <class Search>
class TranslateService : public ModelServiceTask {
private:
  Ptr<Options> options_;
  std::vector<Ptr<ExpressionGraph>> graphs_;
  std::vector<std::vector<Ptr<Scorer>>> scorers_;

  std::vector<Ptr<Vocab>> srcVocabs_;
  Ptr<Vocab> trgVocab_;
  Ptr<const data::ShortlistGenerator> shortlistGenerator_;

  size_t numDevices_;

public:
  virtual ~TranslateService() {}

  TranslateService(Ptr<Options> options)
    : options_(New<Options>(options->clone())) {
    // initialize vocabs
    options_->set("inference", true);
    options_->set("shuffle", "none");

    auto vocabPaths = options_->get<std::vector<std::string>>("vocabs");
    std::vector<int> maxVocabs = options_->get<std::vector<int>>("dim-vocabs");

    for(size_t i = 0; i < vocabPaths.size() - 1; ++i) {
      Ptr<Vocab> vocab = New<Vocab>(options_, i);
      vocab->load(vocabPaths[i], maxVocabs[i]);
      srcVocabs_.emplace_back(vocab);
    }

    trgVocab_ = New<Vocab>(options_, vocabPaths.size() - 1);
    trgVocab_->load(vocabPaths.back());

    // load lexical shortlist
    if(options_->hasAndNotEmpty("shortlist")) {
      auto slOptions = options_->get<std::vector<std::string>>("shortlist");
      ABORT_IF(slOptions.empty(), "No path to shortlist file given");
      std::string filename = slOptions[0];
      if(data::isBinaryShortlist(filename))
        shortlistGenerator_ = New<data::BinaryShortlistGenerator>(
            options_, srcVocabs_.front(), trgVocab_, 0, 1, vocabPaths.front() == vocabPaths.back());
      else
        shortlistGenerator_ = New<data::LexicalShortlistGenerator>(
            options_, srcVocabs_.front(), trgVocab_, 0, 1, vocabPaths.front() == vocabPaths.back());
    }

    // get device IDs
    auto devices = Config::getDevices(options_);
    numDevices_ = devices.size();

    // preload models
    std::vector<std::vector<io::Item>> model_items_;
    auto models = options->get<std::vector<std::string>>("models");
    for(auto model : models) {
      auto items = io::loadItems(model);
      model_items_.push_back(std::move(items));
    }

    // initialize scorers
    for(auto device : devices) {
      auto graph = New<ExpressionGraph>(true);

      auto precison = options_->get<std::vector<std::string>>("precision", {"float32"});
      graph->setDefaultElementType(typeFromString(precison[0])); // only use first type, used for parameter type in graph
      graph->setDevice(device);
      graph->getBackend()->configureDevice(options_);
      graph->reserveWorkspaceMB(options_->get<size_t>("workspace"));
      graphs_.push_back(graph);

      auto scorers = createScorers(options_, model_items_);
      for(auto scorer : scorers) {
        scorer->init(graph);
        if(shortlistGenerator_)
          scorer->setShortlistGenerator(shortlistGenerator_);
      }
      scorers_.push_back(scorers);
    }
  }

  std::string run(const std::string& input) override {
    // split tab-separated input into fields if necessary
    auto inputs = options_->get<bool>("tsv", false)
                      ? convertTsvToLists(input, options_->get<size_t>("tsv-fields", 1))
                      : std::vector<std::string>({input});
    auto corpus_ = New<data::TextInput>(inputs, srcVocabs_, options_);
  #if USE_PTHREADS
    data::BatchGenerator<data::TextInput> batchGenerator(corpus_, options_);
  #else
    // Set to false to run non-async mode
    data::BatchGenerator<data::TextInput> batchGenerator(corpus_, options_, nullptr, false);
  #endif

    auto collector = New<StringCollector>(options_->get<bool>("quiet-translation", false));
    auto printer = New<OutputPrinter>(options_, trgVocab_);
    size_t batchId = 0;

    batchGenerator.prepare();

    {
#if USE_PTHREADS
      ThreadPool threadPool_(numDevices_, numDevices_);
#endif

      for(auto batch : batchGenerator) {
        auto task = [=](size_t id) {
          thread_local Ptr<ExpressionGraph> graph;
          thread_local std::vector<Ptr<Scorer>> scorers;

          if(!graph) {
            graph = graphs_[id % numDevices_];
            scorers = scorers_[id % numDevices_];
          }

          auto search = New<Search>(options_, scorers, trgVocab_);
          auto histories = search->search(graph, batch);

          for(auto history : histories) {
            std::stringstream best1;
            std::stringstream bestn;
            printer->print(history, best1, bestn);
            collector->add((long)history->getLineNum(), best1.str(), bestn.str());
          }
        };

#if USE_PTHREADS
        threadPool_.enqueue(task, batchId);
#else
        task(batchId);
#endif
        batchId++;
      }
    }

    auto translations = collector->collect(options_->get<bool>("n-best"));
    return utils::join(translations, "\n");
  }

private:
  // Converts a multi-line input with tab-separated source(s) and target sentences into separate lists
  // of sentences from source(s) and target sides, e.g.
  // "src1 \t trg1 \n src2 \t trg2" -> ["src1 \n src2", "trg1 \n trg2"]
  std::vector<std::string> convertTsvToLists(const std::string& inputText, size_t numFields) {
    std::vector<std::string> outputFields(numFields);

    std::string line;
    std::vector<std::string> lineFields(numFields);
    std::istringstream inputStream(inputText);
    bool first = true;
    while(std::getline(inputStream, line)) {
      utils::splitTsv(line, lineFields, numFields);
      for(size_t i = 0; i < numFields; ++i) {
        if(!first)
          outputFields[i] += "\n";  // join sentences with a new line sign
        outputFields[i] += lineFields[i];
      }
      if(first)
        first = false;
    }

    return outputFields;
  }
};
}  // namespace marian
