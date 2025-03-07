#include <stdio.h>

#include "uds.h"

#define COLOR_RESET   "\x1b[0m"
#define COLOR_YELLOW  "\x1b[33m"
#define COLOR_BLUE    "\x1b[34m"
#define COLOR_RED     "\x1b[31m"
#define COLOR_GREEN   "\x1b[32m"

typedef enum {
  JSON_TOKEN_CURLY_LBRACE,
  JSON_TOKEN_CURLY_RBRACE,
  JSON_TOKEN_SQUARE_LBRACE,
  JSON_TOKEN_SQUARE_RBRACE,
  JSON_TOKEN_STRING,
  JSON_TOKEN_NUMBER,
  JSON_TOKEN_BOOLEAN,
  JSON_TOKEN_NULL,
  JSON_TOKEN_COMMA,
  JSON_TOKEN_COLON,
  JSON_TOKEN_EOF,
  JSON_TOKEN_INVALID
} JSON_TOKEN_TYPE;

typedef struct JSON_Token {
  JSON_TOKEN_TYPE type;
  Slice literal;
} Json_token;

void print_token(Json_token *token);

typedef enum JSON_NODE_TYPE
{
    JSON_NODE_OBJECT,
    JSON_NODE_ARRAY,
    JSON_NODE_STRING,
    JSON_NODE_NUMBER,
    JSON_NODE_BOOLEAN,
    JSON_NODE_NULL
} JSON_NODE_TYPE;

typedef struct Json_node
{
    JSON_NODE_TYPE type;
    union
    {
        HashMap map;
        Vector array;
        char *string_value;
        double number_value;
        int bool_value;
    };
} Json_node;

typedef struct json_lexer {
  char *content;
  size_t pos;
  size_t read_pos;
  size_t length;
  char ch;
} json_lexer;

typedef struct json_parser {
  json_lexer lexer;
} json_parser;

typedef struct Json_object {
  Json_node root;
} Json_object;

void advance(json_lexer *lexer)
{
  if (lexer->read_pos >= lexer->length) {
    lexer->ch = EOF;
  } else {
    lexer->ch = lexer->content[lexer->read_pos];
  }
  lexer->pos = lexer->read_pos;
  lexer->read_pos++;
}

void init_lexer(json_lexer *lexer, size_t length)
{
  lexer->length = length;
  lexer->pos = 0;
  lexer->read_pos= 0;
  advance(lexer);
}

int json_load_file(json_lexer *lexer, const char *file_path)
{
  FILE *f = fopen(file_path, "rb");
  if (!f) {
    fprintf(stderr, "ERROR! can't open file %s\n", file_path);
    return 0;
  }
  fseek(f, 0, SEEK_END);
  size_t content_len = ftell(f);
  rewind(f);

  lexer->content = (char *)malloc(content_len + 1);
  if (!lexer->content) {
    fprintf(stderr, "ERROR! Couldn't allocate memory for file %s\n", file_path);
    fclose(f);
    return 0;
  }

  size_t bytes_read = fread(lexer->content, 1, content_len, f);
  lexer->content[bytes_read] = '\0';
  fclose(f);

  init_lexer(lexer, bytes_read);
  return 1;
}

void skip_white_space(json_lexer *lexer)
{
    while (isspace(lexer->ch))
    {
        advance(lexer);
    }
}

Slice read_string(json_lexer *lexer)
{
  size_t start = lexer->pos + 1;
  advance(lexer);
  while (lexer->ch != '"' && lexer->ch != '\0')
  {
      advance(lexer);
  }

  size_t len = lexer->pos - start;
  Slice str = {
      .data = lexer->content + start,
      .length = len};
  return str;
}

Slice read_number(json_lexer *lexer)
{
  size_t start = lexer->pos;
  while (isdigit(lexer->ch) || lexer->ch == '.' || lexer->ch == '-') {
      advance(lexer);
  }

  size_t len = lexer->pos - start;
  Slice number = {
      .data = lexer->content + start,
      .length = len
  };
  return number;
}

