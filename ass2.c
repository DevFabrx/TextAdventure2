//------------------------------------------------------------------------------
// ass2.c
//
// This is a text based adventure game where the end of the game depends on the
// user's decisions.
//
// Group: Group 10, study assistant David Bidner
//
// Authors: Fabian Obermayer 01131905
// Daniel Krems 00930736
//------------------------------------------------------------------------------
//


// Includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


// Constants
#define LINE_BUFFER 80
#define EXIT_SUCCESS 0
#define TRUE 1
#define FALSE 0
#define USAGE_ERROR_TEXT "Usage: ./ass2 "
#define OUT_OF_MEMORY_ERROR_TEXT "[ERR] Out of memory.\n"
#define FILE_READ_ERROR_TEXT "[ERR] Could not read file"
#define INPUT_ERROR_TEXT "[ERR] Please enter A or B.\n"


// Typedef Structs
typedef struct _Chapter_
{
  char* title_;
  char* text_;
  struct _Chapter_* next_A_;
  struct _Chapter_* next_B_;
} Chapter;


// Typedef Enum
typedef enum _ErrorCodes_
{
  SUCCESS,
  USAGE_ERROR,
  OUT_OF_MEMORY_ERROR,
  FILE_READ_ERROR,
  USER_INPUT_ERROR,
}ErrorCodes;

// Function Prototypes
int parseErrorCode(int error_code, char** current_filename);
int parseCommandLineInput(char** command_line_input, int argc, char* argv[],
                          char** current_filename);
char* readFile(FILE* file);
int gameLoop(Chapter* root_chapter);
int isCorrupt(char* data);
Chapter* createChapters(char* chapter_data, char** current_filename);
void freeAll(Chapter* root_chapter);
int printChapterToConsole(Chapter* chapter);



//-----------------------------------------------------------------------------
///
/// The main program that reads in all the game files and handles the gamers
/// input
///
/// @param argc number of inputs (is 1 if 0 inputs)
/// @param argv char* with the input string
/// @return always zero
//
int main(int argc, char* argv[])
{
  char* command_line_input;
  char* current_filename;
  // Parses the command line command_line_input and handles all the possible
  // errors.
  int input_error = parseErrorCode(parseCommandLineInput(&command_line_input,
                                                         argc, argv,
                                                         &current_filename),
                                   &command_line_input);
  if(input_error != 0)
  {
    return input_error;
  }
  //int length = strlen(command_line_input);
  //strcpy(current_filename, command_line_input);
  FILE* file = fopen(command_line_input, "r");
  if(file == NULL) // check if fopen returned an error
  {
    return parseErrorCode(FILE_READ_ERROR, &current_filename);
  }

  char* root_data = readFile(file);
  fclose(file);

  if(isCorrupt(root_data)) // check if file data is corrupt
  {
    return FILE_READ_ERROR;
  }
  Chapter* root_chapter = createChapters(root_data, &current_filename);
  if(root_chapter == NULL)
  {
    return parseErrorCode(FILE_READ_ERROR, &current_filename);
  }
  //Chapter* current_chapter = root_chapter;
  int game_loop_error = gameLoop(root_chapter);
  if(game_loop_error == 0)
  {
    return EXIT_SUCCESS;
  }
  freeAll(root_chapter);
  return EXIT_SUCCESS;
}

