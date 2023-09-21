
// Generated from antlr4/simple_graph_grammar.g4 by ANTLR 4.11.1


#include "simple_graph_grammarLexer.h"


using namespace antlr4;



using namespace antlr4;

namespace {

struct Simple_graph_grammarLexerStaticData final {
  Simple_graph_grammarLexerStaticData(std::vector<std::string> ruleNames,
                          std::vector<std::string> channelNames,
                          std::vector<std::string> modeNames,
                          std::vector<std::string> literalNames,
                          std::vector<std::string> symbolicNames)
      : ruleNames(std::move(ruleNames)), channelNames(std::move(channelNames)),
        modeNames(std::move(modeNames)), literalNames(std::move(literalNames)),
        symbolicNames(std::move(symbolicNames)),
        vocabulary(this->literalNames, this->symbolicNames) {}

  Simple_graph_grammarLexerStaticData(const Simple_graph_grammarLexerStaticData&) = delete;
  Simple_graph_grammarLexerStaticData(Simple_graph_grammarLexerStaticData&&) = delete;
  Simple_graph_grammarLexerStaticData& operator=(const Simple_graph_grammarLexerStaticData&) = delete;
  Simple_graph_grammarLexerStaticData& operator=(Simple_graph_grammarLexerStaticData&&) = delete;

