#include <stdio.h>
#include <stdlib.h>

#define NOB_IMPLEMENTATION
#include "./nob.h"

typedef enum {
  TK_NONE,

  TK_OPEN_CURLY_BRACE,
  TK_CLOSE_CURLY_BRACE,
  TK_OPEN_SQ_BRACKET,
  TK_CLOSE_SQ_BRACKET,
  TK_COLON,
  TK_COMMA,
  
  TK_STRING,
  TK_FLOAT,

  TK_COUNT
} Token_Kind;

const char *GetKind(Token_Kind kind) {
  switch (kind) {
    case TK_NONE: return "NONE";
    case TK_OPEN_CURLY_BRACE: return "{";
    case TK_CLOSE_CURLY_BRACE: return "}";
    case TK_OPEN_SQ_BRACKET: return "[";
    case TK_CLOSE_SQ_BRACKET: return "]";
    case TK_COLON: return ":";
    case TK_COMMA: return ",";
    case TK_STRING: return "STRING";
    case TK_FLOAT: return "FLOAT";
    default: return "";
  }
}

typedef struct {
  Token_Kind kind;
  char *text;
  float num;
} Token;

typedef struct {
  Token *items;
  size_t capacity;
  size_t count;
  size_t current_token;
} Tokens;

typedef struct {
  char **items;
  size_t capacity;
  size_t count;
} Items;

typedef struct {
  char *key;
  char *value;
} KV;

typedef struct {
  KV *items;
  size_t capacity;
  size_t count;
} Data;

bool is_alpha(char c) {
  return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'));
}

bool is_integer(char c) {
  return c >= '0' && c <='9';
}

bool is_alpha_numeric(char c) {
  return is_alpha(c) || is_integer(c);
}

bool is_valid_char_for_string(char c) {
  return c >= 32 && c <= 126;
}

