int is_easy_password(char *password) 
{
    char* easy_passwords[] = {

        "0", "1", "2", "3", "4", "5", "6", "7", "8", "9","",

        "00", "01", "02", "03", "04", "05", "06", "07", "08", "09",

        "11", "22", "33", "44", "55", "66", "77", "88", "99", "000", "0000",

        "111", "222", "333", "444", "555", "666", "777", "888", "999", "0123",

        "1000", "1234", "4321", "9999", "1111", "2222", "3333", "4444", "5555", "6666", "7777", "8888", "9999",

    };

	int i;

	if(password[0] == '\n')
		return 1;  // Easy password
    for (i = 0; i < sizeof(easy_passwords) / sizeof(easy_passwords[0]); i++) {

        if (strcmp(password, easy_passwords[i]) == 0) {
            return 1;  // Easy password
        }
    }
    return 0;  // Strong password
}