  std::vector<antlr4::dfa::DFA> decisionToDFA;
  antlr4::atn::PredictionContextCache sharedContextCache;
  const std::vector<std::string> ruleNames;
  const std::vector<std::string> channelNames;
  const std::vector<std::string> modeNames;
  const std::vector<std::string> literalNames;
  const std::vector<std::string> symbolicNames;
  const antlr4::dfa::Vocabulary vocabulary;
  antlr4::atn::SerializedATNView serializedATN;
  std::unique_ptr<antlr4::atn::ATN> atn;
};

::antlr4::internal::OnceFlag simple_graph_grammarlexerLexerOnceFlag;
Simple_graph_grammarLexerStaticData *simple_graph_grammarlexerLexerStaticData = nullptr;

void simple_graph_grammarlexerLexerInitialize() {
  assert(simple_graph_grammarlexerLexerStaticData == nullptr);
  auto staticData = std::make_unique<Simple_graph_grammarLexerStaticData>(
    std::vector<std::string>{
      "T__0", "T__1", "T__2", "T__3", "T__4", "T__5", "T__6", "T__7", "T__8", 
      "T__9", "T__10", "T__11", "T__12", "T__13", "T__14", "T__15", "T__16", 
      "T__17", "IF", "THEN", "ELSE", "AS", "VEC", "REWRITE_TO", "FORALL", 
      "QM", "EQ", "STAR", "LPAR", "RPAR", "COL", "QPAR", "PPAR", "OTHERS", 
      "SPACE", "COMMENT", "LINE_COMMENT"
    },
    std::vector<std::string>{
      "DEFAULT_TOKEN_CHANNEL", "HIDDEN"
    },
    std::vector<std::string>{
      "DEFAULT_MODE"
    },
    std::vector<std::string>{
      "", "';'", "'del'", "'new'", "'set'", "'\\U0001D709'", "'@'", "'\\u2113'", 
      "'\\U0001D70B'", "'\\u03C6'", "','", "'label'", "'src'", "'dst'", 
      "'--'", "'->'", "'<-'", "'hook'", "'||'", "'if'", "'then'", "'else'", 
      "'as'", "'>>'", "'\\u21AA'", "'\\u2200'", "'\\u003F'", "'='", "'*'", 
      "'('", "')'", "':'", "'['", "']'"
    },
    std::vector<std::string>{
      "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", 
      "", "", "IF", "THEN", "ELSE", "AS", "VEC", "REWRITE_TO", "FORALL", 
      "QM", "EQ", "STAR", "LPAR", "RPAR", "COL", "QPAR", "PPAR", "OTHERS", 
      "SPACE", "COMMENT", "LINE_COMMENT"
    }
  );
  static const int32_t serializedATNSegment[] = {
  	4,0,37,208,6,-1,2,0,7,0,2,1,7,1,2,2,7,2,2,3,7,3,2,4,7,4,2,5,7,5,2,6,7,
  	6,2,7,7,7,2,8,7,8,2,9,7,9,2,10,7,10,2,11,7,11,2,12,7,12,2,13,7,13,2,14,
  	7,14,2,15,7,15,2,16,7,16,2,17,7,17,2,18,7,18,2,19,7,19,2,20,7,20,2,21,
  	7,21,2,22,7,22,2,23,7,23,2,24,7,24,2,25,7,25,2,26,7,26,2,27,7,27,2,28,
  	7,28,2,29,7,29,2,30,7,30,2,31,7,31,2,32,7,32,2,33,7,33,2,34,7,34,2,35,
  	7,35,2,36,7,36,1,0,1,0,1,1,1,1,1,1,1,1,1,2,1,2,1,2,1,2,1,3,1,3,1,3,1,
  	3,1,4,1,4,1,5,1,5,1,6,1,6,1,7,1,7,1,8,1,8,1,9,1,9,1,10,1,10,1,10,1,10,
  	1,10,1,10,1,11,1,11,1,11,1,11,1,12,1,12,1,12,1,12,1,13,1,13,1,13,1,14,
  	1,14,1,14,1,15,1,15,1,15,1,16,1,16,1,16,1,16,1,16,1,17,1,17,1,17,1,18,
  	1,18,1,18,1,19,1,19,1,19,1,19,1,19,1,20,1,20,1,20,1,20,1,20,1,21,1,21,
  	1,21,1,22,1,22,1,22,1,23,1,23,1,24,1,24,1,25,1,25,1,26,1,26,1,27,1,27,
  	1,28,1,28,1,29,1,29,1,30,1,30,1,31,1,31,1,32,1,32,1,33,4,33,173,8,33,
  	11,33,12,33,174,1,34,4,34,178,8,34,11,34,12,34,179,1,34,1,34,1,35,1,35,
  	1,35,1,35,5,35,188,8,35,10,35,12,35,191,9,35,1,35,1,35,1,35,1,35,1,35,
  	1,36,1,36,1,36,1,36,5,36,202,8,36,10,36,12,36,205,9,36,1,36,1,36,1,189,
  	0,37,1,1,3,2,5,3,7,4,9,5,11,6,13,7,15,8,17,9,19,10,21,11,23,12,25,13,
  	27,14,29,15,31,16,33,17,35,18,37,19,39,20,41,21,43,22,45,23,47,24,49,
  	25,51,26,53,27,55,28,57,29,59,30,61,31,63,32,65,33,67,34,69,35,71,36,
  	73,37,1,0,3,11,0,9,10,13,13,32,32,40,42,45,45,58,59,61,62,91,91,93,93,
  	124,124,8704,8704,3,0,9,10,13,13,32,32,2,0,10,10,13,13,211,0,1,1,0,0,
  	0,0,3,1,0,0,0,0,5,1,0,0,0,0,7,1,0,0,0,0,9,1,0,0,0,0,11,1,0,0,0,0,13,1,
  	0,0,0,0,15,1,0,0,0,0,17,1,0,0,0,0,19,1,0,0,0,0,21,1,0,0,0,0,23,1,0,0,
  	0,0,25,1,0,0,0,0,27,1,0,0,0,0,29,1,0,0,0,0,31,1,0,0,0,0,33,1,0,0,0,0,
  	35,1,0,0,0,0,37,1,0,0,0,0,39,1,0,0,0,0,41,1,0,0,0,0,43,1,0,0,0,0,45,1,
  	0,0,0,0,47,1,0,0,0,0,49,1,0,0,0,0,51,1,0,0,0,0,53,1,0,0,0,0,55,1,0,0,
  	0,0,57,1,0,0,0,0,59,1,0,0,0,0,61,1,0,0,0,0,63,1,0,0,0,0,65,1,0,0,0,0,
  	67,1,0,0,0,0,69,1,0,0,0,0,71,1,0,0,0,0,73,1,0,0,0,1,75,1,0,0,0,3,77,1,
  	0,0,0,5,81,1,0,0,0,7,85,1,0,0,0,9,89,1,0,0,0,11,91,1,0,0,0,13,93,1,0,
  	0,0,15,95,1,0,0,0,17,97,1,0,0,0,19,99,1,0,0,0,21,101,1,0,0,0,23,107,1,
  	0,0,0,25,111,1,0,0,0,27,115,1,0,0,0,29,118,1,0,0,0,31,121,1,0,0,0,33,
  	124,1,0,0,0,35,129,1,0,0,0,37,132,1,0,0,0,39,135,1,0,0,0,41,140,1,0,0,
  	0,43,145,1,0,0,0,45,148,1,0,0,0,47,151,1,0,0,0,49,153,1,0,0,0,51,155,
  	1,0,0,0,53,157,1,0,0,0,55,159,1,0,0,0,57,161,1,0,0,0,59,163,1,0,0,0,61,
  	165,1,0,0,0,63,167,1,0,0,0,65,169,1,0,0,0,67,172,1,0,0,0,69,177,1,0,0,
  	0,71,183,1,0,0,0,73,197,1,0,0,0,75,76,5,59,0,0,76,2,1,0,0,0,77,78,5,100,
  	0,0,78,79,5,101,0,0,79,80,5,108,0,0,80,4,1,0,0,0,81,82,5,110,0,0,82,83,
  	5,101,0,0,83,84,5,119,0,0,84,6,1,0,0,0,85,86,5,115,0,0,86,87,5,101,0,
  	0,87,88,5,116,0,0,88,8,1,0,0,0,89,90,5,120585,0,0,90,10,1,0,0,0,91,92,
  	5,64,0,0,92,12,1,0,0,0,93,94,5,8467,0,0,94,14,1,0,0,0,95,96,5,120587,
  	0,0,96,16,1,0,0,0,97,98,5,966,0,0,98,18,1,0,0,0,99,100,5,44,0,0,100,20,
  	1,0,0,0,101,102,5,108,0,0,102,103,5,97,0,0,103,104,5,98,0,0,104,105,5,
  	101,0,0,105,106,5,108,0,0,106,22,1,0,0,0,107,108,5,115,0,0,108,109,5,
  	114,0,0,109,110,5,99,0,0,110,24,1,0,0,0,111,112,5,100,0,0,112,113,5,115,
  	0,0,113,114,5,116,0,0,114,26,1,0,0,0,115,116,5,45,0,0,116,117,5,45,0,
  	0,117,28,1,0,0,0,118,119,5,45,0,0,119,120,5,62,0,0,120,30,1,0,0,0,121,
  	122,5,60,0,0,122,123,5,45,0,0,123,32,1,0,0,0,124,125,5,104,0,0,125,126,
  	5,111,0,0,126,127,5,111,0,0,127,128,5,107,0,0,128,34,1,0,0,0,129,130,
  	5,124,0,0,130,131,5,124,0,0,131,36,1,0,0,0,132,133,5,105,0,0,133,134,
  	5,102,0,0,134,38,1,0,0,0,135,136,5,116,0,0,136,137,5,104,0,0,137,138,
  	5,101,0,0,138,139,5,110,0,0,139,40,1,0,0,0,140,141,5,101,0,0,141,142,
  	5,108,0,0,142,143,5,115,0,0,143,144,5,101,0,0,144,42,1,0,0,0,145,146,
  	5,97,0,0,146,147,5,115,0,0,147,44,1,0,0,0,148,149,5,62,0,0,149,150,5,
  	62,0,0,150,46,1,0,0,0,151,152,5,8618,0,0,152,48,1,0,0,0,153,154,5,8704,
  	0,0,154,50,1,0,0,0,155,156,5,63,0,0,156,52,1,0,0,0,157,158,5,61,0,0,158,
  	54,1,0,0,0,159,160,5,42,0,0,160,56,1,0,0,0,161,162,5,40,0,0,162,58,1,
  	0,0,0,163,164,5,41,0,0,164,60,1,0,0,0,165,166,5,58,0,0,166,62,1,0,0,0,
  	167,168,5,91,0,0,168,64,1,0,0,0,169,170,5,93,0,0,170,66,1,0,0,0,171,173,
  	8,0,0,0,172,171,1,0,0,0,173,174,1,0,0,0,174,172,1,0,0,0,174,175,1,0,0,
  	0,175,68,1,0,0,0,176,178,7,1,0,0,177,176,1,0,0,0,178,179,1,0,0,0,179,
  	177,1,0,0,0,179,180,1,0,0,0,180,181,1,0,0,0,181,182,6,34,0,0,182,70,1,
  	0,0,0,183,184,5,47,0,0,184,185,5,42,0,0,185,189,1,0,0,0,186,188,9,0,0,
  	0,187,186,1,0,0,0,188,191,1,0,0,0,189,190,1,0,0,0,189,187,1,0,0,0,190,
  	192,1,0,0,0,191,189,1,0,0,0,192,193,5,42,0,0,193,194,5,47,0,0,194,195,
  	1,0,0,0,195,196,6,35,0,0,196,72,1,0,0,0,197,198,5,47,0,0,198,199,5,47,
  	0,0,199,203,1,0,0,0,200,202,8,2,0,0,201,200,1,0,0,0,202,205,1,0,0,0,203,
  	201,1,0,0,0,203,204,1,0,0,0,204,206,1,0,0,0,205,203,1,0,0,0,206,207,6,
  	36,0,0,207,74,1,0,0,0,5,0,174,179,189,203,1,6,0,0
  };
  staticData->serializedATN = antlr4::atn::SerializedATNView(serializedATNSegment, sizeof(serializedATNSegment) / sizeof(serializedATNSegment[0]));

  antlr4::atn::ATNDeserializer deserializer;
  staticData->atn = deserializer.deserialize(staticData->serializedATN);

  const size_t count = staticData->atn->getNumberOfDecisions();
  staticData->decisionToDFA.reserve(count);
  for (size_t i = 0; i < count; i++) { 
    staticData->decisionToDFA.emplace_back(staticData->atn->getDecisionState(i), i);
  }
  simple_graph_grammarlexerLexerStaticData = staticData.release();
}

}

