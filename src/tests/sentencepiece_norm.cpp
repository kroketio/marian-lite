#include "marian.h"

#include "common/cli_helper.h"
#include "common/cli_wrapper.h"
#include "common/definitions.h"
#include "common/options.h"
#include "data/vocab.h"

#include <iostream>
#include <string>

using namespace marian;
using namespace marian::cli;

int main(int argc, char **argv) {
  auto options = New<Options>();

  YAML::Node config;
  auto w = New<CLIWrapper>(config);
  w->add<std::string>("-v,--vocab", "Path to vocab file");

  w->parse(argc, argv);
  options->merge(config);

  auto vocabPath = options->get<std::string>("vocab");
  size_t batchIndex = 0;
  auto vocab = New<Vocab>(options, batchIndex);
  vocab->load(vocabPath);

  std::string line;
  bool addEOS{true}, inference{true};
  while(std::getline(std::cin, line)) {
    std::vector<string_view> byteRanges;
    auto words = vocab->encodeWithByteRanges(line, byteRanges, addEOS, inference);
    bool first = true;

    // If the ByteRanges coming out of SentencePiece are correct, expected
    // result is sourceToken that a word corresponding points to. The output
    // here is compared to an expected output containing unnormalized strings
    // and matches, test passes. Testing requires a SentencePiece model, and
    // hence designed as a test-app with tests in marian-regression-tests.

    for(auto &sourceView : byteRanges) {
      if(not first) {
        std::cout << " ";
        first = false;
      }
      std::cout << sourceView;
    }
    std::cout << std::endl;
  }

  return 0;
}
