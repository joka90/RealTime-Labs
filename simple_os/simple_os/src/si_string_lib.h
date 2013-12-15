#ifndef SI_STRING_LIB
#define SI_STRING_LIB

void si_string_copy(char *destination, char *source); 

int si_string_length(char *string); 

int si_string_compare(char *first, char *second); 

/* si_insert_string: look for first occurence of %s, replace it with replace_string */ 
void si_insert_string(char *string, char *replace_string); 

/* si_insert_int_as_hex_string: look for first occurence of %x, replace it with 
   value as a hex string */ 
void si_insert_int_as_hex(char *string, int value); 

void si_insert_int_as_hex_no_leading_zeros(char *string, int value); 

#endif