//-----------------------------------------------------------------------------
///
/// Reads in all data from the files into the corresponding data structure
///
/// @param chapter_data char* that contains all Chapter data
/// @return new_chapter New Chapter* to the filled data struct
//
Chapter* createChapters(char* chapter_data, char** current_filename)
{
  char* string_copy = (char*) malloc((strlen(chapter_data)+1)*sizeof(char));
  strncpy(string_copy, chapter_data, strlen(chapter_data)+1);
  Chapter* new_chapter = (Chapter*) malloc(sizeof(Chapter));
  char* title = strtok(string_copy, "\n");
  char* chapter_A = strtok(NULL, "\n");
  char* chapter_B = strtok(NULL, "\n");
  char* description = strtok(NULL, "\0");
  new_chapter->title_ = title;
  new_chapter->text_ = description;
  if(strcmp(chapter_A,"-")!=0)
  {
    //free(chapter_A);
    *current_filename = strdup(chapter_A);
    FILE* file = fopen(chapter_A,"r");
    if(file == NULL)
    {
      free(string_copy);
      return NULL;
    }
    //char* chapter_data = readFile(file);
    chapter_data = readFile(file);
    if(isCorrupt(chapter_data) == 1)
    {
      free(string_copy);
      fclose(file);
      return NULL;
    }
    new_chapter->next_A_ = createChapters(chapter_data, current_filename);
    if(new_chapter->next_A_ == NULL)
    {
      fclose(file);
      free(string_copy);
      return NULL;
    }
    fclose(file);
  }
  else
  {
    new_chapter->next_A_ = NULL;
  }
  if(strcmp(chapter_B, "-") != 0)
  {
    //free(chapter_B);
    *current_filename = strdup(chapter_B);
    FILE* file = fopen(chapter_B,"r");
    if(file == NULL)
    {
      free(string_copy);
      return NULL;
    }
    char* chapter_data = readFile(file);
    if(isCorrupt(chapter_data) == 1)
    {
      free(string_copy);
      fclose(file);
      return NULL;
    }
    new_chapter->next_B_ = createChapters(chapter_data, current_filename);
    fclose(file);
  }
  else
  {
    new_chapter->next_B_ = NULL;
  }
  return new_chapter;
}



//-----------------------------------------------------------------------------
///
/// Reads in all data from a file and returns a pointer to the c-string that
/// contains that data
///
/// @param file pointer to the file stream
/// @return char* read c-string
//
char* readFile(FILE* file)
{
  //char* buffer = (char*) malloc(LINE_BUFFER * sizeof(char));
  char* buffer = (char*) malloc(LINE_BUFFER*sizeof(char));
  //int buffer_length = LINE_BUFFER;

  int buffer_length = LINE_BUFFER;
  int length_counter = 0;
  char c;
  while((c=fgetc(file)) != EOF)
  {
    //if(length_counter == buffer_length-1)
    if(length_counter == buffer_length)
    {
      buffer_length += LINE_BUFFER;

      buffer = (char*) realloc(buffer, buffer_length);
      if(buffer == NULL)
      {
        return NULL;
      }
    }
    buffer[length_counter] = c;
    length_counter += 1;
  }
  buffer[length_counter] = '\0';
  return buffer;
}


//-----------------------------------------------------------------------------
///
/// Checks if the read file data is corrupt (does not have the right data
/// structure) and returns 0 if not corrupt and 1 if corrupt.
///
/// @param file pointer to the file stream
/// @return int 1 (TRUE) or 0 (FALSE)
//
int isCorrupt(char* file_data)
{
  char* string_data = (char*) malloc(sizeof(char)*(strlen(file_data)+1));
  strncpy(string_data,file_data, strlen(file_data)+1);
  //string_data = strncpy(string_data, file_data, strlen(file_data));
  strtok(string_data, "\n");
  char* chapter_A = strtok(NULL, "\n");
  char* chapter_A_type = &chapter_A[strlen(chapter_A)-4];
  char* chapter_B = strtok(NULL, "\n");
  char* chapter_B_type = &chapter_B[strlen(chapter_B)-4];
  char* description = strtok(NULL, "\0");

  // Next Chapter A value is correct
  int condition_A = (strcmp(chapter_A, "-") == 0) || (strstr(chapter_A_type, ""
      ".txt") != NULL);
  // Next Chapter B value is correct
  int condition_B = (strcmp(chapter_B, "-") == 0) || (strstr(chapter_B_type, ""
  ".txt") != NULL);
  // Description is correct
  int condition_C = (description != NULL) || (strstr(description, "") != NULL);

  if(condition_A && condition_B && condition_C)
  {
    //free(string_data); TODO blub
    return FALSE;
  }
  else
  {
    free(string_data);
    return TRUE;
  }
}


