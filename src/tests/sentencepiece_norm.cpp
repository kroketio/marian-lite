#include "marian.h"

#include "common/definitions.h"
#include "common/options.h"
#include "common/cli_wrapper.h"
#include "common/cli_helper.h"
#include "data/vocab.h"

#include <iostream>
#include <string>

using namespace marian;
using namespace marian::cli;


int main(int argc, char **argv){
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
  while(std::getline(std::cin, line)){
      std::vector<string_view> alignments;
      auto words = vocab->encodePreservingSource(line, 
                                                 alignments, 
                                                 addEOS, 
                                                 inference);
      bool first = true;

      /*
       * alignments are constructed from ByteRanges. If the ByteRanges coming
       * out of SentencePiece are correct, expected result is sourceToken that a
       * word corresponding points to.
       */

      for(auto sourceView: alignments){ 
          if(not first){
              std::cout << " ";
              first = false;
          }
          std::cout << sourceView;
      } 
      std::cout << std::endl;
  } 

  return 0; 
}