Slice read_keyword(json_lexer *lexer)
{
  size_t start = lexer->pos;
  while (isalpha(lexer->ch))
  {
      advance(lexer);
  }

  size_t len = lexer->pos - start;
  Slice keyword = {
      .data = lexer->content + start,
      .length = len};
  return keyword;
}

Json_token next_token(json_lexer *lexer)
{
  Json_token token;
  skip_white_space(lexer);

  switch (lexer->ch)
  {
    case '{':
      token.type = JSON_TOKEN_CURLY_LBRACE;
      token.literal = _slice("{");
      advance(lexer);
      break;
    case '}':
      token.type = JSON_TOKEN_CURLY_RBRACE;
      token.literal = _slice("}");
      advance(lexer);
      break;
    case '[':
      token.type = JSON_TOKEN_SQUARE_LBRACE;
      token.literal = _slice("[");
      advance(lexer);
      break;
    case ']':
      token.type = JSON_TOKEN_SQUARE_RBRACE;
      token.literal = _slice("]");
      advance(lexer);
      break;
    case ',':
      token.type = JSON_TOKEN_COMMA;
      token.literal = _slice(",");
      advance(lexer);
      break;
    case ':':
      token.type = JSON_TOKEN_COLON;
      token.literal = _slice(":");
      advance(lexer);
      break;
    case '"':
      token.type = JSON_TOKEN_STRING;
      token.literal = read_string(lexer);
      advance(lexer);
      break;
    case EOF:
      token.type = JSON_TOKEN_EOF;
      token.literal = _slice("EOF");
      advance(lexer);
      break;
    case '\n':
    case '\r':
    case '\t':
      advance(lexer);
      break;
    default:
      if (isdigit(lexer->ch) || lexer->ch == '-') {
          token.type = JSON_TOKEN_NUMBER;
          token.literal = read_number(lexer);
      } else if (isalpha(lexer->ch)) {
          token.literal = read_keyword(lexer);
          if (slice_equals(token.literal, _slice("true")) || slice_equals(token.literal, _slice("false")))
          {
              token.type = JSON_TOKEN_BOOLEAN;
          }
          else if (slice_equals(token.literal, _slice("null")))
          {
              token.type = JSON_TOKEN_NULL;
          }
          else
          {
              token.type = JSON_TOKEN_INVALID;
          }
      }
      else
      {
          token.type = JSON_TOKEN_INVALID;
          token.literal = slice_null;
      }
    }
  return token;
}


char *json_token_type_to_string(JSON_TOKEN_TYPE type)
{
    switch (type)
    {
    case JSON_TOKEN_BOOLEAN:
        return "BOOLEAN";
    case JSON_TOKEN_COLON:
        return "COLON";
    case JSON_TOKEN_COMMA:
        return "COMMA";
    case JSON_TOKEN_CURLY_LBRACE:
        return "CURLY_LBRACE";
    case JSON_TOKEN_CURLY_RBRACE:
        return "CURLY_RBRACE";
    case JSON_TOKEN_EOF:
        return "EOF";
    case JSON_TOKEN_INVALID:
        return "INVALID";
    case JSON_TOKEN_NULL:
        return "NULL";
    case JSON_TOKEN_NUMBER:
        return "NUMBER";
    case JSON_TOKEN_SQUARE_LBRACE:
        return "SQUARE_LBRACE";
    case JSON_TOKEN_SQUARE_RBRACE:
        return "SQUARE_RBRACE";
    case JSON_TOKEN_STRING:
        return "STRING";
    default:
      return NULL;
    }
}

void print_token(Json_token *token)
{
    char *type = json_token_type_to_string(token->type);
    //printf(COLOR_RED"############# DEBUG TOKEN INFO #############"COLOR_RESET"\n");
    //printf(COLOR_YELLOW "type: %s" COLOR_RESET " value: " COLOR_BLUE slice_fmt COLOR_RESET "\n", 
    //       type, slice_args(token->literal));
    //printf(COLOR_RED"############################################"COLOR_RESET"\n\n");
    printf("############# DEBUG TOKEN INFO #############""\n");
    printf("type: %s"" value: "slice_fmt"\n", 
           type, slice_args(token->literal));
    printf("############################################""\n\n");
}