//-----------------------------------------------------------------------------
///
/// Checks if there is only one command line input argument and parses that
/// into a .txt string so that the file can be opened.
///
/// @param command_line_input pointer where the parsed string is stored
/// @param argc number of inputs from main
/// @param argv pointer to the input array from main
/// @return int error code
//
int parseCommandLineInput(char** command_line_input, int argc, char* argv[],
                          char** current_filename)
{
  char* file_format = ".txt";
  // check if no user input or too many user inputs
  if(argc != 2)
  {
    *command_line_input = argv[0];
    char* pointer_to_backslash = strrchr(*command_line_input, 92);
    if(pointer_to_backslash == NULL)
    {
      *current_filename = strdup(*command_line_input);
      return USAGE_ERROR;
    }
    *current_filename = strcpy(*command_line_input, pointer_to_backslash+1);
    return USAGE_ERROR;
  }
  char* pointer_to_dot_in_string = strchr(argv[1], '.');
  *command_line_input = argv[1];
  *current_filename = strdup(argv[1]);
  if(pointer_to_dot_in_string != NULL)
  {
    //*command_line_input = strcat(argv[1], file_format);
    //*current_filename = strdup(*command_line_input);
    return SUCCESS;
  }
  // check if .txt is appended or not, if not append it to the file string

  if(pointer_to_dot_in_string == NULL)
  {
    //*pointer_to_dot_in_string='\0';
    //*current_filename = strdup(*command_line_input);
    *command_line_input = strcat(argv[1], file_format);
    *current_filename = strdup(*command_line_input);
    return SUCCESS;
  }
  return SUCCESS;
}


//-----------------------------------------------------------------------------
///
/// This function parses the returned error code from other functions into a
/// readable error string that is written to stdout.
///
/// @param error_code error code
/// @return void
//
int parseErrorCode(int error_code, char** current_filename)
{
  char* filename_copy = strdup(*current_filename);
  char* pointer_to_dot = strrchr(filename_copy, '.');
  pointer_to_dot[0] = 0;
  switch(error_code)
  {
    case 0:
      return 0;
    case 1:
      printf("%s %s\n", USAGE_ERROR_TEXT, filename_copy);
      return USAGE_ERROR;
    case 2:
      printf("%s", OUT_OF_MEMORY_ERROR_TEXT);
      return OUT_OF_MEMORY_ERROR;
    case 3:
      printf("%s %s.\n", FILE_READ_ERROR_TEXT, filename_copy);
      return FILE_READ_ERROR;
    case 4:
      printf("%s", INPUT_ERROR_TEXT);
      break;
  }
  return 0;
}

//-----------------------------------------------------------------------------
///
/// This is the main game loop that prints the current chapter on the screen
/// and handles user input.
///
/// @param  error code
/// @return int error_code
//
int gameLoop(Chapter* chapter)
{
  // TODO "B" input not working properly --> NEED TO FIX
  char input;
  while(TRUE)
  {
    if(printChapterToConsole(chapter)==0)
    {
      return SUCCESS;
    }
    char line[256];
    if (fgets(line, sizeof(line), stdin) == NULL) {
      return 0;
    }

    input = line[0];

    if(input == 'A')
    {
      if(chapter->next_A_== NULL)
      {
        return 0;
      }
      chapter = chapter->next_A_;
    }
    if(input == 'B')
    {
      if(chapter->next_B_== NULL)
      {
        return 0;
      }
      chapter = chapter->next_B_;
    }
    else
    {
      continue;
    }
  }
}


//-----------------------------------------------------------------------------
///
/// Prints the Chapter data in the right format to the console output
///
/// @param  chapter Chapter* to the data of the chapter
/// @return int error_code 0 if end, 1 if false User Input
//
int printChapterToConsole(Chapter* chapter)
{
  printf("------------------------------\n");
  printf("%s\n\n", chapter->title_);
  printf("%s\n\n", chapter->text_);
  if(chapter->next_A_ == NULL && chapter->next_B_ == NULL)
  {
    printf("ENDE\n");
    return 0;
  }
  else
  {
    printf("Deine Wahl [A/B]? ");
    return 1;
  }
}


//-----------------------------------------------------------------------------
///
/// Frees all dynamically allocated memory of the Chapter structs
///
/// @param  root_chapter Root chapter of the binary tree data structure
/// @return void
//
void freeAll(Chapter* root_chapter)
{
  if(root_chapter->next_A_ != NULL)
  {
    freeAll(root_chapter->next_A_);
  }
  if(root_chapter->next_B_ != NULL)
  {
    freeAll(root_chapter->next_B_);
  }
  free(root_chapter->title_);
  free(root_chapter->text_);
  free(root_chapter);
}
