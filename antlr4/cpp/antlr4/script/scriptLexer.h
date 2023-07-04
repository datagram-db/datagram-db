
// Generated from antlr4/script/script.g4 by ANTLR 4.11.1

#pragma once


#include "antlr4-runtime.h"




class  scriptLexer : public antlr4::Lexer {
public:
  enum {
    T__0 = 1, T__1 = 2, T__2 = 3, T__3 = 4, T__4 = 5, T__5 = 6, T__6 = 7, 
    T__7 = 8, T__8 = 9, T__9 = 10, T__10 = 11, T__11 = 12, T__12 = 13, T__13 = 14, 
    T__14 = 15, T__15 = 16, T__16 = 17, T__17 = 18, T__18 = 19, T__19 = 20, 
    T__20 = 21, T__21 = 22, T__22 = 23, T__23 = 24, T__24 = 25, T__25 = 26, 
    T__26 = 27, T__27 = 28, T__28 = 29, T__29 = 30, T__30 = 31, T__31 = 32, 
    T__32 = 33, T__33 = 34, T__34 = 35, T__35 = 36, T__36 = 37, T__37 = 38, 
    T__38 = 39, T__39 = 40, T__40 = 41, TYPEOF = 42, SIGMA = 43, ASSERT = 44, 
    STRINGT = 45, DOUBLET = 46, BOOLT = 47, INTT = 48, START = 49, KIND = 50, 
    LISTT = 51, SUBTYPE = 52, ALPAREN = 53, ARPAREN = 54, LPAREN = 55, RPAREN = 56, 
    BOOL = 57, AND_TYPE = 58, OR_TYPE = 59, FUN = 60, NOT = 61, IF = 62, 
    THEN = 63, ELSE = 64, REMOVE = 65, FROM = 66, LOG = 67, POW = 68, SIN = 69, 
    COS = 70, TAN = 71, PHI = 72, ELL = 73, XI = 74, INJ = 75, OBJ = 76, 
    FLAT = 77, SELFX = 78, VARPHI = 79, CROSS = 80, SCRIPT = 81, GSQL = 82, 
    CREATEDB = 83, VARNAME = 84, CREATE = 85, ELECT = 86, MAP = 87, DISJOINT = 88, 
    FOLD = 89, EscapedString = 90, NUMBER = 91, INTEGER = 92, SPACE = 93, 
    COMMENT = 94, LINE_COMMENT = 95
  };

  explicit scriptLexer(antlr4::CharStream *input);

  ~scriptLexer() override;


  std::string getGrammarFileName() const override;

  const std::vector<std::string>& getRuleNames() const override;

  const std::vector<std::string>& getChannelNames() const override;

  const std::vector<std::string>& getModeNames() const override;

  const antlr4::dfa::Vocabulary& getVocabulary() const override;

  antlr4::atn::SerializedATNView getSerializedATN() const override;

  const antlr4::atn::ATN& getATN() const override;

  // By default the static state used to implement the lexer is lazily initialized during the first
  // call to the constructor. You can call this function if you wish to initialize the static state
  // ahead of time.
  static void initialize();

private:

  // Individual action functions triggered by action() above.

  // Individual semantic predicate functions triggered by sempred() above.

};