Json_token peek_token(json_lexer *lexer)
{
    json_lexer temp = *lexer;
    return next_token(&temp);
}

int parse(json_parser *parser, Json_node *node);


int parse_object(json_parser *parser, Json_node *node)
{
  hashmap_new(&node->map, compare_strings, hash_string);
  node->type = JSON_NODE_OBJECT;

  while (peek_token(&parser->lexer).type != JSON_TOKEN_CURLY_RBRACE) {
    Json_token key = next_token(&parser->lexer);
    char *key_str;

    if (key.type != JSON_TOKEN_STRING) {
      printf("ERROR! Expected token string got %s\n", json_token_type_to_string(key.type));
      return 0;
    }

    if (!slice_to_owned(key.literal, &key_str)) {
      printf("ERROR! Couldn't allocate memory for key string\n");
      return 0;
    }

    Json_token expected = next_token(&parser->lexer);
    if (expected.type != JSON_TOKEN_COLON) {
      printf("ERROR! Expected token \":\" got %s\n", json_token_type_to_string(expected.type));
      return 0;
    }

    Json_node value = {0};
    if (!parse(parser, &value)) {
      printf("ERROR! Couldn't parse object\n");
      return 0;
    }

    Json_node *value_ptr = (Json_node *)malloc(sizeof(Json_node));
    if (!value_ptr) {
      printf("ERROR! Couldn't allocate memory for Json_node value\n");
      return 0;
    }
    *value_ptr = value;

    if (!hashmap_insert(&node->map, key_str, value_ptr)) {
      free(value_ptr);
      return 0;
    }

    Json_token peek = peek_token(&parser->lexer);
    if (peek.type == JSON_TOKEN_CURLY_RBRACE) {
      break;
    } else if (peek.type == JSON_TOKEN_COMMA) {
      next_token(&parser->lexer); // Consume ","
    } else {
      printf("ERROR! Expected token \",\" (COMMA) or \"}\" (CURLY_RBRACE), but got %s\n", json_token_type_to_string(peek.type));
      print_token(&peek);
      return 0;
    }
  }

  next_token(&parser->lexer); // Consume "}"
  return 1;
}

int parse_array(json_parser *parser, Json_node *node)
{
  if (!vector_new(&node->array, sizeof(Json_node), 1)) {
    return 0;
  }
  node->type = JSON_NODE_ARRAY;

  while (peek_token(&parser->lexer).type != JSON_TOKEN_SQUARE_RBRACE)
  {
    Json_node value = {0};
    if (!parse(parser, &value)) {
      printf("ERROR! Couldn't parse\n");
      return 0;
    }

    vector_push_back(&node->array, &value);
    Json_token peek = peek_token(&parser->lexer);
    if (peek.type == JSON_TOKEN_SQUARE_RBRACE) {
      break;
    } else if (peek.type == JSON_TOKEN_COMMA) {
      next_token(&parser->lexer); // Consume ","
    } else {
      printf("ERROR! Expected token \",\" or \"]\" but got %s\n", json_token_type_to_string(peek.type));
      print_token(&peek);
      return 0;
    }
  }

  next_token(&parser->lexer); // Consume "]"
  return 1;
}

int parse(json_parser *parser, Json_node *node)
{
  Json_token token = next_token(&parser->lexer);
  switch (token.type) {
    case JSON_TOKEN_CURLY_LBRACE:
      {
        if (!parse_object(parser, node)) {
          return 0;
        }
      }
      break;
    case JSON_TOKEN_SQUARE_LBRACE:
      {
        if (!parse_array(parser, node)) {
          return 0;
        }
      }
      break;
    case JSON_TOKEN_STRING:
      {
        node->type = JSON_NODE_STRING;
        if (!slice_to_owned(token.literal, &node->string_value)) {
          printf("ERROR! Couldn't allocate memory for string\n");
          return 0;
        }
      }
      break;
    case JSON_TOKEN_NUMBER:
      {
        node->type = JSON_NODE_NUMBER;
        char *string_number;
        if (!slice_to_owned(token.literal, &string_number)) {
          printf("ERROR! Couldn't allocate memory for string\n");
          return 0;
        }
        node->number_value = atof(string_number);
        free(string_number);
      }
      break;
    case JSON_TOKEN_BOOLEAN:
      {
        node->type = JSON_NODE_BOOLEAN;
        node->bool_value = slice_equals(token.literal, _slice("true"));
      }
      break;
    case JSON_TOKEN_NULL:
      {
        node->type = JSON_NODE_NULL;
      }
      break;
    case JSON_TOKEN_EOF:
      {
        return 1;
      }
    default:
      {
        printf("ERROR! Unexpected token while parsing\n");
        print_token(&token);
        return 0;
      }
      break;
  }
  return 1;
}

