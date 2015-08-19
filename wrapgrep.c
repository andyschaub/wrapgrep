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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <stdbool.h>
#include <ctype.h>
#include "wrapgrep.h"

#define BUF_LEN			256
#define	ARRAY_LEN		16
#define OPTION			1
#define PATTERN			1
#define FILE_NAME		2
#define CORRECT_ARG_NUM	3
#define	HELP_NUM		2

void *get_mem (int num_bytes)
{
	void *mem = malloc (num_bytes);

	if (mem)
		return (mem);

	fprintf (stderr, "FATAL ERROR: Can't allocate %d bytes.\n", num_bytes);

	exit (EXIT_FAILURE);
}

void free_mem (void *mem)
{
	free (mem);
}

symbol *create_symbol (FILE *input, bool *eof, int *line)
{
	char c;
	int i = 0;
	char buffer [BUF_LEN];
	char *word;
	symbol *new_symbol;

	if ((c = skip_space (input, eof, line)) != EOF)
	{
		buffer [i++] = c;

		while (((c = get_char (input, eof)) != EOF) && (i < BUF_LEN))
		{
			if (is_space (c))
				break;

			buffer [i++] = c;
		}

		buffer [i++] = '\0';
	}

	if (i == 0)
	{
		word = "";
	}
	else
	{
		word = get_mem (i);
		strcpy (word, buffer);
	}

	new_symbol = get_mem (sizeof (symbol));

	new_symbol->word = word;
	new_symbol->line = (*line);

	if ((c == '\n') || (c == '\f'))
		++(*line);

	return (new_symbol);
}

void destroy_symbol (symbol *sym)
{
	free_mem (sym->word);
	free_mem (sym);
}

void destroy_symbol_list (symbol *sym)
{
	symbol *current;
	symbol *next;

	current = sym;

	while (current)
	{
		next = current->next;
		destroy_symbol (current);
		current = next;
	}
}

void print_symbol (symbol *sym)
{
	printf ("SYMBOL:\n");
	printf ("    Line = %05d\n", sym->line);
	printf ("    Curr = %p\n", sym);
	printf ("    Prev = %p\n", sym->prev);
	printf ("    Next = %p\n", sym->next);
	printf ("    Word = %s\n", sym->word);
	printf ("\n");
}

void print_symbol_list (symbol *sym)
{
	symbol *current;

	current = sym;

	while (current)
	{
		print_symbol (current);
		current = current->next;
	}
}

bool is_space (char c)
{
	if (isspace (c))
		return (true);

	return (false);
}

char get_char (FILE *input, bool *eof)
{
	char c;

	if (*eof)
		return (EOF);

	for (;;)
	{
		c = fgetc (input);

		if (c == EOF)
			break;

		if (isascii (c))
			break;
	}

	if (c == EOF)
		*eof = true;

	return (c);
}

char skip_space (FILE *input, bool *eof, int *line)
{
	char c;

	for (c = get_char (input, eof); is_space (c);
		c = get_char (input, eof))
		{
			if ((c == '\n') || (c == '\f'))
				++(*line);
		}

	return (c);
}

char *skip_white_text (char *pattern)
{
	while ((*pattern) && is_space (*pattern))
		++pattern;

	return (pattern);
}

char *find_word (char *pattern, char **word)
{
	char *p;
	char *q;

	p = pattern;
	q = pattern + strlen (pattern);

	if (*p == '\0')
		return (NULL);

	while (*p)
	{
		p = skip_white_text (p);
		(*word) = p;
		while ((*p) && (! is_space (*p)))
			++p;
		*p = '\0';
	}

	if (p == q)
		return (p);

	return (p + 1);
}

int create_pattern_array (char *array[], int len, char *pattern)
{
	int i;
	char *p;
	char *word;

	if (*pattern == '\0')
		return (0);

	p = pattern;

	for (i = 0; (i < len) && (*p); i++)
	{
		p = find_word (p, &word);
		array [i] = get_mem (strlen (word));
		strcpy (array [i], word);
	}

	return (i);
}

void destroy_pattern_array (char* array[], int len)
{
	int i;

	for (i = 0; i < len; i++)
		free_mem (array [i]);
}

void print_pattern_array (char *array [], int len)
{
	int i;

	printf ("-----\n");	

	printf ("%05d Symbols:\n", len);

	for (i = 0; i < len; i++)
		printf ("Symbol [%05d]: %s\n", i, array [i]);

	printf ("-----\n");	
}

