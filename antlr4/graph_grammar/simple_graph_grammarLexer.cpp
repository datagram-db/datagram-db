
// Generated from antlr4/graph_grammar/simple_graph_grammar.g4 by ANTLR 4.11.1


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
      "T__17", "T__18", "T__19", "T__20", "T__21", "T__22", "T__23", "T__24", 
      "T__25", "T__26", "T__27", "WHERE", "IF", "THEN", "ELSE", "AS", "VEC", 
      "REWRITE_TO", "FORALL", "QM", "EQ", "STAR", "LPAR", "RPAR", "COL", 
      "QPAR", "PPAR", "EscapedString", "OTHERS", "SPACE", "COMMENT", "LINE_COMMENT"
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
      "'SCRIPT'", "'\\u2260'", "'<'", "'\\u2264'", "'\\u2228'", "'\\u2227'", 
      "'TEST'", "'unmatched'", "'.'", "'matched'", "'--'", "'->'", "'<-'", 
      "'hook'", "'||'", "'where'", "'if'", "'then'", "'else'", "'as'", "'>>'", 
      "'\\u21AA'", "'\\u2200'", "'\\u003F'", "'='", "'*'", "'('", "')'", 
      "':'", "'['", "']'"
    },
    std::vector<std::string>{
      "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", 
      "", "", "", "", "", "", "", "", "", "", "", "", "WHERE", "IF", "THEN", 
      "ELSE", "AS", "VEC", "REWRITE_TO", "FORALL", "QM", "EQ", "STAR", "LPAR", 
      "RPAR", "COL", "QPAR", "PPAR", "EscapedString", "OTHERS", "SPACE", 
      "COMMENT", "LINE_COMMENT"
    }
  );
  static const int32_t serializedATNSegment[] = {
  	4,0,49,291,6,-1,2,0,7,0,2,1,7,1,2,2,7,2,2,3,7,3,2,4,7,4,2,5,7,5,2,6,7,
  	6,2,7,7,7,2,8,7,8,2,9,7,9,2,10,7,10,2,11,7,11,2,12,7,12,2,13,7,13,2,14,
  	7,14,2,15,7,15,2,16,7,16,2,17,7,17,2,18,7,18,2,19,7,19,2,20,7,20,2,21,
  	7,21,2,22,7,22,2,23,7,23,2,24,7,24,2,25,7,25,2,26,7,26,2,27,7,27,2,28,
  	7,28,2,29,7,29,2,30,7,30,2,31,7,31,2,32,7,32,2,33,7,33,2,34,7,34,2,35,
  	7,35,2,36,7,36,2,37,7,37,2,38,7,38,2,39,7,39,2,40,7,40,2,41,7,41,2,42,
  	7,42,2,43,7,43,2,44,7,44,2,45,7,45,2,46,7,46,2,47,7,47,2,48,7,48,1,0,
  	1,0,1,1,1,1,1,1,1,1,1,2,1,2,1,2,1,2,1,3,1,3,1,3,1,3,1,4,1,4,1,5,1,5,1,
  	6,1,6,1,7,1,7,1,8,1,8,1,9,1,9,1,10,1,10,1,10,1,10,1,10,1,10,1,11,1,11,
  	1,11,1,11,1,12,1,12,1,12,1,12,1,13,1,13,1,13,1,13,1,13,1,13,1,13,1,14,
  	1,14,1,15,1,15,1,16,1,16,1,17,1,17,1,18,1,18,1,19,1,19,1,19,1,19,1,19,
  	1,20,1,20,1,20,1,20,1,20,1,20,1,20,1,20,1,20,1,20,1,21,1,21,1,22,1,22,
  	1,22,1,22,1,22,1,22,1,22,1,22,1,23,1,23,1,23,1,24,1,24,1,24,1,25,1,25,
  	1,25,1,26,1,26,1,26,1,26,1,26,1,27,1,27,1,27,1,28,1,28,1,28,1,28,1,28,
  	1,28,1,29,1,29,1,29,1,30,1,30,1,30,1,30,1,30,1,31,1,31,1,31,1,31,1,31,
  	1,32,1,32,1,32,1,33,1,33,1,33,1,34,1,34,1,35,1,35,1,36,1,36,1,37,1,37,
  	1,38,1,38,1,39,1,39,1,40,1,40,1,41,1,41,1,42,1,42,1,43,1,43,1,44,1,44,
  	1,44,1,44,5,44,248,8,44,10,44,12,44,251,9,44,1,44,1,44,1,45,4,45,256,
  	8,45,11,45,12,45,257,1,46,4,46,261,8,46,11,46,12,46,262,1,46,1,46,1,47,
  	1,47,1,47,1,47,5,47,271,8,47,10,47,12,47,274,9,47,1,47,1,47,1,47,1,47,
  	1,47,1,48,1,48,1,48,1,48,5,48,285,8,48,10,48,12,48,288,9,48,1,48,1,48,
  	1,272,0,49,1,1,3,2,5,3,7,4,9,5,11,6,13,7,15,8,17,9,19,10,21,11,23,12,
  	25,13,27,14,29,15,31,16,33,17,35,18,37,19,39,20,41,21,43,22,45,23,47,
  	24,49,25,51,26,53,27,55,28,57,29,59,30,61,31,63,32,65,33,67,34,69,35,
  	71,36,73,37,75,38,77,39,79,40,81,41,83,42,85,43,87,44,89,45,91,46,93,
  	47,95,48,97,49,1,0,4,2,0,34,34,92,92,11,0,9,10,13,13,32,32,40,42,45,45,
  	58,59,61,62,91,91,93,93,124,124,8704,8704,3,0,9,10,13,13,32,32,2,0,10,
  	10,13,13,296,0,1,1,0,0,0,0,3,1,0,0,0,0,5,1,0,0,0,0,7,1,0,0,0,0,9,1,0,
  	0,0,0,11,1,0,0,0,0,13,1,0,0,0,0,15,1,0,0,0,0,17,1,0,0,0,0,19,1,0,0,0,
  	0,21,1,0,0,0,0,23,1,0,0,0,0,25,1,0,0,0,0,27,1,0,0,0,0,29,1,0,0,0,0,31,
  	1,0,0,0,0,33,1,0,0,0,0,35,1,0,0,0,0,37,1,0,0,0,0,39,1,0,0,0,0,41,1,0,
  	0,0,0,43,1,0,0,0,0,45,1,0,0,0,0,47,1,0,0,0,0,49,1,0,0,0,0,51,1,0,0,0,
  	0,53,1,0,0,0,0,55,1,0,0,0,0,57,1,0,0,0,0,59,1,0,0,0,0,61,1,0,0,0,0,63,
  	1,0,0,0,0,65,1,0,0,0,0,67,1,0,0,0,0,69,1,0,0,0,0,71,1,0,0,0,0,73,1,0,
  	0,0,0,75,1,0,0,0,0,77,1,0,0,0,0,79,1,0,0,0,0,81,1,0,0,0,0,83,1,0,0,0,
  	0,85,1,0,0,0,0,87,1,0,0,0,0,89,1,0,0,0,0,91,1,0,0,0,0,93,1,0,0,0,0,95,
  	1,0,0,0,0,97,1,0,0,0,1,99,1,0,0,0,3,101,1,0,0,0,5,105,1,0,0,0,7,109,1,
  	0,0,0,9,113,1,0,0,0,11,115,1,0,0,0,13,117,1,0,0,0,15,119,1,0,0,0,17,121,
  	1,0,0,0,19,123,1,0,0,0,21,125,1,0,0,0,23,131,1,0,0,0,25,135,1,0,0,0,27,
  	139,1,0,0,0,29,146,1,0,0,0,31,148,1,0,0,0,33,150,1,0,0,0,35,152,1,0,0,
  	0,37,154,1,0,0,0,39,156,1,0,0,0,41,161,1,0,0,0,43,171,1,0,0,0,45,173,
  	1,0,0,0,47,181,1,0,0,0,49,184,1,0,0,0,51,187,1,0,0,0,53,190,1,0,0,0,55,
  	195,1,0,0,0,57,198,1,0,0,0,59,204,1,0,0,0,61,207,1,0,0,0,63,212,1,0,0,
  	0,65,217,1,0,0,0,67,220,1,0,0,0,69,223,1,0,0,0,71,225,1,0,0,0,73,227,
  	1,0,0,0,75,229,1,0,0,0,77,231,1,0,0,0,79,233,1,0,0,0,81,235,1,0,0,0,83,
  	237,1,0,0,0,85,239,1,0,0,0,87,241,1,0,0,0,89,243,1,0,0,0,91,255,1,0,0,
  	0,93,260,1,0,0,0,95,266,1,0,0,0,97,280,1,0,0,0,99,100,5,59,0,0,100,2,
  	1,0,0,0,101,102,5,100,0,0,102,103,5,101,0,0,103,104,5,108,0,0,104,4,1,
  	0,0,0,105,106,5,110,0,0,106,107,5,101,0,0,107,108,5,119,0,0,108,6,1,0,
  	0,0,109,110,5,115,0,0,110,111,5,101,0,0,111,112,5,116,0,0,112,8,1,0,0,
  	0,113,114,5,120585,0,0,114,10,1,0,0,0,115,116,5,64,0,0,116,12,1,0,0,0,
  	117,118,5,8467,0,0,118,14,1,0,0,0,119,120,5,120587,0,0,120,16,1,0,0,0,
  	121,122,5,966,0,0,122,18,1,0,0,0,123,124,5,44,0,0,124,20,1,0,0,0,125,
  	126,5,108,0,0,126,127,5,97,0,0,127,128,5,98,0,0,128,129,5,101,0,0,129,
  	130,5,108,0,0,130,22,1,0,0,0,131,132,5,115,0,0,132,133,5,114,0,0,133,
  	134,5,99,0,0,134,24,1,0,0,0,135,136,5,100,0,0,136,137,5,115,0,0,137,138,
  	5,116,0,0,138,26,1,0,0,0,139,140,5,83,0,0,140,141,5,67,0,0,141,142,5,
  	82,0,0,142,143,5,73,0,0,143,144,5,80,0,0,144,145,5,84,0,0,145,28,1,0,
  	0,0,146,147,5,8800,0,0,147,30,1,0,0,0,148,149,5,60,0,0,149,32,1,0,0,0,
  	150,151,5,8804,0,0,151,34,1,0,0,0,152,153,5,8744,0,0,153,36,1,0,0,0,154,
  	155,5,8743,0,0,155,38,1,0,0,0,156,157,5,84,0,0,157,158,5,69,0,0,158,159,
  	5,83,0,0,159,160,5,84,0,0,160,40,1,0,0,0,161,162,5,117,0,0,162,163,5,
  	110,0,0,163,164,5,109,0,0,164,165,5,97,0,0,165,166,5,116,0,0,166,167,
  	5,99,0,0,167,168,5,104,0,0,168,169,5,101,0,0,169,170,5,100,0,0,170,42,
  	1,0,0,0,171,172,5,46,0,0,172,44,1,0,0,0,173,174,5,109,0,0,174,175,5,97,
  	0,0,175,176,5,116,0,0,176,177,5,99,0,0,177,178,5,104,0,0,178,179,5,101,
  	0,0,179,180,5,100,0,0,180,46,1,0,0,0,181,182,5,45,0,0,182,183,5,45,0,
  	0,183,48,1,0,0,0,184,185,5,45,0,0,185,186,5,62,0,0,186,50,1,0,0,0,187,
  	188,5,60,0,0,188,189,5,45,0,0,189,52,1,0,0,0,190,191,5,104,0,0,191,192,
  	5,111,0,0,192,193,5,111,0,0,193,194,5,107,0,0,194,54,1,0,0,0,195,196,
  	5,124,0,0,196,197,5,124,0,0,197,56,1,0,0,0,198,199,5,119,0,0,199,200,
  	5,104,0,0,200,201,5,101,0,0,201,202,5,114,0,0,202,203,5,101,0,0,203,58,
  	1,0,0,0,204,205,5,105,0,0,205,206,5,102,0,0,206,60,1,0,0,0,207,208,5,
  	116,0,0,208,209,5,104,0,0,209,210,5,101,0,0,210,211,5,110,0,0,211,62,
  	1,0,0,0,212,213,5,101,0,0,213,214,5,108,0,0,214,215,5,115,0,0,215,216,
  	5,101,0,0,216,64,1,0,0,0,217,218,5,97,0,0,218,219,5,115,0,0,219,66,1,
  	0,0,0,220,221,5,62,0,0,221,222,5,62,0,0,222,68,1,0,0,0,223,224,5,8618,
  	0,0,224,70,1,0,0,0,225,226,5,8704,0,0,226,72,1,0,0,0,227,228,5,63,0,0,
  	228,74,1,0,0,0,229,230,5,61,0,0,230,76,1,0,0,0,231,232,5,42,0,0,232,78,
  	1,0,0,0,233,234,5,40,0,0,234,80,1,0,0,0,235,236,5,41,0,0,236,82,1,0,0,
  	0,237,238,5,58,0,0,238,84,1,0,0,0,239,240,5,91,0,0,240,86,1,0,0,0,241,
  	242,5,93,0,0,242,88,1,0,0,0,243,249,5,34,0,0,244,248,8,0,0,0,245,246,
  	5,92,0,0,246,248,7,0,0,0,247,244,1,0,0,0,247,245,1,0,0,0,248,251,1,0,
  	0,0,249,247,1,0,0,0,249,250,1,0,0,0,250,252,1,0,0,0,251,249,1,0,0,0,252,
  	253,5,34,0,0,253,90,1,0,0,0,254,256,8,1,0,0,255,254,1,0,0,0,256,257,1,
  	0,0,0,257,255,1,0,0,0,257,258,1,0,0,0,258,92,1,0,0,0,259,261,7,2,0,0,
  	260,259,1,0,0,0,261,262,1,0,0,0,262,260,1,0,0,0,262,263,1,0,0,0,263,264,
  	1,0,0,0,264,265,6,46,0,0,265,94,1,0,0,0,266,267,5,47,0,0,267,268,5,42,
  	0,0,268,272,1,0,0,0,269,271,9,0,0,0,270,269,1,0,0,0,271,274,1,0,0,0,272,
  	273,1,0,0,0,272,270,1,0,0,0,273,275,1,0,0,0,274,272,1,0,0,0,275,276,5,
  	42,0,0,276,277,5,47,0,0,277,278,1,0,0,0,278,279,6,47,0,0,279,96,1,0,0,
  	0,280,281,5,47,0,0,281,282,5,47,0,0,282,286,1,0,0,0,283,285,8,3,0,0,284,
  	283,1,0,0,0,285,288,1,0,0,0,286,284,1,0,0,0,286,287,1,0,0,0,287,289,1,
  	0,0,0,288,286,1,0,0,0,289,290,6,48,0,0,290,98,1,0,0,0,7,0,247,249,257,
  	262,272,286,1,6,0,0
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
