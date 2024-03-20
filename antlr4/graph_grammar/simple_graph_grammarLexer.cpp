
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
      "T__25", "T__26", "T__27", "T__28", "WHERE", "IF", "THEN", "ELSE", 
      "AS", "VEC", "REWRITE_TO", "FORALL", "QM", "EQ", "STAR", "LPAR", "RPAR", 
      "COL", "QPAR", "PPAR", "EscapedString", "OTHERS", "SPACE", "COMMENT", 
      "LINE_COMMENT"
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
      "'TEST'", "'unmatched'", "'.'", "'matched'", "'FILL'", "'--'", "'->'", 
      "'<-'", "'hook'", "'||'", "'where'", "'if'", "'then'", "'else'", "'as'", 
      "'>>'", "'\\u21AA'", "'\\u2200'", "'\\u003F'", "'='", "'*'", "'('", 
      "')'", "':'", "'['", "']'"
    },
    std::vector<std::string>{
      "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", 
      "", "", "", "", "", "", "", "", "", "", "", "", "", "WHERE", "IF", 
      "THEN", "ELSE", "AS", "VEC", "REWRITE_TO", "FORALL", "QM", "EQ", "STAR", 
      "LPAR", "RPAR", "COL", "QPAR", "PPAR", "EscapedString", "OTHERS", 
      "SPACE", "COMMENT", "LINE_COMMENT"
    }
  );
  static const int32_t serializedATNSegment[] = {
  	4,0,50,298,6,-1,2,0,7,0,2,1,7,1,2,2,7,2,2,3,7,3,2,4,7,4,2,5,7,5,2,6,7,
  	6,2,7,7,7,2,8,7,8,2,9,7,9,2,10,7,10,2,11,7,11,2,12,7,12,2,13,7,13,2,14,
  	7,14,2,15,7,15,2,16,7,16,2,17,7,17,2,18,7,18,2,19,7,19,2,20,7,20,2,21,
  	7,21,2,22,7,22,2,23,7,23,2,24,7,24,2,25,7,25,2,26,7,26,2,27,7,27,2,28,
  	7,28,2,29,7,29,2,30,7,30,2,31,7,31,2,32,7,32,2,33,7,33,2,34,7,34,2,35,
  	7,35,2,36,7,36,2,37,7,37,2,38,7,38,2,39,7,39,2,40,7,40,2,41,7,41,2,42,
  	7,42,2,43,7,43,2,44,7,44,2,45,7,45,2,46,7,46,2,47,7,47,2,48,7,48,2,49,
  	7,49,1,0,1,0,1,1,1,1,1,1,1,1,1,2,1,2,1,2,1,2,1,3,1,3,1,3,1,3,1,4,1,4,
  	1,5,1,5,1,6,1,6,1,7,1,7,1,8,1,8,1,9,1,9,1,10,1,10,1,10,1,10,1,10,1,10,
  	1,11,1,11,1,11,1,11,1,12,1,12,1,12,1,12,1,13,1,13,1,13,1,13,1,13,1,13,
  	1,13,1,14,1,14,1,15,1,15,1,16,1,16,1,17,1,17,1,18,1,18,1,19,1,19,1,19,
  	1,19,1,19,1,20,1,20,1,20,1,20,1,20,1,20,1,20,1,20,1,20,1,20,1,21,1,21,
  	1,22,1,22,1,22,1,22,1,22,1,22,1,22,1,22,1,23,1,23,1,23,1,23,1,23,1,24,
  	1,24,1,24,1,25,1,25,1,25,1,26,1,26,1,26,1,27,1,27,1,27,1,27,1,27,1,28,
  	1,28,1,28,1,29,1,29,1,29,1,29,1,29,1,29,1,30,1,30,1,30,1,31,1,31,1,31,
  	1,31,1,31,1,32,1,32,1,32,1,32,1,32,1,33,1,33,1,33,1,34,1,34,1,34,1,35,
  	1,35,1,36,1,36,1,37,1,37,1,38,1,38,1,39,1,39,1,40,1,40,1,41,1,41,1,42,
  	1,42,1,43,1,43,1,44,1,44,1,45,1,45,1,45,1,45,5,45,255,8,45,10,45,12,45,
  	258,9,45,1,45,1,45,1,46,4,46,263,8,46,11,46,12,46,264,1,47,4,47,268,8,
  	47,11,47,12,47,269,1,47,1,47,1,48,1,48,1,48,1,48,5,48,278,8,48,10,48,
  	12,48,281,9,48,1,48,1,48,1,48,1,48,1,48,1,49,1,49,1,49,1,49,5,49,292,
  	8,49,10,49,12,49,295,9,49,1,49,1,49,1,279,0,50,1,1,3,2,5,3,7,4,9,5,11,
  	6,13,7,15,8,17,9,19,10,21,11,23,12,25,13,27,14,29,15,31,16,33,17,35,18,
  	37,19,39,20,41,21,43,22,45,23,47,24,49,25,51,26,53,27,55,28,57,29,59,
  	30,61,31,63,32,65,33,67,34,69,35,71,36,73,37,75,38,77,39,79,40,81,41,
  	83,42,85,43,87,44,89,45,91,46,93,47,95,48,97,49,99,50,1,0,4,2,0,34,34,
  	92,92,11,0,9,10,13,13,32,32,40,42,45,45,58,59,61,62,91,91,93,93,124,124,
  	8704,8704,3,0,9,10,13,13,32,32,2,0,10,10,13,13,303,0,1,1,0,0,0,0,3,1,
  	0,0,0,0,5,1,0,0,0,0,7,1,0,0,0,0,9,1,0,0,0,0,11,1,0,0,0,0,13,1,0,0,0,0,
  	15,1,0,0,0,0,17,1,0,0,0,0,19,1,0,0,0,0,21,1,0,0,0,0,23,1,0,0,0,0,25,1,
  	0,0,0,0,27,1,0,0,0,0,29,1,0,0,0,0,31,1,0,0,0,0,33,1,0,0,0,0,35,1,0,0,
  	0,0,37,1,0,0,0,0,39,1,0,0,0,0,41,1,0,0,0,0,43,1,0,0,0,0,45,1,0,0,0,0,
  	47,1,0,0,0,0,49,1,0,0,0,0,51,1,0,0,0,0,53,1,0,0,0,0,55,1,0,0,0,0,57,1,
  	0,0,0,0,59,1,0,0,0,0,61,1,0,0,0,0,63,1,0,0,0,0,65,1,0,0,0,0,67,1,0,0,
  	0,0,69,1,0,0,0,0,71,1,0,0,0,0,73,1,0,0,0,0,75,1,0,0,0,0,77,1,0,0,0,0,
  	79,1,0,0,0,0,81,1,0,0,0,0,83,1,0,0,0,0,85,1,0,0,0,0,87,1,0,0,0,0,89,1,
  	0,0,0,0,91,1,0,0,0,0,93,1,0,0,0,0,95,1,0,0,0,0,97,1,0,0,0,0,99,1,0,0,
  	0,1,101,1,0,0,0,3,103,1,0,0,0,5,107,1,0,0,0,7,111,1,0,0,0,9,115,1,0,0,
  	0,11,117,1,0,0,0,13,119,1,0,0,0,15,121,1,0,0,0,17,123,1,0,0,0,19,125,
  	1,0,0,0,21,127,1,0,0,0,23,133,1,0,0,0,25,137,1,0,0,0,27,141,1,0,0,0,29,
  	148,1,0,0,0,31,150,1,0,0,0,33,152,1,0,0,0,35,154,1,0,0,0,37,156,1,0,0,
  	0,39,158,1,0,0,0,41,163,1,0,0,0,43,173,1,0,0,0,45,175,1,0,0,0,47,183,
  	1,0,0,0,49,188,1,0,0,0,51,191,1,0,0,0,53,194,1,0,0,0,55,197,1,0,0,0,57,
  	202,1,0,0,0,59,205,1,0,0,0,61,211,1,0,0,0,63,214,1,0,0,0,65,219,1,0,0,
  	0,67,224,1,0,0,0,69,227,1,0,0,0,71,230,1,0,0,0,73,232,1,0,0,0,75,234,
  	1,0,0,0,77,236,1,0,0,0,79,238,1,0,0,0,81,240,1,0,0,0,83,242,1,0,0,0,85,
  	244,1,0,0,0,87,246,1,0,0,0,89,248,1,0,0,0,91,250,1,0,0,0,93,262,1,0,0,
  	0,95,267,1,0,0,0,97,273,1,0,0,0,99,287,1,0,0,0,101,102,5,59,0,0,102,2,
  	1,0,0,0,103,104,5,100,0,0,104,105,5,101,0,0,105,106,5,108,0,0,106,4,1,
  	0,0,0,107,108,5,110,0,0,108,109,5,101,0,0,109,110,5,119,0,0,110,6,1,0,
  	0,0,111,112,5,115,0,0,112,113,5,101,0,0,113,114,5,116,0,0,114,8,1,0,0,
  	0,115,116,5,120585,0,0,116,10,1,0,0,0,117,118,5,64,0,0,118,12,1,0,0,0,
  	119,120,5,8467,0,0,120,14,1,0,0,0,121,122,5,120587,0,0,122,16,1,0,0,0,
  	123,124,5,966,0,0,124,18,1,0,0,0,125,126,5,44,0,0,126,20,1,0,0,0,127,
  	128,5,108,0,0,128,129,5,97,0,0,129,130,5,98,0,0,130,131,5,101,0,0,131,
  	132,5,108,0,0,132,22,1,0,0,0,133,134,5,115,0,0,134,135,5,114,0,0,135,
  	136,5,99,0,0,136,24,1,0,0,0,137,138,5,100,0,0,138,139,5,115,0,0,139,140,
  	5,116,0,0,140,26,1,0,0,0,141,142,5,83,0,0,142,143,5,67,0,0,143,144,5,
  	82,0,0,144,145,5,73,0,0,145,146,5,80,0,0,146,147,5,84,0,0,147,28,1,0,
  	0,0,148,149,5,8800,0,0,149,30,1,0,0,0,150,151,5,60,0,0,151,32,1,0,0,0,
  	152,153,5,8804,0,0,153,34,1,0,0,0,154,155,5,8744,0,0,155,36,1,0,0,0,156,
  	157,5,8743,0,0,157,38,1,0,0,0,158,159,5,84,0,0,159,160,5,69,0,0,160,161,
  	5,83,0,0,161,162,5,84,0,0,162,40,1,0,0,0,163,164,5,117,0,0,164,165,5,
  	110,0,0,165,166,5,109,0,0,166,167,5,97,0,0,167,168,5,116,0,0,168,169,
  	5,99,0,0,169,170,5,104,0,0,170,171,5,101,0,0,171,172,5,100,0,0,172,42,
  	1,0,0,0,173,174,5,46,0,0,174,44,1,0,0,0,175,176,5,109,0,0,176,177,5,97,
  	0,0,177,178,5,116,0,0,178,179,5,99,0,0,179,180,5,104,0,0,180,181,5,101,
  	0,0,181,182,5,100,0,0,182,46,1,0,0,0,183,184,5,70,0,0,184,185,5,73,0,
  	0,185,186,5,76,0,0,186,187,5,76,0,0,187,48,1,0,0,0,188,189,5,45,0,0,189,
  	190,5,45,0,0,190,50,1,0,0,0,191,192,5,45,0,0,192,193,5,62,0,0,193,52,
  	1,0,0,0,194,195,5,60,0,0,195,196,5,45,0,0,196,54,1,0,0,0,197,198,5,104,
  	0,0,198,199,5,111,0,0,199,200,5,111,0,0,200,201,5,107,0,0,201,56,1,0,
  	0,0,202,203,5,124,0,0,203,204,5,124,0,0,204,58,1,0,0,0,205,206,5,119,
  	0,0,206,207,5,104,0,0,207,208,5,101,0,0,208,209,5,114,0,0,209,210,5,101,
  	0,0,210,60,1,0,0,0,211,212,5,105,0,0,212,213,5,102,0,0,213,62,1,0,0,0,
  	214,215,5,116,0,0,215,216,5,104,0,0,216,217,5,101,0,0,217,218,5,110,0,
  	0,218,64,1,0,0,0,219,220,5,101,0,0,220,221,5,108,0,0,221,222,5,115,0,
  	0,222,223,5,101,0,0,223,66,1,0,0,0,224,225,5,97,0,0,225,226,5,115,0,0,
  	226,68,1,0,0,0,227,228,5,62,0,0,228,229,5,62,0,0,229,70,1,0,0,0,230,231,
  	5,8618,0,0,231,72,1,0,0,0,232,233,5,8704,0,0,233,74,1,0,0,0,234,235,5,
  	63,0,0,235,76,1,0,0,0,236,237,5,61,0,0,237,78,1,0,0,0,238,239,5,42,0,
  	0,239,80,1,0,0,0,240,241,5,40,0,0,241,82,1,0,0,0,242,243,5,41,0,0,243,
  	84,1,0,0,0,244,245,5,58,0,0,245,86,1,0,0,0,246,247,5,91,0,0,247,88,1,
  	0,0,0,248,249,5,93,0,0,249,90,1,0,0,0,250,256,5,34,0,0,251,255,8,0,0,
  	0,252,253,5,92,0,0,253,255,7,0,0,0,254,251,1,0,0,0,254,252,1,0,0,0,255,
  	258,1,0,0,0,256,254,1,0,0,0,256,257,1,0,0,0,257,259,1,0,0,0,258,256,1,
  	0,0,0,259,260,5,34,0,0,260,92,1,0,0,0,261,263,8,1,0,0,262,261,1,0,0,0,
  	263,264,1,0,0,0,264,262,1,0,0,0,264,265,1,0,0,0,265,94,1,0,0,0,266,268,
  	7,2,0,0,267,266,1,0,0,0,268,269,1,0,0,0,269,267,1,0,0,0,269,270,1,0,0,
  	0,270,271,1,0,0,0,271,272,6,47,0,0,272,96,1,0,0,0,273,274,5,47,0,0,274,
  	275,5,42,0,0,275,279,1,0,0,0,276,278,9,0,0,0,277,276,1,0,0,0,278,281,
  	1,0,0,0,279,280,1,0,0,0,279,277,1,0,0,0,280,282,1,0,0,0,281,279,1,0,0,
  	0,282,283,5,42,0,0,283,284,5,47,0,0,284,285,1,0,0,0,285,286,6,48,0,0,
  	286,98,1,0,0,0,287,288,5,47,0,0,288,289,5,47,0,0,289,293,1,0,0,0,290,
  	292,8,3,0,0,291,290,1,0,0,0,292,295,1,0,0,0,293,291,1,0,0,0,293,294,1,
  	0,0,0,294,296,1,0,0,0,295,293,1,0,0,0,296,297,6,49,0,0,297,100,1,0,0,
  	0,7,0,254,256,264,269,279,293,1,6,0,0
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