symbol *parse_file (char *name, symbol *anchor)
{
	FILE *input;
	char *word;
	bool at_eof;
	int line;
	symbol *current;
	symbol *previous;

	at_eof = false;
	line = 1;
	current = NULL;
	previous = NULL;

	if ((input = fopen (name, "r")))
	{		
		while (! at_eof)
		{			
			if (current)
				previous = current;

			current = create_symbol (input, &at_eof, &line);

			if (! anchor)
			{
				anchor = current;
				current->prev = NULL;		
			}

			if (previous)
			{
				current->prev = previous;
				previous->next = current;
			}
		}

		fclose (input);
	}

	return (anchor);	
}

void print_help ()
{
	printf ("\n");
	printf ("wrapgrep searches a given file for an array of symbols expressed\n");
	printf ("in a pattern as a string with symbols (e.g., words) separated\n");
	printf ("by whitespace in it. It converts the file-to-be-searched into a\n");
	printf ("list of symbols or words separated by whitespace (spaces, tabs,\n");
	printf ("newlines, etc.), keeping track of which line each symbol or\n");
	printf ("word is on. wrapgrep then compares the pattern array against\n");
	printf ("the list of to-be-searched symbols using a substring matching\n");
	printf ("algorithm and sends matches to stdout with the file name and\n");
	printf ("actual symbols in the file with the line number in parentheses.\n");
	printf ("\n");
	printf ("Suppose you have a file named andy-audio.html with several\n");
	printf ("instances of the pattern \"Andy Audio Plumb II\" in it, sometimes\n");
	printf ("split across lines. If you enter this command in your terminal:\n");
	printf ("\n");
	printf ("$wrapgrep \"Andy Audio Plumb II\" andy-audio.html\n");
	printf ("\n");
	printf ("You might get an output like this:\n");
	printf ("\n");
	printf ("andy-audio.html: content=\"Andy (7) Audio (7) Plumb (7) II\"> (7)\n"); 
	printf ("andy-audio.html: <title>Andy (11) Audio (11) Plumb (11) II</title> (11)\n"); 
	printf ("andy-audio.html: Andy (256) Audio (256) Plumb (257) II</h2> (257)\n");
	printf ("\n");
	printf ("This indicates that you have three matches to the pattern\n");
	printf ("\"Andy Audio Plumb II\" one all on line 7, one all on line 11, and\n");
	printf ("one on both lines 256 and 257. As you can see, some matches\n");
	printf ("have additional non-whitespace characters associated with the\n");
	printf ("pattern, such as \"Andy\" matching \"<title>Andy\". This helps to\n");
	printf ("give you context so that the results are useful but not limited\n");
	printf ("to a line-by-line search.\n");
	printf ("\n");
	printf ("Because wrapgrep can only search one file at a time, you would need\n");
	printf ("to put it in a batch file or use the find command with the appropriate\n");
	printf ("options.\n");
	printf ("\n");
}

void pattern_match (char *array [], int len, symbol *anchor, char *file_name)
{
	symbol *current = anchor;

	while (current)
	{
		symbol *p;
		symbol *q;
		int i;
		int j;

		p = current;
		q = current;
	
		for (i = 0; (i < len) && (p) && (strstr (p->word, array [i])); )
		{
			++i;
			p = p->next;
		}

		if (i == len)
		{
			int j;

			printf ("%s:", file_name);

			for (j = 0; (j < len); j++)
			{
				printf (" %s (%d)", q->word, q->line);
				q = q->next;				
			}

			putchar ('\n');
		}

		current = current->next;
	}
}

char *copy_string (char *dest, char *src, int len)
{
	int i;

	for (i = 0; (i < len) && (src [i]); )
	{
		dest [i] = src [i];
		++i;
	}

	dest [i] = '\0';

	return (dest);
}

int main (int arc, char *argv [])
{
	char *pattern;
	char buffer [BUF_LEN];
	char *name;
	symbol *anchor = NULL;
	char *pattern_array [ARRAY_LEN];
	int pattern_len = 0;

	if ((arc == HELP_NUM) && (! strcmp (argv [OPTION], "--help")))
	{
		print_help ();

		return (EXIT_SUCCESS);
	}

	if (arc != CORRECT_ARG_NUM)
	{
		fprintf (stderr,
			"USAGE: wrapgrep <\"search pattern\"> <input file name>\n");

		fprintf (stderr,
			"HELP: wrapgrep --help\n");

		return (EXIT_FAILURE);
	}

	pattern = argv [PATTERN];
	name = argv [FILE_NAME];

	copy_string (buffer, pattern, BUF_LEN);	
	pattern_len = create_pattern_array (pattern_array, ARRAY_LEN, buffer);
	anchor = parse_file (name, anchor);

	// debugff
	// print_pattern_array (pattern_array, pattern_len);
	// print_symbol_list (anchor);
	// debug

	pattern_match (pattern_array, pattern_len, anchor, name);
	destroy_pattern_array (pattern_array, pattern_len);
	destroy_symbol_list (anchor);

	return (EXIT_SUCCESS);
}