Token GetToken(Nob_String_Builder sb, size_t *At) {
  Token t = {0};
  t.kind = TK_NONE;
  while (*At < sb.count) {
    char c = sb.items[*At];
    switch (c) {
      case '{': t.kind = TK_OPEN_CURLY_BRACE; break;
      case '}': t.kind = TK_CLOSE_CURLY_BRACE; break;
      case '[': t.kind = TK_OPEN_SQ_BRACKET; break;
      case ']': t.kind = TK_CLOSE_SQ_BRACKET; break;
      case ':': t.kind = TK_COLON; break;
      case ',': t.kind = TK_COMMA; break;
      // if newline, just skip over the character
      case '\r':
      case '\n': break;
      case '"': {
        *At += 1;
        c = sb.items[*At];
        char *buffer = malloc(sizeof(char)*2048);
        size_t idx = 0;
        while (*At < sb.count && is_valid_char_for_string(c)) {
          if (c == '\\') {
            if (*At + 1 < sb.count && sb.items[*At+1] == '"') {
              buffer[idx] = c;
              idx += 1;
              *At += 1;
              c = sb.items[*At];
            }
          } else if (c == '"') {
              break;
          }
          buffer[idx] = c;
          idx += 1;
          *At += 1;
          c = sb.items[*At];
        }
        t.kind = TK_STRING;
        t.text = buffer;
      } break;
      default: {
        bool is_numeric = false;
        char *buffer = malloc(sizeof(char)*2048);
        size_t idx = 0;

          /*check for positive whole number*/
        if (is_integer(c)) {
          is_numeric = true;
        } else if 
          /*check for negative float without initial 0*/
          (c == '-' && (
                    (*At+1 < sb.count && sb.items[*At+1] == '.') &&
                    (*At+2 < sb.count && is_integer(sb.items[*At+2]))
                  )) {
          is_numeric = true;
          buffer[idx] = c;
          *At += 1;
          idx += 1;
          c = sb.items[*At];
          buffer[idx] = c;
          *At += 1;
          idx += 1;
          c = sb.items[*At];
        } else if
          /*check for negative whole number*/
          (c == '-' && (*At+1 < sb.count && is_integer(sb.items[*At+1]))) {
          is_numeric = true;
          buffer[idx] = c;
          *At += 1;
          idx += 1;
          c = sb.items[*At];
        } else if 
          /*check for positive float without initial 0*/
          (c == '.' && (*At+1 < sb.count && is_integer(sb.items[*At+1]))) {
          is_numeric = true;
          buffer[idx] = c;
          *At += 1;
          idx += 1;
          c = sb.items[*At];
        }

        if (is_numeric) {
          char *buffer = malloc(sizeof(char)*2048);
          size_t idx = 0;
          while (*At < sb.count && 
                  (
                    is_integer(c) ||
                    (
                      c == '.' && *At+1 < sb.count && is_integer(sb.items[*At+1])
                    )
                  )
                ) {
            buffer[idx] = c;
            idx += 1;
            *At += 1;
            c = sb.items[*At];
          }
          if (c == ',')
            *At -= 1;
          t.kind = TK_FLOAT;
          t.num = (float)atof(buffer);
        }
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

Token get_empty_token() {
  Token t = {0};
  t.kind = TK_NONE;
  return t;
}

bool expect_token(Token t, Token_Kind kind) {
  if (t.kind != kind) {
    nob_log(NOB_ERROR, "Expected %s but got %s", GetKind(kind), GetKind(t.kind));
    return false;
  }
  return true;
}

Token get_next_token(Tokens *tokens) {
  Token t = tokens->items[tokens->current_token];
  tokens->current_token += 1;
  if (tokens->current_token >= tokens->count)
    return get_empty_token();
  return t;
}

Token peek(Tokens *tokens, size_t count) {
  size_t num = tokens->current_token + count - 1;
  if (num < tokens->count) {
    return tokens->items[num];
  }
  return get_empty_token();
}
/*
Items get_keys(Tokens *tokens) {
  tokens->current_token = 0;
  Items items = {0};
  char *key = get_next_key(tokens);
  while (key) {
    nob_da_append(&items, key);
    key = get_next_key(tokens);
  }
  return items;
}

Items get_values(Tokens *tokens) {
  tokens->current_token = 0;
  Items items = {0};
  char *value = get_next_value(tokens);
  while (value) {
    nob_da_append(&items, value);
    value = get_next_value(tokens);
  }
  return items;
}

Data get_data(Tokens *tokens) {
  Data data = {0};
  
  Items keys = get_keys(tokens);
  Items values = get_values(tokens);
  assert (keys.count == values.count);
  for (size_t i = 0; i < keys.count; ++i) {
    KV kv = {0};
    kv.key = keys.items[i];
    kv.value = values.items[i];
    nob_da_append(&data, kv);
  }
  return data;
}

void test(Tokens *tokens) {
  tokens->current_token = 0;

  char *key = get_next_key(tokens);
  while (key) {
    nob_log(NOB_INFO, "%s", key);
    key = get_next_key(tokens);
  }

}
*/

int main() {

  //const char *filePath = "./data/nasa.json";
  const char *filePath = "./data/weather.json";
  
  Nob_String_Builder sb = {0};
  if (!nob_read_entire_file(filePath, &sb)) return 1;

  Tokens tokens = Tokenize(sb);
  //test(&tokens);
  /*Data data = get_data(&tokens);
  for (size_t i = 0; i < data.count; ++i) {
    KV kv = data.items[i];
    nob_log(NOB_INFO, "KEY: %s", kv.key);
    nob_log(NOB_INFO, "VALUE: %s", kv.value);
    nob_log(NOB_INFO, "--------------------------");
  }*/

#if 1
  for (size_t i = 0; i < tokens.count; ++i) {
    Token t = tokens.items[i];
    nob_log(NOB_INFO, "KIND: %s", GetKind(t.kind));
    if (t.text && strlen(t.text) > 0) {
      nob_log(NOB_INFO, "%s", t.text);
    } if (t.num > 0) {
      nob_log(NOB_INFO, "%f", t.num);
    }
    nob_log(NOB_INFO, "-------------------------");
  }
#endif

  return 0;
}
