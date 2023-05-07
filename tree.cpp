/*
 * The Tree Command
 */

#include <infos.h>

// ------------Helper functions BEGIN--------------

/**
 * @brief
 *  Checks if the path is a directory
 * @param path the path to check
 * @return true if it is a directory
 * @return false if it is not a directory
 */
bool is_directory(char *path)
{
    HDIR dir = opendir(path, 0);
    if (is_error(dir))
    {
        return false;
    }
    closedir(dir);
    return true;
}
/**
 * @brief
 *  Copies a string from source upto n characters, snipped from strings.cpp
 * @param dest  - buffer to which we write
 * @param src  - source from which we copy
 * @param n  - number upto which we copy
 */
void strncpy(char *dest, const char *src, int n)
{
    while (--n && *src)
    {
        *dest++ = *src++;
    }

    // HMMM
    *dest = 0;
}
/**
 * @brief
 *  Does similar to copying except it does 2 strings, hence concatenating them.
 * @param dest - buffer to which we write
 * @param str1  - source from which we copy
 * @param str2  - source from which we concatenate with first source
 * @param n  - concatenate upto to n places
 */
void strcat(char *dest, const char *str1, const char *str2, int n)
{
    while (--n && *str1)
    {
        *dest++ = *str1++;
    }
    while (--n && *str2)
    {
        *dest++ = *str2++;
    }
    // HMMM
    *dest = 0;
}

/**
 * @brief
 *  Pointer arithematic helper class
 * @param str - static string whose address is incremented
 * @param n - incremented n times
 * @return const char* - resulting address moved
 */
const char *increase_pointer(const char *str, int n)
{

    for (int i = 0; i < n; i++)
    {
        /* code */
        *str++;
    }
    return str;
}

// ------------Helper functions END--------------

// -----------Classes BEGIN -----------

/**
 * @brief
 * Regex class used for pattern matching.
 */
class regex
{
private:
    // class member used to store the operation
    char op;

    /**
     * @brief
     *  Finds a local pattern i.e. looks for a next pattern to recursively look for matching eg: a*b*, pat_index = 0, returns a*
     *                                                                                                 pat_index = 2, returns b*
     * @param buffer : written to this destination
     * @param pattern : the whole pattern array
     * @param pat_index : stores the index of the end of previous pattern.
     */
    void find_local_pattern(char *buffer, const char *pattern, int *pat_index, int n)
    {
        // some variables we need
        int i = 0;
        // this makes it easier to index and work with than raw addresses.
        char str1[n + 2];
        strncpy(str1, pattern, strlen(pattern) + 1);
        // parse whats inside the brackets
        if (str1[*pat_index] == '(')
        {
            *pat_index += 1;

            // parse range if it is a range
            if (str1[*pat_index + 1] == '-')
            {
                char a = str1[*pat_index];
                char b = str1[*pat_index + 2];
                int n = b - a;
                // make sure its a valid range
                if (n < 0)
                {
                    printf("Invalid range");
                    exit(-1);
                }
                // add the characters to the buffer
                for (char j = a; j <= (int)b; j++)
                {
                    buffer[i] = j;
                    i += 1;
                }
                *pat_index += 3;
            }
            // else parse what's inside
            else
            {
                // parse and store in buffer until we encounter closing bracket, potential out of bounds if there isn't a )

                while (str1[*pat_index] != ')')
                {
                    buffer[i] = str1[*pat_index];
                    i += 1;
                    *pat_index += 1;
                }
                buffer[i] = 0;
            }
            *pat_index += 1;
        }

        // if not brackets then parse the character
        else
        {
            strncpy(buffer, &str1[*pat_index], 2);
            *pat_index += 1;
        }

        // Operations are *,?, otherwise N to indicate No operation
        if (str1[*pat_index] == '*')
        {
            op = '*';
            *pat_index += 1;
        }
        else if (str1[*pat_index] == '?')
        {
            op = '?';
            *pat_index += 1;
        }
        else
        {
            op = 'N';
        }
    }

public:
    /**
     * @brief
     *  Main regex function that checks if the string matches the pattern
     * @param pattern - the whole pattern to look out for in the string
     * @param str - the string that is pattern matched
     * @param loca_pattern - the local pattern that is used to see if it recusively matches
     * @param str_index - stores how far we have looked into the string
     * @param pat_index - stores how far we have looked into the pattern
     * @return true - if the string matches the regular expression
     * @return false - if the string doesnt match the regular expression
     */
    bool match(const char *pattern, const char *str, char *loca_pattern, int str_index, int pat_index)
    {
        // copying to make indexing easier
        char str1[strlen(str) + 2];
        strncpy(str1, str, strlen(str) + 1);
        // if there is no local pattern, lets find one in pattern
        if (strlen(loca_pattern) == 0)
        {
            char local_pattern[strlen(pattern) + 5];

            find_local_pattern(local_pattern, pattern, &pat_index, strlen(pattern) + 5);

            return match(pattern, str, local_pattern, str_index, pat_index);
        }
        // else parse the local pattern with the string
        else
        {

            // are we at the end of the string, have we matched to the entire string?
            if (strlen(str1) == str_index)
            {

                return true;
            }

            bool matches_pattern = true;
            // checks if each character in the pattern matches the strindex
            for (int i = 0; i < strlen(loca_pattern); i++)
            {
                matches_pattern = matches_pattern && str1[str_index + i] == loca_pattern[i];
            }

            // if the pattern is matches, lets keep
            if (matches_pattern)
            {
                int aa = strlen(loca_pattern);
                str_index = str_index + aa;
                // if * keep matching
                if (op == '*')
                {
                    return match(pattern, str, loca_pattern, str_index, pat_index);
                }
                // if ? or normal
                else
                {

                    // find the next pattern making sure we havent run outta patterns
                    if (pat_index == strlen(pattern) && strlen(str1) != str_index)
                        return false;

                    char local_pattern[strlen(pattern) + 5];

                    find_local_pattern(local_pattern, pattern, &pat_index, strlen(pattern) + 5);
                    return match(pattern, str, local_pattern, str_index, pat_index);
                }
            }
            // pattern doesnt match? either change the pattern or end it
            else
            {
                // end the pattern if we ran out
                if (pat_index == strlen(pattern))
                    return false;
                // change the pattern
                char local_pattern[strlen(pattern) + 5];

                find_local_pattern(local_pattern, pattern, &pat_index, strlen(pattern) + 5);
                return match(pattern, str, local_pattern, str_index, pat_index);
            }
            return false;
        }
    }
};
/**
 * @brief
 * Tree class which is used for the actual traversal of the directories.
 */
