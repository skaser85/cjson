#include <stdio.h>
#include <stdlib.h>

#define NOB_IMPLEMENTATION
#include "./nob.h"

#define SPACES_FOR_INDENT 4
#define PRETTY_PRINT true

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
    TK_TRUE,
    TK_FALSE,
    TK_NULL,

    TK_COUNT
} Token_Kind;

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

typedef enum {
    JK_NONE,

    JK_OBJECT,
    JK_ARRAY,

    JK_STRING,
    JK_FLOAT,
    JK_BOOLEAN,
    JK_NULL,

    JSON_COUNT
} Json_Kind;

struct Json_Element;

typedef struct Json_Element {
    Json_Kind kind;
    const char *key;
    bool open;

    union {
        char *text;
        float num;
        bool boolean;
        struct Json_Element *object;
        struct Json_Element *array;
    } value;

    struct Json_Element *next;

} Json_Element;

const char *GetTokenKind(Token_Kind kind) {
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
        case TK_TRUE: return "TRUE";
        case TK_FALSE: return "FALSE";
        case TK_NULL: return "NULL";
        default: return "";
    }
}

const char *GetJsonKind(Json_Kind kind) {
    switch (kind) {
        case JK_NONE: return "NONE";
        case JK_OBJECT: return "OBJECT";
        case JK_ARRAY: return "ARRAY";
        case JK_STRING: return "STRING";
        case JK_FLOAT: return "FLOAT";
        case JK_BOOLEAN: return "BOOLEAN";
        case JK_NULL: return "NULL";
        default: return "INVALID";
    }
}


