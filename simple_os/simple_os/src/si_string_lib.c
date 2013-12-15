#include "si_string_lib.h"

// #include <stdio.h>

static char hex_chars[] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'}; 

void si_string_copy(char *destination, char *source)
{
    char *source_ptr; 
    char *dest_ptr; 

    source_ptr = source; 
    dest_ptr = destination; 

    while ((*source_ptr) != '\0')
    {
        *dest_ptr = *source_ptr; 
        source_ptr++; 
        dest_ptr++; 
    }
    // terminate with null
    *dest_ptr = '\0'; 
}

int si_string_length(char *string)
{
    int length; 
    char *string_ptr; 

    length = 0; 
    string_ptr = string; 
  
    while ((*string_ptr) != '\0')
    {
        length++; 
        string_ptr++; 
    }
   
    return length; 
}

int si_string_compare(char *first, char *second)
{
    char *first_ptr; 
    char *second_ptr; 
    int equal; 

    if (si_string_length(first) != si_string_length(second))
    {
        return 1; 
    }

    first_ptr = first; 
    second_ptr = second; 

    equal = 1; 

    while ((*first_ptr) != '\0' && equal)
    {
        equal = equal && (*first_ptr == *second_ptr); 
        first_ptr++; 
        second_ptr++; 
    }

    if (equal)
    {
        return 0; 
    }
    else
    {
        return 1; 
    }
}

static int index_of_first_occurrance(char find_char, char *string)
{
    int char_found; 
    int found_index; 
    int index; 
    char *string_ptr; 

    string_ptr = string; 

    char_found = 0; 
    found_index = -1; 
    index = 0; 

    while ((!char_found) && (string[index] != '\0'))
    {
        char_found = string[index] == find_char; 
        if (char_found)
	{
            found_index = index; 
	}
        index++; 
    }
    
    return found_index; 
}

void si_insert_string(char *string, char *replace_string)
{
    // look for first occurence of %s, replace it with replace_string
    int percent_sign_index; 
    int s_index; 

    int string_length; 
    int replace_string_length; 

    int pos; 

    percent_sign_index = index_of_first_occurrance('%', string); 

    if (percent_sign_index < 0)
    {
        // fail silently
        return; 
    }

    s_index = percent_sign_index + 1; 

    string_length = si_string_length(string); 
    replace_string_length = si_string_length(replace_string); 

    if (string[s_index] != 's')
    {
        // fail silently
        return; 
    }

    if (replace_string_length <= 0)
    {
        // fail silently
        return; 
    }

    // make room for new string
    if (replace_string_length == 1)
    {
        for (pos = s_index+1; pos <= string_length; pos++)
        {
            string[pos-1] = string[pos]; 
        }
    }
    else // replace_string_length > 1 for sure
    {
        for (pos = string_length; pos >= s_index+1; pos--)
        {
            string[pos+replace_string_length-2] = string[pos]; 
        }
    
    }

    // do the replacement
    for (pos = s_index-1; pos <= s_index-1 + replace_string_length-1; pos++)
    {
        string[pos] = replace_string[pos - s_index + 1]; 
    }

    // printf("string %s after rep has length %d", string, si_string_length(string)); 
}

void si_insert_int_as_hex(char *string, int value)
{
    // look for first occurence of %x, replace it with value as a hex string
    int percent_sign_index; 
    int x_index; 

    int string_length; 
    int replace_string_length; 

    int pos; 

    percent_sign_index = index_of_first_occurrance('%', string); 

    if (percent_sign_index < 0)
    {
        // fail silently
        return; 
    }

    x_index = percent_sign_index + 1; 

    string_length = si_string_length(string); 
    replace_string_length = 10; 

    if (string[x_index] != 'x')
    {
        // fail silently
        return; 
    }

    // make room for new string
    for (pos = string_length; pos >= x_index+1; pos--)
    {
        string[pos+replace_string_length-2] = string[pos]; 
    }

    // do the replacement
    string[x_index-1] = '0'; 
    string[x_index] = 'x'; 
    string[x_index+1] = hex_chars[(value & 0xF0000000) >> 28]; 
    string[x_index+2] = hex_chars[(value & 0x0F000000) >> 24]; 
    string[x_index+3] = hex_chars[(value & 0x00F00000) >> 20]; 
    string[x_index+4] = hex_chars[(value & 0x000F0000) >> 16]; 
    string[x_index+5] = hex_chars[(value & 0x0000F000) >> 12]; 
    string[x_index+6] = hex_chars[(value & 0x00000F00) >> 8]; 
    string[x_index+7] = hex_chars[(value & 0x000000F0) >> 4]; 
    string[x_index+8] = hex_chars[(value & 0x0000000F)]; 

    // printf("string %s after rep has length %d", string, si_string_length(string)); 
}

void si_insert_int_as_hex_no_leading_zeros(char *string, int value)
{
    // look for first occurence of %x, replace it with value as a hex string
    int percent_sign_index; 
    int x_index; 
    int x_index_orig; 

    int string_length; 
    int replace_string_length; 

    int pos; 
    int shift_value; 

    char ch_digit; 

    int n_zeros_removed; 

    // int length; 

    int start_move_pos; 
    int stop_move_pos; 
    int n_chars_moved; 

    percent_sign_index = index_of_first_occurrance('%', string); 

    if (percent_sign_index < 0)
    {
        // fail silently
        return; 
    }

    x_index = percent_sign_index + 1; 

    string_length = si_string_length(string); 
    replace_string_length = 10; 

    if (string[x_index] != 'x')
    {
        // fail silently
        return; 
    }

    // make room for new string
    start_move_pos = x_index+1 + replace_string_length-2; 
    stop_move_pos = string_length + replace_string_length-2; 
    n_chars_moved = 0; 

    for (pos = string_length; pos >= x_index+1; pos--)
    {
        string[pos+replace_string_length-2] = string[pos]; 
        n_chars_moved++; 
    }

    // do the replacement
    // string[x_index-1] = '0'; 
    // string[x_index] = 'x'; 

    x_index_orig = x_index; 

    x_index-=2; 

    n_zeros_removed = 0; 

    for (shift_value = 28; shift_value >= 4; shift_value--)
    {
        ch_digit = hex_chars[(value & 0xF0000000) >> shift_value]; 
        if (ch_digit != '0') 
        {
            x_index++;
            string[x_index] = ch_digit; 
        }
        else
        {
            n_zeros_removed++; 
        }
    }

    x_index++; 
    string[x_index] = hex_chars[(value & 0x0000000F)]; 

    // adjust room for new string
    for (pos = start_move_pos; pos <= stop_move_pos; pos++)
    {
        string[x_index + pos-start_move_pos+1] = string[pos]; 
    }

    // length = si_string_length(string); 
    // string[length] = 'E'; 
    // string[length+1] = '\0'; 

    // printf("string %s after rep has length %d", string, si_string_length(string)); 
}