class tree
{
private:
    // variables are for tree class, hardcoded character array to make it neater
    char *indent = "|   ";
    char *file = "|--- ";
    // for summary at the end
    int files = 0;
    int directories = 0;
    bool is_pattern = false;

public:
    /**
     * @brief Set the is_pattern vaiable
     *
     * @param s boolean used to set
     */
    void set_is_pattern(bool s) { is_pattern = s; }
    /**
     * @brief
     * Traverse the directories and print them out in tree manner with or without the optional parameter,
     * @param path the path to traverse
     * @param prefix the prefix indentation usually how inside it is in the file
     * @param pattern if there is a pattern, then pattern match it to check
     * @return int return 0 or 1, 0 if it was successfull otherwise 1
     */
    int traverse(const char *path, char *prefix, const char *pattern)
    {
        // just in case check if the directory is valid
        HDIR dir = opendir(path, 0);
        if (is_error(dir))
        {
            printf("Unable to open directory '%s' for reading.\n", path);
            return 1;
        }
        // loop through the files in this directory
        struct dirent de;
        while (readdir(dir, &de))
        {
            // if the optional -P argument is included make sure it matches the pattern
            if (is_pattern)
            {
                regex r;

                if (!r.match(pattern, de.name, "", 0, 0))
                    continue;
            }
            // construct a valid path to check if it a valid directory
            int n = strlen(de.name) + strlen(path) + 3;
            char buffer[n + 2];
            strcat(buffer, path, "/", n);
            strcat(buffer, buffer, de.name, n);
            // printing of the tree
            printf(prefix);
            printf(file);
            printf("%s", de.name);
            printf("\n");
            // check if the buffer path is a directory and then recursively parse it
            if (is_directory(buffer))
            {
                directories += 1;
                int n = strlen(prefix) + strlen(indent) + 3;
                char new_prefix[n + 2];
                strcat(new_prefix, prefix, indent, n);
                traverse(buffer, new_prefix, pattern);
            }
            // else we know its a file
            else
                files += 1;
        }
        closedir(dir);
        // this makes sure that, it is the intial directory without indentation
        if (prefix == "")
        {
            printf("%d directories, %d files \n", directories, files);
        }
        return 0;
    }
};

// -----------Classes END -----------

// main function
int main(const char *cmdline)
{

    tree T;

    const char *path, *pattern;
    bool is_pattern = false;
    // taken from ls
    if (!cmdline || strlen(cmdline) == 0)
    {
        path = "/usr";
    }
    else
    {
        // store the index of the only 2 possible whitespaces eg: tree [directory]  -P   [pattern]
        //                                                                     s1-^ s2-^
        int s1, s2;
        s1 = 0;
        s2 = 0;
        // for loop that stores the index of the whitespace.
        for (int i = 0; i < strlen(cmdline); i++)
        {
            if (cmdline[i] == ' ')
            {

                if (not s1)
                {
                    s1 = i;
                }
                else
                {
                    s2 = i;
                }
            }
        }
        // make sure there is a whitespace
        if (s1 != 0)
        {
            // two strings one for path or -P
            char str1[s1 + 1], str2[s2 - s1 + 1];
            strncpy(str1, cmdline, s1 + 1);
            // increment the pointer
            cmdline = increase_pointer(cmdline, s1);
            // if it is of the form tree -P <pattern>
            if (strcmp(str1, "-P") == 0)
            {

                path = "/usr";
                // safety statement
                if (!cmdline)
                {
                    printf("No pattern given with -P argument");
                    return 1;
                }
                is_pattern = true;
                T.set_is_pattern(is_pattern);
                return T.traverse(path, "", cmdline);
            }
            // else its tree <directory> -P <pattern>
            else
            {

                // remove whitespace
                cmdline = increase_pointer(cmdline, 1);
                strncpy(str2, cmdline, 3);
                if (strcmp(str2, "-P") == 0)
                {
                    cmdline = increase_pointer(cmdline, 3);
                    if (!cmdline)
                    {
                        printf("No pattern given with -P argument");
                        return 1;
                    }
                    // traverse these statement
                    is_pattern = true;
                    T.set_is_pattern(is_pattern);
                    return T.traverse(str1, "", cmdline);
                }
                else
                {
                    printf("Invalid argument");
                    return 1;
                }
            }
        }
        else
            path = cmdline;
    }
    // usually normal tree <directory> or tree
    T.set_is_pattern(is_pattern);
    return T.traverse(path, "", pattern);
}