bool is_integer(char c) {
    return c >= '0' && c <='9';
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
            case 'n': 
                      {
                          if (sb.items[*At-1] != '\\') {
                              if (sb.items[*At+1] == 'u' &&
                                      sb.items[*At+2] == 'l' &&
                                      sb.items[*At+3] == 'l') {
                                  t.kind = TK_NULL;
                              }
                          }
                      } break;
            case 't': 
                      {
                          if (sb.items[*At+1] == 'r' &&
                                  sb.items[*At+2] == 'u' &&
                                  sb.items[*At+3] == 'e') {
                              t.kind = TK_TRUE;
                          }
                      } break;
            case 'f': 
                      {
                          if (sb.items[*At+1] == 'a' &&
                                  sb.items[*At+2] == 'l' &&
                                  sb.items[*At+3] == 's' &&
                                  sb.items[*At+4] == 'e') {
                              t.kind = TK_FALSE;
                          }
                      } break;
                      // if newline, just skip over the character
            case '\r':
            case '\n': break;
            case '"': 
                       {
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
            default: 
                       {
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

Json_Element *GetNextNode(Json_Element *root) {
    if (!root->next && root->kind == JK_NONE)
        return root;
    if (root->kind == JK_OBJECT)
        return GetNextNode(root->value.object);
    if (root->kind == JK_ARRAY)
        return GetNextNode(root->value.array);
    Json_Element *n = root->next;
    while (n) {
        if ((n->kind == JK_OBJECT || n->kind == JK_ARRAY) && n->open) {
            return GetNextNode(n);
        }
        if (!n->next)
            break;
        n = n->next;
    }
    return n;
}

Json_Element ParseTokens(Tokens tokens) {
    Json_Element root = {0};

    for (size_t i = 0; i < tokens.count; ++i) {
        Token t = tokens.items[i];
        Json_Element *tail = GetNextNode(&root);
        switch (t.kind) {
            case TK_NONE: 
            {
                nob_log(NOB_ERROR, "Invalid Token kind: TK_NONE!");
                return root;
            } break;
            case TK_OPEN_CURLY_BRACE: 
            {
                tail->kind = JK_OBJECT;
                tail->open = true;
                Json_Element *j = malloc(sizeof(Json_Element));
                memset(j, 0, sizeof(Json_Element));
                j->open = true;
                tail->value.object = j;
                Json_Element *j2 = malloc(sizeof(Json_Element));
                memset(j2, 0, sizeof(Json_Element));
                j2->open = true;
                tail->value.object->next = j2;
            } break;
            case TK_CLOSE_CURLY_BRACE: 
            {
                if (tail->kind == JK_OBJECT) {
                    tail->open = false;
                }
            } break;
            case TK_OPEN_SQ_BRACKET:
            {
               tail->kind = JK_ARRAY;
               tail->open = true;
               Json_Element *j = malloc(sizeof(Json_Element));
               memset(j, 0, sizeof(Json_Element));
               j->open = true;
               tail->value.array = j;
            } break;
            case TK_CLOSE_SQ_BRACKET:
            {
                if (tail->kind == JK_ARRAY) {
                    tail->open = false;
                }
            } break;
            case TK_COLON:  break; // do i need to do anything here?
            case TK_COMMA: break; // do i need to do anything here?
            case TK_STRING: 
            {
                if (tail->key) {
                    tail->kind = JK_STRING;
                    tail->value.text = t.text;
                    tail->open = false;
                   
                    Json_Element *n = malloc(sizeof(Json_Element));
                    memset(n, 0, sizeof(Json_Element));
                    n->open = true;
                    tail->next = n;

                } else {
                    tail->key = t.text;
                    tail->open = true;
                }
            } break;
            case TK_FLOAT: 
            {
                if (tail->key) {
                    tail->kind = JK_FLOAT;
                    tail->value.num = t.num;
                    tail->open = false;
                    Json_Element *n = malloc(sizeof(Json_Element));
                    memset(n, 0, sizeof(Json_Element));
                    n->open = true;
                    tail->next = n;

                }
            } break;
            case TK_TRUE: 
            {
                if (tail->key) {
                    tail->kind = JK_BOOLEAN;
                    tail->value.boolean = true;
                    tail->open = false;
                    Json_Element *n = malloc(sizeof(Json_Element));
                    memset(n, 0, sizeof(Json_Element));
                    n->open = true;
                    tail->next = n;

                }
            } break;
            case TK_FALSE: 
            {
                if (tail->key) {
                    tail->kind = JK_BOOLEAN;
                    tail->value.boolean = false;
                    tail->open = false;
                    Json_Element *n = malloc(sizeof(Json_Element));
                    memset(n, 0, sizeof(Json_Element));
                    n->open = true;
                    tail->next = n;


                }
            } break;
            case TK_NULL: 
            {
                if (tail->key) {
                    tail->kind = JK_NULL;
                    tail->open = false;
                    Json_Element *n = malloc(sizeof(Json_Element));
                    memset(n, 0, sizeof(Json_Element));
                    n->open = true;
                    tail->next = n;

                }
            } break;
            default: break;
            
        }
    }

    return root;
}

const char* str_repeat(char* str, size_t times) {
    if (times < 1) return NULL;
    char *ret = malloc(sizeof(str) * times + 1);
    if (ret == NULL) return NULL;
    strcpy(ret, str);
    while (--times > 0) {
        strcat(ret, str);
    }
    return ret;
}

void Append2Json(Nob_String_Builder *sb, const char* text, size_t indent_amount) {
    if (PRETTY_PRINT) {
        const char* spaces = str_repeat(" ", indent_amount);
        if (spaces) {
            nob_sb_appendf(sb, "%s%s", spaces, text);
        } else {
            nob_sb_appendf(sb, "%s", text);
        }
    } else {
        nob_sb_appendf(sb, "%s", text);
    }
}


Nob_String_Builder Tokens2Json(Tokens tokens) {
    Nob_String_Builder sb = {0};
    size_t indent_amount = 0;

    for (size_t i = 0; i < tokens.count; ++i) {
        Token t = tokens.items[i];
        switch (t.kind) {
            case TK_OPEN_CURLY_BRACE: 
                {
                    Append2Json(&sb, "{", indent_amount);
                    nob_da_append(&sb, '\n');
                    indent_amount += SPACES_FOR_INDENT;
                } break;
            case TK_CLOSE_CURLY_BRACE: 
                {
                    indent_amount -= SPACES_FOR_INDENT;
                    if (sb.items[sb.count] != '\n')
                        nob_da_append(&sb, '\n');
                    Append2Json(&sb, "}", indent_amount);
                    nob_da_append(&sb, '\n');
                } break;
            case TK_OPEN_SQ_BRACKET: 
                {
                    Append2Json(&sb, "[", indent_amount);
                    indent_amount += SPACES_FOR_INDENT;
                    nob_da_append(&sb, '\n');
                } break;
            case TK_CLOSE_SQ_BRACKET: 
                {
                    indent_amount -= SPACES_FOR_INDENT;
                    Append2Json(&sb, "]", indent_amount);
                    nob_da_append(&sb, '\n');
                } break;
            case TK_STRING: 
                {
                    char buff[strlen(t.text)];
                    sprintf(buff, "\"%s\"", t.text);
                    Append2Json(&sb, (const char *)buff, indent_amount);
                } break;
            case TK_FLOAT: 
                {
                    char buff[100];
                    sprintf(buff, "%f", t.num);
                    Append2Json(&sb, (const char *)buff, 0);
                } break;
            case TK_COLON: 
                {
                    Append2Json(&sb, ": ", 0);
                } break;
            case TK_COMMA: 
                {
                    Append2Json(&sb, ",", 0);
                    nob_da_append(&sb, '\n');
                } break;
            case TK_NULL: 
                {
                    Append2Json(&sb, "null", 0);
                } break;
            case TK_TRUE: 
                {
                    Append2Json(&sb, "true", 0);
                } break;
            case TK_FALSE: 
                {
                    Append2Json(&sb, "false", 0);
                } break;
            default: nob_log(NOB_ERROR, "Unknown token: %s", GetTokenKind(t.kind));
        }
    }

    return sb;
}

int main() {

    //const char *filePath = "./data/nasa.json";
    const char *filePath = "./data/weather.json";

    Nob_String_Builder sb = {0};
    if (!nob_read_entire_file(filePath, &sb)) return 1;

    Tokens tokens = Tokenize(sb);

    Json_Element root = ParseTokens(tokens);

    nob_log(NOB_INFO, "%s", root.key);

    //Nob_String_Builder result = Tokens2Json(tokens);

    //   FILE *fp = fopen("./dump.json", "w");
    //   fprintf(fp, "%s", result.items);
    //   fclose(fp);

#if 0
    FILE *fp = fopen("./info.txt", "w");
    for (size_t i = 0; i < tokens.count; ++i) {
        Token t = tokens.items[i];
        if (t.kind != TK_STRING && t.kind != TK_FLOAT)
            fprintf(fp, "%s\n", GetTokenKind(t.kind));
        if (t.text && strlen(t.text) > 0) {
            fprintf(fp, "%s\n", t.text);
        } if (t.num > 0) {
            fprintf(fp, "%f\n", t.num);
        }
        //fprintf(fp, "--------------------------\n");
    }

    fclose(fp);
#endif

    return 0;
}
