#include <stdio.h>
#include <string.h>

void is_palindrome(char str[]);

int main()
{
  char str[] = "racecar";
  char str2[] = "nope";
  is_palindrome(str);  
  is_palindrome(str2);
  return 0;
}

void is_palindrome(char str[])
{
  int i = 0; // Start of string index
  int j = strlen(str) - 1; // End of string index

  while (j > 1)
  {
    if (str[i++] != str[j--])
    {
      printf("%s is not a palindrome!\n", str);
      return;
    }
  }
  printf("%s IS a palindrome!\n", str);
}