int json_parse(json_parser *parser, const char *file_path, Json_object *obj)
{
  if (!json_load_file(&parser->lexer, file_path))
  {
    return 0;
  }

  if (!parse(parser, &obj->root)) {
    return 0;
  }

  free(parser->lexer.content);
  return 1;
}

void json_free(Json_node *node) 
{
  switch (node->type) {
    case JSON_NODE_ARRAY:
      {
        for (size_t i = 0; i < node->array.size; i++) {
          json_free(vector_get_ref_at(&node->array, i));
        }
        vector_deallocate(&node->array);
      }
      break;
    case JSON_NODE_STRING:
      {
        free(node->string_value);
      }
      break;
    case JSON_NODE_OBJECT:
      {
        for (size_t i = 0; i < BUCKETS_SIZE; i++) {
          if (!node->map.buckets[i]) {
            continue;
          } else {
            HashMapEntry *entry = node->map.buckets[i];
            while (entry) {
              HashMapEntry *temp = entry;
              entry = entry->next;
              Json_node *value = (Json_node *)temp->value;
              free(temp->key);
              json_free(value);
              free(value);
            }
          }
        }
        hashmap_deallocate(&node->map);
      }
      break;
    default:
      break;
  }
}

void json_unload(Json_object *obj)
{
  json_free(&obj->root);
}

int json_search_key(Json_node* root, char* key, Json_node** value)
{
  if (!root || !key || !*key || !value) {
    fprintf(stderr, "Error! Some parameter are missing\n");
    return 0;
  }

  *value = (Json_node*)hashmap_search(&root->map, key);
  if (*value == NULL) {
    fprintf(stderr, "ERROR! key \"%s\" doesn't exist\n", key);
    return 0;
  }
  return 1;
}

void json_print_value(Json_node* value)
{
  switch (value->type) {
    case JSON_NODE_STRING: {
      printf("%s", value->string_value);
    }
    break; 
    case JSON_NODE_BOOLEAN: {
      printf("%s", value->bool_value ? "true" : "false");
    }
    break;
    case JSON_NODE_NULL: {
      printf("NULL");
    }
    break;
    case JSON_NODE_NUMBER: {
      printf("%2.f\n", value->number_value);
    }
    break;
    case JSON_NODE_ARRAY: {
      for (size_t i = 0; i < value->array.size; i++) {
        Json_node *n = vector_get_ref_at(&value->array, i);
        json_print_value(n);
        if (i != vector_get_size(&value->array) - 1) {
          printf(" ");
        }
      }
    }
    break;
    case JSON_NODE_OBJECT: {
        for (size_t i = 0; i < BUCKETS_SIZE; i++) {
          if (!value->map.buckets[i]) {
            continue;
          } else {
            HashMapEntry *entry = value->map.buckets[i];
            while (entry) {
              HashMapEntry *temp = entry;
              entry = entry->next;
              Json_node *value = (Json_node *)temp->value;
              printf("key: \"%s\" ", (char*)temp->key);
              printf("value: \"");
              json_print_value(value);
              printf("\"");
              printf("\n");
            }
          }
        }
      }
    break;
  }
}

int main()
{
  json_parser parser = {0};
  Json_object object = {0};
  Json_node* node;
  if (!json_parse(&parser, "example2.json", &object)) {
    return 1;
  }

  if (!json_search_key(&object.root, "work", &node)) {
    return 1;
  }
  json_print_value(node);
  printf("\n");
  json_unload(&object);
  return 0;
}