#ifndef GMSTR_H
#define GMSTR_H

/**
 * @brief trims spaces from start and end of string, doesn't copy
 */
char* trim(char *str){
	if(str == NULL) { return NULL; }
	if(str[0] == '\0') { return str; }


	/* Move the front and back pointers to address the first non-whitespace
	 * characters from each end. */
	char* frontp = str;
	while( isspace((unsigned char)*frontp) ){
		frontp++;
	}
	
	size_t len = strlen(str);
	char* endp = str + len -1;
	if( endp != frontp ){
		while( isspace((unsigned char) *endp) && endp != frontp ) {
			endp--;
		}
	}

	if( str + len - 1 != endp ){
		*(endp + 1) = '\0'; // trim end
	} else if( frontp != str &&  endp == frontp ) {
		*str = '\0'; // empty str
	}

	/* Shift the string so that it starts at str so that if it's dynamically
	 * allocated, we can still free it on the returned pointer.  Note the reuse
	 * of endp to mean the front of the string buffer now. */
	endp = str;
	if(frontp != str){
		while(*frontp){
			*endp++ = *frontp++;
		}
		*endp = '\0';
	}

	return str;
}
/**
 * @brief returns true if x has y at the start of the string
 */
bool contains_at_start(char* x, char* y){
	for(unsigned int i=0; i < strlen(y); i++){
		if(x[i] != y[i]){
			return false;
		}
	}
	return true;
}


#endif