simple_graph_grammarLexer::simple_graph_grammarLexer(CharStream *input) : Lexer(input) {
  simple_graph_grammarLexer::initialize();
  _interpreter = new atn::LexerATNSimulator(this, *simple_graph_grammarlexerLexerStaticData->atn, simple_graph_grammarlexerLexerStaticData->decisionToDFA, simple_graph_grammarlexerLexerStaticData->sharedContextCache);
}

simple_graph_grammarLexer::~simple_graph_grammarLexer() {
  delete _interpreter;
}

std::string simple_graph_grammarLexer::getGrammarFileName() const {
  return "simple_graph_grammar.g4";
}

const std::vector<std::string>& simple_graph_grammarLexer::getRuleNames() const {
  return simple_graph_grammarlexerLexerStaticData->ruleNames;
}

const std::vector<std::string>& simple_graph_grammarLexer::getChannelNames() const {
  return simple_graph_grammarlexerLexerStaticData->channelNames;
}

const std::vector<std::string>& simple_graph_grammarLexer::getModeNames() const {
  return simple_graph_grammarlexerLexerStaticData->modeNames;
}

const dfa::Vocabulary& simple_graph_grammarLexer::getVocabulary() const {
  return simple_graph_grammarlexerLexerStaticData->vocabulary;
}

antlr4::atn::SerializedATNView simple_graph_grammarLexer::getSerializedATN() const {
  return simple_graph_grammarlexerLexerStaticData->serializedATN;
}

const atn::ATN& simple_graph_grammarLexer::getATN() const {
  return *simple_graph_grammarlexerLexerStaticData->atn;
}




void simple_graph_grammarLexer::initialize() {
  ::antlr4::internal::call_once(simple_graph_grammarlexerLexerOnceFlag, simple_graph_grammarlexerLexerInitialize);
}