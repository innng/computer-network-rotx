/* recebe string de clientes e retorna versao decodificada */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>


// ceaser cypher decoder
char* decoder(char* cypher, int X) {
	
	int len = strlen(cypher);
	char aux[len];
	char *str = malloc(len);

	int i = 0;
	int letter = 0;
	for (i = 0; i < len; i++) {
		if(cypher[i] >= 97 && cypher[i] <= 122) {
			letter = cypher[i] - X;
		}
		else if(cypher[i] >= 65 && cypher[i] <= 90) {
			letter = cypher[i] + 32 - X;
		}
		else return NULL;

		if(letter < 0)
			letter = 25 + letter;

		aux[i] = letter;
	}

	strncpy(str, aux, len);
	return str;
}

int main() {
	char* str = decoder("SXGLP", 3);

	if(str == NULL)
		printf("OIII\n");
	else printf("A decodificaçao e: %s\n", str);
	return 0;

}
