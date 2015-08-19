//   Copyright 2015 Robert Andrew Schaub
//
//   Licensed under the Apache License, Version 2.0 (the "License");
//   you may not use this file except in compliance with the License.
//   You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
//   Unless required by applicable law or agreed to in writing, software
//   distributed under the License is distributed on an "AS IS" BASIS,
//   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//   See the License for the specific language governing permissions and
//   limitations under the License.

typedef struct symbols
{
	char *word;
	int line;
	struct symbols *prev;
	struct symbols *next;
} symbol;

void *get_mem (int num_bytes);
void free_mem (void *mem);
symbol *create_symbol (FILE *input, bool *eof, int *line);
void destroy_symbol (symbol *sym);
void destroy_symbol_list (symbol *sym);
void print_symbol (symbol *sym);
void print_symbol_list (symbol *sym);
bool is_space (char c);
char get_char (FILE *input, bool *eof);
char skip_space (FILE *input, bool *eof, int *line);
char *skip_white_text (char *pattern);
char *find_word (char *pattern, char **word);
int create_pattern_array (char *array[], int len, char *pattern);
void destroy_pattern_array (char* array[], int len);
void print_pattern_array (char *array [], int len);
symbol *parse_file (char *name, symbol *anchor);
void print_help ();
void pattern_match (char *array [], int len, symbol *anchor, char *file_name);
char *copy_string (char *dest, char *src, int len);