#ifndef LEXER_HPP
#define LEXER_HPP

#include <algorithm>
#include <string>
#include <string_view>

#include "LexerToken.hpp"
#include "SourceLocation.hpp"

namespace Lexer {


class Lexer
{
  std::string data;
  unsigned checkpoint[3] = { 0, 0, 0 };
  unsigned pos = 0;
  unsigned x_pos = 1;
  unsigned y_pos = 1;


public:
  explicit Lexer(const std::string &data) : data(data) {}

  LexerToken nextNWToken()
  {
    auto t = nextToken();
    while (t.type == LexerTokenType::Space || t.type == LexerTokenType::Tab || t.type == LexerTokenType::Newline)
      t = doGetNextToken();
    return t;
  }

  LexerToken nextToken()
  {
    auto &&r = doGetNextToken();
    return r;
  }

  SourceLocation currentLocation() const { return SourceLocation(y_pos, x_pos); }

  static bool isInt(std::string_view data)
  {
    return std::all_of(data.begin(), data.end(), [](auto c) { return c >= '0' && c <= '9'; });
  }

private:
  void saveCheckpoint()
  {
    checkpoint[0] = pos;
    checkpoint[1] = x_pos;
    checkpoint[2] = y_pos;
  }

  void restoreCheckpoint()
  {
    pos = checkpoint[0];
    x_pos = checkpoint[1];
    y_pos = checkpoint[2];
  }

  char next_char()
  {
    if (pos >= data.size()) { return '\0'; }
    x_pos++;
    if (data[pos] == '\n') {
      y_pos++;
      x_pos = 1;
    }
    return data[pos++];
  }

  char peek_next_char()
  {
    if (pos >= data.size()) { return '\0'; }
    return data[pos];
  }

  LexerToken doGetNextToken()
  {
    const SourceLocation location = currentLocation();
    const auto startPos = pos;
    const char nchar = next_char();
    if (nchar == '\0') return { {}, location, LexerTokenType::Eof };
    if (nchar == '\n') return { {}, location, LexerTokenType::Newline };
    if (nchar == '\t') return { data.substr(startPos, 1), location, LexerTokenType::Tab };
    if (nchar == ' ') return { fetch_consecutive(startPos, ' '), location, LexerTokenType::Space };
    if (nchar == '(') return { data.substr(startPos, 1), location, LexerTokenType::ParenOpen };
    if (nchar == ')') return { data.substr(startPos, 1), location, LexerTokenType::ParenClose };
    if (nchar == '{') return { data.substr(startPos, 1), location, LexerTokenType::BraceOpen };
    if (nchar == '}') return { data.substr(startPos, 1), location, LexerTokenType::BraceClose };
    if (nchar == '+') return { data.substr(startPos, 1), location, LexerTokenType::PlusToken };
    if (nchar == '/') return { data.substr(startPos, 1), location, LexerTokenType::DivideToken };
    if (nchar == '*') return { data.substr(startPos, 1), location, LexerTokenType::MultiplyToken };
    if (nchar == '-') return { data.substr(startPos, 1), location, LexerTokenType::MinusToken };
    if (nchar == ';') return { data.substr(startPos, 1), location, LexerTokenType::SemicolonToken };
    if (nchar == '>') {
      saveCheckpoint();
      if (next_char() == '=') { return { data.substr(startPos, 2), location, LexerTokenType::GreaterEqualToken }; }
      restoreCheckpoint();
      return { data.substr(startPos, 1), location, LexerTokenType::GreaterThanToken };
    }
    if (nchar == '<') {
      saveCheckpoint();
      if (next_char() == '=') { return { data.substr(startPos, 2), location, LexerTokenType::LessEqualToken }; }
      restoreCheckpoint();
      return { data.substr(startPos, 1), location, LexerTokenType::LessThanToken };
    }
    if (nchar == '=') {
      saveCheckpoint();
      if (next_char() == '=') { return { data.substr(startPos, 2), location, LexerTokenType::EqualToken }; }
      restoreCheckpoint();
      return { data.substr(startPos, 1), location, LexerTokenType::AssignToken };
    }
    if (nchar == '!') {
      saveCheckpoint();
      if (next_char() == '=') { return { data.substr(startPos, 2), location, LexerTokenType::NotEqualToken }; }
      restoreCheckpoint();
      return { data.substr(startPos, 1), location, LexerTokenType::NotToken };
    }

    if (nchar == '#') {
      int i = 1;
      while (next_char() != '#') { i++; }
      return { data.substr(startPos, i + 1), location, LexerTokenType::StringToken };
    }


    if (!(isAlpha(nchar) || isNumeric(nchar))) {
      return { data.substr(startPos, 1), location, LexerTokenType::Unknown };
    }

    auto substr = next_valid_sequences(startPos);
    // a.fd, 343.352, 334, etc
    if (std::all_of(substr.begin(), substr.end(), Lexer::isNumeric)) {
      if (std::all_of(substr.begin(), substr.end(), Lexer::isInteger)) {
        return { substr, location, LexerTokenType::IntToken };
      } else {
        return { substr, location, LexerTokenType::FloatToken };
      }
    }
    if (substr == "print" || substr == "Print") {
      return { substr, location, LexerTokenType::PrintToken };
    } else if (substr == "if") {
      return { substr, location, LexerTokenType::IfToken };
    } else if (substr == "else") {
      return { substr, location, LexerTokenType::ElseToken };
    } else if (substr == "let") {
      return { substr, location, LexerTokenType::LetToken };
    }
    return { substr, location, LexerTokenType::VarToken };
  }


  std::string fetch_consecutive(int startPos, char ch)
  {
    auto count = 1;
    for (; peek_next_char() == ch; ++count, next_char())
      ;
    return data.substr(startPos, count);
  }

  std::string next_valid_sequences(int from)
  {
    int counts = 0;
    while (true) {
      char c = peek_next_char();
      counts++;
      if (!(isAlpha(c) || isNumeric(c))) break;
      next_char();
    }
    return data.substr(from, counts);
  }

  static bool isNumeric(char c) { return (c >= '0' && c <= '9') || c == '.'; }

  static bool isInteger(char c) { return (c >= '0' && c <= '9'); }

  static bool isAlpha(char c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '.'; }
};

};
#endif// LEXER_HPP
