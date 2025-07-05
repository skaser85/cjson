#include <stdio.h>

#define NOB_IMPLEMENTATION
#include "./nob.h"

typedef enum {
  TK_NONE,

  TK_OPEN_CURLY_BRACE,
  TK_CLOSE_CURLY_BRACE,
  TK_DOUBLE_QUOTE,
  TK_COLON,
  TK_COMMA,
  TK_STRING,

  TK_COUNT
} TOKEN_KIND;

const char *GetKind(TOKEN_KIND kind) {
  switch (kind) {
    case TK_NONE: return "NONE";
    case TK_OPEN_CURLY_BRACE: return "{";
    case TK_CLOSE_CURLY_BRACE: return "}";
    case TK_DOUBLE_QUOTE: return "\"";
    case TK_COLON: return ":";
    case TK_COMMA: return ",";
    case TK_STRING: return "STRING";
    default: return "";
  }
}

typedef struct {
  TOKEN_KIND kind;
  char *text;
} Token;

typedef struct {
  Token *items;
  size_t capacity;
  size_t count;
} Tokens;

bool is_alpha(char c) {
  return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'));
}

bool is_integer(char c) {
  return c >= '0' && c <='9';
}

bool is_alpha_numeric(char c) {
  return is_alpha(c) || is_integer(c);
}

bool is_valid_char_for_text(char c) {
  return (c != '"') && (c >= 32 && c <= 126);
}

Token GetToken(Nob_String_Builder sb, size_t *At) {
  Token t = {0};
  t.kind = TK_NONE;
  while (*At < sb.count) {
    char c = sb.items[*At];
    if (is_alpha_numeric(c)) {
      char *buffer = malloc(sizeof(char)*2048);
      size_t idx = 0;
      while (*At < sb.count && is_valid_char_for_text(c)) {
        buffer[idx] = c;
        idx += 1;
        *At += 1;
        c = sb.items[*At];
      }
      if (c == '"')
        *At -= 1;
      t.kind = TK_STRING;
      t.text = strdup(buffer);
    } else {
      switch (c) {
        case '{': t.kind = TK_OPEN_CURLY_BRACE; break;
        case '}': t.kind = TK_CLOSE_CURLY_BRACE; break;
        case '"': t.kind = TK_DOUBLE_QUOTE; break;
        case ':': t.kind = TK_COLON; break;
        case ',': t.kind = TK_COMMA; break;
        // if newline, just skip over the character
        case '\r':
        case '\n': break;
      }
    }
    
    *At += 1;
    if (t.kind != TK_NONE)
      break;
  }
  return t;
}

Tokens Tokenize(Nob_String_Builder sb) {
  Tokens tokens = {0};
  size_t At = 0;
  Token t = GetToken(sb, &At);
  while (t.kind != TK_NONE) {
    nob_da_append(&tokens, t);
    t = GetToken(sb, &At);
  }

  return tokens;
}

int main() {

  const char *filePath = "./nasa.json";

  Nob_String_Builder sb = {0};
  if (!nob_read_entire_file(filePath, &sb)) return 1;

  Tokens tokens = Tokenize(sb);

  for (size_t i = 0; i < tokens.count; ++i) {
    Token t = tokens.items[i];
    nob_log(NOB_INFO, "KIND: %s", GetKind(t.kind));
    if (t.text && strlen(t.text) > 0) {
      nob_log(NOB_INFO, "%s", t.text);
    }
    nob_log(NOB_INFO, "-------------------------");
  }

  return 0;
}
